QT       += core gui printsupport widgets
QT+= sql
CONFIG += c++17
CONFIG += console

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    connection.cpp \
    main.cpp \
    mainwindow.cpp \
    EyeSaverButton.cpp \
    AuthWidgets.cpp

HEADERS += \
    connection.h \
    mainwindow.h \
    EyeSaverButton.h \
    AuthWidgets.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

SUBDIRS += \
    Atelier_Connexion.pro \
    Atelier_Connexion.pro
