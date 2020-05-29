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

#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H

#include <QObject>
#include <QHash>

#include "UIInterface/UIInterface.h"

class ClientHandler : public QObject
{
    Q_OBJECT

private:
    class QSettings* config;
    UIInterface*& uiInterface;

    class QHash<class QWebSocket*, class Client *> clients;

public:
    explicit ClientHandler(QObject *parent, UIInterface*& _uiInterface, QSettings* config);
    ~ClientHandler();

    void newClient(class QWebSocket *socket);
    void removeClient(class QWebSocket *socket);

    bool isClientPresent(const QString& name);
};

#endif // CLIENTHANDLER_H
