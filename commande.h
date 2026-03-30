#ifndef COMMANDE_H
#define COMMANDE_H

#include <QString>
#include <QDate>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QVariant>

class Commande
{
private:
    int id_commande;
    QDate date_commande;
    QString etat_commande;
    QString nom_client;
    QDate date_livraison;

public:
    // Constructors
    Commande();
    Commande(int, QDate, QString, QString, QDate);

    // Getters
    int getID() const { return id_commande; }
    QDate getDateCommande() const { return date_commande; }
    QString getEtat() const { return etat_commande; }
    QString getNomClient() const { return nom_client; }
    QDate getDateLivraison() const { return date_livraison; }

    // Setters
    void setID(int id) { id_commande = id; }
    void setDateCommande(QDate date) { date_commande = date; }
    void setEtat(QString etat) { etat_commande = etat; }
    void setNomClient(QString nom) { nom_client = nom; }
    void setDateLivraison(QDate date) { date_livraison = date; }

    // CRUD Methods
    bool ajouter();
    QSqlQueryModel* afficher();
    bool supprimer(int id);
    bool modifier();
    QString getLastError() const { return lastError; }

private:
    QString lastError;
};

#endif // COMMANDE_H
