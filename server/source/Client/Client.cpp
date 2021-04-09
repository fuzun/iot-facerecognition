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
#include <QGraphicsPixmapItem>
#include <QThreadPool>
#include <QApplication>
#include <QPainter>
#include <QTimer>
#include <QListWidgetItem>
#include <QThread>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#include "Client/ClientWorker.h"
#include "DLIBWorker/DLIBWorker.h"
#include "ClientDialog/ClientDialog.h"
#include "ClientHandler/ClientHandler.h"

Client::Client(QObject *parent, QWebSocket* _socket, QSettings* config)
    : QObject(parent),
    socket(_socket)
{
    qRegisterMetaType<QVector<QPair<QRect, QString>>>();
    qRegisterMetaType<QVector<QPair<float,QString>>>();

    qRegisterMetaType<Client::Command>();

    worker = new ClientWorker(nullptr, config, &settings);
    worker->moveToThread(&workerThread);

    QObject::connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    workerThread.start();

    QMetaObject::invokeMethod(worker, &ClientWorker::init, Qt::BlockingQueuedConnection);

    QObject::connect(worker, &ClientWorker::log, this, &Client::log);
    QObject::connect(worker, &ClientWorker::commandReceived, this, &Client::processCommand);
    QObject::connect(worker, &ClientWorker::sendCommand, this, &Client::sendCommand);

    QObject::connect(worker->getDlibWorker(), &DLIBWorker::doneFace, this, &Client::processDlibWorkerFaceResults);
    QObject::connect(worker->getDlibWorker(), &DLIBWorker::doneObject, this, &Client::processDlibWorkerObjectResults);

    QObject::connect(socket, &QWebSocket::binaryMessageReceived, this, &Client::processBinaryMessage);

    QObject::connect(socket, &QWebSocket::textMessageReceived, worker, &ClientWorker::processTextMessage);
    QObject::connect(socket, &QWebSocket::binaryMessageReceived, worker, &ClientWorker::processBinaryMessage);

    QObject::connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, [this]() {
        assert(false);
        log(QString("Socket error: %1").arg(socket->errorString()));
    });

    QObject::connect(socket, &QWebSocket::sslErrors, this, [this](const QList<QSslError> &errors) {
        assert(false);
        for (const auto& it : errors)
        {
            log(QString("Socket SSL Error: %1").arg(it.errorString()));
        }
    });

    QObject::connect(socket, &QWebSocket::disconnected, this, [this]() {
        log(QString("Disconnected. Close reason: %1").arg(socket->closeReason()));
    });

    clearSecondaryDisplayTimer = new QTimer(this);
    clearSecondaryDisplayTimer->setInterval(1000);
    connect(clearSecondaryDisplayTimer, &QTimer::timeout, [this](){
        secondaryDisplay->setPixmap(QPixmap());
    });
}

Client::~Client()
{
    workerThread.quit();
    workerThread.wait();

    socket->flush();
}

QString Client::getName() const
{
    return name;
}

void Client::setPrimaryDisplayItem(QGraphicsPixmapItem* item)
{
    primaryDisplay = item;
}

void Client::setSecondaryDisplayItem(QGraphicsPixmapItem* item)
{
    secondaryDisplay = item;
}

void Client::setTertiaryDisplayItem(QGraphicsPixmapItem *item)
{
    tertiaryDisplay = item;
}

void Client::processBinaryMessage(const QByteArray& data)
{
    if(dialog)
    {
        dlib::array2d<dlib::rgb_pixel> img(DLIBWorker::decodeJPEG(data, nullptr));

        if (img.size() <= 0)
            return;

        QPixmap pixmap(QPixmap::fromImage(QImage((unsigned char*)dlib::image_data(img), img.nc(), img.nr(), QImage::Format_RGB888)));
        primaryDisplay->setPixmap(pixmap);
    }
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

void Client::processDlibWorkerFaceResults(const QVector<QPair<QRect, QString>>& results)
{
    // log("DLIB could not find any face in the given image frame!"); // this bloats the log
    if(results.size() > 0)
    {
        if(dialog)
        {
            // process dlib output on dialog window
            clearSecondaryDisplayTimer->stop();
            QPixmap pixmap(primaryDisplay->pixmap().width(), primaryDisplay->pixmap().height());
            pixmap.fill(Qt::transparent);
            for(const auto &it : results)
            {
                QPainter paint(&pixmap);

                QColor colorLine(128, 0, 0, 200);
                QColor colorText(255, 255, 255, 255);

                QFont font;
                font.setPointSize(18);
                paint.setFont(font);

                const QRect& rect = it.first;
                paint.setPen(QPen(colorLine, 3));
                paint.drawRect(rect);

                const QString& str = it.second;

                QFontMetrics fMetrics(paint.font());
                QRect textRect(rect.x(), rect.y() + rect.height(), fMetrics.size(Qt::TextSingleLine, str).width(), fMetrics.size(Qt::TextSingleLine, str).height());
                paint.fillRect(textRect, QBrush(colorLine));

                paint.setPen(colorText);
                paint.drawText(textRect, str);

                secondaryDisplay->setPixmap(pixmap);
            }
            clearSecondaryDisplayTimer->start();
        }
    }
}

void Client::processDlibWorkerObjectResults(const QVector<QPair<float, QString> > &results)
{
    if (results.isEmpty())
        return;

    if (dialog)
    {
        QPixmap pixmap(primaryDisplay->pixmap().width(), primaryDisplay->pixmap().height());
        pixmap.fill(Qt::transparent);

        int counter = 0;
        for (const auto& it : results)
        {
            QPainter paint(&pixmap);

            QColor colorLine(128, 0, 0, 200);
            QColor colorText(255, 255, 255, 255);

            QFont font;
            font.setPointSize(12);
            paint.setFont(font);

            const QString& str = QString("%1: %2").arg(it.first).arg(it.second);

            QFontMetrics fMetrics(paint.font());
            QRect textRect(20, 20 + fMetrics.size(Qt::TextSingleLine, str).height() * counter, fMetrics.size(Qt::TextSingleLine, str).width(), fMetrics.size(Qt::TextSingleLine, str).height());
            paint.fillRect(textRect, QBrush(colorLine));

            paint.setPen(colorText);
            paint.drawText(textRect, str);

            tertiaryDisplay->setPixmap(pixmap);

            ++counter;
        }
    }
}

QListWidgetItem* Client::getListWidgetItem() const
{
    return listItem;
}

ClientDialog* Client::getDialog() const
{
    return dialog;
}

void Client::setListWidgetItem(QListWidgetItem* _listItem)
{
    listItem = _listItem;
}

void Client::setDialog(ClientDialog* _dialog)
{
    dialog = _dialog;
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

void Client::processCommand(ClientWorker::Command cmd, const QVariant &ctx)
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
            emit clientNameChanged(name);

            sendCommand(Command::SETTING_NAME, name);
            break;
        }

        default:
            break;
    }
}
