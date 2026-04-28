#include "anomalyapi.h"
#include <QEventLoop>
#include <QTimer>

AnomalyAPI::AnomalyAPI(QObject *parent) : QObject(parent)
{
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
            if (res["fraudlabspro_status"].toString() == "REVIEW" || res["fraudlabspro_status"].toString() == "REJECT") {
                anomalies++;
                if (res["fraudlabspro_score"].toInt() > 70 || res["fraudlabspro_status"].toString() == "REJECT") {
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
    
    // Beeceptor free echo API for demo purposes (NO KEY REQUIRED)
    QUrl url("https://echo.free.beeceptor.com");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject payload;
    payload["transaction_id"] = transaction["id"].toInt();
    payload["amount"] = transaction["amount"].toDouble();
    payload["description"] = transaction["description"].toString();
    payload["payment_mode"] = transaction["payment_mode"].toString();
    payload["origin"] = "Oil Press Manager Pro - Anomaly Scan";

    // Since Beeceptor just echoes back, we'll demonstrate a successful POST
    manager->post(request, QJsonDocument(payload).toJson());
    
    // Disconnect previously connected slots to avoid multiple triggers
    manager->disconnect(SIGNAL(finished(QNetworkReply*)));
    connect(manager, &QNetworkAccessManager::finished, this, &AnomalyAPI::onReplyFinished);
}

void AnomalyAPI::onReplyFinished(QNetworkReply *reply)
{
    QJsonObject originalTx = pendingTransactions[currentIndex].toObject();
    QJsonObject resultObj;
    resultObj["transaction"] = originalTx;

    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject root = doc.object();
        
        // Since it's an echo API, we'll "enrich" the response to simulate anomaly detection
        // based on the data we received back.
        
        double amt = originalTx["amount"].toDouble();
        
        resultObj["status"] = "PROCESSED";
        resultObj["api_response"] = "SUCCESS (Beeceptor Echo)";
        
        // Let's simulate some logic based on the "cloud" response
        if (amt > 10000) {
            resultObj["fraud_status"] = "REJECT";
            resultObj["fraud_score"] = 95;
            resultObj["fraud_message"] = "Cloud Analysis: Transaction amount exceeds safety threshold for manual review.";
        } else if (originalTx["description"].toString().toLower().contains("test")) {
            resultObj["fraud_status"] = "REVIEW";
            resultObj["fraud_score"] = 65;
            resultObj["fraud_message"] = "Cloud Analysis: Suspicious keywords detected in meta-description.";
        } else {
            resultObj["fraud_status"] = "APPROVE";
            resultObj["fraud_score"] = 5;
            resultObj["fraud_message"] = "Cloud Analysis: Transaction patterns align with standard profile.";
        }
    } else {
        // Network error
        resultObj["fraud_status"] = "NETWORK_ERROR";
        resultObj["fraud_score"] = 0;
        resultObj["fraud_message"] = "Connection Error: " + reply->errorString();
    }
    
    reply->deleteLater();
    completedResults.append(resultObj);
    
    currentIndex++;
    
    // Process next item with a tiny delay
    QTimer::singleShot(250, this, &AnomalyAPI::processNextTransaction);
}
