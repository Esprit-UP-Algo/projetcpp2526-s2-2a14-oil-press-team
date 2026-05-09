#include "smsapi.h"
#include "ConfigManager.h"
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QMessageBox>

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
    // STAGE 1: Robust Sanitization
    // We remove all non-digits, then handle the 216 prefix
    QString sanitizedNumber = toNumber;
    sanitizedNumber.remove(QRegularExpression("[^\\d]")); // Strip spaces, +, -, etc.

    // If it starts with 00216, remove the 00
    if (sanitizedNumber.startsWith("00216")) {
        sanitizedNumber.remove(0, 2);
    }
    // If it's 8 digits long (typical Tunis local), prefix with 216
    if (!sanitizedNumber.startsWith("216") && sanitizedNumber.length() == 8) {
        sanitizedNumber = "216" + sanitizedNumber;
    }


    // Endpoint from Config
    QUrl url(ConfigManager::getInstance().getSmsEndpoint());
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Auth: Bearer Token
    QString apiKey = ConfigManager::getInstance().getSmsToken();
    if (apiKey.isEmpty()) {
        // apiKey = "36qQASFUsnp6TEG7fPyId8v5j!G1XFPbatDnCL0xjcctcneBYSFAkNiNZ!Rkoy9b9dsSu2pKtcCCYtJLC2XBcqMOsLOzn06ka1SJAK55";
        apiKey = "key36qQASFUsnp6TEG7fPyId8v5j!G1XFPbatDnCL0xjcctcneBYSFAkNiNZ!Rkoy9b9dsSu2pKtcCCYtJLC2XBcqMOsLOzn06ka1SJAK55"; // Provide a default or handle the error
    }
    request.setRawHeader("Authorization", "Bearer " + apiKey.trimmed().toUtf8());

    // Payload: Type 55 is MANDATORY per docs
    QJsonObject payload;
    payload.insert("type", "55"); 
    payload.insert("sender", ConfigManager::getInstance().getSmsSender());

    QJsonObject smsObj;
    smsObj.insert("mobile", sanitizedNumber);
    smsObj.insert("sms", messageBody);

    QJsonArray smsArray;
    smsArray.append(smsObj);
    payload.insert("sms", smsArray);

    QJsonDocument doc(payload);
    networkManager->post(request, doc.toJson());

    qDebug() << "SmsAPI (v8 - Robust) sent to:" << sanitizedNumber;
}

void SmsAPI::onReplyFinished(QNetworkReply *reply)
{
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray responseData = reply->readAll();


    bool isSuccess = false;
    QString displayMsg = "";
    QString errorDetail = "";

    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
        
        // Documentation shows the response is an ARRAY of objects: [{"mobile":..., "status":...}]
        if (jsonDoc.isArray()) {
            QJsonArray arr = jsonDoc.array();
            if (!arr.isEmpty()) {
                QJsonObject resObj = arr.at(0).toObject();
                // status might be a number or a string in JSON, toVariant().toInt() handles both
                int apiStatus = resObj.value("status").toVariant().toInt();

                if (apiStatus == 200) {
                    isSuccess = true;
                    displayMsg = "Message accepted by provider (Status 200).";
                } else {
                    // Map common Tunisie SMS error codes
                    switch(apiStatus) {
                        case 401: errorDetail = "401: Invalid API Key"; break;
                        case 402: errorDetail = "402: Insufficient Credit"; break;
                        case 442: errorDetail = "442: Sender NOT Authorized! (Check your dashbaord)"; break;
                        default:  errorDetail = QString("API Error Code: %1").arg(apiStatus); break;
                    }
                    displayMsg = errorDetail + "\n\nRaw Body: " + QString(responseData);
                }
            } else {
                displayMsg = "Empty JSON array received from server.";
            }
        } else if (responseData.trimmed() == "1") {
            isSuccess = true;
            displayMsg = "Legacy confirmation received (1).";
        } else {
            displayMsg = "Non-standard response format:\n" + QString(responseData);
        }
    } else {
        displayMsg = QString("Network Error %1: %2\nTarget URL: %4\nBody: %3")
            .arg(statusCode)
            .arg(reply->errorString())
            .arg(QString(responseData))
            .arg(reply->url().toString());
    }

    if (isSuccess) {
        QMessageBox::information(nullptr, "SMS API - SUCCESS", displayMsg);
        emit smsSent(true, displayMsg);
    } else {
        QMessageBox::critical(nullptr, "SMS API - ERROR", displayMsg);
        emit smsSent(false, displayMsg);
    }
    reply->deleteLater();
}
