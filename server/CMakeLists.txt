# iot-facerecognition-server
# Author: fuzun, 2020
# License: GPLv3

cmake_minimum_required(VERSION 3.14)

project(iot-facerecognition-server LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR OFF)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check https://doc.qt.io/qt/deployment-android.html for more information.
# They need to be set before the find_package(...) calls below.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

if (MSVC)
        add_definitions(/bigobj)
        add_definitions(/FS)
        add_definitions(/arch:AVX)
endif (MSVC)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Widgets WebSockets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Widgets WebSockets REQUIRED)

set(PROJECT_SOURCES
		source/ClientDialog/ClientDialog.cpp
		source/ClientHandler/ClientHandler.cpp
		source/Client/Client.cpp
		source/DLIBWorker/DLIBWorker.cpp
		source/UIInterface/UIInterface.cpp
		source/Server/Server.cpp
		source/SocketHandler/SocketHandler.cpp
		source/main.cpp
		source/MainWindow/MainWindow.cpp
		
		source/ClientDialog/ClientDialog.ui
		source/MainWindow/MainWindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(iot-facerecognition-server
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(iot-facerecognition-server SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(iot-facerecognition-server
          ${PROJECT_SOURCES}
        )
    endif()
endif()

# copy config file
# TODO: generate config file dynamically
configure_file(config.ini config.ini COPYONLY)

# copy self-signed test certificate
configure_file(test.crt test.crt COPYONLY)
configure_file(test.key test.key COPYONLY)

include(FetchContent)
FetchContent_Declare(dlib
    GIT_REPOSITORY https://github.com/davisking/dlib.git
    GIT_TAG        v19.21
)
FetchContent_MakeAvailable(dlib)

target_include_directories(iot-facerecognition-server
	PRIVATE
		source/ClientDialog
		source/ClientHandler
		source/Client
		source/DLIBWorker
		source/UIInterface
		source/MainWindow
		source/Server
		source/SocketHandler
		source
)

target_compile_definitions(iot-facerecognition-server
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG> VER=${APP_VERSION})
target_link_libraries(iot-facerecognition-server
  PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::WebSockets dlib::dlib)
  
