#ifndef INTERVENTION_H
#define INTERVENTION_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QDate>

class Intervention {
public:
    Intervention();
    Intervention(int idMachine, int idPersonnel, QDate date);

    bool ajouter();
    static QSqlQueryModel* afficher();
    
    QString getLastError() const;

private:
    int idMachine;
    int idPersonnel;
    QDate dateIntervention;
    QString lastError;
};

#endif // INTERVENTION_H
