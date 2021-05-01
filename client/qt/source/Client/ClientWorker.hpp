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
#ifndef CLIENTWORKER_H
#define CLIENTWORKER_H

#include <QObject>

#include "common/Log.hpp"

class ClientWorker : public QObject
{
    Q_OBJECT

    friend class Client;

    class QWebSocket *socket = nullptr;

    inline static const QString keyContext {"context"};
    inline static const QString keyCommand {"command"};

    inline const QWebSocket* getSocket() const { return socket; };

public:
    explicit ClientWorker(QObject *parent = nullptr);

    enum class Command : unsigned int
    {
        SETTING_NAME = 1,
        MESSAGE = 2,
        MESSAGE_TAG_FACE = 3,
        MESSAGE_TAG_OBJECT = 4,
        SETTING_OBJDETECTIONENABLED = 5,
        SETTING_LABELCOUNT = 6,
        SETTING_DETERMINISTICOBJECTDETECTION = 7,
        SETTING_FACERECOGNITIONENABLED = 8,
        INIT_REQUEST = 9
    };

private:
    qint64 sendTextMessage(const QString& ctx);
    qint64 sendBinaryMessage(const QByteArray& ctx);

signals:
    void initRequest();

private slots:
    void processCommand(ClientWorker::Command cmd, const QVariant& ctx);
    void processTextMessage(const QString& message);
    void init();

public slots:
    void sendCommand(ClientWorker::Command cmd, const QVariant& ctx = QVariant());
    void connect(const QUrl& url);
    void disconnect();

    QString serverAddress();
    QVariant serverPort();

    void sendMessage(const QString& string);

    void sendData(const QByteArray& data);

signals:
    void log(const QVariant& str, Log::LogType type = Log::INFORMATION);

    void faceTagReceived(const QVariantList &);
    void objectTagReceived(const QVariantList &);
    void commandReceived(ClientWorker::Command cmd, const QVariant& ctx);

    void sentBytes(qint64 size);
    void receivedBytes(qint64 size);
};

Q_DECLARE_METATYPE(ClientWorker::Command)

#endif // CLIENTWORKER_H
