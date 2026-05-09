#ifndef TRACKINGAPI_H
#define TRACKINGAPI_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QList>

// Checkpoint in the delivery route (city name + coords from geocoding)
struct TrackingCheckpoint {
    QString location;
    QString message;
    QString timestamp;
    double lat = 0.0;
    double lon = 0.0;
};

class TrackingAPI : public QObject
{
    Q_OBJECT

public:
    explicit TrackingAPI(QObject *parent = nullptr);
    ~TrackingAPI();

    // Opens OpenStreetMap in the browser showing the route from origin to clientAddress
    // originAddress = your warehouse/company address
    // clientAddress = the client's delivery address
    static void openDeliveryRouteInBrowser(const QString &originAddress, const QString &clientAddress);

    // Geocodes an address and returns lat/lon via signal
    void geocodeAddress(const QString &address);

signals:
    void trackingDataReceived(bool success, const QList<TrackingCheckpoint> &checkpoints, const QString &errorMsg);
    void geocodeFinished(bool success, double lat, double lon, const QString &errorMsg);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *networkManager;
};

#endif // TRACKINGAPI_H
