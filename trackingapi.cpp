#include "trackingapi.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDesktopServices>
#include <QUrl>
#include <QUrlQuery>
#include <QDebug>

TrackingAPI::TrackingAPI(QObject *parent) : QObject(parent)
{
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished, this, &TrackingAPI::onReplyFinished);
}

TrackingAPI::~TrackingAPI()
{
}

// ─── Static: Open OpenStreetMap in the browser with the delivery route ────────
// Uses OpenStreetMap Directions (OSRM) — 100% FREE, no API key needed
void TrackingAPI::openDeliveryRouteInBrowser(const QString &originAddress,
                                              const QString &clientAddress)
{
    // Build an OpenStreetMap directions URL
    // Format: https://www.openstreetmap.org/directions?from=ORIGIN&to=DEST
    QString from = QUrl::toPercentEncoding(originAddress);
    QString to   = QUrl::toPercentEncoding(clientAddress);

    QString urlStr = QString("https://www.openstreetmap.org/directions?engine=fossgis_osrm_car&route=%1;%2")
                         .arg(from, to);

    // Fallback: simpler format that always works
    QString fallback = QString("https://www.google.com/maps/dir/?api=1&origin=%1&destination=%2&travelmode=driving")
                           .arg(QString(QUrl::toPercentEncoding(originAddress)),
                                QString(QUrl::toPercentEncoding(clientAddress)));

    qDebug() << "Opening delivery route:" << fallback;
    QDesktopServices::openUrl(QUrl(fallback));
}

// ─── Geocode an address using Nominatim (OpenStreetMap free geocoder) ─────────
void TrackingAPI::geocodeAddress(const QString &address)
{
    QUrl url("https://nominatim.openstreetmap.org/search");
    QUrlQuery params;
    params.addQueryItem("q", address);
    params.addQueryItem("format", "json");
    params.addQueryItem("limit", "1");
    url.setQuery(params);

    QNetworkRequest request(url);
    // Nominatim requires a User-Agent header
    request.setRawHeader("User-Agent", "OilPressManager/1.0");

    networkManager->get(request);
}

void TrackingAPI::onReplyFinished(QNetworkReply *reply)
{
    QList<TrackingCheckpoint> checkpoints;

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);

        // Nominatim returns a JSON array
        if (doc.isArray() && !doc.array().isEmpty()) {
            QJsonObject result = doc.array().first().toObject();
            double lat = result.value("lat").toString().toDouble();
            double lon = result.value("lon").toString().toDouble();
            QString displayName = result.value("display_name").toString();

            TrackingCheckpoint cp;
            cp.location  = displayName;
            cp.message   = "Address found";
            cp.lat       = lat;
            cp.lon       = lon;
            checkpoints.append(cp);

            emit geocodeFinished(true, lat, lon, "");
            emit trackingDataReceived(true, checkpoints, "");
        } else {
            emit geocodeFinished(false, 0, 0, "Address not found.");
            emit trackingDataReceived(false, checkpoints, "Address not found.");
        }
    } else {
        QString err = reply->errorString();
        qDebug() << "TrackingAPI network error:" << err;
        emit geocodeFinished(false, 0, 0, err);
        emit trackingDataReceived(false, checkpoints, err);
    }
    reply->deleteLater();
}
