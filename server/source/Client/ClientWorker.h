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
#ifndef CLIENTWORKER_H
#define CLIENTWORKER_H

#include <QObject>
#include <QVariant>
#include <QThread>

#include "Client.h"

class DLIBWorker;
class QSettings;
struct Settings;

class ClientWorker : public QObject
{
    Q_OBJECT

    friend class Client;

    using Command = Client::Command;

    QSettings *config = nullptr;
    Settings *settings = nullptr;

    DLIBWorker *dlibWorker = nullptr;

    inline const DLIBWorker* getDlibWorker() const { return dlibWorker; };

    QThread dlibWorkerThread;

    inline static const auto keyCommand {Client::keyCommand};
    inline static const auto keyContext {Client::keyContext};

public:
    explicit ClientWorker(QObject *parent = nullptr,
        QSettings *_config = nullptr,
        Settings *_settings = nullptr);

    ~ClientWorker();

private:
signals:
    void processImage(const QByteArray& buffer);
    void sendCommand(Client::Command cmd, const QVariant& ctx = QVariant());

private slots:
    void processDlibWorkerFaceResults(const QVector<QPair<QRect, QString>>& results);
    void processDlibWorkerObjectResults(const QStringList& results);

    void processTextMessage(const QString& message);
    void processBinaryMessage(const QByteArray& data);
    void processCommand(Client::Command cmd, const QVariant& ctx);

    void init();

public slots:


signals:
    void log(const QString& str);
    void throwException(const std::exception &e);
    void commandReceived(Client::Command cmd, const QVariant& ctx);

};

#endif // CLIENTWORKER_H
