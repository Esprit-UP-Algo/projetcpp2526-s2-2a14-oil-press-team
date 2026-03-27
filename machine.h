#ifndef MACHINE_H
#define MACHINE_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>

class Machine
{
public:
    // Constructors
    Machine();
    Machine(int id, QString nom, QString type, QString etat, int heures, int seuil);

    // Getters
    int getId() const;
    QString getNom() const;
    QString getType() const;
    QString getEtat() const;
    int getHeures() const;
    int getSeuil() const;

    // Setters
    void setId(int id);
    void setNom(const QString &nom);
    void setType(const QString &type);
    void setEtat(const QString &etat);
    void setHeures(int heures);
    void setSeuil(int seuil);

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
    int heures;
    int seuilMaintenance;
    QString lastError;
};

#endif // MACHINE_H
