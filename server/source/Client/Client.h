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

class Client : public QObject
{
    Q_OBJECT

private:
    enum class Command : unsigned char
    {
        CHANGE_NAME = '0',
        MESSAGE = '1'
    };

    class QWebSocket* socket;

    QString name;

    class QGraphicsPixmapItem* primaryDisplay;
    class QGraphicsPixmapItem* secondaryDisplay;

    class DLIBWorker *dlibWorker;
    bool dlibWorkerFree;

    class QTimer* clearSecondaryDisplayTimer;

    class QListWidgetItem* listItem;
    class ClientDialog* dialog;

public:
    explicit Client(QObject *parent, QWebSocket* _socket, class QSettings* config);
    ~Client();

    void sendTextMessage(const QString& string);
    void sendBinaryMessage(const QByteArray& data);

    void setPrimaryDisplayItem(QGraphicsPixmapItem* item);
    void setSecondaryDisplayItem(QGraphicsPixmapItem* item);

    QString getName() const;

    QListWidgetItem* getListWidgetItem();
    ClientDialog* getDialog();

    void setListWidgetItem(QListWidgetItem* _listItem);
    void setDialog(ClientDialog* _dialog);

public slots:
    void processTextMessage(const QString& string);
    void processBinaryMessage(const QByteArray& data);

private slots:
    void processDlibWorkerResults(const QVector<QPair<QRect, QString>>& results);
    void throwException(const char* str);

public:
signals:
    void clientNameChanged(const QString& name);
    void log(const QString& str);

};

#endif // CLIENT_H
