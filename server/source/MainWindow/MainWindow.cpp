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

#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QListWidgetItem>
#include <QMessageBox>

#include "Server/Server.h"

MainWindow::MainWindow(QWidget *parent, Server *_server)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , server(_server)
{
    ui->setupUi(this);
    setWindowTitle(windowTitle() + " - v" + VER);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_quitButton_clicked()
{
    emit end();
}

void MainWindow::on_aboutButton_clicked()
{
    QMessageBox::information(this, "About...", QString("iot-facerecognition-server<br/>Address: <a href=\"https://github.com/fuzun/iot-facerecognition\">GitHub Repository</a><br/>Author: fuzun<br/>Version: ") + VER);
}
