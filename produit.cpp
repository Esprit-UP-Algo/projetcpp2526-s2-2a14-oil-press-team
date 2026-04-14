#include "produit.h"
#include <QDebug>
#include <QSqlError>

Produit::Produit() {
  idContenair = 0;
  datePress = QDate::currentDate();
  quantite = 0;
  ref = "";
  viscosite = "";
  couleur = "";
  test = "";
  capacite = 0;
  idMachine = 0;
  pu = 0.0f;
}

Produit::Produit(int idContenair, QDate datePress, int quantite, QString ref,
                 QString viscosite, QString couleur, QString test, int capacite, int idMachine, float pu) {
  this->idContenair = idContenair;
  this->datePress = datePress;
  this->quantite = quantite;
  this->ref = ref;
  this->viscosite = viscosite;
  this->couleur = couleur;
  this->test = test;
  this->capacite = capacite;
  this->idMachine = idMachine;
  this->pu = pu;
}

// Getters
int Produit::getIdContenair() const { return idContenair; }
QDate Produit::getDatePress() const { return datePress; }
int Produit::getQuantite() const { return quantite; }
QString Produit::getRef() const { return ref; }
QString Produit::getViscosite() const { return viscosite; }
QString Produit::getCouleur() const { return couleur; }
QString Produit::getTest() const { return test; }
int Produit::getCapacite() const { return capacite; }
int Produit::getIdMachine() const { return idMachine; }
float Produit::getPu() const { return pu; }

// Setters
void Produit::setIdContenair(int id) { this->idContenair = id; }
void Produit::setDatePress(QDate date) { this->datePress = date; }
void Produit::setQuantite(int val) { this->quantite = val; }
void Produit::setRef(QString val) { this->ref = val; }
void Produit::setViscosite(QString val) { this->viscosite = val; }
void Produit::setCouleur(QString val) { this->couleur = val; }
void Produit::setTest(QString val) { this->test = val; }
void Produit::setCapacite(int val) { this->capacite = val; }
void Produit::setIdMachine(int id) { this->idMachine = id; }
void Produit::setPu(float val) { this->pu = val; }

bool Produit::ajouter() {
  QSqlQuery query;
  // Using MAX+1 for the primary key if it's not and identity/sequence
  query.prepare(
      "INSERT INTO PRODUIT (ID_CONTENAIR, DATE_PRESS, QUANTITE, "
      "REF, VISCOSITE, COULEUR, TEST, CAPACITE, ID_MACHINE, PU) "
      "VALUES ((SELECT NVL(MAX(ID_CONTENAIR),0)+1 FROM PRODUIT), :date, :qnt, :ref, :visc, :col, :tst, :cap, :idM, :pu)");
  query.bindValue(":date", datePress);
  query.bindValue(":qnt", quantite);
  query.bindValue(":ref", ref);
  query.bindValue(":visc", viscosite);
  query.bindValue(":col", couleur);
  query.bindValue(":tst", test);
  query.bindValue(":cap", capacite);
  query.bindValue(":idM", idMachine);
  query.bindValue(":pu", pu);

  if (!query.exec()) {
    lastError = query.lastError().text();
    qDebug() << "Produit Ajouter Error:" << lastError;
    return false;
  }
  return true;
}

bool Produit::supprimer(int id) {
  QSqlQuery query;
  query.prepare("DELETE FROM PRODUIT WHERE ID_CONTENAIR = :id");
  query.bindValue(":id", id);
  if (!query.exec()) {
    lastError = query.lastError().text();
    return false;
  }
  return true;
}

bool Produit::modifier() {
  QSqlQuery query;
  query.prepare(
      "UPDATE PRODUIT SET DATE_PRESS = :date, "
      "QUANTITE = :qnt, REF = :ref, VISCOSITE = :visc, "
      "COULEUR = :col, TEST = :tst, CAPACITE = :cap, ID_MACHINE = :idM, PU = :pu "
      "WHERE ID_CONTENAIR = :id");
  query.bindValue(":id", idContenair);
  query.bindValue(":date", datePress);
  query.bindValue(":qnt", quantite);
  query.bindValue(":ref", ref);
  query.bindValue(":visc", viscosite);
  query.bindValue(":col", couleur);
  query.bindValue(":tst", test);
  query.bindValue(":cap", capacite);
  query.bindValue(":idM", idMachine);
  query.bindValue(":pu", pu);

  if (!query.exec()) {
    lastError = query.lastError().text();
    return false;
  }
  return true;
}

QSqlQueryModel *Produit::afficher() {
  QSqlQueryModel *model = new QSqlQueryModel();
  model->setQuery(
      "SELECT ID_CONTENAIR, DATE_PRESS, QUANTITE, REF, "
      "VISCOSITE, COULEUR, TEST, CAPACITE, ID_MACHINE, PU FROM PRODUIT ORDER BY DATE_PRESS DESC");

  model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID Contenair"));
  model->setHeaderData(1, Qt::Horizontal, QObject::tr("Date Pressage"));
  model->setHeaderData(2, Qt::Horizontal, QObject::tr("Quantité"));
  model->setHeaderData(3, Qt::Horizontal, QObject::tr("Réf"));
  model->setHeaderData(4, Qt::Horizontal, QObject::tr("Viscosité"));
  model->setHeaderData(5, Qt::Horizontal, QObject::tr("Couleur"));
  model->setHeaderData(6, Qt::Horizontal, QObject::tr("Test"));
  model->setHeaderData(7, Qt::Horizontal, QObject::tr("Capacité"));
  model->setHeaderData(8, Qt::Horizontal, QObject::tr("ID Machine"));
  model->setHeaderData(9, Qt::Horizontal, QObject::tr("PU"));

  return model;
}

QString Produit::getLastError() const { return lastError; }
