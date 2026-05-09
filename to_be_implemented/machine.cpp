#include "machine.h"
#include <QSqlError>
#include <QDebug>

Machine::Machine() {
    id = 0; nom = ""; type = ""; etat = ""; localisation = ""; heures = 0; seuilMaintenance = 0;
    dateDerniereMaintenance = QDate::currentDate();
}

Machine::Machine(int id, QString nom, QString type, QString etat, int heures, int seuil, QString loc, QDate dateM) {
    this->id = id; this->nom = nom; this->type = type; this->etat = etat; 
    this->heures = heures; this->seuilMaintenance = seuil; this->localisation = loc; 
    this->dateDerniereMaintenance = dateM.isNull() ? QDate::currentDate() : dateM;
}

int Machine::getId() const { return id; }
QString Machine::getNom() const { return nom; }
QString Machine::getType() const { return type; }
QString Machine::getEtat() const { return etat; }
int Machine::getHeures() const { return heures; }
int Machine::getSeuil() const { return seuilMaintenance; }
QString Machine::getLocalisation() const { return localisation; }
QDate Machine::getDateM() const { return dateDerniereMaintenance; }

void Machine::setId(int id) { this->id = id; }
void Machine::setNom(const QString &nom) { this->nom = nom; }
void Machine::setType(const QString &type) { this->type = type; }
void Machine::setEtat(const QString &etat) { this->etat = etat; }
void Machine::setHeures(int heures) { this->heures = heures; }
void Machine::setSeuil(int seuil) { this->seuilMaintenance = seuil; }
void Machine::setLocalisation(const QString &loc) { this->localisation = loc; }
void Machine::setDateM(QDate dateM) { this->dateDerniereMaintenance = dateM; }

bool Machine::ajouter() {
    QSqlQuery query;
    query.prepare("INSERT INTO MACHINE (ID_MACHINE, NOM_MACHINE, TYPE_MACHINE, ETAT_MACHINE, DATEDERNIEREMAINTENANCE, HEURESFONCTIONNEMENT, SEUILMAINTENANCE, LOCALISATION) "
                  "VALUES ((SELECT NVL(MAX(ID_MACHINE), 0) + 1 FROM MACHINE), :nom, :type, :etat, :dateM, :heures, :seuil, :loc)");
    query.bindValue(":nom", nom);
    query.bindValue(":type", type);
    query.bindValue(":etat", etat);
    query.bindValue(":dateM", dateDerniereMaintenance);
    query.bindValue(":heures", heures);
    query.bindValue(":seuil", seuilMaintenance);
    query.bindValue(":loc", localisation);

    if (!query.exec()) {
        lastError = query.lastError().text();
        return false;
    }
    return true;
}

bool Machine::modifier() {
    QSqlQuery query;
    query.prepare("UPDATE MACHINE SET NOM_MACHINE = :nom, TYPE_MACHINE = :type, ETAT_MACHINE = :etat, "
                  "DATEDERNIEREMAINTENANCE = :dateM, HEURESFONCTIONNEMENT = :heures, SEUILMAINTENANCE = :seuil, LOCALISATION = :loc WHERE ID_MACHINE = :id");
    query.bindValue(":id", id);
    query.bindValue(":nom", nom);
    query.bindValue(":type", type);
    query.bindValue(":etat", etat);
    query.bindValue(":dateM", dateDerniereMaintenance);
    query.bindValue(":heures", heures);
    query.bindValue(":seuil", seuilMaintenance);
    query.bindValue(":loc", localisation);

    if (!query.exec()) {
        lastError = query.lastError().text();
        return false;
    }
    return true;
}

bool Machine::supprimer(int id) {
    QSqlQuery query;
    query.prepare("DELETE FROM MACHINE WHERE ID_MACHINE = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        lastError = query.lastError().text();
        return false;
    }
    return true;
}

QSqlQueryModel* Machine::afficher() {
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery("SELECT ID_MACHINE, NOM_MACHINE, TYPE_MACHINE, ETAT_MACHINE, HEURESFONCTIONNEMENT, SEUILMAINTENANCE, LOCALISATION FROM MACHINE");
    return model;
}

QString Machine::getLastError() const { return lastError; }
