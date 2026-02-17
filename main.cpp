#include <QApplication>
#include "mainwindow.h"
#include "AuthWidgets.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    AuthWindow authWin;
    MainWindow mainWin;
    
    // Transitions
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
