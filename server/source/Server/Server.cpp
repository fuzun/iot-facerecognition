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

#include "MainWindow/MainWindow.h"
#include "ClientHandler/ClientHandler.h"
#include "SocketHandler/SocketHandler.h"
#include "UIInterface/UIInterface.h"

#include "config.h"

Server::Server(QObject *parent)
    : QObject(parent)
{
    config = new QSettings(QApplication::applicationDirPath() + "/" + CONFIG_FILENAME, QSettings::IniFormat, this);

    config->beginGroup(CONFIG_GUI);
    bool guiEnabled = config->value(CONFIG_GUI_ENABLE, CONFIG_GUI_DEFAULT_ENABLE).toBool();
    config->endGroup();

    if(guiEnabled)
    {
        mainWindow = new MainWindow(nullptr);
        connect(mainWindow, &MainWindow::end, []()
        {
            // fix: halt and terminate all threads before quit
            QApplication::quit();
        });
        mainWindow->show();
    }
    else
    {
        mainWindow = nullptr;
    }

    uiInterface = new UIInterface(this, mainWindow, config);
    clientHandler = new ClientHandler(this, uiInterface, config);
    socketHandler = new SocketHandler(this, uiInterface, clientHandler, config);
}

Server::~Server()
{
    if(mainWindow)
        delete mainWindow;
}
