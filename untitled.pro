<<<<<<< HEAD
QT       += core gui printsupport widgets
<<<<<<< HEAD

CONFIG += c++17
=======
=======
QT       += core gui printsupport widgets network
>>>>>>> dc185cb74cfe3fdfe3b5f96ea6f317761d8ccd9a
QT+= sql
CONFIG += c++17
CONFIG += console
>>>>>>> main

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
<<<<<<< HEAD
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h
=======
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
    smtp.cpp

HEADERS += \
    article.h \
    connection.h \
    mainwindow.h \
    transaction.h \
    EyeSaverButton.h \
    AuthWidgets.h \
    produit.h \
<<<<<<< HEAD
    commande.h
>>>>>>> main
=======
    commande.h \
    personnel.h \
    machine.h \
    smtp.h
>>>>>>> dc185cb74cfe3fdfe3b5f96ea6f317761d8ccd9a

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
<<<<<<< HEAD
=======

SUBDIRS += \
    Atelier_Connexion.pro \
    Atelier_Connexion.pro
>>>>>>> main
