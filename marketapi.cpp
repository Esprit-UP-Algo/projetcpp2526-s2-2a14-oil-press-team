#include "marketapi.h"

MarketAPI::MarketAPI(QObject *parent) : QObject(parent)
{
    manager = new QNetworkAccessManager(this);
}

void MarketAPI::fetchNews(const QString &query)
{
    // NewsAPI.org URL - Everything about olive oil market
    // IMPORTANT: Visit https://newsapi.org to get your own FREE API key
    QString apiKey = "key18b5988d0c2347fcbd1d03032530c2c3";
    
    QUrl url(QString("https://newsapi.org/v2/everything?q=%1&sortBy=publishedAt&pageSize=12")
             .arg(query));
    
    QNetworkRequest request(url);
    request.setRawHeader("X-Api-Key", apiKey.toUtf8());
    request.setRawHeader("User-Agent", "OilPressManager/1.0"); // NewsAPI requires a User-Agent
    
    manager->get(request);
    
    connect(manager, &QNetworkAccessManager::finished, this, &MarketAPI::onReplyFinished);
}

void MarketAPI::onReplyFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject root = doc.object();
        if (root.value("status").toString() == "ok") {
            emit newsReceived(root.value("articles").toArray());
        } else {
            emit errorOccurred("API Error: " + root.value("message").toString());
        }
    } else {
        emit errorOccurred("Network Error: " + reply->errorString());
    }
    reply->deleteLater();
}
