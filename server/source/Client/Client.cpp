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

#include "DLIBWorker/DLIBWorker.h"
#include "ClientDialog/ClientDialog.h"
#include "ClientHandler/ClientHandler.h"

Client::Client(QObject *parent, QWebSocket* _socket, QSettings* config)
    : QObject(parent)
    , socket(_socket)
{
    qRegisterMetaType<QVector<QPair<QRect, QString>>>();

    dlibWorkerThread = new QThread(this);
    dlibWorker = new DLIBWorker(config, &settings);
    dlibWorker->moveToThread(dlibWorkerThread);
    connect(dlibWorker, &DLIBWorker::throwException, this, &Client::throwException);
    connect(dlibWorkerThread, &QThread::finished, dlibWorker, &QObject::deleteLater);
    connect(this, &Client::process, dlibWorker, &DLIBWorker::process);
    connect(dlibWorker, &DLIBWorker::doneFace, this, &Client::processDlibWorkerFaceResults);
    connect(dlibWorker, &DLIBWorker::doneObject, this, &Client::processDlibWorkerObjectResults);
    connect(dlibWorker, &DLIBWorker::log, this, &Client::log);
    dlibWorkerThread->start();

    QObject::connect(socket, &QWebSocket::textMessageReceived, this, &Client::processTextMessage);
    QObject::connect(socket, &QWebSocket::binaryMessageReceived, this, &Client::processBinaryMessage);

    clearSecondaryDisplayTimer = new QTimer(this);
    clearSecondaryDisplayTimer->setInterval(1000);
    connect(clearSecondaryDisplayTimer, &QTimer::timeout, [this](){
        secondaryDisplay->setPixmap(QPixmap());
    });

    log("Started processing incoming images. (if any)");
}

Client::~Client()
{
    log("Stopping processing...");

    dlibWorkerThread->quit();

    // TODO fix this later
    while(!dlibWorkerThread->wait(0))
    {
        QApplication::processEvents();
    }

    socket->flush();
    socket->close();
}

void Client::sendCommand(Client::Command cmd, const QVariant &ctx)
{
    QJsonObject obj;

    obj["command"] = static_cast<int>(cmd);
    if (ctx.isValid())
        obj["context"] = QJsonValue::fromVariant(ctx);

    QJsonDocument jDoc(obj);

    sendTextMessage(QString(jDoc.toJson(QJsonDocument::JsonFormat::Compact)));
}

void Client::throwException(const QString& str)
{
    std::cout << "FATAL ERROR: " << str.toStdString() << std::endl;
    throw str;
}

void Client::processTextMessage(const QString& string)
{
    QJsonDocument jDoc = QJsonDocument::fromJson(string.toUtf8());

    if (jDoc.isNull() || jDoc.isEmpty() || !jDoc.isObject())
        return;

    const unsigned int command = (unsigned)(jDoc["command"].toInt(-1));

    QJsonValue ctxVal = jDoc["context"];
    QVariant context;

    if (ctxVal != QJsonValue::Undefined)
        context = ctxVal.toVariant();

    switch(command)
    {
        // Client wants to change its name:
        case (unsigned int)Command::SETTING_NAME:
        {
//            ClientHandler* cHandler = qobject_cast<ClientHandler *>(parent());
            QString _name = context.toString();
            if (_name == name)
                break;

            if (_name == "")
                _name = "noname";

//            if(cHandler->isClientPresent(_name))
//            {
//                _name.append("(1)");
//            }

//            if(name == "?")
//            {
//                log("Client has connected!");
//            }
            name = _name;
            emit clientNameChanged(name);

            sendCommand(Command::SETTING_NAME, name);
            break;
        }

        // Client wants to send a message:
        case (unsigned int)Command::MESSAGE:
        {
            emit log(" says: " + context.toString());
            break;
        }

        case (unsigned int)Command::SETTING_LABELCOUNT:
        {
            settings.labelCount = context.toULongLong();

            sendCommand(Command::SETTING_LABELCOUNT, settings.labelCount.load());
            break;
        }

        case (unsigned int)Command::SETTING_OBJDETECTIONENABLED:
        {
            settings.objectDetectionEnabled = context.toBool();

            sendCommand(Command::SETTING_LABELCOUNT, settings.objectDetectionEnabled.load());
            break;
        }

        // implement later ...
    }
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
    // log("Received image frame with size: " + QString::number(data.size()) + " bytes.");
    if(!dlibWorker->isBusy())
    {
        // log("Received image frame with size: " + QString::number(data.size()) + " bytes. Processing it...");
        emit process(data);
    }

    if(dialog)
    {
        dlib::array2d<dlib::rgb_pixel> img(DLIBWorker::constructImgFromBuffer(data));

        QPixmap pixmap(QPixmap::fromImage(QImage((unsigned char*)dlib::image_data(img), img.nc(), img.nr(), QImage::Format_RGB888)));
        primaryDisplay->setPixmap(pixmap);
    }
}

void Client::processDlibWorkerFaceResults(const QVector<QPair<QRect, QString>>& results)
{
    // log("DLIB could not find any face in the given image frame!"); // this bloats the log
    if(results.size() > 0)
    {
        log("DLIB has found faces with following properties:");

        // send dlib output to client
        QVariantList ctx;
        for(const auto &it : results)
        {
            QVariantHash i;
            int x = it.first.x();
            int y = it.first.y();
            int width = it.first.width();
            int height = it.first.height();
            QString tag = it.second;

            i["x"] = x;
            i["y"] = y;
            i["width"] = width;
            i["height"] = height;
            i["tag"] = tag;

            ctx.push_back(i);
            log(QString("Tag: \"%0\" - X: %1, Y: %2, W: %3, H: %4")
                    .arg(tag)
                    .arg(x)
                    .arg(y)
                    .arg(width)
                    .arg(height));
        }
        log("Sending found face properties to the client.");
        sendCommand(Command::MESSAGE_TAG_FACE, ctx);

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

void Client::processDlibWorkerObjectResults(const QStringList &results)
{
    if (results.isEmpty())
        return;

    log("DLIB has found objects: " + results.join(", "));
    log("Sending found object properties to the client.");
    sendCommand(Command::MESSAGE_TAG_OBJECT, results);

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

            const QString& str = it;

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

void Client::sendTextMessage(const QString &string)
{
    socket->sendTextMessage(string);
}

void Client::sendBinaryMessage(const QByteArray &data)
{
    socket->sendBinaryMessage(data);
}

QListWidgetItem* Client::getListWidgetItem()
{
    return listItem;
}

ClientDialog* Client::getDialog()
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
