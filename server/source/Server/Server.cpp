/*
*    iot-facerecognition-server
*
*    Copyright (C) 2020, fuzun
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "Server.h"

#include <QSettings>
#include <QApplication>
#include <QWebSocket>
#include <QFile>

#include "MainWindow/MainWindow.h"
#include "SocketHandler/SocketHandler.h"
#include "Client/Client.h"

#include "config.h"

#ifdef NDEBUG
#include <iostream>
#endif

Q_DECLARE_METATYPE(QWebSocket*)

Server::Server(QObject *parent)
    : QObject(parent)
{
    config = new QSettings(QApplication::applicationDirPath() + "/" + CONFIG_FILENAME, QSettings::IniFormat, this);

    config->beginGroup(CONFIG_GUI);
    bool guiEnabled = config->value(CONFIG_GUI_ENABLE, CONFIG_GUI_DEFAULT_ENABLE).toBool();
    config->endGroup();

    config->beginGroup(CONFIG_LOG);
    bool logEnabled = config->value(CONFIG_LOG_ENABLE, CONFIG_LOG_DEFAULT_ENABLE).toBool();
    QString logFileName = config->value(CONFIG_LOG_LOCATION, CONFIG_LOG_DEFAULT_LOCATION).toString();
    config->endGroup();

    if (logEnabled)
    {
        logFile = new QFile(logFileName, this);
        if (!logFile->open(QFile::Text | QFile::Append))
        {
            log("Can't write to log file. Logging to file is disabled.");
        }
        else
        {
            logStream = new QTextStream(logFile);
        }
    }

    if (guiEnabled)
    {
        mainWindow = new MainWindow(nullptr);
        connect(mainWindow, &MainWindow::end, this, &Server::end, Qt::QueuedConnection);
        mainWindow->show();
    }

    qRegisterMetaType<QWebSocket*>();

    socketHandler = new SocketHandler(this, config);

    if (mainWindow)
    {
        connect(socketHandler, &SocketHandler::newClient, mainWindow, &MainWindow::newClient);
        connect(socketHandler, &SocketHandler::log, mainWindow, &MainWindow::print);
    }

    connect(socketHandler, &SocketHandler::log, this, &Server::log);

    connect(socketHandler, &SocketHandler::newClient, this, [this](Client* client) {
        connect(client, &Client::log, this, &Server::log);
    });
}

Server::~Server()
{
    logStream->flush();
    delete logStream;

    if(logFile->isOpen())
    {
        logFile->flush();
        logFile->close();
    }

    delete logFile;
}

QString Server::generateDateTime()
{
    QDateTime dateTime = QDateTime::currentDateTime();
    return dateTime.toString("dd-MM-yyyy / hh:mm:ss");
}

void Server::log(const QString &message)
{
    QString preface = QString("[%1]: ").arg(generateDateTime());

    if ( const auto ptrClient = qobject_cast<Client*>(sender()) )
    {
        preface += ptrClient->getName() + ": ";
    }

#ifdef NDEBUG
    std::cout << preface.toStdString() << message.toStdString() << std::endl;
#else
    qDebug((preface + message).toStdString().c_str());
#endif

    if (logStream)
    {
        *logStream << preface << message;
        Qt::endl(*logStream);
    }
}
