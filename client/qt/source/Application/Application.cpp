/*
 * iot-facerecognition-client-qt
 * Copyright (C) 2021 - fuzun
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Application.hpp"

#include <QCoreApplication>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQmlContext>
#include <QFile>
#include <QTextStream>
#ifdef NDEBUG
#include <stdexcept>
#endif

#include "common/Log.hpp"
#include "common/SettingObject.hpp"
#include "Client/Client.hpp"
#include "Interface/Interface.hpp"
#include "Interface/InterfaceSettings.hpp"
#include "QMLGLYUVWidget/QMLGLYUVWidget.hpp"
#include "Camera/CameraController.hpp"
#include "Statistics/Statistics.hpp"

#include "common/common.hpp"

Application::Application(QObject *parent, QApplication* application)
    : QObject(parent)
{
    QQuickStyle::setStyle("Universal");

    Q_INIT_RESOURCE(qml);
    Q_INIT_RESOURCE(assets);

    qRegisterMetaType<size_t>("size_t");

    qmlRegisterType<QMLGLYUVWidget>("com.iotfacerecognition.components", 1, 0, "YUVRendererWidget");
    qmlRegisterUncreatableMetaObject(Log::staticMetaObject, "com.iotfacerecognition.log", 1, 0, "Log", "This type is uncreatable!");
    qmlRegisterUncreatableType<SettingObject>("com.iotfacerecognition.settings", 1, 0, "SettingObject", "This type is uncreatable!");

    engine = new QQmlApplicationEngine(this);

    statistics = new Statistics(this);
    engine->rootContext()->setContextProperty("statistics", statistics);

    client = new Client(this, statistics);
    engine->rootContext()->setContextProperty("client", client);

    camera = new CameraController(this, statistics);
    engine->rootContext()->setContextProperty("camera", camera);

    connect(client, &Client::connectedChanged, this, [this]() {
        if (client->connected())
            connect(camera, &CameraController::jpegReady, client, &Client::sendData);
        else
            disconnect(camera, &CameraController::jpegReady, client, &Client::sendData);
    });

    interface = new Interface(this);
    engine->rootContext()->setContextProperty("intf", interface);
    connect(interface, &Interface::setLoggingToFile, this, &Application::setLoggingToFile);

    qRegisterMetaType<Log::LogType>();
    connect(camera, &CameraController::log, interface, &Interface::log);
    connect(client, &Client::log, interface, &Interface::log);
    connect(interface, &Interface::log, this, &Application::log);

    const QUrl url(QStringLiteral("qrc:/source/qml/main.qml"));
    connect(engine, &QQmlApplicationEngine::objectCreated,
                     application, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine->load(url);
}

Application::~Application()
{
    if(logStream)
        logStream->flush();
    delete logStream;
}

void Application::log(const QVariant &str, Log::LogType type)
{
    assert(type != Log::ERROR);

    const Log log(str, type);

    QString ctx = QString("[%1] %2: %3").arg(log.timeStamp(), log.typeStr(), log.ctx().toString());

    if (logStream)
        *logStream << ctx;

#ifndef NDEBUG
    qDebug() << ctx;
#else
    std::string msg = ctx.toStdString();

    switch (type)
    {
    case Log::LogType::ERROR:
        qCritical(msg.c_str());
        if (!interface->settings()->testMode())
        {
            throw std::runtime_error(msg.c_str());
        }
        break;
    case Log::LogType::WARNING:
        qWarning(msg.c_str());
        break;
    case Log::LogType::INFORMATION:
    default:
        qInfo(msg.c_str());
        break;
    }
#endif
}

void Application::setLoggingToFile(bool enabled)
{
    if (enabled)
    {
        if (logFile)
            return;

        logFile = new QFile("log.txt", this);
        if(!logFile->open(QFile::Text | QFile::Append))
        {
            log(tr("Could not open log output file!"), Log::ERROR);
            delete logFile;
            logFile = nullptr;
        }
        else
        {
            logStream = new QTextStream(logFile);
        }
    }
    else
    {
        if (!logFile)
            return;

        logStream->flush();
        logFile->flush();
        logFile->close();

        delete logStream;
        logStream = nullptr;
        delete logFile;
        logFile = nullptr;
    }
}
