#include "connection.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

Connection::Connection()
{

}

bool Connection::createconnect()
{
    bool test = false;
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    db.setDatabaseName(
        "Driver={Oracle in XE};"
        "Dbq=localhost:1521/XE;"
        "Uid=OILPRESSTEAM;"
        "Pwd=123456;"
        );

    if (db.open()) {
        test = true;
        // --- Auto-migration: ensure NUMERO_TELEPHONE column exists ---
        QSqlQuery checkTel;
        checkTel.exec("SELECT COUNT(*) FROM USER_TAB_COLUMNS WHERE TABLE_NAME='COMMANDE' AND COLUMN_NAME='NUMERO_TELEPHONE'");
        if (checkTel.next() && checkTel.value(0).toInt() == 0) {
            QSqlQuery addTel;
            addTel.exec("ALTER TABLE COMMANDE ADD (NUMERO_TELEPHONE VARCHAR2(20))");
            qDebug() << "Migration: added NUMERO_TELEPHONE column";
        }

        // --- Auto-migration: rename TRACKING_NUMBER -> DELIVERY_STATUS if needed ---
        QSqlQuery checkOld;
        checkOld.exec("SELECT COUNT(*) FROM USER_TAB_COLUMNS WHERE TABLE_NAME='COMMANDE' AND COLUMN_NAME='TRACKING_NUMBER'");
        if (checkOld.next() && checkOld.value(0).toInt() > 0) {
            QSqlQuery renameCol;
            renameCol.exec("ALTER TABLE COMMANDE RENAME COLUMN TRACKING_NUMBER TO DELIVERY_STATUS");
            qDebug() << "Migration: renamed TRACKING_NUMBER to DELIVERY_STATUS";
        }
        // --- Add DELIVERY_STATUS if it still doesn't exist ---
        QSqlQuery checkNew;
        checkNew.exec("SELECT COUNT(*) FROM USER_TAB_COLUMNS WHERE TABLE_NAME='COMMANDE' AND COLUMN_NAME='DELIVERY_STATUS'");
        if (checkNew.next() && checkNew.value(0).toInt() == 0) {
            QSqlQuery addCol;
            addCol.exec("ALTER TABLE COMMANDE ADD (DELIVERY_STATUS VARCHAR2(50) DEFAULT 'Preparing')");
            qDebug() << "Migration: added DELIVERY_STATUS column";
        }

        // --- Auto-migration: ensure PRIX_UNITAIRE exists in PRODUIT ---
        QSqlQuery checkPrix;
        checkPrix.exec("SELECT COUNT(*) FROM USER_TAB_COLUMNS WHERE TABLE_NAME='PRODUIT' AND COLUMN_NAME='PRIX_UNITAIRE'");
        if (checkPrix.next() && checkPrix.value(0).toInt() == 0) {
            QSqlQuery addPrix;
            addPrix.exec("ALTER TABLE PRODUIT ADD (PRIX_UNITAIRE NUMBER(10,2) DEFAULT 0)");
            qDebug() << "Migration: added PRIX_UNITAIRE column to PRODUIT";
        }

        // --- Auto-migration: ensure USAGE_COUNT exists in ARTICLE ---
        QSqlQuery checkUsage;
        checkUsage.exec("SELECT COUNT(*) FROM USER_TAB_COLUMNS WHERE TABLE_NAME='ARTICLE' AND COLUMN_NAME='USAGE_COUNT'");
        if (checkUsage.next() && checkUsage.value(0).toInt() == 0) {
            QSqlQuery addUsage;
            addUsage.exec("ALTER TABLE ARTICLE ADD (USAGE_COUNT NUMBER DEFAULT 0)");
            qDebug() << "Migration: added USAGE_COUNT column to ARTICLE";
        }
        // --- Auto-migration: ensure CODE exists in MACHINE ---
        QSqlQuery checkCode;
        checkCode.exec("SELECT COUNT(*) FROM USER_TAB_COLUMNS WHERE TABLE_NAME='MACHINE' AND COLUMN_NAME='CODE'");
        if (checkCode.next() && checkCode.value(0).toInt() == 0) {
            QSqlQuery addCode;
            addCode.exec("ALTER TABLE MACHINE ADD (CODE VARCHAR2(4))");
            QSqlQuery updateCode;
            updateCode.exec("UPDATE MACHINE SET CODE = TO_CHAR(MOD(ABS(DBMS_RANDOM.RANDOM), 9000) + 1000) WHERE CODE IS NULL");
            qDebug() << "Migration: added CODE column to MACHINE";
        }
    } else {
        lastError = db.lastError().text();
    }

    return test;
}

