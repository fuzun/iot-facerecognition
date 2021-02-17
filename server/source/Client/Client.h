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

struct Settings
{
    std::atomic<bool> objectDetectionEnabled = true;
    std::atomic<size_t> labelCount = 5;
    std::atomic<bool> deterministicObjectDetection = false;
    std::atomic<bool> faceRecognitionEnabled = true;
};

class QWebSocket;
class QGraphicsPixmapItem;
class DLIBWorker;
class QTimer;
class QListWidgetItem;
class ClientDialog;
class QThread;
class QSettings;

class Client : public QObject
{
    Q_OBJECT

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

    Settings settings;

    QWebSocket* socket;

    QString name {"?"};

    QGraphicsPixmapItem* primaryDisplay;
    QGraphicsPixmapItem* secondaryDisplay;
    QGraphicsPixmapItem* tertiaryDisplay;

    DLIBWorker *dlibWorker;

    QTimer* clearSecondaryDisplayTimer;

    QListWidgetItem* listItem = nullptr;
    ClientDialog* dialog = nullptr;

    QThread* dlibWorkerThread;

    void sendCommand(Command cmd, const QVariant& ctx = QVariant());

public:
    explicit Client(QObject *parent, QWebSocket* _socket, QSettings* config);
    ~Client();

    void sendTextMessage(const QString& string);
    void sendBinaryMessage(const QByteArray& data);

    void setPrimaryDisplayItem(QGraphicsPixmapItem* item);
    void setSecondaryDisplayItem(QGraphicsPixmapItem* item);
    void setTertiaryDisplayItem(QGraphicsPixmapItem* item);

    QString getName() const;

    QListWidgetItem* getListWidgetItem() const;
    ClientDialog* getDialog() const;

    void setListWidgetItem(QListWidgetItem* _listItem);
    void setDialog(ClientDialog* _dialog);

public slots:
    void processTextMessage(const QString& string);
    void processBinaryMessage(const QByteArray& data);

private slots:
    void processDlibWorkerFaceResults(const QVector<QPair<QRect, QString>>& results);
    void processDlibWorkerObjectResults(const QStringList& results);

    void throwException(const std::exception &e);

public:
signals:
    void clientNameChanged(const QString& name);
    void log(const QString& str);
    void process(const QByteArray& buffer);

};

#endif // CLIENT_H
