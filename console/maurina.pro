#-------------------------------------------------
#
# Project created by QtCreator 2013-12-17T09:01:51
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = maurina
TEMPLATE = app


SOURCES += main.cpp\
        MainWindow.cpp \
    aboutWindow.cpp \
    configWindow.cpp

HEADERS  += MainWindow.h \
    aboutWindow.h \
    configWindow.h

FORMS    += MainWindow.ui \
    aboutWindow.ui \
    configWindow.ui

RESOURCES += \
    main.qrc

TRANSLATIONS = languages/maurina_es.ts

win32:RC_FILE = maurina.rc
