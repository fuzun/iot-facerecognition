# iot-facerecognition-client-qt
# Author: fuzun, 2021
# License: AGPLv3

VERSION = 1.0.0
DEFINES += VERSION=\\\"$$VERSION\\\"

QT += widgets quick virtualkeyboard quickcontrols2 websockets charts

CONFIG += c++17

INCLUDEPATH += $$PWD/source

QT_CONFIG -= no-pkg-config
CONFIG += link_pkgconfig

CONFIG(release, debug|release) : DEFINES += NDEBUG

if (linux-rasp*) {
    message("Target device is Raspberry Pi...")

    LIBS += -L$$[QT_SYSROOT]/lib/arm-linux-gnueabihf

    message("Checking if OpenMaxIL++ available")

    exists("$$[QT_SYSROOT]/opt/vc/lib/OpenMaxIL++/libOpenMaxIL++*") {
        message("OpenMaxIL++ library is found!")
        LIBS += -L$$[QT_SYSROOT]/opt/vc/lib/OpenMaxIL++ -lOpenMaxIL++
        INCLUDEPATH += $$[QT_SYSROOT]/opt/vc/include/OpenMaxIL++
        DEFINES += CAM_OMX

        LIBS += -lbcm_host -lvcos -lvchiq_arm -lrt -lpthread -ldl -lopenmaxil -lvcsm

        DEFINES += HAVE_LIBOPENMAX=2 OMX OMX_SKIP64BIT USE_EXTERNAL_OMX HAVE_LIBBCM_HOST USE_EXTERNAL_LIBBCM_HOST USE_VCHIQ_ARM
    } else {
        error("Could not found OpenMaxIL++ library! Check the README file and https://github.com/dridri/OpenMaxIL-cpp repository for more information!")
    }
}

!contains (DEFINES, CAM_OMX) {
    message("Qt Multimedia backend will be used for the camera")
    QT += multimedia
    DEFINES += CAM_QTMULTIMEDIA

    packagesExist(libturbojpeg) {
        message("pkgconfig found libturboconfig! It will be used for fast RGB to JPEG encoding!")
        DEFINES += TURBOJPEG_AVAILABLE
        PKGCONFIG += libturbojpeg
    } else : contains(QMAKE_HOST.arch, x86_64) : exists("C:\libjpeg-turbo64\lib\turbojpeg.lib") {
        message("libjpeg-turbo is found! It will be used for fast RGB to JPEG encoding!")
        DEFINES += TURBOJPEG_AVAILABLE
        LIBS += -LC:\libjpeg-turbo64\lib -lturbojpeg
        INCLUDEPATH += C:\libjpeg-turbo64\include
    } else {
        message("libjpeg-turbo library could not be found! JPEG encoding will be slow!")
    }
}


HEADERS += \
    source/Application/Application.hpp \
    source/Camera/CameraController.hpp \
    source/Camera/CameraSettings.hpp \
    source/Camera/CameraWorker.hpp \
    source/Client/Client.hpp \
    source/Client/ClientSettings.hpp \
    source/Client/ClientWorker.hpp \
    source/Interface/Interface.hpp \
    source/Interface/InterfaceSettings.hpp \
    source/Interface/LogModel.hpp \
    source/QMLGLYUVWidget/QMLGLYUVWidget.hpp \
    source/Statistics/Statistics.hpp \
    source/Statistics/StatisticsSettings.hpp \
    source/common/BaseSettings.hpp \
    source/common/Log.hpp \
    source/common/SettingObject.hpp \
    source/common/ThreadSafeBuffer.hpp \
    source/common/YUVFrame.hpp \
    source/common/common.hpp

contains (DEFINES, CAM_QTMULTIMEDIA) {
    HEADERS += source/Camera/YUVProviderVideoSurface.hpp
    HEADERS += source/yuv2rgb/yuv_rgb.h
}

SOURCES += \
        source/Application/Application.cpp \
        source/Camera/CameraController.cpp \
        source/Camera/CameraWorker.cpp \
        source/Client/Client.cpp \
        source/Client/ClientWorker.cpp \
        source/Interface/Interface.cpp \
        source/Interface/LogModel.cpp \
        source/QMLGLYUVWidget/QMLGLYUVWidget.cpp \
        source/Statistics/Statistics.cpp \
        source/common/Log.cpp \
        source/main.cpp

contains (DEFINES, CAM_QTMULTIMEDIA) {
    SOURCES += source/Camera/YUVProviderVideoSurface.cpp
    SOURCES += source/yuv2rgb/yuv_rgb.c
}

RESOURCES += qml.qrc \
    assets/assets.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
