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
  Article(int id, QString nom, int quantite, QString unite, int prixUnitaire, QDate dateAchat);

  // Getters
  int getId() const;
  QString getNom() const;
  int getQuantite() const;
  QString getUnite() const;
  int getPrixUnitaire() const;
  QDate getDateAchat() const;

  // Setters
  void setId(int id);
  void setNom(const QString &nom);
  void setQuantite(int quantite);
  void setUnite(const QString &unite);
  void setPrixUnitaire(int prix);
  void setDateAchat(const QDate &date);

  // CRUD
  bool ajouter();
  bool supprimer(int id);
  bool modifier();
  QSqlQueryModel *afficher();
  QString getLastError() const;

private:
  int id;
  QString nom;
  int quantite;
  QString unite;
  int prixUnitaire;
  QDate dateAchat;
  QString lastError;
};

#endif // ARTICLE_H
