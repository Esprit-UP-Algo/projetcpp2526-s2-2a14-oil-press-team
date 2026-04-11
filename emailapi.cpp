#include "emailapi.h"
#include <QAuthenticator>

EmailAPI::EmailAPI(QObject *parent) : QObject(parent)
{
    manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &EmailAPI::onReplyFinished);
}

void EmailAPI::setCredentials(const QString &apiKey, const QString &apiSecret)
{
    m_apiKey = apiKey;
    m_apiSecret = apiSecret;
}

void EmailAPI::sendEmail(const QString &to, const QString &subject, const QString &body)
{
    QUrl url("https://api.mailjet.com/v3.1/send");
    QNetworkRequest request(url);
    
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    // Basic Auth
    QString auth = QString("%1:%2").arg(m_apiKey).arg(m_apiSecret);
    QByteArray authData = auth.toLocal8Bit().toBase64();
    request.setRawHeader("Authorization", "Basic " + authData);

    // Payload
    QJsonObject fromObj;
    fromObj["Email"] = m_senderEmail;
    fromObj["Name"] = "Oil Press Manager";

    QJsonObject toObj;
    toObj["Email"] = to;
    toObj["Name"] = "Admin";

    QJsonArray toArray;
    toArray.append(toObj);

    QJsonObject messageObj;
    messageObj["From"] = fromObj;
    messageObj["To"] = toArray;
    messageObj["Subject"] = subject;
    messageObj["TextPart"] = body;

    QJsonArray messagesArray;
    messagesArray.append(messageObj);

    QJsonObject mainObj;
    mainObj["Messages"] = messagesArray;

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
