#include "ocrscannerapi.h"
#include <QFileInfo>
#include <QRegularExpression>
#include <QHttpPart>

OCRScannerAPI::OCRScannerAPI(QObject *parent) : QObject(parent) {
    manager = new QNetworkAccessManager(this);
}

void OCRScannerAPI::scanInvoice(const QString &imagePath) {
    QUrl url("https://api.ocr.space/parse/image");
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    // API Key (Using a public trial key provided by OCR.space for demo)
    QHttpPart keyPart;
    keyPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"apikey\""));
    keyPart.setBody("helloworld"); 
    multiPart->append(keyPart);

    // Language
    QHttpPart langPart;
    langPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"language\""));
    langPart.setBody("eng");
    multiPart->append(langPart);

    // OCREngine 2 (Better for diverse layouts and invoices)
    QHttpPart enginePart;
    enginePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"OCREngine\""));
    enginePart.setBody("2");
    multiPart->append(enginePart);

    // isTable=true helps with structured data like invoices
    QHttpPart tablePart;
    tablePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"isTable\""));
    tablePart.setBody("true");
    multiPart->append(tablePart);

    // Image File
    QFile *file = new QFile(imagePath);
    if (!file->open(QIODevice::ReadOnly)) {
        delete multiPart;
        return;
    }

    QHttpPart filePart;
    QString ext = QFileInfo(imagePath).suffix().toLower();
    if (ext == "pdf") {
        filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/pdf"));
    } else if (ext == "png") {
        filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/png"));
    } else if (ext == "bmp") {
        filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/bmp"));
    } else {
        filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/jpeg"));
    }

    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, 
        QVariant(QString("form-data; name=\"file\"; filename=\"%1\"").arg(QFileInfo(imagePath).fileName())));
    filePart.setBodyDevice(file);
    file->setParent(multiPart); 
    multiPart->append(filePart);

    QNetworkRequest request(url);
    QNetworkReply *reply = manager->post(request, multiPart);
    multiPart->setParent(reply); 

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onReplyFinished(reply);
    });
}

void OCRScannerAPI::onReplyFinished(QNetworkReply *reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject root = doc.object();
        QJsonArray results = root["ParsedResults"].toArray();

        if (!results.isEmpty()) {
            QString text = results[0].toObject()["ParsedText"].toString();
            
            // Logic to find Amount (Improved Regex)
            double foundAmount = 0.0;
            // Matches "Total", "Montant", etc. followed by digits, spaces, dots or commas
            QRegularExpression amtRegex("(?:Total|Amount|Sum|Net|Due|TOTAL|Montant|TTC)[:\\s]*([\\d\\s,.]+)");
            QRegularExpressionMatchIterator it = amtRegex.globalMatch(text);
            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
                QString valStr = match.captured(1).trimmed();
                valStr.remove(" "); // Remove thousands separator spaces

                // Heuristic: if it contains both comma and dot, determine which is decimal
                if (valStr.contains(",") && valStr.contains(".")) {
                    if (valStr.lastIndexOf(",") > valStr.lastIndexOf(".")) {
                        valStr.remove("."); valStr.replace(",", "."); // Comma is decimal
                    } else {
                        valStr.remove(","); // Dot is decimal
                    }
                } else if (valStr.contains(",")) {
                    valStr.replace(",", "."); // Assume comma is decimal (common in TN/FR)
                }

                bool ok = false;
                double val = valStr.toDouble(&ok);
                if (ok && val > foundAmount) foundAmount = val;
            }

            // Logic to find Date (Improved Regex)
            QString foundDate = "";
            // Matches various date formats: YYYY-MM-DD, DD/MM/YYYY, DD.MM.YYYY, etc.
            QRegularExpression dateRegex("(\\d{1,2}[-/.]\\d{1,2}[-/.]\\d{2,4})|(\\d{4}[-/.]\\d{1,2}[-/.]\\d{1,2})");
            QRegularExpressionMatch dateMatch = dateRegex.match(text);
            if (dateMatch.hasMatch()) foundDate = dateMatch.captured(0);

            emit scanFinished(true, foundAmount, foundDate, text);
        } else {
            emit scanFinished(false, 0, "", "No text found in image.");
        }
    } else {
        emit scanFinished(false, 0, "", "Network Error: " + reply->errorString());
    }
    reply->deleteLater();
}
