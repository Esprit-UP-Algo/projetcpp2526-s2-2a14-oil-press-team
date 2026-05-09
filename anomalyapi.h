#ifndef ANOMALYAPI_H
#define ANOMALYAPI_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QUrl>
#include <QUrlQuery>
#include <QString>

class AnomalyAPI : public QObject
{
    Q_OBJECT
public:
    explicit AnomalyAPI(QObject *parent = nullptr);
    void scanTransactions(const QJsonArray &transactions);

signals:
    void scanFinished(const QJsonObject &report);
    void errorOccurred(const QString &error);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *manager;
    QJsonArray pendingTransactions;
    QJsonArray completedResults;
    int currentIndex;

    void processNextTransaction();
};

#endif // ANOMALYAPI_H
