#ifndef EMAILAPI_H
#define EMAILAPI_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>

class EmailAPI : public QObject
{
    Q_OBJECT
public:
    explicit EmailAPI(QObject *parent = nullptr);
    
    // Replace with your real keys
    void setCredentials(const QString &apiKey);
    void setSenderEmail(const QString &email);
    void sendEmail(const QString &to, const QString &subject, const QString &body);

signals:
    void finished(bool success, const QString &errorMsg);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *manager;
    QString m_apiKey;
    QString m_senderEmail = "nour.benrhoumakok@gmail.com"; // Default sender
};

#endif // EMAILAPI_H
