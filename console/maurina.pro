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
    aboutWindow.cpp

HEADERS  += MainWindow.h \
    aboutWindow.h

FORMS    += MainWindow.ui \
    aboutWindow.ui

RESOURCES += \
    main.qrc

TRANSLATIONS = languages/maurina_es.ts

win32:RC_FILE = maurina.rc
