/*
 * iot-facerecognition-server
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
#include <QVariant>

#include <QThread>

class QWebSocket;
class QGraphicsPixmapItem;
class QTimer;
class QListWidgetItem;
class QThread;
class QSettings;
class DLIBWorker;

struct Settings
{
    std::atomic<bool> objectDetectionEnabled = true;
    std::atomic<size_t> labelCount = 5;
    std::atomic<bool> deterministicObjectDetection = false;
    std::atomic<bool> faceRecognitionEnabled = true;
};

class Client : public QObject
{
    Q_OBJECT

    Settings settings;
    DLIBWorker *dlibWorker = nullptr;
    QString name {"?"};

    QThread dlibWorkerThread;

protected:
    QWebSocket *socket = nullptr;


public:
    inline static const char * keyCommand = "command";
    inline static const char * keyContext = "context";

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

    explicit Client(QObject *parent, QWebSocket* _socket, QSettings* _config);
    ~Client();


private:
    qint64 sendTextMessage(const QString& ctx);
    qint64 sendBinaryMessage(const QByteArray& ctx);

public slots:
    void sendCommand(Client::Command cmd, const QVariant& ctx = QVariant());
    QString getName() const;

private slots:
    void processFaceResults(const QVector<QPair<QRect, QString>>& results);
    void processObjectResults(const QVector<QPair<float, QString>> &results);

    void processTextMessage(const QString& message);
    void processBinaryMessage(const QByteArray& data);
    void processCommand(Client::Command cmd, const QVariant& ctx);

public:
signals:
    void nameChanged(const QString& name);
    void commandReceived(Client::Command cmd, const QVariant& ctx);

    void primaryDisplayUpdated(const QPixmap& pixmap);

    void doneFace(const QVector<QPair<QRect, QString>>&);
    void doneObject(const QVector<QPair<float, QString>> &);

    void log(const QString& str);

private:
signals:
    void processImage(const QByteArray& buffer);

};

Q_DECLARE_METATYPE(Client::Command);

#endif // CLIENT_H
