# iot-facerecognition-server
# Author: fuzun, 2020
# License: GPLv3

# TODO: Use CMake instead.

VERSION = 1.1.0
DEFINES += VER=\\\"$$VERSION\\\"

QT      += core gui websockets

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


QMAKE_CXXFLAGS_RELEASE += /arch:AVX
QMAKE_CXXFLAGS += /bigobj

# CUDA
CONFIG += CUDA_ENABLED
if(CUDA_ENABLED) {
    INCLUDEPATH += $$(CUDA_PATH)/include
    win32 {
        LIBS += -L$$(CUDA_PATH)/lib/x64
    }
    LIBS += -lcudnn -lcudart -lcurand -lcublas -lcusolver
}

# DLIB
# Change below according to the build environment.
INCLUDEPATH += $$(DLIB_DIR)/include
win32 {
    LIBS += -L$$(DLIB_DIR)/lib
}
CONFIG(debug, debug|release){
    LIBS += -ldlibd
} else {
    LIBS += -ldlib
}

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    ./assets/Assets.qrc
