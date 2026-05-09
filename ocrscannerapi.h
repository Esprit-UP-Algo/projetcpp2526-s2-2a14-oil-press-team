#ifndef OCRSCANNERAPI_H
#define OCRSCANNERAPI_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class OCRScannerAPI : public QObject {
    Q_OBJECT
public:
    explicit OCRScannerAPI(QObject *parent = nullptr);
    void scanInvoice(const QString &imagePath);

signals:
    void scanFinished(bool success, double amount, const QString &date, const QString &desc, const QString &mode, const QString &rawText);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *manager;
};

#endif // OCRSCANNERAPI_H
