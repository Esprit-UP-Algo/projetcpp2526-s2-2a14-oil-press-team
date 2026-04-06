#include "personnel.h"
#include <QDebug>

Personnel::Personnel() {
    cin = 0;
    nom = "";
    salaire = 0.0;
    adresse = "";
    tel = "";
    experience = 0;
    grade = "";
    role = "";
}

Personnel::Personnel(int cin, QString nom, double salaire, QString adresse, QString tel, int exp, QString grade, QString role) {
    this->cin = cin;
    this->nom = nom;
    this->salaire = salaire;
    this->adresse = adresse;
    this->tel = tel;
    this->experience = exp;
    this->grade = grade;
    this->role = role;
}

// Getters
int Personnel::getCin() const { return cin; }
QString Personnel::getNom() const { return nom; }
double Personnel::getSalaire() const { return salaire; }
QString Personnel::getAdresse() const { return adresse; }
QString Personnel::getTel() const { return tel; }
int Personnel::getExperience() const { return experience; }
QString Personnel::getGrade() const { return grade; }
QString Personnel::getRole() const { return role; }

// Setters
void Personnel::setCin(int cin) { this->cin = cin; }
void Personnel::setNom(const QString &nom) { this->nom = nom; }
void Personnel::setSalaire(double salaire) { this->salaire = salaire; }
void Personnel::setAdresse(const QString &adresse) { this->adresse = adresse; }
void Personnel::setTel(const QString &tel) { this->tel = tel; }
void Personnel::setExperience(int exp) { this->experience = exp; }
void Personnel::setGrade(const QString &grade) { this->grade = grade; }
void Personnel::setRole(const QString &role) { this->role = role; }

// CRUD: Ajouter
bool Personnel::ajouter() {
    QSqlQuery query;
    query.prepare("INSERT INTO PERSONNEL (ID_PERSONNEL, NOM_PERSONNEL, SALAIRE_BRUT, ADRESSE, TEL, EXPERIENCE, GRADE, ROLE) "
                  "VALUES (:cin, :nom, :salaire, :adresse, :tel, :exp, :grade, :role)");
    query.bindValue(":cin", cin);
    query.bindValue(":nom", nom);
    query.bindValue(":salaire", salaire);
    query.bindValue(":adresse", adresse);
    query.bindValue(":tel", tel);
    query.bindValue(":exp", experience);
    query.bindValue(":grade", grade);
    query.bindValue(":role", role);

    if (!query.exec()) {
        qDebug() << "Ajouter Personnel Error:" << query.lastError().text();
        lastError = query.lastError().text();
        return false;
    }
    return true;
}

// CRUD: Supprimer
bool Personnel::supprimer(int cin) {
    QSqlQuery query;
    query.prepare("DELETE FROM PERSONNEL WHERE ID_PERSONNEL = :cin");
    query.bindValue(":cin", cin);

    if (!query.exec()) {
        qDebug() << "Supprimer Personnel Error:" << query.lastError().text();
        lastError = query.lastError().text();
        return false;
    }
    return true;
}

// CRUD: Modifier
bool Personnel::modifier() {
    QSqlQuery query;
    query.prepare("UPDATE PERSONNEL SET NOM_PERSONNEL = :nom, SALAIRE_BRUT = :salaire, ADRESSE = :adresse, "
                  "TEL = :tel, EXPERIENCE = :exp, GRADE = :grade, ROLE = :role WHERE ID_PERSONNEL = :cin");
    query.bindValue(":cin", cin);
    query.bindValue(":nom", nom);
    query.bindValue(":salaire", salaire);
    query.bindValue(":adresse", adresse);
    query.bindValue(":tel", tel);
    query.bindValue(":exp", experience);
    query.bindValue(":grade", grade);
    query.bindValue(":role", role);

    if (!query.exec()) {
        qDebug() << "Modifier Personnel Error:" << query.lastError().text();
        lastError = query.lastError().text();
        return false;
    }
    return true;
}

// CRUD: Afficher
QSqlQueryModel* Personnel::afficher() {
    QSqlQueryModel* model = new QSqlQueryModel();
    model->setQuery("SELECT ID_PERSONNEL, NOM_PERSONNEL, SALAIRE_BRUT, ADRESSE, TEL, EXPERIENCE, GRADE, ROLE FROM PERSONNEL");

    if (model->lastError().isValid()) {
        qDebug() << "Afficher Personnel Error:" << model->lastError().text();
    }

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("CIN"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Name"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Salary"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Address"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Phone"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Exp"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Grade"));
    model->setHeaderData(7, Qt::Horizontal, QObject::tr("Role"));

    return model;
}

QString Personnel::getLastError() const {
    return lastError;
}
