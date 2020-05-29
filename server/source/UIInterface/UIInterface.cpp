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

#include "UIInterface.h"

#include <QDateTime>
#include <QListWidgetItem>
#include <QTextEdit>
#include <QHash>
#include <QImage>
#include <QPixmap>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QRegExp>
#include <QCloseEvent>
#include <QSettings>
#include <QFile>
#include <QTextStream>
#include <iostream>

#include "MainWindow/MainWindow.h"
#include "ui_MainWindow.h"
#include "ClientDialog/ClientDialog.h"
#include "ui_ClientDialog.h"
#include "Client/Client.h"
#include "config.h"

UIInterface::UIInterface(QObject* parent, MainWindow *&_mainWindow, QSettings* config)
    : QObject(parent)
    , mainWindow(_mainWindow)
{
    config->beginGroup(CONFIG_LOG);
    m_logEnabled = config->value(CONFIG_LOG_ENABLE, CONFIG_LOG_DEFAULT_ENABLE).toBool();
    m_logFile = config->value(CONFIG_LOG_LOCATION, CONFIG_LOG_DEFAULT_LOCATION).toString();
    config->endGroup();

    if(m_logEnabled)
    {
        logFile = new QFile(m_logFile, this);
        if(!logFile->open(QFile::Text | QFile::Append))
        {
            m_logEnabled = false;
            log("Can't write to log file. Logging to file is disabled.");
            fStream = nullptr;
        }
        else
        {
            fStream = new QTextStream(logFile);
        }
    }

    clientList = mainWindow->ui->clientList;
    serverLog = mainWindow->ui->serverLog;

    if(mainWindow)
    {
        clientListItemMap = new QHash<class QListWidgetItem *, class Client *>;
        clientDialogMap = new QHash<class ClientDialog *, class Client *>;
        connect(clientList, &QListWidget::itemDoubleClicked, this, &UIInterface::on_clientList_itemDoubleClicked);
    }
}

UIInterface::~UIInterface()
{
    if(fStream)
    {
        delete fStream;
    }

    if(logFile->isOpen())
    {
        logFile->flush();
        logFile->close();
    }

    delete clientDialogMap;
    delete clientListItemMap;
}

QString UIInterface::generateDateTime()
{
    QDateTime dateTime = QDateTime::currentDateTime();
    return dateTime.toString("dd-MM-yyyy / hh:mm:ss");
}

void UIInterface::logEvent(const QString& string, class Client *client)
{
    QString stamp = generateDateTime();

    QString message = "<b>[" + stamp + "]</b> ";
    QString message2 = message;
    if(client)
    {
        message2 += " <i>" + client->getName() + "</i>: ";
    }
    message += string;
    message2 += string;

    QRegExp htmlTagMark("<[^>]*>");
    if(client)
    {
        if(ClientDialog* dialog = client->getDialog())
        {
            dialog->ui->clientLog->append(message + "<br/>");
        }
        if(mainWindow)
        {
            serverLog->append(message2 + "<br/>");
        }
        QString stripped = message2.remove(htmlTagMark);
        std::cout << stripped.toStdString() << std::endl;
        if(fStream)
            *fStream << stripped << "\n";
    }
    else
    {
        if(mainWindow)
        {
            serverLog->append(message + "<br/>");
        }
        QString stripped = message.remove(htmlTagMark);
        std::cout << stripped.toStdString() << std::endl;
        if(fStream)
            *fStream << stripped << "\n";
    }
}

void UIInterface::log(const QString &string)
{
    logEvent(string, nullptr);
}

void UIInterface::clientLog(const QString& string)
{
    Client *client = qobject_cast<Client*>(sender());
    logEvent(string, client);
}

void UIInterface::setClientName(const QString& name)
{
    Client* client = qobject_cast<Client *>(sender());
    client->getListWidgetItem()->setText(name);
}

void UIInterface::newClient(Client *client)
{
    if(!client)
        return;

    QListWidgetItem* item = new QListWidgetItem(client->getName());
    clientList->addItem(item);
    client->setListWidgetItem(item);

    clientListItemMap->insert(item, client);

    connect(client, &Client::clientNameChanged, this, &UIInterface::setClientName);
    connect(client, &Client::log, this, &UIInterface::clientLog, Qt::ConnectionType::QueuedConnection);
}

void UIInterface::removeClient(Client *client)
{
    QListWidgetItem* item = client->getListWidgetItem();
    clientList->removeItemWidget(item);
    clientListItemMap->remove(item);

    ClientDialog* dialog = client->getDialog();
    if(dialog)
    {
        client->setDialog(nullptr);
        dialog->close();
    }

    client->setListWidgetItem(nullptr);
    delete item;
}

void UIInterface::on_clientList_itemDoubleClicked(QListWidgetItem *item)
{
    Client* client = clientListItemMap->find(item).value();

    if(!client->getDialog())
    {
        ClientDialog* clientDialog = new ClientDialog(mainWindow);
        client->setDialog(clientDialog);
        if(client->getDialog())
        {
            client->getDialog()->setWindowTitle(client->getName());
        }

        QGraphicsScene* scene = new QGraphicsScene(clientDialog->ui->graphicsView);
        clientDialog->ui->graphicsView->setScene(scene);

        QGraphicsPixmapItem* primary = new QGraphicsPixmapItem();
        QGraphicsPixmapItem* secondary = new QGraphicsPixmapItem();
        scene->addItem(primary);
        scene->addItem(secondary);

        clientDialogMap->insert(clientDialog, client);
        connect(clientDialog, &ClientDialog::sDestroy, this, &UIInterface::on_ClientDialog_destroy);
        clientDialog->show();

        client->setPrimaryDisplayItem(primary);
        client->setSecondaryDisplayItem(secondary);

    }
}

void UIInterface::on_ClientDialog_destroy()
{
    ClientDialog* dialog = qobject_cast<ClientDialog *>(sender());
    Client* client = clientDialogMap->find(dialog).value();
    clientDialogMap->remove(dialog);
    if(dialog)
        dialog->destroy();
    client->setDialog(nullptr);
}
