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

#include "DLIBWorker/DLIBWorker.h"
#include "ClientDialog/ClientDialog.h"
#include "ClientHandler/ClientHandler.h"

Client::Client(QObject *parent, QWebSocket* _socket, QSettings* config)
    : QObject(parent)
    , socket(_socket)
{
    dialog = nullptr;
    listItem = nullptr;
    name = QString("?");

    dlibWorkerThread = new QThread(this);
    dlibWorker = new DLIBWorker(config);
    dlibWorker->moveToThread(dlibWorkerThread);
    connect(dlibWorker, &DLIBWorker::throwException, this, &Client::throwException);
    connect(dlibWorkerThread, &QThread::finished, dlibWorker, &QObject::deleteLater);
    connect(this, &Client::process, dlibWorker, &DLIBWorker::process);
    connect(dlibWorker, &DLIBWorker::done, this, &Client::processDlibWorkerResults);
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

    qRegisterMetaType<QVector<QPair<QRect, QString>>>();
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

void Client::throwException(const std::string& str)
{
    std::cout << "FATAL ERROR: " << str << std::endl;
    throw str;
}

void Client::processTextMessage(const QString& string)
{
    QStringList list = string.split(":");
    if(list.size() != 2)
        return;

    const QString& strCommand = list.at(0);
    const QString& strContext = list.at(1);
    if(strCommand.length() == 1 && strContext >= 0)
    {
        const unsigned char cmd = strCommand.toStdString().c_str()[0];

        switch(cmd)
        {
            // Client wants to change its name:
            case (int)Command::CHANGE_NAME:
            {
                ClientHandler* cHandler = qobject_cast<ClientHandler *>(parent());
                QString _name = strContext;
                if(cHandler->isClientPresent(strContext))
                {
                    _name.append("(1)");
                }

                if(name == "?")
                {
                    log("Client has connected!");
                }
                name = _name;
                emit clientNameChanged(name);
                break;
            }

            // Client wants to send a message:
            case (int)Command::MESSAGE:
            {
                emit log(" says: " + strContext);
                break;
            }

            // implement later ...
        }
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

void Client::processDlibWorkerResults(const QVector<QPair<QRect, QString>>& results)
{
    // log("DLIB could not find any face in the given image frame!");
    if(results.size() > 0)
    {
        log("DLIB has found faces with following properties:");
        // send dlib output to client
        QString buffer;
        for(const auto &it : results)
        {
            buffer += QString::number(it.first.x()) + "," + QString::number(it.first.y()) + "," + QString::number(it.first.width() + it.first.x()) + "," + QString::number(it.first.height() + it.first.y()) + "," + it.second + ":";
            log(QString("Tag: \"%0\" - X: %1, Y: %2, W: %3, H: %4").arg(it.second).arg(QString::number(it.first.x())).arg(QString::number(it.first.y())).arg(QString::number(it.first.width())).arg(QString::number(it.first.height())));
        }
        buffer.chop(1);
        log("Sending found face properties to the client.");
        sendTextMessage(buffer);

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
