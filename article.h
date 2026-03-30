#ifndef ARTICLE_H
#define ARTICLE_H

#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QString>

class Article {
public:
  // Constructors
  Article();
  Article(int id, QString nom, int quantite, int seuilMinimal, QString unite = "");

  // Getters
  int getId() const;
  QString getNom() const;
  int getQuantite() const;
  int getSeuilMinimal() const;
  QString getUnite() const;

  // Setters
  void setId(int id);
  void setNom(const QString &nom);
  void setQuantite(int quantite);
  void setSeuilMinimal(int seuil);
  void setUnite(const QString &unite);

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
  int seuilMinimal;
  QString unite;
  QString lastError;
};

#endif // ARTICLE_H
