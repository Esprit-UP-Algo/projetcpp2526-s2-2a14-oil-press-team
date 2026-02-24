#include "produit.h"
#include <QDebug>
#include <QSqlError>

Produit::Produit() {
  idProduit = 0;
  idContenair = "";
  datePress = QDate::currentDate();
  quantite = 0.0;
  ref = "";
  viscosite = 0.0;
  couleur = "";
  test = "";
  idMachine = 0;
}

Produit::Produit(int idProduit, QString idContenair, QDate datePress,
                 double quantite, QString ref, double viscosite,
                 QString couleur, QString test, int idMachine) {
  this->idProduit = idProduit;
  this->idContenair = idContenair;
  this->datePress = datePress;
  this->quantite = quantite;
  this->ref = ref;
  this->viscosite = viscosite;
  this->couleur = couleur;
  this->test = test;
  this->idMachine = idMachine;
}

// Getters
int Produit::getIdProduit() const { return idProduit; }
QString Produit::getIdContenair() const { return idContenair; }
QDate Produit::getDatePress() const { return datePress; }
double Produit::getQuantite() const { return quantite; }
QString Produit::getRef() const { return ref; }
double Produit::getViscosite() const { return viscosite; }
QString Produit::getCouleur() const { return couleur; }
QString Produit::getTest() const { return test; }
int Produit::getIdMachine() const { return idMachine; }

// Setters
void Produit::setIdProduit(int id) { this->idProduit = id; }
void Produit::setIdContenair(QString id) { this->idContenair = id; }
void Produit::setDatePress(QDate date) { this->datePress = date; }
void Produit::setQuantite(double val) { this->quantite = val; }
void Produit::setRef(QString val) { this->ref = val; }
void Produit::setViscosite(double val) { this->viscosite = val; }
void Produit::setCouleur(QString val) { this->couleur = val; }
void Produit::setTest(QString val) { this->test = val; }
void Produit::setIdMachine(int id) { this->idMachine = id; }

bool Produit::ajouter() {
  QSqlQuery idQuery;
  idQuery.prepare("SELECT COUNT(*) + 1 FROM PRODUIT");
  if (!idQuery.exec() || !idQuery.next()) {
    lastError = idQuery.lastError().text();
    return false;
  }
  int nextId = idQuery.value(0).toInt();

  QSqlQuery query;
  query.prepare(
      "INSERT INTO PRODUIT (ID_PRODUIT, ID_CONTENAIR, DATE_PRESS, QUANTITE, "
      "REF, VISCOSITE, COULEUR, TEST, ID_MACHINE) "
      "VALUES (:id, :idC, :date, :qnt, :ref, :visc, :col, :tst, :idM)");
  query.bindValue(":id", nextId);
  query.bindValue(":idC", idContenair);
  query.bindValue(":date", datePress);
  query.bindValue(":qnt", quantite);
  query.bindValue(":ref", ref);
  query.bindValue(":visc", viscosite);
  query.bindValue(":col", couleur);
  query.bindValue(":tst", test);
  query.bindValue(":idM", idMachine);

  if (!query.exec()) {
    lastError = query.lastError().text();
    qDebug() << "Produit Ajouter Error:" << lastError;
    return false;
  }
  return true;
}

bool Produit::supprimer(int id) {
  QSqlQuery query;
  query.prepare("DELETE FROM PRODUIT WHERE ID_PRODUIT = :id");
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
      "UPDATE PRODUIT SET ID_CONTENAIR = :idC, DATE_PRESS = :date, "
      "QUANTITE = :qnt, REF = :ref, VISCOSITE = :visc, "
      "COULEUR = :col, TEST = :tst, ID_MACHINE = :idM "
      "WHERE ID_PRODUIT = :id");
  query.bindValue(":id", idProduit);
  query.bindValue(":idC", idContenair);
  query.bindValue(":date", datePress);
  query.bindValue(":qnt", quantite);
  query.bindValue(":ref", ref);
  query.bindValue(":visc", viscosite);
  query.bindValue(":col", couleur);
  query.bindValue(":tst", test);
  query.bindValue(":idM", idMachine);

  if (!query.exec()) {
    lastError = query.lastError().text();
    return false;
  }
  return true;
}

QSqlQueryModel *Produit::afficher() {
  QSqlQueryModel *model = new QSqlQueryModel();
  model->setQuery(
      "SELECT ID_PRODUIT, ID_CONTENAIR, DATE_PRESS, QUANTITE, REF, "
      "VISCOSITE, COULEUR, TEST, ID_MACHINE FROM PRODUIT ORDER BY DATE_PRESS DESC");

  model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
  model->setHeaderData(1, Qt::Horizontal, QObject::tr("Conteneur"));
  model->setHeaderData(2, Qt::Horizontal, QObject::tr("Date Pressage"));
  model->setHeaderData(3, Qt::Horizontal, QObject::tr("Quantité"));
  model->setHeaderData(4, Qt::Horizontal, QObject::tr("Réf"));
  model->setHeaderData(5, Qt::Horizontal, QObject::tr("Viscosité"));
  model->setHeaderData(6, Qt::Horizontal, QObject::tr("Couleur"));
  model->setHeaderData(7, Qt::Horizontal, QObject::tr("Test"));
  model->setHeaderData(8, Qt::Horizontal, QObject::tr("ID Machine"));

  return model;
}

QString Produit::getLastError() const { return lastError; }
