#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QString>
#include <QJsonObject>
#include <QFile>
#include <QJsonDocument>
#include <QDebug>
#include <QMessageBox>
#include <QCoreApplication>
#include <QDir>

class ConfigManager {
public:
    static ConfigManager& getInstance() {
        static ConfigManager instance;
        return instance;
    }

    QString getSmsToken() const { return getValue("api_keys", "tunisie_sms"); }
    QString getBrevoKey() const { return getValue("api_keys", "brevo"); }
    QString getGeminiKey() const { return getValue("api_keys", "gemini"); }
    
    // Email Settings
    QString getSenderEmail() const { return getValue("email_settings", "sender_email"); }
    QString getAdminEmail() const { return getValue("email_settings", "admin_email"); }
    QString getSmtpUser() const { return getValue("email_settings", "smtp_user"); }
    QString getSmtpPass() const { return getValue("email_settings", "smtp_pass"); }

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
        QString path = "config.json";
        qDebug() << "Checking for config at current dir:" << QDir::currentPath() + "/" + path;
        
        if (!QFile::exists(path)) {
            path = QCoreApplication::applicationDirPath() + "/config.json";
            qDebug() << "Not found. Checking app dir:" << path;
        }

        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "CRITICAL: Could not open config.json at" << path;
            qWarning() << "Please ensure config.json exists in either the working directory or the application directory.";
            return;
        }
        qDebug() << "Successfully loaded config from:" << path;

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
