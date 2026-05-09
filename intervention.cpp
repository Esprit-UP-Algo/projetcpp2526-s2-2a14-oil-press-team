#include "intervention.h"
#include <QSqlError>
#include <QDebug>

Intervention::Intervention() : idMachine(0), idPersonnel(0), dateIntervention(QDate::currentDate()) {}

Intervention::Intervention(int idMachine, int idPersonnel, QDate date)
    : idMachine(idMachine), idPersonnel(idPersonnel), dateIntervention(date) {}

bool Intervention::ajouter() {
    QSqlQuery query;
    query.prepare("INSERT INTO INTERVENIR (ID_MACHINE, ID_PERSONNEL, DATE_INTERVENTION) VALUES (:idM, :idP, :dateI)");
    query.bindValue(":idM", idMachine);
    query.bindValue(":idP", idPersonnel);
    query.bindValue(":dateI", dateIntervention);
    
    if (!query.exec()) {
        lastError = query.lastError().text();
        qDebug() << "Intervention::ajouter Error:" << lastError;
        return false;
    }
    return true;
}

QSqlQueryModel* Intervention::afficher() {
    QSqlQueryModel *model = new QSqlQueryModel();
    // Join with MACHINE and PERSONNEL to get readable names
    model->setQuery("SELECT M.NOM_MACHINE, P.NOM_PERSONNEL, I.DATE_INTERVENTION "
                    "FROM INTERVENIR I "
                    "JOIN MACHINE M ON I.ID_MACHINE = M.ID_MACHINE "
                    "JOIN PERSONNEL P ON I.ID_PERSONNEL = P.ID_PERSONNEL "
                    "ORDER BY I.DATE_INTERVENTION DESC");
    
    if (model->lastError().isValid()) {
        qDebug() << "Intervention::afficher Error:" << model->lastError().text();
    }
    
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Machine"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Technician"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Date"));
    
    return model;
}

QString Intervention::getLastError() const { return lastError; }
