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
  Produit(int idContenair, QDate datePress, int quantite, QString ref, 
          QString viscosite, QString couleur, QString test, int capacite, int idMachine, float pu);

  // Getters
  int getIdContenair() const;
  QDate getDatePress() const;
  int getQuantite() const;
  QString getRef() const;
  QString getViscosite() const;
  QString getCouleur() const;
  QString getTest() const;
  int getCapacite() const;
  int getIdMachine() const;
  float getPu() const;

  // Setters
  void setIdContenair(int id);
  void setDatePress(QDate date);
  void setQuantite(int val);
  void setRef(QString val);
  void setViscosite(QString val);
  void setCouleur(QString val);
  void setTest(QString val);
  void setCapacite(int val);
  void setIdMachine(int id);
  void setPu(float val);

  // CRUD
  bool ajouter();
  bool supprimer(int id);
  bool modifier();
  QSqlQueryModel *afficher();
  QString getLastError() const;

private:
  int idContenair;
  QDate datePress;
  int quantite;
  QString ref;
  QString viscosite;
  QString couleur;
  QString test;
  int capacite;
  int idMachine;
  float pu;
  QString lastError;
};

#endif // PRODUIT_H
