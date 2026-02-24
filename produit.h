#ifndef PRODUIT_H
#define PRODUIT_H

#include <QDate>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QString>

class Produit {
public:
  // Constructors
  Produit();
  Produit(int idProduit, QString idContenair, QDate datePress, double quantite,
          QString ref, double viscosite, QString couleur, QString test, int idMachine);

  // Getters
  int getIdProduit() const;
  QString getIdContenair() const;
  QDate getDatePress() const;
  double getQuantite() const;
  QString getRef() const;
  double getViscosite() const;
  QString getCouleur() const;
  QString getTest() const;
  int getIdMachine() const;

  // Setters
  void setIdProduit(int id);
  void setIdContenair(QString id);
  void setDatePress(QDate date);
  void setQuantite(double val);
  void setRef(QString val);
  void setViscosite(double val);
  void setCouleur(QString val);
  void setTest(QString val);
  void setIdMachine(int id);

  // CRUD
  bool ajouter();
  bool supprimer(int id);
  bool modifier();
  QSqlQueryModel *afficher();
  QString getLastError() const;

private:
  int idProduit;
  QString idContenair;
  QDate datePress;
  double quantite;
  QString ref;
  double viscosite;
  QString couleur;
  QString test;
  int idMachine;
  QString lastError;
};

#endif // PRODUIT_H
