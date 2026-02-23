#ifndef MACHINE_H
#define MACHINE_H

#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QString>

class Machine {
public:
    // Constructors
    Machine();
    Machine(int id, const QString &nom, const QString &type,
            const QString &statut, int heures);

    // Getters
    int getId() const;
    QString getNom() const;
    QString getType() const;
    QString getStatut() const;
    int getHeures() const;

    // Setters
    void setId(int id);
    void setNom(const QString &nom);
    void setType(const QString &type);
    void setStatut(const QString &statut);
    void setHeures(int heures);

    // CRUD
    bool ajouter();
    bool supprimer(int id);
    bool modifier();
    QSqlQueryModel *afficher();
    QString getLastError() const;

private:
    int id;
    QString nom;
    QString type;
    QString statut;
    int heures;
    QString lastError;
};

#endif // MACHINE_H
