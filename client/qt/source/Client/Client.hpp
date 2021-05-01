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

#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QSslError>
#include <QThread>

#include "ClientWorker.hpp"
#include "common/Log.hpp"

class ClientWorker;
class ClientSettings;
class SettingObject;
class Statistics;
class QTimer;

class Client : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QList<SettingObject *> settingModel READ settingModel NOTIFY settingModelChanged)
    Q_PROPERTY(ClientSettings* settings READ settings CONSTANT)
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)

    Q_PROPERTY(qint64 receivedBytes READ receivedBytes WRITE setReceivedBytes NOTIFY receivedBytesChanged)
    Q_PROPERTY(qint64 sentBytes READ sentBytes WRITE setSentBytes NOTIFY sentBytesChanged)

    Q_PROPERTY(qint64 totalReceivedBytes READ totalReceivedBytes NOTIFY totalReceivedBytesChanged)
    Q_PROPERTY(qint64 totalSentBytes READ totalSentBytes NOTIFY totalSentBytesChanged)


    QThread workerThread;
    ClientWorker *worker = nullptr;

    bool m_connected = false;

    QTimer *m_trafficMeter = nullptr;
    size_t m_elapsedTime = 0;
    qint64 m_receivedBytes[2] { 0 };
    qint64 m_sentBytes[2] { 0 };

    inline static const auto keyContext { ClientWorker::keyContext };
    inline static const auto keyCommand { ClientWorker::keyCommand };

private:
    ClientSettings* m_settings;
    QList<SettingObject *> m_settingModel;

    void setupSettings();

private slots:
    void processCommand(ClientWorker::Command cmd, const QVariant& ctx = QVariant());

public:
    explicit Client(QObject *parent = nullptr, Statistics *statistics = nullptr);
    ~Client();

    using Command = ClientWorker::Command;

    QString serverAddress();
    quint16 serverPort();

    ClientSettings* settings() const;
    bool connected() const;

    QList<SettingObject *> settingModel() const;

    qint64 receivedBytes() const;

    qint64 sentBytes() const;

    qint64 totalReceivedBytes() const;

    qint64 totalSentBytes() const;

public slots:
    void setReceivedBytes(qint64 receivedBytes);

    void setSentBytes(qint64 sentBytes);

signals:
    void log(const QVariant& str, Log::LogType type = Log::INFORMATION);

    void faceTagReceived(const QVariantList &);
    void objectTagReceived(const QVariantList &);

    void connectedChanged(bool connected);
    void settingModelChanged(QList<SettingObject *> settingModel);

    Q_INVOKABLE void connect(const QUrl& url);
    Q_INVOKABLE void disconnect();

    void sendCommand(ClientWorker::Command cmd, const QVariant& ctx = QVariant());
    Q_INVOKABLE void sendMessage(const QString& string);

    void sendData(const QByteArray& data);
    void receivedBytesChanged(qint64 receivedBytes);
    void sentBytesChanged(qint64 sentBytes);
    void totalReceivedBytesChanged(qint64 totalReceivedBytes);
    void totalSentBytesChanged(qint64 totalSentBytes);
};

#endif // CLIENT_H
