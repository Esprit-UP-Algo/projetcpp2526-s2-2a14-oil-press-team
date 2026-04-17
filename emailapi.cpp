#include "emailapi.h"
#include <QAuthenticator>

EmailAPI::EmailAPI(QObject *parent) : QObject(parent)
{
    manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &EmailAPI::onReplyFinished);
}

void EmailAPI::setCredentials(const QString &apiKey)
{
    m_apiKey = apiKey;
}

void EmailAPI::sendEmail(const QString &to, const QString &subject, const QString &body)
{
    QUrl url("https://api.brevo.com/v3/smtp/email");
    QNetworkRequest request(url);
    
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("api-key", m_apiKey.toUtf8());

    // Payload for Brevo API v3
    QJsonObject senderObj;
    senderObj["name"] = "Oil Press Manager";
    senderObj["email"] = m_senderEmail;

    QJsonObject toObj;
    toObj["email"] = to;
    toObj["name"] = "Admin";

    QJsonArray toArray;
    toArray.append(toObj);

    // Convert newlines to HTML br tags for better formatting in Brevo
    QString htmlBody = QString("<html><body><h3>%1</h3><p>%2</p></body></html>").arg(subject).arg(body);
    htmlBody.replace("\n", "<br>");

    QJsonObject mainObj;
    mainObj["sender"] = senderObj;
    mainObj["to"] = toArray;
    mainObj["subject"] = subject;
    mainObj["htmlContent"] = htmlBody;

    QJsonDocument doc(mainObj);
    manager->post(request, doc.toJson());
}

void EmailAPI::onReplyFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        emit finished(true, "Sent successfully.");
    } else {
        QString errorDetail = reply->readAll();
        emit finished(false, QString("Error %1: %2").arg(reply->error()).arg(errorDetail));
    }
    reply->deleteLater();
}
