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

#ifndef CLIENTDIALOG_H
#define CLIENTDIALOG_H

#include <QDialog>
#include <QGraphicsPixmapItem>
#include <QTimer>

namespace Ui {
class ClientDialog;
}

class Client;

class ClientDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ClientDialog(Client* client, QWidget *parent = nullptr);
    ~ClientDialog();

private:
    Ui::ClientDialog *ui;

    QGraphicsPixmapItem primaryDisplay;
    QGraphicsPixmapItem secondaryDisplay;
    QGraphicsPixmapItem tertiaryDisplay;

    QTimer secondaryDisplayCleaner;
    QTimer tertiaryDisplayCleaner;

private slots:
    void on_okButton_clicked();

public slots:
    void updatePrimaryDisplay(const QPixmap& pixmap);

    void drawTextBox(const QVector<QPair<float, QString>> & list);
    void drawFaceIdentificators(const QVector<QPair<QRect, QString>>& identificators);

    void print(const QString& message);

};

#endif // CLIENTDIALOG_H
