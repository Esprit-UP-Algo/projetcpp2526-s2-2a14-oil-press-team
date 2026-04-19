#include "transaction.h"
#include <QDebug>
#include <QSqlError>

// Default constructor
Transaction::Transaction() {
    idTransaction = 0;
    montant = 0.0;
    dateTransaction = QDate::currentDate();
    typeTransaction = "";
    modePaiement = "";
    description = "";
    idCommande = 0;
}

// Parameterized constructor
Transaction::Transaction(int idTransaction, double montant, const QDate &dateTransaction,
                         const QString &typeTransaction, const QString &modePaiement,
                         const QString &description, int idCommande) {
    this->idTransaction = idTransaction;
    this->montant = montant;
    this->dateTransaction = dateTransaction;
    this->typeTransaction = typeTransaction;
    this->modePaiement = modePaiement;
    this->description = description;
    this->idCommande = idCommande;
}

// --- Getters ---
int Transaction::getIdTransaction() const { return idTransaction; }
double Transaction::getMontant() const { return montant; }
QDate Transaction::getDateTransaction() const { return dateTransaction; }
QString Transaction::getTypeTransaction() const { return typeTransaction; }
QString Transaction::getModePaiement() const { return modePaiement; }
QString Transaction::getDescription() const { return description; }
int Transaction::getIdCommande() const { return idCommande; }

// --- Setters ---
void Transaction::setIdTransaction(int id) { this->idTransaction = id; }
void Transaction::setMontant(double montant) { this->montant = montant; }
void Transaction::setDateTransaction(const QDate &date) { this->dateTransaction = date; }
void Transaction::setTypeTransaction(const QString &type) { this->typeTransaction = type; }
void Transaction::setModePaiement(const QString &mode) { this->modePaiement = mode; }
void Transaction::setDescription(const QString &desc) { this->description = desc; }
void Transaction::setIdCommande(int idCommande) { this->idCommande = idCommande; }

// --- CRUD: Ajouter (Create) ---
bool Transaction::ajouter() {
    // Step 1: Get next ID in a separate query (ODBC S1010 forbids subqueries in VALUES)
    QSqlQuery idQuery;
    idQuery.prepare("SELECT NVL(MAX(ID_TRANSACTION), 0) + 1 FROM FINANCE");
    if (!idQuery.exec() || !idQuery.next()) {
        qDebug() << "Transaction ID fetch Error:" << idQuery.lastError().text();
        lastError = idQuery.lastError().text();
        return false;
    }
    int nextId = idQuery.value(0).toInt();

    // Step 2: Insert with the calculated ID
    QSqlQuery query;
    query.prepare(
        "INSERT INTO FINANCE (ID_TRANSACTION, MONTANT, DATE_TRANSACTION, "
        "TYPE_TRANSACTION, MODE_PAIEMENT, DESCRIPTION, ID_COMMANDE) "
        "VALUES (:id, :montant, :dateTransaction, :typeTransaction, "
        ":modePaiement, :description, :idCommande)");
    query.bindValue(":id", nextId);
    query.bindValue(":montant", montant);
    query.bindValue(":dateTransaction", dateTransaction);
    query.bindValue(":typeTransaction", typeTransaction);
    query.bindValue(":modePaiement", modePaiement);
    query.bindValue(":description", description);
    query.bindValue(":idCommande", idCommande);

    if (!query.exec()) {
        qDebug() << "Transaction Ajouter Error:" << query.lastError().text();
        lastError = query.lastError().text();
        return false;
    }
    return true;
}

// --- CRUD: Supprimer (Delete) ---
bool Transaction::supprimer(int id) {
    QSqlQuery query;
    query.prepare("DELETE FROM FINANCE WHERE ID_TRANSACTION = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Transaction Supprimer Error:" << query.lastError().text();
        lastError = query.lastError().text();
        return false;
    }
    return true;
}

// --- CRUD: Modifier (Update) ---
bool Transaction::modifier() {
    QSqlQuery query;
    query.prepare(
        "UPDATE FINANCE SET MONTANT = :montant, "
        "DATE_TRANSACTION = :dateTransaction, "
        "TYPE_TRANSACTION = :typeTransaction, "
        "MODE_PAIEMENT = :modePaiement, "
        "DESCRIPTION = :description, "
        "ID_COMMANDE = :idCommande "
        "WHERE ID_TRANSACTION = :id");
    query.bindValue(":id", idTransaction);
    query.bindValue(":montant", montant);
    query.bindValue(":dateTransaction", dateTransaction);
    query.bindValue(":typeTransaction", typeTransaction);
    query.bindValue(":modePaiement", modePaiement);
    query.bindValue(":description", description);
    query.bindValue(":idCommande", idCommande);

    if (!query.exec()) {
        qDebug() << "Transaction Modifier Error:" << query.lastError().text();
        lastError = query.lastError().text();
        return false;
    }
    return true;
}

// --- CRUD: Afficher (Read) ---
QSqlQueryModel* Transaction::afficher() {
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery(
        "SELECT ID_TRANSACTION, MONTANT, DATE_TRANSACTION, TYPE_TRANSACTION, "
        "MODE_PAIEMENT, DESCRIPTION, ID_COMMANDE FROM FINANCE "
        "ORDER BY DATE_TRANSACTION DESC");

    if (model->lastError().isValid()) {
        qDebug() << "Transaction Afficher Error:" << model->lastError().text();
    }

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Amount"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Date"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Type"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Payment Mode"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Description"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Order ID"));

    return model;
}

// --- Get Last Error ---
QString Transaction::getLastError() const { return lastError; }

// --- Anomaly Detection Methods ---

QString Transaction::checkDateAnomaly(const QDate &date) {
    if (date > QDate::currentDate()) {
        return "• Transaction date is in the future.\n";
    }
    return "";
}

QString Transaction::checkDuplicateAnomaly(int id, double amount, const QDate &date, const QString &desc) {
    QSqlQuery q;
    // We use TRUNC to compare only the date part in Oracle.
    // We use CAST and TRIM to handle the CLOB 'DESCRIPTION' field safely for comparison.
    q.prepare("SELECT COUNT(*) FROM FINANCE WHERE MONTANT = :m "
              "AND TRUNC(DATE_TRANSACTION) = :dt "
              "AND (UPPER(TRIM(CAST(DESCRIPTION AS VARCHAR2(4000)))) = UPPER(TRIM(:desc)) "
              "OR (DESCRIPTION IS NULL AND :desc_empty = 1)) "
              "AND ID_TRANSACTION != :id");
    q.bindValue(":m", amount);
    q.bindValue(":dt", date);
    q.bindValue(":desc", desc);
    q.bindValue(":desc_empty", desc.isEmpty() ? 1 : 0);
    q.bindValue(":id", id);
    
    if (!q.exec()) {
        qDebug() << "Duplicate Check Query Error:" << q.lastError().text();
        return "";
    }
    
    if (q.next() && q.value(0).toInt() > 0) {
        return "• Potential Duplicate: Another identical payment exists.\n";
    }
    return "";
}

QString Transaction::checkAmountMismatch(int orderId, double recordedAmount) {
    if (orderId <= 0) return "";
    
    QSqlQuery q;
    // Calculate expected total from Order details (Contenir + Produit)
    q.prepare("SELECT SUM(P.PRIX_UNITAIRE * C.QUANTITE_DEMANDEE) "
              "FROM CONTENIR C JOIN PRODUIT P ON C.ID_CONTENAIR = P.ID_CONTENAIR "
              "WHERE C.ID_COMMANDE = :id");
    q.bindValue(":id", orderId);
    
    if (q.exec() && q.next()) {
        double expected = q.value(0).toDouble();
        if (expected > 0 && qAbs(expected - recordedAmount) > 0.01) {
            return QString("• Amount mismatch: Order total is %1 TND, but record shows %2 TND.\n")
                   .arg(expected, 0, 'f', 2).arg(recordedAmount, 0, 'f', 2);
        }
    }
    return "";
}
