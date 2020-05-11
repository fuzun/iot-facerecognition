# iot-facerecognition-server
# Author: fuzun, 2020
# License: GPLv3

VERSION = 1.0.0
DEFINES += VER=\\\"$$VERSION\\\"

QT       += core gui websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 console

DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += $$PWD/source

SOURCES += \
    source/ClientDialog/ClientDialog.cpp \
    source/ClientHandler/ClientHandler.cpp \
    source/Client/Client.cpp \
    source/DLIBWorker/DLIBWorker.cpp \
    source/UIInterface/UIInterface.cpp \
    source/Server/Server.cpp \
    source/SocketHandler/SocketHandler.cpp \
    source/main.cpp \
    source/MainWindow/MainWindow.cpp

HEADERS += \
    source/ClientDialog/ClientDialog.h \
    source/ClientHandler/ClientHandler.h \
    source/Client/Client.h \
    source/DLIBWorker/DLIBWorker.h \
    source/UIInterface/UIInterface.h \
    source/MainWindow/MainWindow.h \
    source/Server/Server.h \
    source/SocketHandler/SocketHandler.h \
    source/config.h

FORMS += \
    source/ClientDialog/ClientDialog.ui \
    source/MainWindow/MainWindow.ui


# dlib
INCLUDEPATH += $$PWD/dlib
SOURCES += dlib/dlib/all/source.cpp
DEFINES += DLIB_NO_GUI_SUPPORT
CONFIG(debug, debug|release){
DEFINES += ENABLE_ASSERTS
}

# OpenCV
INCLUDEPATH += $$(OPENCV_SDK_DIR)/include
# Change below if MSVC is not used for compiling
OCV_COMPILER_SPECIFIC = vc15
LIBS += -L$$(OPENCV_SDK_DIR)/x64/$$OCV_COMPILER_SPECIFIC/lib
CONFIG(debug, debug|release){
LIBS += -lopencv_world430d
} else {
LIBS += -lopencv_world430
}

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    ../assets/Assets.qrc
