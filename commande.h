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
    QString reference;
    QDate date_commande;
    QString etat_commande;
    QString nom_client;
    QString adresse_client;
    QDate date_livraison;
    QString numeroTelephone;
    QString deliveryStatus; // Physical delivery stage: Preparing/Dispatched/In Transit/Delivered

public:
    // Constructors
    Commande();
    Commande(int, QString, QDate, QString, QString, QString, QDate, QString, QString);

    // Getters
    int getID() const { return id_commande; }
    QString getReference() const { return reference; }
    QDate getDateCommande() const { return date_commande; }
    QString getEtat() const { return etat_commande; }
    QString getNomClient() const { return nom_client; }
    QString getAdresseClient() const { return adresse_client; }
    QDate getDateLivraison() const { return date_livraison; }
    QString getNumeroTelephone() const { return numeroTelephone; }
    QString getDeliveryStatus() const { return deliveryStatus; }

    // Setters
    void setID(int id) { id_commande = id; }
    void setReference(QString ref) { reference = ref; }
    void setDateCommande(QDate date) { date_commande = date; }
    void setEtat(QString etat) { etat_commande = etat; }
    void setNomClient(QString nom) { nom_client = nom; }
    void setAdresseClient(QString adr) { adresse_client = adr; }
    void setDateLivraison(QDate date) { date_livraison = date; }
    void setNumeroTelephone(QString t) { numeroTelephone = t; }
    void setDeliveryStatus(QString s) { deliveryStatus = s; }

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
