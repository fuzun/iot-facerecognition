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

#include "Client.hpp"

#include <QWebSocket>
#include <QString>
#include <QRect>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QTimer>
#include <QQmlEngine>

#include "ClientWorker.hpp"
#include "ClientSettings.hpp"
#include "common/SettingObject.hpp"
#include "Statistics/Statistics.hpp"

Client::Client(QObject *parent, Statistics *statistics) :
    QObject(parent),
    m_settings(new ClientSettings(this))
{
    qRegisterMetaType<Command>();
    qRegisterMetaType<QList<QSslError>>();

    qmlRegisterUncreatableType<ClientSettings>("com.iotfacerecognition.settings", 1, 0, "ClientSettings", "");

    worker = new ClientWorker;
    worker->moveToThread(&workerThread);

    QObject::connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    workerThread.start();

    QMetaObject::invokeMethod(worker, &ClientWorker::init, Qt::BlockingQueuedConnection);


    QObject::connect(worker, &ClientWorker::log, this, &Client::log);
    QObject::connect(worker, &ClientWorker::faceTagReceived, this, &Client::faceTagReceived);
    QObject::connect(worker, &ClientWorker::objectTagReceived, this, &Client::objectTagReceived);
    QObject::connect(worker, &ClientWorker::commandReceived, this, &Client::processCommand);
    QObject::connect(worker, &ClientWorker::receivedBytes, this, [this](qint64 bytes) {
        setReceivedBytes(bytes);
    });
    QObject::connect(worker, &ClientWorker::sentBytes, this, [this](qint64 bytes) {
        setSentBytes(bytes);
    });
    QObject::connect(worker, &ClientWorker::initRequest, this, [this]() {
        for (const auto &i : std::as_const(m_settingModel))
        {
            i->invokeSlot();
        }
    });

    QObject::connect(this, &Client::sendCommand, worker, &ClientWorker::sendCommand);
    QObject::connect(this, &Client::sendMessage, worker, &ClientWorker::sendMessage);
    QObject::connect(this, &Client::connect, worker, &ClientWorker::connect);
    QObject::connect(this, &Client::disconnect, worker, &ClientWorker::disconnect);
    QObject::connect(this, &Client::sendData, worker, &ClientWorker::sendData);

    QObject::connect(worker->getSocket(), &QWebSocket::sslErrors, this, [this](const QList<QSslError> &errors) {
            const auto ignoreSSLErrors = m_settings->ignoreSSLErrors();

            for (const auto& it : errors)
            {
                emit log(QString("QWebSocket: SSL Error %1").arg(it.errorString()), ignoreSSLErrors ? Log::WARNING : Log::ERROR);
            }

            if (ignoreSSLErrors)
                worker->socket->ignoreSslErrors();
        }, Qt::BlockingQueuedConnection);

    QObject::connect(worker->getSocket(), &QWebSocket::connected, this, [this]() {
        m_connected = true;
        emit connectedChanged(m_connected);
    });
    QObject::connect(worker->getSocket(), &QWebSocket::disconnected, this, [this]() {
        m_connected = false;
        emit connectedChanged(m_connected);
    });

    QObject::connect(this, &Client::connectedChanged, this, [this](bool connected) {
        if (connected)
        {
           emit log(QString("Connected to %1:%2!").arg(serverAddress()).arg(serverPort()));
        }
        else
        {
           emit log(QString("Disconnected!"));
        }
    });

    m_trafficMeter = new QTimer(this);
    m_trafficMeter->setInterval(1000);

    const auto _common = [this]() {
        m_sentBytes[1] = m_sentBytes[0];
        m_receivedBytes[1] = m_receivedBytes[0];

        ++m_elapsedTime;
    };

    if (statistics)
    {
        statistics->registerSeries(Statistics::MaximumObjectPredictionRateLineSeries);

        statistics->registerSeries(Statistics::InternetUsageSentLineSeries);
        statistics->registerSeries(Statistics::InternetUsageReceivedLineSeries);
        statistics->registerSeries(Statistics::InternetUsageSentCumulativeLineSeries);
        statistics->registerSeries(Statistics::InternetUsageReceivedCumulativeLineSeries);

        statistics->registerSeries(Statistics::TaggedFacesLineSeries);
        statistics->registerSeries(Statistics::UntaggedFacesLineSeries);

        statistics->registerSeries(Statistics::RecognizedMaxPredictionObjectsPieSeries);
        statistics->registerSeries(Statistics::RecognizedFacesPieSeries);

        QObject::connect(m_trafficMeter, &QTimer::timeout, this, [this, statistics, _common]() {
            statistics->addData(Statistics::InternetUsageSentLineSeries, QPointF(m_elapsedTime, sentBytes() / 1024.f));
            statistics->addData(Statistics::InternetUsageReceivedLineSeries, QPointF(m_elapsedTime, receivedBytes() ));


            statistics->addData(Statistics::InternetUsageSentCumulativeLineSeries, QPointF(QDateTime::currentMSecsSinceEpoch(), m_sentBytes[0] / 1024.f / 1024.f));
            statistics->addData(Statistics::InternetUsageReceivedCumulativeLineSeries, QPointF(QDateTime::currentMSecsSinceEpoch(), m_receivedBytes[0] ));

            _common();
        });

        QObject::connect(worker, &ClientWorker::objectTagReceived, this, [statistics](const QVariantList& list) {
            statistics->addData(Statistics::MaximumObjectPredictionRateLineSeries,
                QPointF(QDateTime::currentMSecsSinceEpoch(),
                    list[0].toMap()["prediction"].toFloat()));


            statistics->incrementData(Statistics::RecognizedMaxPredictionObjectsPieSeries, list[0].toMap()["label"].toString() );
        });

        QObject::connect(worker, &ClientWorker::faceTagReceived, this, [statistics](const QVariantList& list) {
            int taggedFaces =0 , untaggedFaces = 0;
            for (const auto& i : list)
            {
                const auto map = i.toMap();


                if (map["tag"].toString().isEmpty())
                    ++untaggedFaces;
                else
                    ++taggedFaces;

                QString tag = map["tag"].toString();
                if (tag.isEmpty())
                    tag = QString("Untagged");

                statistics->incrementData(Statistics::RecognizedFacesPieSeries, tag);
            }

            statistics->addData(Statistics::TaggedFacesLineSeries, QPointF(QDateTime::currentMSecsSinceEpoch(), taggedFaces));
            statistics->addData(Statistics::UntaggedFacesLineSeries, QPointF(QDateTime::currentMSecsSinceEpoch(), untaggedFaces));
        });

        m_trafficMeter->start();
    }
    else
    {
        QObject::connect(m_trafficMeter, &QTimer::timeout, this, _common);
    }

    setupSettings();
}

Client::~Client()
{
    workerThread.quit();
    workerThread.wait();
}

void Client::setupSettings()
{
    using SettingType = SettingObject::SettingType;

    static const auto& newSetting = SettingObject::newSetting;
    static const auto& makePropPair = SettingObject::makePropertyPair;

    newSetting(m_settingModel, this,
        {
            tr("Client Name"),
            {{SettingObject::keyType, SettingType::TextField}},
            makePropPair(m_settings, "clientName"),
            [this](const QVariant& value) { sendCommand(Client::Command::SETTING_NAME, value); }
        });

    newSetting(m_settingModel, this,
        {
            tr("Maximum label count for object recognition:"),
            {{SettingObject::keyType, SettingType::TextFieldInt},
                {SettingObject::keyMin, 1},
                {SettingObject::keyMax, 10}},
            makePropPair(m_settings, "labelCount"),
            [this](const QVariant& value) { sendCommand(Client::Command::SETTING_LABELCOUNT, value); }
        });

    newSetting(m_settingModel, this,
        {
            tr("Should the server try to detect and recognize objects?"),
            {{SettingObject::keyType, SettingType::ToggleButton}},
            makePropPair(m_settings, "objectDetectionEnabled"),
            [this](const QVariant& value) { sendCommand(Client::Command::SETTING_OBJDETECTIONENABLED, value); }
        });

    newSetting(m_settingModel, this,
        {
            tr("Should the server try to detect and recognize faces?"),
            {{SettingObject::keyType, SettingType::ToggleButton}},
            makePropPair(m_settings, "faceRecognitionEnabled"),
            [this](const QVariant& value) { sendCommand(Client::Command::SETTING_FACERECOGNITIONENABLED, value); }
        });

    newSetting(m_settingModel, this,
        {
            tr("Should the object detection be deterministic? (no randomization)"),
            {{SettingObject::keyType, SettingType::ToggleButton}},
            makePropPair(m_settings, "deterministicObjectDetection"),
            [this](const QVariant& value) { sendCommand(Client::Command::SETTING_DETERMINISTICOBJECTDETECTION, value); }
        });

    newSetting(m_settingModel, this,
        {
            tr("Should SSL errors be ignored? (WARNING: Only enable this for testing purposes!)"),
            {{SettingObject::keyType, SettingType::ToggleButton}},
            makePropPair(m_settings, "ignoreSSLErrors")
        });

    emit settingModelChanged(m_settingModel);
}

void Client::processCommand(ClientWorker::Command cmd, const QVariant &ctx)
{
    switch(cmd)
    {
    case Command::SETTING_NAME:
    {
        m_settings->setClientName(ctx.toString());
        break;
    }

    case Command::SETTING_LABELCOUNT:
    {
        m_settings->setLabelCount(ctx.toULongLong());
        break;
    }

    case Command::SETTING_OBJDETECTIONENABLED:
    {
        m_settings->setObjectDetectionEnabled(ctx.toBool());
        break;
    }

    case Command::SETTING_DETERMINISTICOBJECTDETECTION:
    {
        m_settings->setDeterministicObjectDetection(ctx.toBool());
        break;
    }

    case Command::SETTING_FACERECOGNITIONENABLED:
    {
        m_settings->setFaceRecognitionEnabled(ctx.toBool());
        break;
    }

    default:
        break;
    }
}

QString Client::serverAddress()
{
    assert(worker);

    QString ret;
    QMetaObject::invokeMethod(worker, &ClientWorker::serverAddress, Qt::BlockingQueuedConnection, &ret);

    return ret;
}

quint16 Client::serverPort()
{
    assert(worker);

    QVariant ret;
    QMetaObject::invokeMethod(worker, &ClientWorker::serverPort, Qt::BlockingQueuedConnection, &ret);

    return ret.toUInt();
}

ClientSettings *Client::settings() const
{
    return m_settings;
}

bool Client::connected() const
{
    return m_connected;
}

QList<SettingObject *> Client::settingModel() const
{
    return m_settingModel;
}

qint64 Client::receivedBytes() const
{
    const auto ret = m_receivedBytes[0] - m_receivedBytes[1];

    return ret < 0 ? 0 : ret;
}

qint64 Client::sentBytes() const
{
    const auto ret = m_sentBytes[0] - m_sentBytes[1];

    return ret < 0 ? 0 : ret;
}

qint64 Client::totalReceivedBytes() const
{
    return m_receivedBytes[0];
}

qint64 Client::totalSentBytes() const
{
    return m_sentBytes[0];
}

void Client::setReceivedBytes(qint64 receivedBytes)
{
    if (receivedBytes < 0)
        return;

    m_receivedBytes[0] += receivedBytes;
    emit receivedBytesChanged(m_receivedBytes[0]);

    emit totalReceivedBytesChanged(totalReceivedBytes());
}

void Client::setSentBytes(qint64 sentBytes)
{
    if (sentBytes < 0)
        return;

    m_sentBytes[0] += sentBytes;
    emit sentBytesChanged(m_sentBytes[0]);

    emit totalSentBytesChanged(totalSentBytes());
}

