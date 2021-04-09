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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHash>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QListWidgetItem;
class Client;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    QHash<QListWidgetItem *, Client *> clientListItemMap;

public:
    MainWindow(QWidget *parent = nullptr, class Server* _server = nullptr);
    ~MainWindow();


public slots:
    void newClient(class Client* client);
    void print(const QString& message);

signals:
    void end();

private slots:
    void on_quitButton_clicked();
    void on_aboutButton_clicked();



private:
    Ui::MainWindow *ui;
    class Server* server;

    QString getClientName(Client *client);
};
#endif // MAINWINDOW_H
