#include "connection.h"
#include <QSqlError>
#include <QDebug>

Connection::Connection()
{

}

bool Connection::createconnect()
{bool test=false;
QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    db.setDatabaseName(
        "Driver={Oracle in XE};"
        "Dbq=localhost:1521/XE;"
        "Uid=OILPRESSTEAM;"
        "Pwd=123456;"
        );

if (db.open()){
test=true;
}




    return  test;
}
