#include "consultantagent.h"
#include <QRegularExpression>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

const QString GEMINI_API_KEY = "KEYAIzaSyB3BrvNfwWKgNSCKncX9Hr29Ob-S3WwQ3I";

ConsultantAgent::ConsultantAgent(QObject *parent) : QObject(parent)
{
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished, this, &ConsultantAgent::handleAiResponse);
    initializeKnowledge();
}

void ConsultantAgent::initializeKnowledge()
{
}

void ConsultantAgent::requestAiAdvice(const QString &userInput)
{
    QString snapshot = getInventorySnapshot();
    
    QString endpoint = "https://generativelanguage.googleapis.com/v1/models/gemini-2.5-flash:generateContent?key=" + GEMINI_API_KEY;
    QUrl url(endpoint);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QString prompt = "You are the Oil Quality Expert Advisor for an industrial oil press. "
                     "STRICT RULE: You ONLY answer questions related to oil quality, production standards, and product improvement. "
                     "If a user asks about anything else, politely state that you only specialize in oil quality.\n\n"
                     "Context: Current Batch Inventory (JSON):\n" + snapshot + "\n\n"
                     "User Question: " + userInput;
    
    QJsonObject textPart;
    textPart["text"] = prompt;
    QJsonArray parts; parts.append(textPart);
    QJsonObject contentObj;
    contentObj["role"] = "user";
    contentObj["parts"] = parts;
    QJsonArray contents; contents.append(contentObj);
    QJsonObject root; root["contents"] = contents;
    
    QJsonDocument doc(root);
    QNetworkReply *reply = networkManager->post(request, doc.toJson());
    reply->setProperty("type", "chat");
}

void ConsultantAgent::requestPriceForecast()
{
    QString endpoint = "https://generativelanguage.googleapis.com/v1/models/gemini-2.5-flash:generateContent?key=" + GEMINI_API_KEY;
    QUrl url(endpoint);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QString prompt = "You are a highly advanced agricultural economic model forecasting olive oil prices. "
                     "Generate a 6-month price forecast for Olive Oil in Tunisia (currency: DT/Liter) starting from next month. "
                     "The current base price is 28.50 DT. Factor in typical seasonal weather trends and market variables. "
                     "Return ONLY a valid JSON array of 6 objects. Do not include markdown formatting or explanation. "
                     "Example format: [{\"date\":\"2024-06\", \"weather\":\"Hot and Dry\", \"price\":28.75}, ...]";
    
    QJsonObject textPart;
    textPart["text"] = prompt;
    QJsonArray parts; parts.append(textPart);
    QJsonObject contentObj;
    contentObj["role"] = "user";
    contentObj["parts"] = parts;
    QJsonArray contents; contents.append(contentObj);
    QJsonObject root; root["contents"] = contents;
    
    QJsonDocument doc(root);
    QNetworkReply *reply = networkManager->post(request, doc.toJson());
    reply->setProperty("type", "forecast");
}

void ConsultantAgent::auditInventory()
{
    QString snapshot = getInventorySnapshot();
    
    QString endpoint = "https://generativelanguage.googleapis.com/v1/models/gemini-2.5-flash:generateContent?key=" + GEMINI_API_KEY;
    QUrl url(endpoint);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QString prompt = "You are the Oil Quality Expert Advisor. Please perform a comprehensive audit of our current inventory.\n\n"
                     "Current Inventory Data:\n" + snapshot + "\n\n"
                     "TASK:\n"
                     "1. Analyze viscosity, color, and test results for each batch.\n"
                     "2. Identify any batches that fall below premium quality standards.\n"
                     "3. Provide specific advice on how to improve the overall quality of our oil (storage, pressing, filtration).\n"
                     "4. Suggest pricing adjustments if quality variation exists.\n\n"
                     "Format your response with professional headings and clear bullet points.";
    
    QJsonObject textPart;
    textPart["text"] = prompt;
    QJsonArray parts; parts.append(textPart);
    QJsonObject contentObj;
    contentObj["role"] = "user";
    contentObj["parts"] = parts;
    QJsonArray contents; contents.append(contentObj);
    QJsonObject root; root["contents"] = contents;
    
    QJsonDocument doc(root);
    QNetworkReply *reply = networkManager->post(request, doc.toJson());
    reply->setProperty("type", "chat");
}

void ConsultantAgent::handleAiResponse(QNetworkReply *reply)
{
    QString type = reply->property("type").toString();
    QByteArray responseStr = reply->readAll();
    
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(responseStr);
        QJsonObject root = doc.object();
        
        if (root.contains("candidates")) {
            QJsonArray candidatesArray = root["candidates"].toArray();
            if (!candidatesArray.isEmpty()) {
                QJsonObject contentObj = candidatesArray[0].toObject()["content"].toObject();
                QJsonArray partsArray = contentObj["parts"].toArray();
                if (!partsArray.isEmpty()) {
                    QString text = partsArray[0].toObject()["text"].toString();
                    
                    if (type == "forecast") {
                        // Clean markdown and parse JSON array
                        QString cleaned = text.replace("```json", "").replace("```", "").trimmed();
                        QJsonDocument forecastDoc = QJsonDocument::fromJson(cleaned.toUtf8());
                        if (forecastDoc.isArray()) {
                            emit forecastReady(forecastDoc.array());
                        } else {
                            qDebug() << "Failed to parse forecast JSON:" << cleaned;
                            emit forecastReady(QJsonArray());
                        }
                    } else {
                        emit responseReady(text);
                    }
                    reply->deleteLater();
                    return;
                }
            }
        }
        
        if (type == "forecast") emit forecastReady(QJsonArray());
        else emit responseReady("Error: Unexpected API response format.");
    } else {
        QString errorMsg = reply->errorString();
        if (!responseStr.isEmpty()) {
            errorMsg += "\nBody: " + QString(responseStr);
        }
        
        if (type == "forecast") emit forecastReady(QJsonArray());
        else emit responseReady("API Error: " + errorMsg);
    }
    reply->deleteLater();
}

QString ConsultantAgent::getInventorySnapshot()
{
    QJsonArray arr;
    QSqlQuery q;
    if (!q.exec("SELECT ID_CONTENAIR, REF, VISCOSITE, COULEUR, TEST, QUANTITE, PRIX_UNITAIRE FROM PRODUIT")) {
        qDebug() << "DB Error in snapshot:" << q.lastError().text();
        return "[]";
    }
    
    while(q.next()) {
        QJsonObject obj;
        obj["id"] = q.value(0).toInt();
        obj["ref"] = q.value(1).toString();
        obj["viscosity"] = q.value(2).toString();
        obj["color"] = q.value(3).toString();
        obj["quality_test"] = q.value(4).toString();
        obj["quantity"] = q.value(5).toInt();
        obj["price"] = q.value(6).toDouble();
        arr.append(obj);
    }
    QJsonDocument doc(arr);
    return QString(doc.toJson(QJsonDocument::Compact));
}
