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

#include "ClientWorker.h"

#include <QThread>
#include <QJsonObject>
#include <QJsonDocument>

#include "DLIBWorker/DLIBWorker.h"
#include "Client/Client.h"

ClientWorker::ClientWorker(QObject *parent, QSettings *_config, Settings *_settings) :
    QObject(parent),
    config(_config),
    settings(_settings)
{
    assert(config);
    assert(settings);
}

ClientWorker::~ClientWorker()
{
    emit log("Stopping processing...");

    dlibWorkerThread.quit();
    dlibWorkerThread.wait();
}

void ClientWorker::processDlibWorkerFaceResults(const QVector<QPair<QRect, QString> > &results)
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
    }
}

void ClientWorker::processDlibWorkerObjectResults(const QVector<QPair<float, QString>> &results)
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

    log("DLIB has found objects: " + strList.join(","));
    log("Sending found object properties to the client.");
    sendCommand(Command::MESSAGE_TAG_OBJECT, list);
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

void ClientWorker::processBinaryMessage(const QByteArray &data)
{
    if(!dlibWorker->isBusy())
    {
        emit processImage(data);
    }
}

void ClientWorker::processCommand(ClientWorker::Command cmd, const QVariant &ctx)
{
    switch(cmd)
    {
        case Command::MESSAGE:
        {
            emit log(" says: " + ctx.toString());
            break;
        }

        case Command::SETTING_LABELCOUNT:
        {
            settings->labelCount = ctx.toULongLong();

            sendCommand(Command::SETTING_LABELCOUNT, settings->labelCount.load());
            break;
        }

        case Command::SETTING_OBJDETECTIONENABLED:
        {
            settings->objectDetectionEnabled = ctx.toBool();

            sendCommand(Command::SETTING_OBJDETECTIONENABLED, settings->objectDetectionEnabled.load());
            break;
        }

        case Command::SETTING_DETERMINISTICOBJECTDETECTION:
        {
            settings->deterministicObjectDetection = ctx.toBool();

            sendCommand(Command::SETTING_DETERMINISTICOBJECTDETECTION, settings->deterministicObjectDetection.load());
            break;
        }

        case Command::SETTING_FACERECOGNITIONENABLED:
        {
            settings->faceRecognitionEnabled = ctx.toBool();

            sendCommand(Command::SETTING_FACERECOGNITIONENABLED, settings->faceRecognitionEnabled.load());
            break;
        }

        default:
            break;
    }
}

void ClientWorker::init()
{
    QObject::connect(this, &ClientWorker::commandReceived, this, &ClientWorker::processCommand);

    dlibWorker = new DLIBWorker(config, settings);
    dlibWorker->moveToThread(&dlibWorkerThread);
    connect(&dlibWorkerThread, &QThread::finished, dlibWorker, &QObject::deleteLater);
    connect(this, &ClientWorker::processImage, dlibWorker, &DLIBWorker::process);
    connect(dlibWorker, &DLIBWorker::doneFace, this, &ClientWorker::processDlibWorkerFaceResults);
    connect(dlibWorker, &DLIBWorker::doneObject, this, &ClientWorker::processDlibWorkerObjectResults);
    connect(dlibWorker, &DLIBWorker::log, this, &ClientWorker::log);
    dlibWorkerThread.start();
}

