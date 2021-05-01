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
#include "CameraController.hpp"

#ifdef CAM_QTMULTIMEDIA
#include <QCameraInfo>
#include <QCamera>
#endif

#include <QQmlEngine>

#include "CameraWorker.hpp"
#include "CameraSettings.hpp"
#include "common/SettingObject.hpp"
#include "common/YUVFrame.hpp"
#include "Statistics/Statistics.hpp"

CameraController::CameraController(QObject *parent, Statistics *statistics)
    : QObject(parent),
    m_settings(new CameraSettings(this))
{
#ifdef CAM_QTMULTIMEDIA
    qRegisterMetaType<const QCameraInfo&>();
#endif

    qRegisterMetaType<YUVFrame *>();

    qmlRegisterUncreatableType<CameraSettings>("com.iotfacerecognition.settings", 1, 0, "CameraSettings", "");

    m_currentFrame = new YUVFrame;

    worker = new CameraWorker(nullptr, m_currentFrame, m_settings);
    worker->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    workerThread.start();

    connect(this, &CameraController::start, worker, &CameraWorker::start);
    connect(this, &CameraController::stop, worker, &CameraWorker::stop);
    connect(worker, &CameraWorker::log, this, &CameraController::log);
    connect(worker, &CameraWorker::frameBufferUpdated, this, &CameraController::frameBufferUpdated);
    connect(worker, &CameraWorker::started, this, &CameraController::started);
    connect(worker, &CameraWorker::stopped, this, &CameraController::stopped);

    fpsTimer.setInterval(1000);

    connect(&fpsTimer, &QTimer::timeout, this, [this]() {
        emit fpsChanged(m_fps);
        m_fps = 0;
        ++m_elapsedTime;
    });

    connect(this, &CameraController::frameBufferUpdated, this, [this]() {
        ++m_fps;
    });

    connect(worker, &CameraWorker::jpegReady, this, &CameraController::jpegReady);

    QMetaObject::invokeMethod(worker, &CameraWorker::init, Qt::BlockingQueuedConnection);

    setupSettings();

    connect(this, &CameraController::started, this, [this](){
        fpsTimer.start();
    });

    connect(this, &CameraController::stopped, this, [this](){
        fpsTimer.stop();
    });

    if (statistics)
    {
        statistics->registerSeries(Statistics::FPSLineSeries);

        connect(this, &CameraController::fpsChanged, this, [this, statistics](int fps) {
            statistics->addData(Statistics::FPSLineSeries, QPointF(m_elapsedTime, fps));
        });
    }
}

CameraController::~CameraController()
{
    workerThread.quit();
    workerThread.wait();

    delete m_currentFrame;
}

QList<SettingObject *> CameraController::settingModel() const
{
    return m_settingModel;
}

CameraSettings *CameraController::settings() const
{
    return m_settings;
}

QSize CameraController::frameSize() const
{
    assert(m_currentFrame);

    QSize ret;

    if (m_currentFrame)
    {
        ret.setWidth(m_currentFrame->width);
        ret.setHeight(m_currentFrame->height);
    }

    return ret;
}

void CameraController::setupSettings()
{
    using SettingType = SettingObject::SettingType;

    static const auto& newSetting = SettingObject::newSetting;
    static const auto& makePropPair = SettingObject::makePropertyPair;

    newSetting(m_intfSettingModel, this,
        {
            tr("Line width:"),
            {{SettingObject::keyType, SettingType::TextFieldInt},
                {SettingObject::keyMin, 1}},
            makePropPair(m_settings, "lineWidth")
        });

    newSetting(m_intfSettingModel, this,
        {
            tr("Label font point size:"),
            {{SettingObject::keyType, SettingType::TextFieldInt},
                {SettingObject::keyMin, 6}},
            makePropPair(m_settings, "labelPointSize")
        });

    newSetting(m_intfSettingModel, this,
        {
            tr("Accent color:"),
            {{SettingObject::keyType, SettingType::TextField}},
            makePropPair(m_settings, "accentColor")
        });

    newSetting(m_intfSettingModel, this,
        {
            tr("How long should the face indicators remain until the new one is received? (in msecs) \n "
               "(Setting it -1 will make it shown indefinitely)"),
            {{SettingObject::keyType, SettingType::TextFieldInt},
                {SettingObject::keyMin, -1}},
            makePropPair(m_settings, "faceListDuration")
        });

    newSetting(m_intfSettingModel, this,
        {
            tr("How long should the object indicators remain until the new one is received? (in msecs) \n "
               "(Setting it -1 will make it shown indefinitely)"),
            {{SettingObject::keyType, SettingType::TextFieldInt},
                {SettingObject::keyMin, -1}},
            makePropPair(m_settings, "objectListDuration")
        });

    newSetting(m_intfSettingModel, this,
        {
            tr("Show FPS"),
            {{SettingObject::keyType, SettingType::ToggleButton}},
            makePropPair(m_settings, "showFPS")
        });

    newSetting(m_intfSettingModel, this,
        {
            tr("Show FPS"),
            {{SettingObject::keyType, SettingType::ToggleButton}},
            makePropPair(m_settings, "showFPS")
        });

    newSetting(m_intfSettingModel, this,
        {
            tr("Show total sent/received?"),
            {{SettingObject::keyType, SettingType::ToggleButton}},
            makePropPair(m_settings, "showTotalInternetUsage")
        });

    emit intfSettingModelChanged(m_intfSettingModel);

#ifdef CAM_QTMULTIMEDIA
    [&]()
    {
       QStringList cameraList;
        const QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
        for (const auto &cameraInfo : cameras)
        {
            cameraList.append(cameraInfo.description());
        }

        if (cameras.isEmpty())
        {
            // no camera found
            return;
        }

        auto cameraSetter = [this](const QCameraInfo& info) {
            QMetaObject::invokeMethod(worker, "createCamera", Qt::QueuedConnection, Q_ARG(QCameraInfo, info));
        };

        int selectedCameraIndex = m_settings->selectedCamera();
        if (selectedCameraIndex < cameras.size() && selectedCameraIndex >= 0)
            cameraSetter(cameras.at(selectedCameraIndex));
        else
            cameraSetter(cameras.at(0));

        newSetting(m_settingModel, this,
            {
                tr("Which camera to use?"),
                {{SettingObject::keyType, SettingType::DropDown},
                 {SettingObject::keyDropDownCtx, cameraList}
                },
                makePropPair(m_settings, "selectedCamera"),
                [cameras, cameraSetter, this] (const QVariant& value) {
                    int index = value.toInt();
                    if (index < cameras.size() && index >= 0)
                    {
                        cameraSetter(cameras.at(value.toInt()));
                        m_settings->setSelectedCamera(index);
                    }
                }
            });
    }();

    newSetting(m_settingModel, this,
        {
            tr("JPEG Quality"),
            {
                {SettingObject::keyType, SettingType::Slider},
                {SettingObject::keyMin, 0},
                {SettingObject::keyMax, 100}
                },
            makePropPair(m_settings, "jpegQuality"),
        });

#elif CAM_OMX
    newSetting(m_settingModel, this,
        {
            tr("Encoder Bitrate KB/S:"),
            {{SettingObject::keyType, SettingType::Slider},
                {SettingObject::keyMin, 1},
                {SettingObject::keyMax, 15360}},
            makePropPair(m_settings, "encBitrateKBPS")
        });

    newSetting(m_settingModel, this,
        {
            tr("Camera Sensor Mode:"),
            {{SettingObject::keyType, SettingType::TextFieldInt},
                {SettingObject::keyMin, 1},
                {SettingObject::keyMax, 7}},
            makePropPair(m_settings, "camSensorMode")
        });

    newSetting(m_settingModel, this,
        {
            tr("Camera Capture Width:"),
            {{SettingObject::keyType, SettingType::TextFieldInt},
                {SettingObject::keyMin, 1}},
            makePropPair(m_settings, "camWidth")
        });

    newSetting(m_settingModel, this,
        {
            tr("Camera Capture Height:"),
            {{SettingObject::keyType, SettingType::TextFieldInt},
                {SettingObject::keyMin, 1}},
            makePropPair(m_settings, "camHeight")
        });

    newSetting(m_settingModel, this,
        {
            tr("Camera Device Number:"),
            {{SettingObject::keyType, SettingType::TextFieldInt},
                {SettingObject::keyMin, 0}},
            makePropPair(m_settings, "camDeviceNumber")
        });

    newSetting(m_settingModel, this,
        {
            tr("Camera Frame Rate:"),
            {{SettingObject::keyType, SettingType::TextFieldInt},
                {SettingObject::keyMin, 1},
                {SettingObject::keyMax, 90}},
            makePropPair(m_settings, "camFrameRate")
        });

    newSetting(m_settingModel, this,
        {
            tr("Camera Mirror Horizontically:"),
            {{SettingObject::keyType, SettingType::ToggleButton}},
            makePropPair(m_settings, "camMirrorH")
        });

    newSetting(m_settingModel, this,
        {
            tr("Camera Mirror Vertically:"),
            {{SettingObject::keyType, SettingType::ToggleButton}},
            makePropPair(m_settings, "camMirrorV")
        });

    newSetting(m_settingModel, this,
        {
            tr("Camera High Speed Mode:"),
            {{SettingObject::keyType, SettingType::ToggleButton}},
            makePropPair(m_settings, "camHighSpeed")
        });

    newSetting(m_settingModel, this,
        {
            tr("Camera Verbose Out:"),
            {{SettingObject::keyType, SettingType::ToggleButton}},
            makePropPair(m_settings, "camVerbose")
        });

    newSetting(m_settingModel, this,
        {
            tr("Camera Brightness (%):"),
            {{SettingObject::keyType, SettingType::TextFieldInt},
                {SettingObject::keyMin, 0},
                {SettingObject::keyMax, 100}},
            makePropPair(m_settings, "camBrightness")
        });

    newSetting(m_settingModel, this,
        {
            tr("Camera Sharpness (-100 to 100):"),
            {{SettingObject::keyType, SettingType::TextFieldInt},
                {SettingObject::keyMin, -100},
                {SettingObject::keyMax, 100}},
            makePropPair(m_settings, "camSharpness")
        });

    newSetting(m_settingModel, this,
        {
            tr("Camera Contrast (-100 to 100):"),
            {{SettingObject::keyType, SettingType::TextFieldInt},
                {SettingObject::keyMin, -100},
                {SettingObject::keyMax, 100}},
            makePropPair(m_settings, "camContrast")
        });

    newSetting(m_settingModel, this,
        {
            tr("Camera White Balance Control:"),
            {
                {SettingObject::keyType, SettingType::DropDown},
                {SettingObject::keyDropDownCtx, QStringList {
                                                     "Off",
                                                     "Auto",
                                                     "SunLight",
                                                     "Cloudy",
                                                     "Shade",
                                                     "Tungsten",
                                                     "Flourescent",
                                                     "Incandescent",
                                                     "Flash",
                                                     "Horizon",
                                                     "GreyWorld"
                                                 }}
            },
            makePropPair(m_settings, "camWhiteBalanceControl")
        });

    newSetting(m_settingModel, this,
        {
            tr("Exposure Control:"),
            {
                {SettingObject::keyType, SettingType::DropDown},
                {SettingObject::keyDropDownCtx, QStringList {
                                                     "Off",
                                                     "Auto",
                                                     "Night",
                                                     "BackLight",
                                                     "SpotLight",
                                                     "Sports",
                                                     "Snow",
                                                     "Beach",
                                                     "LargeAperture",
                                                     "SmallAperture",
                                                     "GreyWorld"
                                                 }}
            },
            makePropPair(m_settings, "camExposureControl")
        });

    newSetting(m_settingModel, this,
        {
            tr("Exposure Metering Mode:"),
            {
                {SettingObject::keyType, SettingType::DropDown},
                {SettingObject::keyDropDownCtx, QStringList {
                                                     "Average",
                                                     "Spot",
                                                     "Matrix"
                                                 }}
            },
            makePropPair(m_settings, "camExposureMeteringMode")
        });

    newSetting(m_settingModel, this,
        {
            tr("ISO Sensitivity:"),
            {{SettingObject::keyType, SettingType::TextFieldInt}},
            makePropPair(m_settings, "camISOSensitivity")
        });

    newSetting(m_settingModel, this,
        {
            tr("Camera Shutter Speed (Microseconds):"),
            {{SettingObject::keyType, SettingType::TextFieldInt},
                {SettingObject::keyMin, 0}},
            makePropPair(m_settings, "camShutterSpeedUS")
        });

    newSetting(m_settingModel, this,
        {
            tr("Camera Image Filter:"),
            {
                {SettingObject::keyType, SettingType::DropDown},
                {SettingObject::keyDropDownCtx, QStringList {
                                                     "None",
                                                     "Noise",
                                                     "Emboss",
                                                     "Negative",
                                                     "Sketch",
                                                     "Oil Paint",
                                                     "Hatch",
                                                     "Gpen",
                                                     "Antialias",
                                                     "DeRing",
                                                     "Solarize"
                                                 }}
            },
            makePropPair(m_settings, "camImageFilter")
        });

    newSetting(m_settingModel, this,
        {
            tr("Camera Frame Stabilization:"),
            {{SettingObject::keyType, SettingType::ToggleButton}},
            makePropPair(m_settings, "camFrameStabilisation")
        });

    newSetting(m_settingModel, this,
        {
            tr("Camera Rotation Angle:"),
            {{SettingObject::keyType, SettingType::TextFieldInt}},
            makePropPair(m_settings, "camRotation")
        });

    newSetting(m_settingModel, this,
        {
            tr("Camera Focus Overlay:"),
            {{SettingObject::keyType, SettingType::ToggleButton}},
            makePropPair(m_settings, "camFocusOverlay")
        });

    newSetting(m_settingModel, this,
        {
            tr("Encoder Verbose Out:"),
            {{SettingObject::keyType, SettingType::ToggleButton}},
            makePropPair(m_settings, "encVerbose")
        });
#endif


    emit settingModelChanged(m_settingModel);
}

QList<SettingObject *> CameraController::intfSettingModel() const
{
    return m_intfSettingModel;
}
