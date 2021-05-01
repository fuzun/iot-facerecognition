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
#include "CameraWorker.hpp"

#ifdef CAM_QTMULTIMEDIA
#include <QCamera>
#include "YUVProviderVideoSurface.hpp"
#endif

#ifdef CAM_OMX
#include <OpenMaxIL++/Camera.h>
#include <OpenMaxIL++/VideoEncode.h>
#include <QApplication>

#include "common/YUVFrame.hpp"
#endif

#include "CameraSettings.hpp"

#ifdef CAM_OMX
using namespace IL;
#endif

CameraWorker::CameraWorker(QObject *parent, YUVFrame *frame, CameraSettings *settings)
    : QObject(parent),
    m_frame(frame),
    m_settings(settings)
{
    assert(m_frame);
    assert(m_settings);
}

CameraWorker::~CameraWorker()
{
    stop();

#ifdef CAM_QTMULTIMEDIA
    delete m_camera;
#endif

#ifdef CAM_OMX
    // bcm_host_deinit();
#endif
}

void CameraWorker::start()
{
#ifdef CAM_QTMULTIMEDIA
    assert(m_camera);

    if (!m_camera || !m_camera->isAvailable())
    {
        emit log("No camera is available", Log::ERROR);
        return;
    }

    m_camera->start();

    emit started();

#else
    m_stop = false;

    emit started();

    m_camera->SetState(Component::StateExecuting);
    m_encoder->SetState(Component::StateExecuting);

    m_camera->SetCapturing(true);

    // implant camera / encoder loop into thread loop:
    QMetaObject::invokeMethod(this, [this]() {
            while (!m_stop)
            {
                if (! m_frame->tryLockForWrite() )
                    QApplication::processEvents();

                m_frame->width = m_camera->width();
                m_frame->height = m_camera->height();
                m_frame->buffer = m_camera->outputPorts()[70].buffer->pBuffer;
                int32_t bufferSize = m_camera->getOutputData(70, nullptr);

                m_frame->unlock();

                emit frameBufferUpdated();

                if (bufferSize > 0)
                {
                    uint8_t* bufferEncoderOut = m_encoder->outputPorts()[201].buffer->pBuffer;

                    if ((bufferSize = m_encoder->getOutputData(nullptr, false)) > 0)
                    {
                        QByteArray buffer(bufferSize, Qt::Initialization::Uninitialized);
                        memcpy(buffer.data(), bufferEncoderOut, bufferSize);

                        emit jpegReady( std::move(buffer) );
                    }
                }

                // TODO: fix this
                // TODO: check interference with jpegReady signal.
                QApplication::processEvents();
            }

            m_camera->SetCapturing(false);

            m_camera->SetState(Component::StateIdle);
            m_encoder->SetState(Component::StateIdle);

            m_frame->lockForWrite();
            m_frame->width = 0;
            m_frame->height = 0;
            m_frame->buffer = nullptr;
            m_frame->unlock();

            emit frameBufferUpdated();
        }, Qt::QueuedConnection);
#endif
}

void CameraWorker::stop()
{
#ifdef CAM_QTMULTIMEDIA
    assert(m_camera);

    if (!m_camera || !m_camera->isAvailable())
    {
        emit log("No camera is available", Log::ERROR);
        return;
    }

    m_camera->stop();

    emit stopped();

#else
    m_stop = true;

    // OpenMaxIL++ uses atexit() to automatically destroy the allocated objects
    /*
    QMetaObject::invokeMethod(this, [this]() {
        delete m_encoder;
        delete m_camera;
        }, Qt::QueuedConnection);
    */

    emit stopped();
#endif
}

void CameraWorker::init()
{
#ifdef CAM_QTMULTIMEDIA
    m_surface = new YUVProviderVideoSurface(this, m_frame, m_settings);
    connect(m_surface, &YUVProviderVideoSurface::updated, this, &CameraWorker::frameBufferUpdated);
    connect(m_surface, &YUVProviderVideoSurface::jpegReady, this, &CameraWorker::jpegReady);
#else
    bcm_host_init();

    m_camera = new Camera(m_settings->camWidth(),
        m_settings->camHeight(),
        m_settings->camDeviceNumber(),
        m_settings->camHighSpeed(),
        m_settings->camSensorMode(),
        m_settings->camVerbose());

    m_encoder = new VideoEncode(m_settings->encBitrateKBPS(),
        VideoEncode::CodingMJPEG,
        true,
        m_settings->encVerbose());

    // m_camera->setResolution(m_settings->camWidth(),
    // m_settings->camHeight());
    // m_camera->setSensorMode(m_settings->camSensorMode());
    m_camera->setFramerate(m_settings->camFrameRate());
    m_camera->setBrightness(m_settings->camBrightness());
    m_camera->setSharpness(m_settings->camSharpness());
    m_camera->setSaturation(m_settings->camSaturation());
    m_camera->setContrast(m_settings->camContrast());
    m_camera->setWhiteBalanceControl(static_cast<Camera::WhiteBalControl>(m_settings->camWhiteBalanceControl()));
    m_camera->setExposureControl(static_cast<Camera::ExposureControl>(m_settings->camExposureControl()));
    m_camera->setExposureValue(static_cast<Camera::ExposureMeteringMode>(m_settings->camExposureMeteringMode()),
        m_settings->camExposureCompensation(),
        m_settings->camISOSensitivity(),
        m_settings->camShutterSpeedUS());
    m_camera->setImageFilter(static_cast<Camera::ImageFilter>(m_settings->camImageFilter()));
    m_camera->setFrameStabilisation(m_settings->camFrameStabilisation());
    m_camera->setMirror(m_settings->camMirrorH(), m_settings->camMirrorV());
    m_camera->setRotation(m_settings->camRotation());

    m_camera->DisableProprietaryTunnels(70);
    m_camera->SetupTunnelVideo(m_encoder);

    m_camera->SetState(Component::StateIdle);
    m_encoder->SetState(Component::StateIdle);

    m_camera->AllocateOutputBuffer(70);
#endif
}

#ifdef CAM_QTMULTIMEDIA
void CameraWorker::createCamera(const QCameraInfo& cameraInfo)
{
    if (m_camera)
    {
        m_camera->deleteLater();
    }

    m_camera = new QCamera(cameraInfo);

    m_camera->setViewfinder(m_surface);
}
#endif
