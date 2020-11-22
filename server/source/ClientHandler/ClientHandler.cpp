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

#include "ClientHandler.h"

#include <QHash>

#include "Client/Client.h"
#include "UIInterface/UIInterface.h"

ClientHandler::ClientHandler(QObject *parent, UIInterface *&_uiInterface, QSettings* _config)
    : QObject(parent)
    , config(_config)
    , uiInterface(_uiInterface)
{ }

ClientHandler::~ClientHandler()
{
    removeAllClients();
}

void ClientHandler::newClient(QWebSocket *socket)
{
    if(!socket)
        return;

    Client *client = new Client(this, socket, config);
    clients.insert(socket, client);
    uiInterface->newClient(client);
}

void ClientHandler::removeClient(QWebSocket *socket)
{
    if(!socket)
        return;

    auto it = clients.find(socket);
    Client *client = it.value();
    uiInterface->removeClient(client);
    if(client)
    {
        delete client;
        clients.erase(it);
    }
}

void ClientHandler::removeAllClients()
{
    if (clients.isEmpty())
        return;

    for(auto& it : clients)
    {
        delete it;
        it = nullptr;
    }
    clients.clear();
}

bool ClientHandler::isClientPresent(const QString &name)
{
    for(const auto& it : clients)
    {
        if(it->getName() == name)
            return true;
    }
    return false;
}
