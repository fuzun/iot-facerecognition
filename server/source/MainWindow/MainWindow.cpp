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
#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QListWidgetItem>
#include <QMessageBox>

#include "Server/Server.h"
#include "Client/Client.h"
#include "ClientDialog/ClientDialog.h"

#include "config.h"

MainWindow::MainWindow(QWidget *parent, Server *_server)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , server(_server)
{
    ui->setupUi(this);
    setWindowTitle(windowTitle() + " - v" + VERSION);

    connect(ui->clientList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
        const auto ptrClient = clientListItemMap[item];

        ClientDialog* clientDialog = new ClientDialog(ptrClient, this);

        clientDialog->setWindowTitle(getClientName(ptrClient));

        connect(ptrClient, &Client::nameChanged, clientDialog, &ClientDialog::setWindowTitle);

        connect(clientDialog, &ClientDialog::finished, this, [this]() {
            ClientDialog* dialog = qobject_cast<ClientDialog *>(sender());

            if(dialog)
                dialog->deleteLater(); // really needed?
        });

        clientDialog->show();
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::newClient(Client *client)
{
    if (!client)
        return;

    connect(client, &Client::log, this, [this](const QString& msg) {
        const auto ptrClient = qobject_cast<Client*>(sender());

        print(QString("<i>%1</i>: %2").arg(getClientName(ptrClient)).arg(msg));
    });

    QListWidgetItem* item = new QListWidgetItem(client->getName());

    clientListItemMap[item] = client;
    connect(client, &Client::nameChanged, this, [item](const QString& name) {
        item->setText(name);
    });

    connect(client, &Client::destroyed, this, [item]() {
        delete item;
    });
    ui->clientList->addItem(item);
}

void MainWindow::on_quitButton_clicked()
{
    emit end();
}

void MainWindow::on_aboutButton_clicked()
{
    QMessageBox::information(this, "About...", QString("iot-facerecognition-server<br/>Address: <a href=\"https://github.com/fuzun/iot-facerecognition\">GitHub Repository</a><br/>Author: fuzun<br/>Version: ") + VERSION);
}

QString MainWindow::getClientName(Client *client)
{
    QString name;
    QMetaObject::invokeMethod(client, "getName", Qt::BlockingQueuedConnection, Q_RETURN_ARG(QString, name));
    return name;
}

void MainWindow::print(const QString &message)
{
    ui->serverLog->append(QString("<b>[%1]</b> %2 <br/>").arg(Server::generateDateTime(), message));
}
