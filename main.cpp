#include <QApplication>
#include <QMessageBox>
#include "mainwindow.h"
#include "AuthWidgets.h"
#include "connection.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyleSheet(
        "QCalendarWidget QWidget { color: #1a1a1a; }"
        "QCalendarWidget QWidget#qt_calendar_navigationbar { background-color: #f0f0f0; }"
        "QCalendarWidget QTableView#qt_calendar_calendarview { background-color: #ffffff; color: #1a1a1a; selection-background-color: #3ddc84; selection-color: white; alternate-background-color: #f9f9f9; }"
        "QCalendarWidget QAbstractItemView:enabled { color: #1a1a1a; background-color: #ffffff; }"
        "QCalendarWidget QAbstractItemView:disabled { color: #999999; }"
        "QCalendarWidget QToolButton { color: #1a1a1a; background-color: transparent; border: none; font-weight: bold; margin: 2px; }"
    );

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


