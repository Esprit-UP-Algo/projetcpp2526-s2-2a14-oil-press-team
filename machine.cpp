#include "machine.h"
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>

// --- Default Constructor ---
Machine::Machine() {
    id = 0;
    nom = "";
    type = "";
    statut = "Normal";
    heures = 0;
}

// --- Parameterized Constructor ---
Machine::Machine(int id, const QString &nom, const QString &type,
                 const QString &statut, int heures) {
    this->id = id;
    this->nom = nom;
    this->type = type;
    this->statut = statut;
    this->heures = heures;
}

// --- Getters ---
int Machine::getId() const { return id; }
QString Machine::getNom() const { return nom; }
QString Machine::getType() const { return type; }
QString Machine::getStatut() const { return statut; }
int Machine::getHeures() const { return heures; }

// --- Setters ---
void Machine::setId(int id) { this->id = id; }
void Machine::setNom(const QString &nom) { this->nom = nom; }
void Machine::setType(const QString &type) { this->type = type; }
void Machine::setStatut(const QString &statut) { this->statut = statut; }
void Machine::setHeures(int heures) { this->heures = heures; }

// --- CRUD: Ajouter (CREATE) ---
bool Machine::ajouter() {
    // Step 1: get next ID (two-step to avoid ODBC subquery constraint)
    QSqlQuery idQuery;
    idQuery.prepare("SELECT NVL(MAX(ID_MACHINE), 0) + 1 FROM MACHINE");
    if (!idQuery.exec() || !idQuery.next()) {
        qDebug() << "Machine ID fetch Error:" << idQuery.lastError().text();
        lastError = idQuery.lastError().text();
        return false;
    }
    int nextId = idQuery.value(0).toInt();

    // Step 2: Insert
    QSqlQuery query;
    query.prepare(
        "INSERT INTO MACHINE (ID_MACHINE, NOM_MACHINE, TYPE_MACHINE, ETAT_MACHINE, HEURESFONCTIONNEMENT) "
        "VALUES (:id, :nom, :type, :statut, :heures)");
    query.bindValue(":id", nextId);
    query.bindValue(":nom", nom);
    query.bindValue(":type", type);
    query.bindValue(":statut", statut);
    query.bindValue(":heures", heures);

    if (!query.exec()) {
        qDebug() << "Machine ajouter Error:" << query.lastError().text();
        lastError = query.lastError().text();
        return false;
    }
    this->id = nextId;
    return true;
}

// --- CRUD: Supprimer (DELETE) ---
bool Machine::supprimer(int id) {
    QSqlQuery query;
    query.prepare("DELETE FROM MACHINE WHERE ID_MACHINE = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Machine supprimer Error:" << query.lastError().text();
        lastError = query.lastError().text();
        return false;
    }
    return true;
}

// --- CRUD: Modifier (UPDATE) ---
bool Machine::modifier() {
    QSqlQuery query;
    query.prepare(
        "UPDATE MACHINE SET NOM_MACHINE = :nom, TYPE_MACHINE = :type, "
        "ETAT_MACHINE = :statut, HEURESFONCTIONNEMENT = :heures WHERE ID_MACHINE = :id");
    query.bindValue(":id", id);
    query.bindValue(":nom", nom);
    query.bindValue(":type", type);
    query.bindValue(":statut", statut);
    query.bindValue(":heures", heures);

    if (!query.exec()) {
        qDebug() << "Machine modifier Error:" << query.lastError().text();
        lastError = query.lastError().text();
        return false;
    }
    return true;
}

// --- CRUD: Afficher (READ) ---
QSqlQueryModel *Machine::afficher() {
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery(
        "SELECT ID_MACHINE, NOM_MACHINE, TYPE_MACHINE, ETAT_MACHINE, HEURESFONCTIONNEMENT "
        "FROM MACHINE ORDER BY ID_MACHINE");

    if (model->lastError().isValid()) {
        qDebug() << "Machine afficher Error:" << model->lastError().text();
    }

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Name"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Type"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Status"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Hours"));

    return model;
}

// --- Get Last Error ---
QString Machine::getLastError() const { return lastError; }
