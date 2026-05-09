#ifndef SMSAPI_H
#define SMSAPI_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>

// Integrated with Tunisie SMS REST JSON API (https://www.tunisiesms.tn)
// Required parameters: api_key, sender, message, recipient
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
