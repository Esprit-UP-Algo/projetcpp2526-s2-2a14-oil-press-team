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
        QString path = "";
        QString absolutePath = "C:/Users/MSI/OneDrive - ESPRIT/Bureau/projetcpp2526-s2-2a14-oil-press-team - Copie/config.json";
        
        if (QFile::exists(absolutePath)) {
            path = absolutePath;
        } else if (QFile::exists(QCoreApplication::applicationDirPath() + "/config.json")) {
            path = QCoreApplication::applicationDirPath() + "/config.json";
        } else if (QFile::exists("config.json")) {
            path = "config.json";
        }

        if (path.isEmpty()) {
            QMessageBox::critical(nullptr, "Config Error", "Could not find config.json anywhere!\nTried: " + absolutePath);
            return;
        }

        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(nullptr, "Config Error", "Found config.json but could not open it: " + path);
            return;
        }

        QByteArray data = file.readAll();
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
        
        if (doc.isNull()) {
            QMessageBox::critical(nullptr, "Config Parse Error", "config.json has invalid JSON format!\nError: " + parseError.errorString());
            file.close();
            return;
        }
        
        configObject = doc.object();
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
