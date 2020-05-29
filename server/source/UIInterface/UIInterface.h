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

#ifndef UIINTERFACE
#define UIINTERFACE

#include <QObject>
#include <QString>

#include "MainWindow/MainWindow.h"

class UIInterface : public QObject
{
    Q_OBJECT

private:
    MainWindow*& mainWindow;

    class QTextEdit*    serverLog;
    class QListWidget*  clientList;

    bool    m_logEnabled;
    QString m_logFile;

    static QString generateDateTime();

    void logEvent(const QString& string, class Client* client = nullptr);
    class ClientDialog* findClientDialog(class Client* client);

    QHash<class QListWidgetItem *, class Client*>* clientListItemMap;
    QHash<class ClientDialog*, class Client *>*    clientDialogMap;

public:
    explicit UIInterface(QObject* parent, MainWindow*& _mainWindow, class QSettings *config);
    ~UIInterface();

    void log(const QString& string);
    void newClient(class Client* client);
    void removeClient(class Client* client);
    void setClientName(const QString& name);


private slots:
    void on_clientList_itemDoubleClicked(class QListWidgetItem *item);

public slots:
    void clientLog(const QString& string);

private slots:
    void on_ClientDialog_destroy();

};

#endif // UIINTERFACE
