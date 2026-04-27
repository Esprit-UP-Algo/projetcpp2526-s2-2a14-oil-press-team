#ifndef ARTICLE_H
#define ARTICLE_H

#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QString>
#include <QDate>

class Article {
public:
  // Constructors
  Article();
  Article(int id, QString nom, int quantite, QString unite, int prixUnitaire, QDate dateAchat, int usageCount = 0);

  // Getters
  int getId() const;
  QString getNom() const;
  int getQuantite() const;
  QString getUnite() const;
  int getPrixUnitaire() const;
  QDate getDateAchat() const;
  int getUsageCount() const;

  // Setters
  void setId(int id);
  void setNom(const QString &nom);
  void setQuantite(int quantite);
  void setUnite(const QString &unite);
  void setPrixUnitaire(int prix);
  void setDateAchat(const QDate &date);
  void setUsageCount(int usageCount);

  // CRUD
  bool ajouter();
  bool supprimer(int id);
  bool modifier();
  QSqlQueryModel *afficher(QString sortBy = "", QString order = "ASC");
  bool exists(QString name, int excludeId = -1);
  QString getLastError() const;

private:
  int id;
  QString nom;
  int quantite;
  QString unite;
  int prixUnitaire;
  QDate dateAchat;
  int usageCount;
  QString lastError;
};

#endif // ARTICLE_H
