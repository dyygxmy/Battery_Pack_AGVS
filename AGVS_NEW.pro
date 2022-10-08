#-------------------------------------------------
#
# Project created by QtCreator 2019-02-13T09:59:09
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AGVS_NEW_2020.06.18_1
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    plcconnect.cpp \
    serverconnect.cpp \
    agvsctrl.cpp \
    mymsleep.cpp \
    pulsecheck.cpp

HEADERS  += mainwindow.h \
    globalvariable.h \
    plcconnect.h \
    serverconnect.h \
    agvsctrl.h \
    mymsleep.h \
    pulsecheck.h

FORMS    += mainwindow.ui
