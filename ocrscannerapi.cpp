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
    keyPart.setBody("keyK81704964888957");
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
            
            // Logic to find Amount (Restored working version)
            double foundAmount = 0.0;
            QRegularExpression amtRegex("(?:Total|Amount|Sum|Net|Due|TOTAL|Montant|TTC)[:\\s]*([\\d\\s,.]+)");
            QRegularExpressionMatchIterator it = amtRegex.globalMatch(text);
            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
                QString valStr = match.captured(1).trimmed();
                valStr.remove(" "); 

                if (valStr.contains(",") && valStr.contains(".")) {
                    if (valStr.lastIndexOf(",") > valStr.lastIndexOf(".")) {
                        valStr.remove("."); valStr.replace(",", "."); 
                    } else {
                        valStr.remove(","); 
                    }
                } else if (valStr.contains(",")) {
                    valStr.replace(",", ".");
                }

                bool ok = false;
                double val = valStr.toDouble(&ok);
                if (ok && val > foundAmount) foundAmount = val;
            }

            // Logic to find Date (Improved Regex)
            QString foundDate = "";
            QRegularExpression dateRegex("(\\d{1,2}[-/.]\\d{1,2}[-/.]\\d{2,4})|(\\d{4}[-/.]\\d{1,2}[-/.]\\d{1,2})");
            QRegularExpressionMatch dateMatch = dateRegex.match(text);
            if (dateMatch.hasMatch()) foundDate = dateMatch.captured(0);

            // Logic to find Payment Mode
            QString foundMode = "Cash"; // Default
            QRegularExpression modeRegex("(?i)(Check|Chèque|Card|Carte|Virement|Bank|Banque|Transfer|Espèces|Cash)");
            QRegularExpressionMatch modeMatch = modeRegex.match(text);
            if (modeMatch.hasMatch()) {
                QString m = modeMatch.captured(0).toLower();
                if (m.contains("check") || m.contains("chèque")) foundMode = "Check";
                else if (m.contains("card") || m.contains("carte")) foundMode = "Card";
                else if (m.contains("virement") || m.contains("bank") || m.contains("transfer")) foundMode = "Transfer";
            }

            // Logic to find Description/Subject
            QString foundDesc = "";
            QRegularExpression descRegex("(?i)(?:Libellé|Description|Designation|Subject|Objet)[:\\s]*([^\\n\\r]+)");
            QRegularExpressionMatch descMatch = descRegex.match(text);
            if (descMatch.hasMatch()) {
                foundDesc = descMatch.captured(1).trimmed();
            } else {
                // Heuristic fallback: take first snippet of the second line or first line
                QStringList lines = text.split("\n", Qt::SkipEmptyParts);
                if (lines.size() > 1) foundDesc = lines[1].left(40).trimmed();
                else foundDesc = text.left(40).trimmed();
            }

            emit scanFinished(true, foundAmount, foundDate, foundDesc, foundMode, text);
        } else {
            emit scanFinished(false, 0, "", "", "", "No text found in image.");
        }
    } else {
        emit scanFinished(false, 0, "", "", "", "Network Error: " + reply->errorString());
    }
    reply->deleteLater();
}
