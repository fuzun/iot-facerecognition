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
#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QSize>

#include "common/Log.hpp"

struct YUVFrame;
class CameraSettings;
class SettingObject;
class CameraWorker;
class Statistics;

class CameraController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(YUVFrame* currentFrame MEMBER m_currentFrame CONSTANT)
    Q_PROPERTY(QList<SettingObject *> settingModel READ settingModel NOTIFY settingModelChanged)
    Q_PROPERTY(QList<SettingObject *> intfSettingModel READ intfSettingModel NOTIFY intfSettingModelChanged)

    Q_PROPERTY(CameraSettings* settings READ settings CONSTANT)

    Q_PROPERTY(int fps MEMBER m_fps NOTIFY fpsChanged)

    YUVFrame* m_currentFrame = nullptr;
    QList<SettingObject *> m_settingModel;
    QList<SettingObject *> m_intfSettingModel;
    CameraSettings* m_settings = nullptr;

    QThread workerThread;
    CameraWorker *worker = nullptr;

    QTimer fpsTimer;
    size_t m_elapsedTime = 0;

  //  size_t frameCount = 0;

public:
    explicit CameraController(QObject *parent = nullptr, Statistics *statistics = nullptr);
    ~CameraController();

    QList<SettingObject *> settingModel() const;
    QList<SettingObject *> intfSettingModel() const;
    CameraSettings* settings() const;

    Q_INVOKABLE QSize frameSize() const;

private:
    void setupSettings();
    int m_fps = 0;

signals:
    void frameBufferUpdated();
    void settingModelChanged(QList<SettingObject *> settingModel);

    void jpegReady(const QByteArray& data);

signals:
    void log(const QVariant& str, Log::LogType type = Log::INFORMATION);

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();

    void fpsChanged(int fps);

    void started();
    void stopped();
    void intfSettingModelChanged(QList<SettingObject *> intfSettingModel);
};

#endif // CAMERACONTROLLER_H
