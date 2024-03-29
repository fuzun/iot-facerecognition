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
#include "Server/Server.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("github.com/fuzun");
    QCoreApplication::setApplicationName("iot-facerecognition-server");
    QCoreApplication::setApplicationVersion(VERSION);

    Server server(nullptr);

    QObject::connect(&server, &Server::end, &app, &QApplication::quit, Qt::QueuedConnection);
    QObject::connect(&app, &QApplication::lastWindowClosed, &app, &QApplication::quit, Qt::QueuedConnection);

    // TODO: Handle SIGTERM to safely terminate the application when it runs in console mode.
    return app.exec();
}
