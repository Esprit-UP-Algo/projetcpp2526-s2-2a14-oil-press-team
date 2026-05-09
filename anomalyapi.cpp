#include "anomalyapi.h"
#include <QEventLoop>
#include <QTimer>
#include <QUrlQuery>

#include <QNetworkProxyFactory>

AnomalyAPI::AnomalyAPI(QObject *parent) : QObject(parent)
{
    // Force using system proxy settings (Windows credentials pass-through)
    QNetworkProxyFactory::setUseSystemConfiguration(true);
    manager = new QNetworkAccessManager(this);
}

void AnomalyAPI::scanTransactions(const QJsonArray &transactions)
{
    if (transactions.isEmpty()) {
        QJsonObject report;
        report["total_scanned"] = 0;
        report["anomalies_found"] = 0;
        report["high_risk"] = 0;
        report["results"] = QJsonArray();
        emit scanFinished(report);
        return;
    }

    pendingTransactions = transactions;
    completedResults = QJsonArray();
    currentIndex = 0;
    
    processNextTransaction();
}

void AnomalyAPI::processNextTransaction()
{
    if (currentIndex >= pendingTransactions.size()) {
        // Compile final report
        QJsonObject report;
        report["total_scanned"] = pendingTransactions.size();
        
        int anomalies = 0;
        int highRisk = 0;
        
        for (const QJsonValue &val : completedResults) {
            QJsonObject res = val.toObject();
            QString status = res["fraudlabspro_status"].toString().toUpper();
            if (status == "REVIEW" || status == "REJECT") {
                anomalies++;
                if (res["fraudlabspro_score"].toInt() > 70 || status == "REJECT") {
                    highRisk++;
                }
            }
        }
        
        report["anomalies_found"] = anomalies;
        report["high_risk"] = highRisk;
        report["results"] = completedResults;
        
        emit scanFinished(report);
        return;
    }

    QJsonObject transaction = pendingTransactions[currentIndex].toObject();
    
    // FraudLabs Pro v1 API — simple GET with query parameters
    QUrl url("https://api.fraudlabspro.com/v1/order/screen");
    QUrlQuery params;
    params.addQueryItem("key", "IUYHB8ITVD1M7UCVW0OM9M45YB4X9WNY");
    params.addQueryItem("format", "json");
    params.addQueryItem("ip", "8.8.8.8");
    params.addQueryItem("amount", QString::number(transaction["amount"].toDouble(), 'f', 2));
    params.addQueryItem("currency", "TND");
    params.addQueryItem("user_order_id", QString::number(transaction["id"].toInt()));
    params.addQueryItem("payment_mode", transaction["payment_mode"].toString());
    url.setQuery(params);

    QNetworkRequest request(url);
    QNetworkReply *reply = manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        this->onReplyFinished(reply);
    });
}

void AnomalyAPI::onReplyFinished(QNetworkReply *reply)
{
    QJsonObject originalTx = pendingTransactions[currentIndex].toObject();
    QJsonObject resultObj;
    resultObj["transaction"] = originalTx;

    // Always read the response body first (even on error, the server may send useful JSON)
    QByteArray responseData = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    QJsonObject root = doc.object();

    if (reply->error() == QNetworkReply::NoError && !root.isEmpty()) {
        resultObj["status"] = "PROCESSED";
        resultObj["fraudlabspro_status"] = root["fraudlabspro_status"].toString();
        resultObj["fraudlabspro_score"] = root["fraudlabspro_score"].toInt();
        resultObj["fraudlabspro_message"] = root["fraudlabspro_message"].toString();
        
    } else {
        // Network error diagnostics — include server response if available
        int errorCode = (int)reply->error();
        QString serverMsg = root.isEmpty() ? reply->errorString() : root["fraudlabspro_message"].toString();
        if (serverMsg.isEmpty()) serverMsg = QString::fromUtf8(responseData.left(200));
        
        resultObj["fraudlabspro_status"] = "NETWORK_ERROR";
        resultObj["fraudlabspro_score"] = 0;
        resultObj["fraudlabspro_message"] = QString("Error %1: %2").arg(errorCode).arg(serverMsg);
    }
    
    reply->deleteLater();
    completedResults.append(resultObj);
    
    currentIndex++;
    
    // Process next item with a tiny delay
    QTimer::singleShot(250, this, &AnomalyAPI::processNextTransaction);
}
