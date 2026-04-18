#ifndef MARKETAPI_H
#define MARKETAPI_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>

class MarketAPI : public QObject
{
    Q_OBJECT
public:
    explicit MarketAPI(QObject *parent = nullptr);
    void fetchNews(const QString &query = "olive oil industry");

signals:
    void newsReceived(const QJsonArray &articles);
    void errorOccurred(const QString &error);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *manager;
};

#endif // MARKETAPI_H
