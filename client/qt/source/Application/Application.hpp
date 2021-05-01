/*
 * iot-facerecognition-client-qt
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
#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>

#include "common/Log.hpp"

class Application : public QObject
{
    Q_OBJECT

public:
    explicit Application(QObject *parent, class QApplication *application);
    ~Application();

private:
    class QQmlApplicationEngine* engine;
    class Interface* interface;
    class Client* client;
    class QMLGLYUVRenderer* renderer;
    class CameraController* camera;
    class Statistics* statistics;

    class QFile* logFile = nullptr;
    class QTextStream* logStream = nullptr;

private slots:
    void log(const QVariant& str, Log::LogType type = Log::INFORMATION);
    void setLoggingToFile(bool enabled);
};

#endif // APPLICATION_H
