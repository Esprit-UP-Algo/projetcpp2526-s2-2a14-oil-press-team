#ifndef CONSULTANTAGENT_H
#define CONSULTANTAGENT_H

#include <QObject>
#include <QString>
#include <QMap>

class ConsultantAgent : public QObject
{
    Q_OBJECT
public:
    explicit ConsultantAgent(QObject *parent = nullptr);
    QString getResponse(const QString &userInput);

private:
    struct ProductInfo {
        QString name;
        QString quality;
        QString price;
        QString summary;
    };
    QMap<QString, ProductInfo> knowledgeBase;
    void initializeKnowledge();
};

#endif // CONSULTANTAGENT_H
