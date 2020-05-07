# iot-facerecognition-server
# Author: fuzun
# License: GPLv3

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += $$PWD/source

SOURCES += \
    source/ClientDialog/ClientDialog.cpp \
    source/Server/Server.cpp \
    source/main.cpp \
    source/MainWindow/MainWindow.cpp

HEADERS += \
    source/ClientDialog/ClientDialog.h \
    source/MainWindow/MainWindow.h \
    source/Server/Server.h \
    source/config.h

FORMS += \
    source/ClientDialog/ClientDialog.ui \
    source/MainWindow/MainWindow.ui

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
