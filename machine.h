#ifndef MACHINE_H
#define MACHINE_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QDate>

class Machine
{
public:
    // Constructors
    Machine();
    Machine(int id, QString nom, QString type, QString etat, int heures, int seuil, QString loc, QDate dateM = QDate());

    // Getters
    int getId() const;
    QString getNom() const;
    QString getType() const;
    QString getEtat() const;
    int getHeures() const;
    int getSeuil() const;
    QString getLocalisation() const;
    QDate getDateM() const;

    // Setters
    void setId(int id);
    void setNom(const QString &nom);
    void setType(const QString &type);
    void setEtat(const QString &etat);
    void setHeures(int heures);
    void setSeuil(int seuil);
    void setLocalisation(const QString &loc);
    void setDateM(QDate dateM);

    // CRUD Operations
    bool ajouter();
    bool supprimer(int id);
    bool modifier();
    QSqlQueryModel* afficher();
    QString getLastError() const;

private:
    int id;
    QString nom;
    QString type;
    QString etat;
    QString localisation;
    int heures;
    int seuilMaintenance;
    QDate dateDerniereMaintenance;
    QString lastError;
};

#endif // MACHINE_H
