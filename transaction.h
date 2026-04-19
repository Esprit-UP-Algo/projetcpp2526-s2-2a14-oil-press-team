#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QString>
#include <QDate>

class Transaction {
public:
    // Constructors
    Transaction();
    Transaction(int idTransaction, double montant, const QDate &dateTransaction,
                const QString &typeTransaction, const QString &modePaiement,
                const QString &description, int idCommande);

    // Getters
    int getIdTransaction() const;
    double getMontant() const;
    QDate getDateTransaction() const;
    QString getTypeTransaction() const;
    QString getModePaiement() const;
    QString getDescription() const;
    int getIdCommande() const;

    // Setters
    void setIdTransaction(int id);
    void setMontant(double montant);
    void setDateTransaction(const QDate &date);
    void setTypeTransaction(const QString &type);
    void setModePaiement(const QString &mode);
    void setDescription(const QString &desc);
    void setIdCommande(int idCommande);

    // CRUD
    bool ajouter();
    bool supprimer(int id);
    bool modifier();
    QSqlQueryModel* afficher();
    QString getLastError() const;

    // Anomaly Detection Methods
    static QString checkDateAnomaly(const QDate &date);
    static QString checkDuplicateAnomaly(int id, double amount, const QDate &date, const QString &desc);
    static QString checkAmountMismatch(int orderId, double recordedAmount);

private:
    int idTransaction;
    double montant;
    QDate dateTransaction;
    QString typeTransaction;
    QString modePaiement;
    QString description;
    int idCommande;
    QString lastError;
};

#endif // TRANSACTION_H
