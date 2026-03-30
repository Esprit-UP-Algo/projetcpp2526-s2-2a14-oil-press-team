#include <QApplication>
#include <QMessageBox>
#include "mainwindow.h"
#include "AuthWidgets.h"
#include "connection.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Connection c;
    bool test = c.createconnect();

    if(!test)
    {
        QMessageBox::critical(nullptr, QObject::tr("database is not open"),
                              QObject::tr("connection failed.\nError: ") + c.getLastError() + QObject::tr("\nClick Cancel to exit."),
                              QMessageBox::Cancel);
        return 0;
    }

    QMessageBox::information(nullptr, QObject::tr("database is open"),
                             QObject::tr("connection successful.\nClick Cancel to continue."),
                             QMessageBox::Cancel);

    AuthWindow authWin;
    MainWindow mainWin;

    QObject::connect(&authWin, &AuthWindow::loginSuccessful, [&](int roleIndex){
        authWin.hide();
        mainWin.applyRole(roleIndex);
        mainWin.show();
    });

    QObject::connect(&mainWin, &MainWindow::logoutRequested, [&](){
        mainWin.hide();
        authWin.show();
    });

    authWin.show();

    return a.exec();
}


