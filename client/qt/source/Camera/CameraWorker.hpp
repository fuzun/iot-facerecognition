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
#ifndef CAMERAWORKER_H
#define CAMERAWORKER_H

#include <QObject>

#include "common/Log.hpp"

#ifdef CAM_QTMULTIMEDIA
class QCamera;
class YUVProviderVideoSurface;
#include <QCameraInfo>
Q_DECLARE_METATYPE(QCameraInfo);
#endif


#ifdef CAM_OMX
namespace IL
{
class Camera;
class VideoEncode;
}
#endif

struct YUVFrame;
class CameraSettings;

class CameraWorker : public QObject
{
    Q_OBJECT

    friend class CameraController;

#ifdef CAM_QTMULTIMEDIA
    QCamera *m_camera = nullptr;
    YUVProviderVideoSurface *m_surface = nullptr;
#endif

#ifdef CAM_OMX
    IL::Camera *m_camera = nullptr;
    IL::VideoEncode *m_encoder = nullptr;

    bool m_stop = false;
#endif

    YUVFrame *m_frame = nullptr;

    CameraSettings *m_settings = nullptr;

public:
    explicit CameraWorker(QObject *parent = nullptr, YUVFrame *frame = nullptr, CameraSettings *settings = nullptr);
    ~CameraWorker();

public slots:
    void start();
    void stop();

private slots:
    void init();

#ifdef CAM_QTMULTIMEDIA
    void createCamera(const QCameraInfo& cameraInfo);
#endif

public:
signals:
    void log(const QVariant& str, Log::LogType type = Log::INFORMATION);

    void frameBufferUpdated();

    void jpegReady(const QByteArray& data);


    void started();
    void stopped();
};

#endif // CAMERAWORKER_H
