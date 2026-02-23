#include "article.h"
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>

// Default constructor
Article::Article() {
  id = 0;
  nom = "";
  quantite = 0;
  seuilMinimal = 0;
}

// Parameterized constructor
Article::Article(int id, QString nom, int quantite, int seuilMinimal) {
  this->id = id;
  this->nom = nom;
  this->quantite = quantite;
  this->seuilMinimal = seuilMinimal;
}

// --- Getters ---
int Article::getId() const { return id; }
QString Article::getNom() const { return nom; }
int Article::getQuantite() const { return quantite; }
int Article::getSeuilMinimal() const { return seuilMinimal; }

// --- Setters ---
void Article::setId(int id) { this->id = id; }
void Article::setNom(const QString &nom) { this->nom = nom; }
void Article::setQuantite(int quantite) { this->quantite = quantite; }
void Article::setSeuilMinimal(int seuil) { this->seuilMinimal = seuil; }

// --- CRUD: Ajouter (Create) ---
bool Article::ajouter() {
  QSqlQuery query;
  query.prepare(
      "INSERT INTO ARTICLE (ID_ARTICLE, NOM_ARTICLE, QUANTITE, SEUIL_MINIMAL) "
      "VALUES ((SELECT NVL(MAX(ID_ARTICLE),0)+1 FROM ARTICLE), :nom, "
      ":quantite, :seuil)");
  query.bindValue(":nom", nom);
  query.bindValue(":quantite", quantite);
  query.bindValue(":seuil", seuilMinimal);

  if (!query.exec()) {
    qDebug() << "Ajouter Error:" << query.lastError().text();
    lastError = query.lastError().text();
    return false;
  }
  return true;
}

// --- CRUD: Supprimer (Delete) ---
bool Article::supprimer(int id) {
  QSqlQuery query;
  query.prepare("DELETE FROM ARTICLE WHERE ID_ARTICLE = :id");
  query.bindValue(":id", id);

  if (!query.exec()) {
    qDebug() << "Supprimer Error:" << query.lastError().text();
    return false;
  }
  return true;
}

// --- CRUD: Modifier (Update) ---
bool Article::modifier() {
  QSqlQuery query;
  query.prepare(
      "UPDATE ARTICLE SET NOM_ARTICLE = :nom, "
      "QUANTITE = :quantite, SEUIL_MINIMAL = :seuil WHERE ID_ARTICLE = :id");
  query.bindValue(":id", id);
  query.bindValue(":nom", nom);
  query.bindValue(":quantite", quantite);
  query.bindValue(":seuil", seuilMinimal);

  if (!query.exec()) {
    qDebug() << "Modifier Error:" << query.lastError().text();
    return false;
  }
  return true;
}

QSqlQueryModel *Article::afficher() {
  QSqlQueryModel *model = new QSqlQueryModel();
  model->setQuery(
      "SELECT ID_ARTICLE, NOM_ARTICLE, QUANTITE, SEUIL_MINIMAL FROM ARTICLE");

  if (model->lastError().isValid()) {
    qDebug() << "Afficher Error:" << model->lastError().text();
  }

  model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
  model->setHeaderData(1, Qt::Horizontal, QObject::tr("Item Name"));
  model->setHeaderData(2, Qt::Horizontal, QObject::tr("Quantity"));
  model->setHeaderData(3, Qt::Horizontal, QObject::tr("Min Threshold"));

  return model;
}

// --- Get Last Error ---
QString Article::getLastError() const { return lastError; }
