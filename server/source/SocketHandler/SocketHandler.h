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
#ifndef SOCKETHANDLER_H
#define SOCKETHANDLER_H

#include <QObject>
#include <QSslError>
#include <QThread>

class QWebSocket;
class QWebSocketServer;
class QSettings;
class Client;

class SocketHandler : public QObject
{
    Q_OBJECT

private:
    QThread socketThread;

    unsigned short          m_port;
    QString                 m_serverName;
    QString                 m_keyFile;
    QString                 m_crtFile;
    int                     m_keyAlgorithm;
    int                     m_encodingFormat;
    bool                    m_testMode;

protected:
    QSettings *m_config;

public:
    explicit SocketHandler(QObject *parent, class QSettings* config);
    ~SocketHandler();

signals:
    void newClient(Client* socket);
    void log(const QString& message);

protected:
    QWebSocketServer* webSocketServer;

private slots:
    void onNewConnection();
    void onDisconnect();
    void onSslErrors(const QList<QSslError>& errors);

signals:

};

#endif // SOCKETHANDLER_H
