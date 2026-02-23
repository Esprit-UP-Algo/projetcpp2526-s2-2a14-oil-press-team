#include "connection.h"

Connection::Connection()
{

}

bool Connection::createconnect()
{bool test=false;
QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
db.setDatabaseName("oilpressmanager");//inserer le nom de la source de données
db.setUserName("oilpressteam");//inserer nom de l'utilisateur
db.setPassword("123456");//inserer mot de passe de cet utilisateur

if (db.open()){
test=true;
}





    return  test;
}
