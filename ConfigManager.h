#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QString>
#include <QJsonObject>
#include <QFile>
#include <QJsonDocument>
#include <QDebug>
#include <QMessageBox>

class ConfigManager {
public:
    static ConfigManager& getInstance() {
        static ConfigManager instance;
        return instance;
    }

    QString getSmsToken() const { return getValue("api_keys", "tunisie_sms"); }
    QString getBrevoKey() const { return getValue("api_keys", "brevo"); }
    QString getGeminiKey() const { return getValue("api_keys", "gemini"); }

    // SMS Configuration
    QString getSmsSender() const { 
        QString s = getValue("sms_config", "sender");
        return s.isEmpty() ? "TunSMS Test" : s;
    }
    QString getSmsEndpoint() const { 
        QString e = getValue("sms_config", "endpoint");
        return e.isEmpty() ? "https://mystudents.tunisiesms.tn/api/sms" : e;
    }

private:
    ConfigManager() {
        loadConfig();
    }

    void loadConfig() {
        QFile file("config.json");
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "Could not open config.json. Features requiring API keys will fail.";
            return;
        }

        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isNull()) {
            configObject = doc.object();
        }
        file.close();
    }

    QString getValue(const QString &category, const QString &key) const {
        if (configObject.contains(category)) {
            QJsonObject catObj = configObject[category].toObject();
            if (catObj.contains(key)) {
                return catObj[key].toString();
            }
        }
        return "";
    }

    QJsonObject configObject;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
};

#endif // CONFIGMANAGER_H
