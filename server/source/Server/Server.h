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
#ifndef SERVER_H
#define SERVER_H

#include <QObject>

class Server : public QObject
{
    Q_OBJECT

private:
    class QSettings     *config = nullptr;
    class MainWindow    *mainWindow = nullptr;
    class SocketHandler *socketHandler = nullptr;

    class QFile* logFile = nullptr;
    class QTextStream* logStream = nullptr;

private slots:
    void log(const QString& message);


public:
    explicit Server(QObject *parent = nullptr);
    ~Server();

    static QString generateDateTime();

signals:
    void end();
};

#endif // SERVER_H
