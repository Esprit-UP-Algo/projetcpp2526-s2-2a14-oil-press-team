QT       += core gui printsupport widgets network serialport
QT+= sql
CONFIG += c++17
CONFIG += console

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    article.cpp \
    connection.cpp \
    main.cpp \
    mainwindow.cpp \
    transaction.cpp \
    EyeSaverButton.cpp \
    AuthWidgets.cpp \
    produit.cpp \
    commande.cpp \
    personnel.cpp \
    machine.cpp \
    emailapi.cpp \
    smsapi.cpp \
    trackingapi.cpp \
    LoginSystem.cpp \
    smtp.cpp \
    marketapi.cpp \
    consultantagent.cpp \
    dealgenerator.cpp \
    workforceplanner.cpp \
    anomalyapi.cpp \
    ocrscannerapi.cpp \
    intervention.cpp

HEADERS += \
    article.h \
    connection.h \
    mainwindow.h \
    transaction.h \
    EyeSaverButton.h \
    AuthWidgets.h \
    produit.h \
    commande.h \
    personnel.h \
    machine.h \
    emailapi.h \
    smsapi.h \
    trackingapi.h \
    LoginSystem.h \
    smtp.h \
    marketapi.h \
    consultantagent.h \
    dealgenerator.h \
    workforceplanner.h \
    anomalyapi.h \
    ocrscannerapi.h \
    GasAlertWidget.h \
    intervention.h

FORMS += \
    mainwindow.ui

DISTFILES += \
    config.json \
    config.json.example
 
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

SUBDIRS += \
    Atelier_Connexion.pro \
    Atelier_Connexion.pro
