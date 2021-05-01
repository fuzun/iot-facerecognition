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
#include "ClientWorker.hpp"

#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRect>

ClientWorker::ClientWorker(QObject *parent) :
    QObject(parent)
{

}

qint64 ClientWorker::sendTextMessage(const QString &ctx)
{
    if(!socket || !socket->isValid())
    {
        emit log(tr("Could not send text message! (socket is not available)"), Log::WARNING);
        return -1;
    }

    auto ret = socket->sendTextMessage(ctx);
    auto pSize = ctx.size();

    if (ret < pSize)
    {
        emit log(tr("Text Package loss. WANTED TO SEND / ACTUALLY SENT: %1 / %2").arg(pSize).arg(ret), Log::WARNING);
    }

    return ret;
}

qint64 ClientWorker::sendBinaryMessage(const QByteArray &ctx)
{
    if(!socket || !socket->isValid())
    {
        emit log(tr("Could not send binary message! (socket is not available)"), Log::WARNING);
        return -1;
    }

    auto ret = socket->sendBinaryMessage(ctx);
    auto pSize = ctx.size();

    if (ret < pSize)
    {
        emit log(QString("Binary Package loss. WANTED TO SEND / ACTUALLY SENT: %1 / %2").arg(pSize).arg(ret), Log::WARNING);
    }

    return ret;
}

void ClientWorker::processCommand(ClientWorker::Command cmd, const QVariant &ctx)
{
    switch (cmd)
    {
    case Command::MESSAGE:
    {
        emit log(tr("Server says: %1").arg(ctx.toString()));
        break;
    }

    case Command::MESSAGE_TAG_FACE:
    {
        const QVariantList list = ctx.toList();

        emit faceTagReceived(ctx.toList());

        for (const auto & i : list)
        {
            const QVariantMap map = i.toMap();
            emit log(tr("Received face tag: \"%0\" - X: %1, Y: %2, W: %3, H: %4")
                         .arg(map["tag"].toString())
                         .arg(map["x"].toInt())
                         .arg(map["y"].toInt())
                         .arg(map["width"].toInt())
                         .arg(map["height"].toInt()));
        }

        break;
    }

    case Command::MESSAGE_TAG_OBJECT:
    {
        const QVariantList list = ctx.toList();

        emit objectTagReceived(list);

        QStringList strList;
        for (const auto& i : list)
        {
            const QVariantMap map = i.toMap();
            strList.append(QString("%1: %2").arg(map["prediction"].toString()).arg(map["label"].toString()));
        }

        emit log(tr("Received object tag: %1").arg(strList.join(", ")));

        break;
    }

    case Command::INIT_REQUEST:
    {
        emit initRequest();
    }

    default:
        break;
    }
}

void ClientWorker::processTextMessage(const QString &message)
{
    QJsonDocument jDoc = QJsonDocument::fromJson(message.toUtf8());

    if (jDoc.isNull() || jDoc.isEmpty() || !jDoc.isObject())
        return;

    const unsigned int command = (unsigned)(jDoc[keyCommand].toInt(-1));

    QJsonValue ctxVal = jDoc[keyContext];
    QVariant context;

    if (ctxVal != QJsonValue::Undefined)
        context = ctxVal.toVariant();

    emit commandReceived(static_cast<Command>(command), context);
}

void ClientWorker::init()
{
    socket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);

    QObject::connect(this, &ClientWorker::commandReceived, this, &ClientWorker::processCommand);
    QObject::connect(socket, &QWebSocket::textMessageReceived, this, &ClientWorker::processTextMessage);

    QObject::connect(socket, &QWebSocket::bytesWritten, this, &ClientWorker::sentBytes);

    const auto receivedBytesEmitter = [this](const auto& frame, bool) {
        emit receivedBytes(frame.size());
    };
    QObject::connect(socket, &QWebSocket::textFrameReceived, this, receivedBytesEmitter);
    QObject::connect(socket, &QWebSocket::binaryFrameReceived, this, receivedBytesEmitter);

    QObject::connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, [this]() {
        emit log(QString("Socket error: %1").arg(socket->errorString()), Log::WARNING);
    });
}

void ClientWorker::sendCommand(ClientWorker::Command cmd, const QVariant &ctx)
{
    QJsonObject obj;

    obj[keyCommand] = static_cast<int>(cmd);
    if (ctx.isValid())
        obj[keyContext] = QJsonValue::fromVariant(ctx);

    QJsonDocument jDoc(obj);

    if (ctx.isValid())
    {
        emit log(tr("Sending command: %1, with context: %2").arg(static_cast<unsigned char>(cmd)).arg(ctx.toString()));
    }
    else
    {
        emit log(tr("Sending command: %1").arg(static_cast<unsigned char>(cmd)));
    }

    sendTextMessage(QString(jDoc.toJson(QJsonDocument::JsonFormat::Compact)));
}

void ClientWorker::connect(const QUrl &url)
{
    emit log(tr("Connecting to %1...").arg(url.toString()));
    socket->open(url);
}

void ClientWorker::disconnect()
{
    emit log(tr("Disconnecting from server..."));
    socket->close();
}

QString ClientWorker::serverAddress()
{
    return socket->peerAddress().toString();
}

QVariant ClientWorker::serverPort()
{
    return QVariant::fromValue(socket->peerPort());
}

void ClientWorker::sendMessage(const QString &string)
{
    sendCommand(Command::MESSAGE, string);
}

void ClientWorker::sendData(const QByteArray &data)
{
    sendBinaryMessage(data);
}

