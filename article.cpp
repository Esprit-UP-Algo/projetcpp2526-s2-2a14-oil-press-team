#include "article.h"
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>

// Default constructor
Article::Article() {
  id = 0;
  nom = "";
  quantite = 0;
  unite = "";
  prixUnitaire = 0;
  dateAchat = QDate::currentDate();
}

// Parameterized constructor
Article::Article(int id, QString nom, int quantite, QString unite, int prixUnitaire, QDate dateAchat) {
  this->id = id;
  this->nom = nom;
  this->quantite = quantite;
  this->unite = unite;
  this->prixUnitaire = prixUnitaire;
  this->dateAchat = dateAchat;
}

// --- Getters ---
int Article::getId() const { return id; }
QString Article::getNom() const { return nom; }
int Article::getQuantite() const { return quantite; }
QString Article::getUnite() const { return unite; }
int Article::getPrixUnitaire() const { return prixUnitaire; }
QDate Article::getDateAchat() const { return dateAchat; }

// --- Setters ---
void Article::setId(int id) { this->id = id; }
void Article::setNom(const QString &nom) { this->nom = nom; }
void Article::setQuantite(int quantite) { this->quantite = quantite; }
void Article::setUnite(const QString &unite) { this->unite = unite; }
void Article::setPrixUnitaire(int prix) { this->prixUnitaire = prix; }
void Article::setDateAchat(const QDate &date) { this->dateAchat = date; }

// --- CRUD: Ajouter (Create) ---
bool Article::ajouter() {
  QSqlQuery query;
  query.prepare(
      "INSERT INTO ARTICLE (ID_ARTICLE, NOM_ARTICLE, QUANTITE, UNITE, PRIX_UNITAIRE, DATE_ACHAT) "
      "VALUES ((SELECT NVL(MAX(ID_ARTICLE),0)+1 FROM ARTICLE), :nom, "
      ":quantite, :unite, :prix, :date_achat)");
  query.bindValue(":nom", nom);
  query.bindValue(":quantite", quantite);
  query.bindValue(":unite", unite);
  query.bindValue(":prix", prixUnitaire);
  query.bindValue(":date_achat", dateAchat);

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
      "QUANTITE = :quantite, UNITE = :unite, PRIX_UNITAIRE = :prix, DATE_ACHAT = :date_achat WHERE ID_ARTICLE = :id");
  query.bindValue(":id", id);
  query.bindValue(":nom", nom);
  query.bindValue(":quantite", quantite);
  query.bindValue(":unite", unite);
  query.bindValue(":prix", prixUnitaire);
  query.bindValue(":date_achat", dateAchat);

  if (!query.exec()) {
    qDebug() << "Modifier Error:" << query.lastError().text();
    return false;
  }
  return true;
}

QSqlQueryModel *Article::afficher(QString sortBy, QString order) {
  QSqlQueryModel *model = new QSqlQueryModel();
  
  QString queryStr = "SELECT ID_ARTICLE, NOM_ARTICLE, QUANTITE, UNITE, PRIX_UNITAIRE, DATE_ACHAT FROM ARTICLE";
  
  if (!sortBy.isEmpty()) {
      queryStr += " ORDER BY " + sortBy + " " + order;
  }
  
  model->setQuery(queryStr);

  if (model->lastError().isValid()) {
    qDebug() << "Afficher Error:" << model->lastError().text();
  }

  model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
  model->setHeaderData(1, Qt::Horizontal, QObject::tr("Item Name"));
  model->setHeaderData(2, Qt::Horizontal, QObject::tr("Quantity"));
  model->setHeaderData(3, Qt::Horizontal, QObject::tr("Unit"));
  model->setHeaderData(4, Qt::Horizontal, QObject::tr("Unit Price"));
  model->setHeaderData(5, Qt::Horizontal, QObject::tr("Date of Purchase"));

  return model;
}

bool Article::exists(QString name, int excludeId) {
  QSqlQuery query;
  if (excludeId == -1) {
    query.prepare("SELECT COUNT(*) FROM ARTICLE WHERE NOM_ARTICLE = :nom");
  } else {
    query.prepare("SELECT COUNT(*) FROM ARTICLE WHERE NOM_ARTICLE = :nom AND ID_ARTICLE != :id");
    query.bindValue(":id", excludeId);
  }
  query.bindValue(":nom", name);

  if (query.exec() && query.next()) {
    return query.value(0).toInt() > 0;
  }
  return false;
}

// --- Get Last Error ---
QString Article::getLastError() const { return lastError; }
