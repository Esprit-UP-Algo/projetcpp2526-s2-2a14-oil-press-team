#include "commande.h"
#include <QDebug>
#include <QSqlError>

Commande::Commande()
{
    id_commande = 0;
    reference = "";
    date_commande = QDate::currentDate();
    etat_commande = "";
    nom_client = "";
    adresse_client = "";
    date_livraison = QDate::currentDate().addDays(7);
    numeroTelephone = "";
    deliveryStatus = "Preparing";
}

Commande::Commande(int id, QString ref, QDate dateC, QString etat, QString nomC, QString adresseC, QDate dateL, QString tel, QString status)
{
    this->id_commande = id;
    this->reference = ref;
    this->date_commande = dateC;
    this->etat_commande = etat;
    this->nom_client = nomC;
    this->adresse_client = adresseC;
    this->date_livraison = dateL;
    this->numeroTelephone = tel;
    this->deliveryStatus = status.isEmpty() ? "Preparing" : status;
}

bool Commande::ajouter()
{
    // Step 1: get next ID
    QSqlQuery idQuery;
    idQuery.prepare("SELECT NVL(MAX(ID_COMMANDE), 0) + 1 FROM COMMANDE");
    if (!idQuery.exec() || !idQuery.next()) {
        lastError = idQuery.lastError().text();
        return false;
    }
    int nextId = idQuery.value(0).toInt();

    // Step 2: Insert
    QSqlQuery query;
    query.prepare("INSERT INTO COMMANDE (ID_COMMANDE, REFERENCE, DATE_COMMANDE, ETAT_COMMANDE, NOM_CLIENT, ADRESSE_CLIENT, DATE_LIVRAISON, NUMERO_TELEPHONE, DELIVERY_STATUS) "
                  "VALUES (:id, :ref, :dateC, :etat, :nom, :adr, :dateL, :tel, :status)");
    query.bindValue(":id", nextId);
    query.bindValue(":ref", reference);
    query.bindValue(":dateC", date_commande);
    query.bindValue(":etat", etat_commande);
    query.bindValue(":nom", nom_client);
    query.bindValue(":adr", adresse_client);
    query.bindValue(":dateL", date_livraison);
    query.bindValue(":tel", numeroTelephone);
    query.bindValue(":status", deliveryStatus);

    if (!query.exec()) {
        lastError = query.lastError().text();
        return false;
    }
    return true;
}

QSqlQueryModel* Commande::afficher()
{
    QSqlQueryModel* model = new QSqlQueryModel();
    model->setQuery("SELECT ID_COMMANDE, REFERENCE, DATE_COMMANDE, ETAT_COMMANDE, NOM_CLIENT, ADRESSE_CLIENT, DATE_LIVRAISON, NUMERO_TELEPHONE, DELIVERY_STATUS FROM COMMANDE");
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Reference"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Date"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("State"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Client"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Delivery Date"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Phone"));
    model->setHeaderData(7, Qt::Horizontal, QObject::tr("Delivery Status"));
    return model;
}

bool Commande::supprimer(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM COMMANDE WHERE ID_COMMANDE = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        lastError = query.lastError().text();
        return false;
    }
    return true;
}

bool Commande::modifier()
{
    QSqlQuery query;
    query.prepare("UPDATE COMMANDE SET REFERENCE = :ref, DATE_COMMANDE = :dateC, ETAT_COMMANDE = :etat, "
                  "NOM_CLIENT = :nom, ADRESSE_CLIENT = :adr, DATE_LIVRAISON = :dateL, "
                  "NUMERO_TELEPHONE = :tel, DELIVERY_STATUS = :status "
                  "WHERE ID_COMMANDE = :id");
    query.bindValue(":id", id_commande);
    query.bindValue(":ref", reference);
    query.bindValue(":dateC", date_commande);
    query.bindValue(":etat", etat_commande);
    query.bindValue(":nom", nom_client);
    query.bindValue(":adr", adresse_client);
    query.bindValue(":dateL", date_livraison);
    query.bindValue(":tel", numeroTelephone);
    query.bindValue(":status", deliveryStatus);

    if (!query.exec()) {
        lastError = query.lastError().text();
        return false;
    }
    return true;
}
