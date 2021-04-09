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
#include "SocketHandler.h"

#include <QWebSocket>
#include <QWebSocketServer>
#include <QSslCertificate>
#include <QSslKey>
#include <QSslError>
#include <QFile>
#include <QSslCertificate>
#include <QSettings>

#include <stdexcept>

#include "Client/Client.h"

#include "config.h"

Q_DECLARE_METATYPE(QWebSocket*)

SocketHandler::SocketHandler(QObject *parent, QSettings* config)
    : QObject(parent),
    m_config(config)
{
    config->beginGroup(CONFIG_CRT);
    m_testMode = config->value(CONFIG_CRT_TESTMODE, CONFIG_CRT_DEFAULT_TESTMODE).toBool();
    m_keyFile = config->value(CONFIG_CRT_KEY_LOCATION, CONFIG_CRT_DEFAULT_KEY_LOCATION).toString();
    m_crtFile = config->value(CONFIG_CRT_CRT_LOCATION, CONFIG_CRT_DEFAULT_CRT_LOCATION).toString();
    m_keyAlgorithm = config->value(CONFIG_CRT_KEYALGORITHM, CONFIG_CRT_DEFAULT_KEYALGORITHM).toInt();
    m_encodingFormat = config->value(CONFIG_CRT_ENCODINGFORMAT, CONFIG_CRT_DEFAULT_ENCODINGFORMAT).toInt();
    config->endGroup();

    config->beginGroup(CONFIG_CONN);
    m_port = static_cast<unsigned short>(config->value(CONFIG_CONN_PORT, CONFIG_CONN_DEFAULT_PORT).toUInt());
    m_serverName = config->value(CONFIG_CONN_SERVERNAME, CONFIG_CONN_DEFAULT_SERVERNAME).toString();
    config->endGroup();

    webSocketServer = new QWebSocketServer(m_serverName, QWebSocketServer::SslMode::SecureMode, nullptr);
    webSocketServer->moveToThread(&socketThread);
    connect(&socketThread, &QThread::finished, webSocketServer, &QObject::deleteLater);
    socketThread.start();

    QSslConfiguration sslConfiguration;
    QFile crtFile(m_crtFile);
    QFile keyFile(m_keyFile);
    crtFile.open(QIODevice::ReadOnly);
    keyFile.open(QIODevice::ReadOnly);
    QSslCertificate certificate(&crtFile, static_cast<QSsl::EncodingFormat>(m_encodingFormat));
    QSslKey sslKey(&keyFile, static_cast<QSsl::KeyAlgorithm>(m_keyAlgorithm), static_cast<QSsl::EncodingFormat>(m_encodingFormat));
    crtFile.close();
    keyFile.close();
    sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);
    sslConfiguration.setLocalCertificate(certificate);
    sslConfiguration.setPrivateKey(sslKey);
    sslConfiguration.setProtocol(QSsl::TlsV1_3OrLater);

    QMetaObject::invokeMethod(webSocketServer, [this, sslConfiguration] () {
            webSocketServer->setSslConfiguration(sslConfiguration);

            if (webSocketServer->listen(QHostAddress::Any, m_port))
            {
                connect(webSocketServer, &QWebSocketServer::newConnection, this, &SocketHandler::onNewConnection);
                connect(webSocketServer, &QWebSocketServer::sslErrors, this, &SocketHandler::onSslErrors);
                emit ("Started listening on port:" + QString::number(m_port));
            }
            else
            {
                throw std::runtime_error("QWebSocketServer can not listen!");
            }
    }, Qt::BlockingQueuedConnection);
}

SocketHandler::~SocketHandler()
{
    QMetaObject::invokeMethod(webSocketServer, [this] () {
            webSocketServer->close();
            emit log("Stopped listening.");
        }, Qt::BlockingQueuedConnection);


    socketThread.quit();
    socketThread.wait();
}

void SocketHandler::onNewConnection()
{
    emit log("A client has connected!");
    QWebSocket *socket = webSocketServer->nextPendingConnection();

    connect(socket, &QWebSocket::disconnected, this, &SocketHandler::onDisconnect);

    Client *client = new Client(nullptr, socket, m_config);
    client->moveToThread(socket->thread());

    emit newClient(client);
}

void SocketHandler::onSslErrors(const QList<QSslError>& errors)
{
    if (!m_testMode)
    {
        QString errStr = "SSL Error:";
        for (const auto &it : errors)
        {
            errStr += "\n" + it.errorString();
        }

        throw std::runtime_error(errStr.toStdString().c_str());
    }
}

void SocketHandler::onDisconnect()
{
    QWebSocket *socket = qobject_cast<QWebSocket *>(sender());
    if (socket)
    {
        socket->deleteLater();
    }
}
