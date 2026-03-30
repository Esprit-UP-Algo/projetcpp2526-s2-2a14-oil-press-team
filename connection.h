#ifndef CONNECTION_H
#define CONNECTION_H
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

class Connection
{
public:
    Connection();
    bool createconnect();
    QString getLastError() const { return lastError; }

private:
    QString lastError;
};

#endif // CONNECTION_H
