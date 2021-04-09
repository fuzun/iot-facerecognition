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

#include "Client.h"

#include <QWebSocket>

#include <QThreadPool>

#include <QApplication>
#include <QPainter>
#include <QTimer>

#include <QThread>

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#include <QMetaMethod>

#include "DLIBWorker/DLIBWorker.h"

Client::Client(QObject *parent, QWebSocket* _socket, QSettings* _config) :
    QObject(parent),
    socket(_socket)
{
    assert(_socket);
    assert(_config);

    qRegisterMetaType<QVector<QPair<QRect, QString>>>();
    qRegisterMetaType<QVector<QPair<float,QString>>>();
    qRegisterMetaType<Client::Command>();

    qRegisterMetaType<QAbstractSocket::SocketError>();

    // defer until proper thread is assigned:
    QMetaObject::invokeMethod(this, [this, _config]() {
            QObject::connect(socket, &QWebSocket::binaryMessageReceived, this, &Client::processBinaryMessage);
            QObject::connect(socket, &QWebSocket::textMessageReceived, this, &Client::processTextMessage);
            QObject::connect(this, &Client::commandReceived, this, &Client::processCommand);

            QObject::connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, [this]() {
                assert(false);
                emit log(QString("Socket error: %1").arg(socket->errorString()));
            });

            QObject::connect(socket, &QWebSocket::sslErrors, this, [this](const QList<QSslError> &errors) {
                assert(false);
                for (const auto& it : errors)
                {
                    emit log(QString("Socket SSL Error: %1").arg(it.errorString()));
                }
            });

            QObject::connect(socket, &QWebSocket::disconnected, this, [this]() {
                emit log(QString("Disconnected. Close reason: %1").arg(socket->closeReason()));
            });


            dlibWorker = new DLIBWorker(_config, &settings);
            dlibWorker->moveToThread(&dlibWorkerThread);
            connect(&dlibWorkerThread, &QThread::finished, dlibWorker, &QObject::deleteLater);
            connect(this, &Client::processImage, dlibWorker, &DLIBWorker::process);
            connect(dlibWorker, &DLIBWorker::doneFace, this, &Client::doneFace);
            connect(dlibWorker, &DLIBWorker::doneObject, this, &Client::doneObject);

            connect(this, &Client::doneFace, this, &Client::processFaceResults);
            connect(this, &Client::doneObject, this, &Client::processObjectResults);

            connect(dlibWorker, &DLIBWorker::log, this, &Client::log);
            dlibWorkerThread.start();

            QObject::connect(socket, &QWebSocket::destroyed, this, [this]() {
                socket = nullptr;
                this->deleteLater();
            });

            sendCommand(Command::INIT_REQUEST);

        }, Qt::QueuedConnection);
}

Client::~Client()
{
    emit log("Stopping processing...");

    dlibWorkerThread.quit();
    dlibWorkerThread.wait();

    if (socket)
        socket->flush();
}

QString Client::getName() const
{
    return name;
}

void Client::processBinaryMessage(const QByteArray& data)
{
    if(!dlibWorker->isBusy())
    {
        emit processImage(data);
    }

    if (isSignalConnected(QMetaMethod::fromSignal(&Client::primaryDisplayUpdated)))
    {
        dlib::array2d<dlib::rgb_pixel> img(DLIBWorker::decodeJPEG(data, nullptr));

        if (img.size() <= 0)
            return;

        emit primaryDisplayUpdated(QPixmap::fromImage(QImage(static_cast<unsigned char*>(dlib::image_data(img)), img.nc(), img.nr(), QImage::Format_RGB888)));
    }
}

void Client::processFaceResults(const QVector<QPair<QRect, QString>>& results)
{
    if (results.isEmpty())
        return;

    emit log("DLIB has found faces with following properties:");

    int maxX = 0, maxY = 0;
    QVariantList ctx;
    for(const auto &it : results)
    {
        QVariantHash i;
        int x = it.first.x();
        int y = it.first.y();
        int width = it.first.width();
        int height = it.first.height();

        if (x + width > maxX)
            maxX = x + width;

        if (y + height > maxY)
            maxY = y + height;

        QString tag = it.second;

        i["x"] = x;
        i["y"] = y;
        i["width"] = width;
        i["height"] = height;
        i["tag"] = tag;

        ctx.push_back(i);
        emit log(QString("Tag: \"%0\" - X: %1, Y: %2, W: %3, H: %4")
                     .arg(tag)
                     .arg(x)
                     .arg(y)
                     .arg(width)
                     .arg(height));
    }
    emit log("Sending found face properties to the client.");
    sendCommand(Command::MESSAGE_TAG_FACE, ctx);
}

void Client::processObjectResults(const QVector<QPair<float, QString> > &results)
{
    if (results.isEmpty())
        return;

    QStringList strList;
    QVariantList list;
    for (const auto& i : results)
    {
        QVariantHash hash;

        hash["prediction"] = i.first;
        hash["label"] = i.second;

        strList.push_back(QString("%1: %2").arg(i.first).arg(i.second));
        list.push_back(hash);
    }

    emit log("DLIB has found objects: " + strList.join(","));
    emit log("Sending found object properties to the client.");
    sendCommand(Command::MESSAGE_TAG_OBJECT, list);
}

void Client::processTextMessage(const QString &message)
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

void Client::sendCommand(Client::Command cmd, const QVariant &ctx)
{
    QJsonObject obj;

    obj[keyCommand] = static_cast<int>(cmd);
    if (ctx.isValid())
        obj[keyContext] = QJsonValue::fromVariant(ctx);

    QJsonDocument jDoc(obj);

    sendTextMessage(QString(jDoc.toJson(QJsonDocument::JsonFormat::Compact)));
}

qint64 Client::sendTextMessage(const QString &ctx)
{
    if(!socket || !socket->isValid())
        return -1;

    auto ret = socket->sendTextMessage(ctx);
    auto pSize = ctx.size();

    if (ret < pSize)
    {
        emit log(QString("Text Package loss. WANTED TO SEND / ACTUALLY SENT: %1 / %2").arg(pSize).arg(ret));
    }

    return ret;
}

qint64 Client::sendBinaryMessage(const QByteArray &ctx)
{
    if(!socket || !socket->isValid())
        return -1;

    auto ret = socket->sendBinaryMessage(ctx);
    auto pSize = ctx.size();

    if (ret < pSize)
    {
        emit log(QString("Binary Package loss. WANTED TO SEND / ACTUALLY SENT: %1 / %2").arg(pSize).arg(ret));
    }

    return ret;
}

void Client::processCommand(Command cmd, const QVariant &ctx)
{
    switch(cmd)
    {
        case Command::SETTING_NAME:
        {
            QString _name = ctx.toString();
            if (_name == name)
                break;

            if (_name == "")
                _name = "noname";

            name = _name;
            emit nameChanged(name);

            sendCommand(Command::SETTING_NAME, name);
            break;
        }

        case Command::MESSAGE:
        {
            emit log("Message: " + ctx.toString());
            break;
        }

        case Command::SETTING_LABELCOUNT:
        {
            settings.labelCount = ctx.toULongLong();

            sendCommand(Command::SETTING_LABELCOUNT, settings.labelCount.load());
            break;
        }

        case Command::SETTING_OBJDETECTIONENABLED:
        {
            settings.objectDetectionEnabled = ctx.toBool();

            sendCommand(Command::SETTING_OBJDETECTIONENABLED, settings.objectDetectionEnabled.load());
            break;
        }

        case Command::SETTING_DETERMINISTICOBJECTDETECTION:
        {
            settings.deterministicObjectDetection = ctx.toBool();

            sendCommand(Command::SETTING_DETERMINISTICOBJECTDETECTION, settings.deterministicObjectDetection.load());
            break;
        }

        case Command::SETTING_FACERECOGNITIONENABLED:
        {
            settings.faceRecognitionEnabled = ctx.toBool();

            sendCommand(Command::SETTING_FACERECOGNITIONENABLED, settings.faceRecognitionEnabled.load());
            break;
        }

        default:
            break;
    }
}
