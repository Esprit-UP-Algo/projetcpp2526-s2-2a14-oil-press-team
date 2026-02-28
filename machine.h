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
    Machine(int id, QString nom, QString type, QString etat, int heures);

    // Getters
    int getId() const;
    QString getNom() const;
    QString getType() const;
    QString getEtat() const;
    int getHeures() const;

    // Setters
    void setId(int id);
    void setNom(const QString &nom);
    void setType(const QString &type);
    void setEtat(const QString &etat);
    void setHeures(int heures);

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
    QString lastError;
};

#endif // MACHINE_H
