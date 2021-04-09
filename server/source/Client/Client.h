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

#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QVariant>
#include <QThread>

struct Settings
{
    std::atomic<bool> objectDetectionEnabled = true;
    std::atomic<size_t> labelCount = 5;
    std::atomic<bool> deterministicObjectDetection = false;
    std::atomic<bool> faceRecognitionEnabled = true;
};

class QWebSocket;
class QGraphicsPixmapItem;
class QTimer;
class QListWidgetItem;
class ClientDialog;
class QThread;
class QSettings;
class ClientWorker;

class Client : public QObject
{
    Q_OBJECT

    Settings settings;

    QWebSocket* socket;

    QString name {"?"};

    QGraphicsPixmapItem* primaryDisplay;
    QGraphicsPixmapItem* secondaryDisplay;
    QGraphicsPixmapItem* tertiaryDisplay;

    QTimer* clearSecondaryDisplayTimer;

    QListWidgetItem* listItem = nullptr;
    ClientDialog* dialog = nullptr;

    QThread workerThread;
    ClientWorker *worker = nullptr;

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
        SETTING_FACERECOGNITIONENABLED = 8
    };

    explicit Client(QObject *parent, QWebSocket* _socket, QSettings* config);
    ~Client();

    void setPrimaryDisplayItem(QGraphicsPixmapItem* item);
    void setSecondaryDisplayItem(QGraphicsPixmapItem* item);
    void setTertiaryDisplayItem(QGraphicsPixmapItem* item);

    QString getName() const;

    QListWidgetItem* getListWidgetItem() const;
    ClientDialog* getDialog() const;

    void setListWidgetItem(QListWidgetItem* _listItem);
    void setDialog(ClientDialog* _dialog);

private:
    qint64 sendTextMessage(const QString& ctx);
    qint64 sendBinaryMessage(const QByteArray& ctx);

public slots:
    void processCommand(Client::Command cmd, const QVariant& ctx);
    void processBinaryMessage(const QByteArray& data);

    void sendCommand(Command cmd, const QVariant& ctx = QVariant());

private slots:
    void processDlibWorkerFaceResults(const QVector<QPair<QRect, QString>>& results);
    void processDlibWorkerObjectResults(const QVector<QPair<float, QString>>& results);

public:
signals:
    void clientNameChanged(const QString& name);
    void log(const QString& str);
};

Q_DECLARE_METATYPE(Client::Command);

#endif // CLIENT_H
