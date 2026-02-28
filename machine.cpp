#include "machine.h"
#include <QSqlError>
#include <QDebug>

Machine::Machine()
{
    id = 0;
    nom = "";
    type = "";
    etat = "";
    heures = 0;
}

Machine::Machine(int id, QString nom, QString type, QString etat, int heures)
{
    this->id = id;
    this->nom = nom;
    this->type = type;
    this->etat = etat;
    this->heures = heures;
}

// Getters
int Machine::getId() const { return id; }
QString Machine::getNom() const { return nom; }
QString Machine::getType() const { return type; }
QString Machine::getEtat() const { return etat; }
int Machine::getHeures() const { return heures; }

// Setters
void Machine::setId(int id) { this->id = id; }
void Machine::setNom(const QString &nom) { this->nom = nom; }
void Machine::setType(const QString &type) { this->type = type; }
void Machine::setEtat(const QString &etat) { this->etat = etat; }
void Machine::setHeures(int heures) { this->heures = heures; }

bool Machine::ajouter()
{
    QSqlQuery query;
    query.prepare("INSERT INTO MACHINE (ID_MACHINE, NOM_MACHINE, TYPE_MACHINE, ETAT_MACHINE, HEURESFONCTIONNEMENT) "
                  "VALUES ((SELECT NVL(MAX(ID_MACHINE), 0) + 1 FROM MACHINE), :nom, :type, :etat, :heures)");
    query.bindValue(":nom", nom);
    query.bindValue(":type", type);
    query.bindValue(":etat", etat);
    query.bindValue(":heures", heures);

    if (!query.exec()) {
        lastError = query.lastError().text();
        qDebug() << "Machine Ajouter Error:" << lastError;
        return false;
    }
    return true;
}

bool Machine::supprimer(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM MACHINE WHERE ID_MACHINE = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        lastError = query.lastError().text();
        qDebug() << "Machine Supprimer Error:" << lastError;
        return false;
    }
    return true;
}

bool Machine::modifier()
{
    QSqlQuery query;
    query.prepare("UPDATE MACHINE SET NOM_MACHINE = :nom, TYPE_MACHINE = :type, "
                  "ETAT_MACHINE = :etat, HEURESFONCTIONNEMENT = :heures WHERE ID_MACHINE = :id");
    query.bindValue(":id", id);
    query.bindValue(":nom", nom);
    query.bindValue(":type", type);
    query.bindValue(":etat", etat);
    query.bindValue(":heures", heures);

    if (!query.exec()) {
        lastError = query.lastError().text();
        qDebug() << "Machine Modifier Error:" << lastError;
        return false;
    }
    return true;
}

QSqlQueryModel* Machine::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery("SELECT ID_MACHINE, NOM_MACHINE, TYPE_MACHINE, ETAT_MACHINE, HEURESFONCTIONNEMENT FROM MACHINE");

    if (model->lastError().isValid()) {
        qDebug() << "Machine Afficher Error:" << model->lastError().text();
    }

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Name"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Type"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Status"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Hours"));

    return model;
}

QString Machine::getLastError() const { return lastError; }
