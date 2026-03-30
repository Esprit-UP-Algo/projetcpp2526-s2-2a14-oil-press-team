#include "commande.h"
#include <QDebug>
#include <QSqlError>

Commande::Commande()
{
    id_commande = 0;
    date_commande = QDate::currentDate();
    etat_commande = "";
    nom_client = "";
    date_livraison = QDate::currentDate().addDays(7);
}

Commande::Commande(int id, QDate dateC, QString etat, QString nomC, QDate dateL)
{
    this->id_commande = id;
    this->date_commande = dateC;
    this->etat_commande = etat;
    this->nom_client = nomC;
    this->date_livraison = dateL;
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
    query.prepare("INSERT INTO COMMANDE (ID_COMMANDE, DATE_COMMANDE, ETAT_COMMANDE, NOM_CLIENT, DATE_LIVRAISON) "
                  "VALUES (:id, :dateC, :etat, :nom, :dateL)");
    query.bindValue(":id", nextId);
    query.bindValue(":dateC", date_commande);
    query.bindValue(":etat", etat_commande);
    query.bindValue(":nom", nom_client);
    query.bindValue(":dateL", date_livraison);

    if (!query.exec()) {
        lastError = query.lastError().text();
        return false;
    }
    return true;
}

QSqlQueryModel* Commande::afficher()
{
    QSqlQueryModel* model = new QSqlQueryModel();
    model->setQuery("SELECT ID_COMMANDE, DATE_COMMANDE, ETAT_COMMANDE, NOM_CLIENT, DATE_LIVRAISON FROM COMMANDE");
    // You can set header data if needed, but since we map to QTableWidget manually, it's optional.
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Date"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("State"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Client"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Delivery"));
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
    query.prepare("UPDATE COMMANDE SET DATE_COMMANDE = :dateC, ETAT_COMMANDE = :etat, NOM_CLIENT = :nom, DATE_LIVRAISON = :dateL "
                  "WHERE ID_COMMANDE = :id");
    query.bindValue(":id", id_commande);
    query.bindValue(":dateC", date_commande);
    query.bindValue(":etat", etat_commande);
    query.bindValue(":nom", nom_client);
    query.bindValue(":dateL", date_livraison);

    if (!query.exec()) {
        lastError = query.lastError().text();
        return false;
    }
    return true;
}
