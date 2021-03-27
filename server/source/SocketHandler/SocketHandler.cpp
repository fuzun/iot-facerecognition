
#include "SocketHandler.h"

#include <QWebSocketServer>
#include <QtNetwork/QSslCertificate>
#include <QtNetwork/QSslKey>
#include <QtNetwork/QSslError>
#include <QFile>
#include <QSslCertificate>
#include <QWebSocket>
#include <QSettings>

#include <stdexcept>

#include "Server/Server.h"
#include "ClientHandler/ClientHandler.h"
#include "UIInterface/UIInterface.h"
#include "config.h"

SocketHandler::SocketHandler(QObject *parent, UIInterface *&_uiInterface, ClientHandler *&_clientHandler, QSettings* config)
    : QObject(parent),
      clientHandler(_clientHandler),
      uiInterface(_uiInterface)
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

    webSocketServer = new QWebSocketServer(m_serverName, QWebSocketServer::SslMode::SecureMode, this);
    uiInterface->log("QWebSocketServer is created!");

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
    webSocketServer->setSslConfiguration(sslConfiguration);

    if (webSocketServer->listen(QHostAddress::Any, m_port))
    {
        connect(webSocketServer, &QWebSocketServer::newConnection, this, &SocketHandler::onNewConnection);
        connect(webSocketServer, &QWebSocketServer::sslErrors, this, &SocketHandler::onSslErrors);
        uiInterface->log("Started listening on port:" + QString::number(m_port));
    }
    else
    {
        throw std::runtime_error("QWebSocketServer can not listen!");
    }
}

SocketHandler::~SocketHandler()
{
    webSocketServer->close();
    uiInterface->log("Stopped listening.");
}

void SocketHandler::onNewConnection()
{
    uiInterface->log("Connecting a client...");
    QWebSocket *socket = webSocketServer->nextPendingConnection();

    connect(socket, &QWebSocket::disconnected, this, &SocketHandler::onDisconnect);
    clientHandler->newClient(socket);
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
    clientHandler->removeClient(socket);
}
