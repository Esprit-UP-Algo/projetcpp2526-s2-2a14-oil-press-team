#include "machine.h"
#include <QSqlError>
#include <QDebug>
#include <random>

Machine::Machine()
{
    id = 0;
    nom = "";
    type = "";
    etat = "";
    localisation = "";
    heures = 0;
    seuilMaintenance = 0;
    dateDerniereMaintenance = QDate::currentDate();
}

Machine::Machine(int id, QString nom, QString type, QString etat, int heures, int seuil, QString loc, QDate dateM)
{
    this->id = id;
    this->nom = nom;
    this->type = type;
    this->etat = etat;
    this->heures = heures;
    this->seuilMaintenance = seuil;
    this->localisation = loc;
    this->dateDerniereMaintenance = dateM;
}

// Getters
int Machine::getId() const { return id; }
QString Machine::getNom() const { return nom; }
QString Machine::getType() const { return type; }
QString Machine::getEtat() const { return etat; }
int Machine::getHeures() const { return heures; }
int Machine::getSeuil() const { return seuilMaintenance; }
QString Machine::getLocalisation() const { return localisation; }
QDate Machine::getDateM() const { return dateDerniereMaintenance; }

// Setters
void Machine::setId(int id) { this->id = id; }
void Machine::setNom(const QString &nom) { this->nom = nom; }
void Machine::setType(const QString &type) { this->type = type; }
void Machine::setEtat(const QString &etat) { this->etat = etat; }
void Machine::setHeures(int heures) { this->heures = heures; }
void Machine::setSeuil(int seuil) { this->seuilMaintenance = seuil; }
void Machine::setLocalisation(const QString &loc) { this->localisation = loc; }
void Machine::setDateM(QDate dateM) { this->dateDerniereMaintenance = dateM; }

bool Machine::ajouter()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1000, 9999);
    
    QString codeStr;
    bool isUnique = false;
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT COUNT(*) FROM MACHINE WHERE CODE = :code");
    
    // Loop until we generate a code that doesn't already exist in the database
    while (!isUnique) {
        codeStr = QString::number(distrib(gen));
        checkQuery.bindValue(":code", codeStr);
        
        if (checkQuery.exec() && checkQuery.next()) {
            if (checkQuery.value(0).toInt() == 0) {
                isUnique = true; // 0 matches found, so this code is perfectly unique!
            }
        } else {
            // If the query fails for some reason, break out to avoid an infinite loop
            qDebug() << "Uniqueness check failed:" << checkQuery.lastError().text();
            break;
        }
    }

    QSqlQuery query;
    query.prepare("INSERT INTO MACHINE (ID_MACHINE, CODE, NOM_MACHINE, TYPE_MACHINE, ETAT_MACHINE, DATEDERNIEREMAINTENANCE, HEURESFONCTIONNEMENT, SEUILMAINTENANCE, LOCALISATION) "
                  "VALUES ((SELECT NVL(MAX(ID_MACHINE), 0) + 1 FROM MACHINE), :code, :nom, :type, :etat, :dateM, :heures, :seuil, :loc)");
    query.bindValue(":code", codeStr);
    query.bindValue(":nom", nom);
    query.bindValue(":type", type);
    query.bindValue(":etat", etat);
    query.bindValue(":dateM", dateDerniereMaintenance);
    query.bindValue(":heures", heures);
    query.bindValue(":seuil", seuilMaintenance);
    query.bindValue(":loc", localisation);

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
    
    // 1. Delete linked records to avoid ORA-02292 (Foreign Key constraints)
    // Clear production records
    query.prepare("DELETE FROM PRODUIRE WHERE ID_MACHINE = :id");
    query.bindValue(":id", id);
    query.exec();

    // Clear interventions
    query.prepare("DELETE FROM INTERVENIR WHERE ID_MACHINE = :id");
    query.bindValue(":id", id);
    query.exec();

    // Clear linked products (orphaned batches)
    query.prepare("DELETE FROM PRODUIT WHERE ID_MACHINE = :id");
    query.bindValue(":id", id);
    query.exec();

    // 2. Finally delete the machine
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
                  "ETAT_MACHINE = :etat, DATEDERNIEREMAINTENANCE = :dateM, HEURESFONCTIONNEMENT = :heures, SEUILMAINTENANCE = :seuil, LOCALISATION = :loc WHERE ID_MACHINE = :id");
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
        qDebug() << "Machine Modifier Error:" << lastError;
        return false;
    }
    return true;
}

QSqlQueryModel* Machine::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery("SELECT ID_MACHINE, CODE, NOM_MACHINE, TYPE_MACHINE, ETAT_MACHINE, HEURESFONCTIONNEMENT, SEUILMAINTENANCE, LOCALISATION FROM MACHINE");

    if (model->lastError().isValid()) {
        qDebug() << "Machine Afficher Error:" << model->lastError().text();
    }

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Code"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Name"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Type"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Status"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Hours"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Threshold"));
    model->setHeaderData(7, Qt::Horizontal, QObject::tr("Location"));

    return model;
}

QString Machine::getLastError() const { return lastError; }
