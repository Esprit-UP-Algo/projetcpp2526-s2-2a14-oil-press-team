#ifndef DEALGENERATOR_H
#define DEALGENERATOR_H

#include <QObject>
#include <QString>
#include <QList>
#include <QRandomGenerator>

struct GeneratedDeal {
    QString title;
    QString supplier;
    QString savings;
    QString description;
    QString url;
};

class DealGenerator : public QObject {
public:
    explicit DealGenerator(QObject *parent = nullptr);
    QList<GeneratedDeal> generateDeals(int count = 6);

private:
    struct DealTemplate {
        QString title;
        QString supplier;
        QString category;
        QString url;
        QString baseDesc;
    };
    QList<DealTemplate> m_templates;
    void initializeTemplates();
};

#endif // DEALGENERATOR_H
