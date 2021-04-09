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

#include "ClientDialog.h"
#include "ui_ClientDialog.h"

#include <QGraphicsScene>

#include "Client/Client.h"
#include "Server/Server.h"

ClientDialog::ClientDialog(Client* client, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ClientDialog)
{
    assert(client);

    ui->setupUi(this);

    QGraphicsScene* scene = new QGraphicsScene(ui->graphicsView);
    ui->graphicsView->setScene(scene);

    scene->addItem(&primaryDisplay);
    scene->addItem(&secondaryDisplay);
    scene->addItem(&tertiaryDisplay);

    secondaryDisplayCleaner.setInterval(3000);
    connect(&secondaryDisplayCleaner, &QTimer::timeout, this, [this]() {
        secondaryDisplay.setPixmap(QPixmap());
    });

    tertiaryDisplayCleaner.setInterval(3000);
    connect(&tertiaryDisplayCleaner, &QTimer::timeout, this, [this]() {
        tertiaryDisplay.setPixmap(QPixmap());
    });

    connect(client, &Client::primaryDisplayUpdated, this, &ClientDialog::updatePrimaryDisplay);
    connect(client, &Client::doneFace, this, &ClientDialog::drawFaceIdentificators);
    connect(client, &Client::doneObject, this, &ClientDialog::drawTextBox);

    connect(client, &Client::destroyed, this, &ClientDialog::accept);

    connect(client, &Client::log, this, &ClientDialog::print);
}

ClientDialog::~ClientDialog()
{
    delete ui;
}

void ClientDialog::on_okButton_clicked()
{
    accept();
}

void ClientDialog::updatePrimaryDisplay(const QPixmap &pixmap)
{
    Client *client = qobject_cast<Client*>(sender());

    if (!client)
        return;

    primaryDisplay.setPixmap(pixmap);
}

void ClientDialog::drawTextBox(const QVector<QPair<float, QString> > &list)
{
    secondaryDisplayCleaner.stop();

    QPixmap pixmap(primaryDisplay.pixmap().width(), primaryDisplay.pixmap().height());
    pixmap.fill(Qt::transparent);

    int counter = 0;
    for (const auto& it : list)
    {
        QPainter paint(&pixmap);

        QColor colorLine(128, 0, 0, 200);
        QColor colorText(255, 255, 255, 255);

        QFont font;
        font.setPointSize(12);
        paint.setFont(font);

        const QString& str = QString("%1: %2").arg(it.first).arg(it.second);

        QFontMetrics fMetrics(paint.font());
        QRect textRect(20, 20 + fMetrics.size(Qt::TextSingleLine, str).height() * counter, fMetrics.size(Qt::TextSingleLine, str).width(), fMetrics.size(Qt::TextSingleLine, str).height());
        paint.fillRect(textRect, QBrush(colorLine));

        paint.setPen(colorText);
        paint.drawText(textRect, str);

        ++counter;
    }

    tertiaryDisplay.setPixmap(pixmap);

    secondaryDisplayCleaner.start();
}

void ClientDialog::drawFaceIdentificators(const QVector<QPair<QRect, QString> > &identificators)
{
    tertiaryDisplayCleaner.stop();

    QPixmap pixmap(primaryDisplay.pixmap().width(), primaryDisplay.pixmap().height());
    pixmap.fill(Qt::transparent);
    for(const auto &it : identificators)
    {
        QPainter paint(&pixmap);

        QColor colorLine(128, 0, 0, 200);
        QColor colorText(255, 255, 255, 255);

        QFont font;
        font.setPointSize(18);
        paint.setFont(font);

        const QRect& rect = it.first;
        paint.setPen(QPen(colorLine, 3));
        paint.drawRect(rect);

        const QString& str = it.second;

        QFontMetrics fMetrics(paint.font());
        QRect textRect(rect.x(), rect.y() + rect.height(), fMetrics.size(Qt::TextSingleLine, str).width(), fMetrics.size(Qt::TextSingleLine, str).height());
        paint.fillRect(textRect, QBrush(colorLine));

        paint.setPen(colorText);
        paint.drawText(textRect, str);
    }

    secondaryDisplay.setPixmap(pixmap);

    tertiaryDisplayCleaner.start();
}

void ClientDialog::print(const QString &message)
{
    ui->clientLog->append(QString("<b>[%1]</b> %2 <br/>").arg(Server::generateDateTime(), message));
}
