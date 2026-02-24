#ifndef PERSONNEL_H
#define PERSONNEL_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlError>

class Personnel {
public:
    // Constructors
    Personnel();
    Personnel(int cin, QString nom, double salaire, QString adresse, QString tel, int exp, QString grade, QString role);

    // Getters
    int getCin() const;
    QString getNom() const;
    double getSalaire() const;
    QString getAdresse() const;
    QString getTel() const;
    int getExperience() const;
    QString getGrade() const;
    QString getRole() const;

    // Setters
    void setCin(int cin);
    void setNom(const QString &nom);
    void setSalaire(double salaire);
    void setAdresse(const QString &adresse);
    void setTel(const QString &tel);
    void setExperience(int exp);
    void setGrade(const QString &grade);
    void setRole(const QString &role);

    // CRUD Operations
    bool ajouter();
    bool supprimer(int cin);
    bool modifier();
    QSqlQueryModel* afficher();

    // Error Handling
    QString getLastError() const;

private:
    int cin;                // Database: ID_PERSONNEL
    QString nom;            // Database: NOM_PERSONNEL
    double salaire;         // Database: SALAIRE_BRUT
    QString adresse;        // Database: ADRESSE
    QString tel;            // Database: TEL
    int experience;         // Database: EXPERIENCE
    QString grade;          // Database: GRADE
    QString role;           // Database: ROLE
    QString lastError;
};

#endif // PERSONNEL_H
