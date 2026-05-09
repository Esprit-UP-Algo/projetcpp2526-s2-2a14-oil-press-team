#ifndef CONSULTANTAGENT_H
#define CONSULTANTAGENT_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class ConsultantAgent : public QObject
{
    Q_OBJECT
public:
    explicit ConsultantAgent(QObject *parent = nullptr);
    void requestAiAdvice(const QString &userInput);
    void auditInventory();

signals:
    void responseReady(const QString &response);

private slots:
    void handleAiResponse(QNetworkReply *reply);

private:
    struct ProductInfo {
        QString name;
        QString quality;
        QString price;
        QString summary;
    };
    QMap<QString, ProductInfo> knowledgeBase;
    QNetworkAccessManager *networkManager;
    void initializeKnowledge();
    QString getInventorySnapshot();
};

#endif // CONSULTANTAGENT_H
