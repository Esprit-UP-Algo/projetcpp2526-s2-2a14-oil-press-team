#include "smsapi.h"
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

SmsAPI::SmsAPI(QObject *parent) : QObject(parent)
{
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished, this, &SmsAPI::onReplyFinished);
}

SmsAPI::~SmsAPI()
{
}

void SmsAPI::sendSMS(const QString &toNumber, const QString &messageBody)
{
    // TextBelt free API - https://textbelt.com
    // Key "textbelt" = 1 free SMS per day (for testing/demos)
    // Buy credits at textbelt.com for unlimited sends
    QUrl url("https://textbelt.com/text");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery postData;
    postData.addQueryItem("phone", toNumber);
    postData.addQueryItem("message", messageBody);
    postData.addQueryItem("key", "textbelt"); // free key: 1 SMS/day

    networkManager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
    qDebug() << "SMS sending via TextBelt to:" << toNumber;
}

void SmsAPI::onReplyFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
        QJsonObject obj = jsonDoc.object();

        bool success = obj.value("success").toBool();
        if (success) {
            qDebug() << "SMS sent successfully via TextBelt!";
            emit smsSent(true, "SMS envoyé avec succès !");
        } else {
            QString error = obj.value("error").toString();
            qDebug() << "TextBelt error:" << error;
            // Don't bother the user — just log it. SMS is informational.
            emit smsSent(false, error);
        }
    } else {
        qDebug() << "Network error sending SMS:" << reply->errorString();
        emit smsSent(false, reply->errorString());
    }
    reply->deleteLater();
}
