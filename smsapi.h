#ifndef SMSAPI_H
#define SMSAPI_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>

// Uses TextBelt free SMS API (https://textbelt.com)
// Free tier: 1 SMS/day with key "textbelt"
// For more: buy credits at textbelt.com (very cheap)
class SmsAPI : public QObject
{
    Q_OBJECT

public:
    explicit SmsAPI(QObject *parent = nullptr);
    ~SmsAPI();

    void sendSMS(const QString &toNumber, const QString &messageBody);

signals:
    void smsSent(bool success, const QString &responseMessage);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *networkManager;
};

#endif // SMSAPI_H
