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
#ifndef CAMERASETTINGS_H
#define CAMERASETTINGS_H

#include "common/BaseSettings.hpp"

#include <atomic>

class CameraSettings : public BaseSettings
{
    Q_OBJECT

    Q_PROPERTY(int lineWidth READ lineWidth WRITE setLineWidth NOTIFY lineWidthChanged)
    Q_PROPERTY(int labelPointSize READ labelPointSize WRITE setLabelPointSize NOTIFY labelPointSizeChanged)
    Q_PROPERTY(QString accentColor READ accentColor WRITE setAccentColor NOTIFY accentColorChanged)
    Q_PROPERTY(int objectListDuration READ objectListDuration WRITE setObjectListDuration NOTIFY objectListDurationChanged)
    Q_PROPERTY(int faceListDuration READ faceListDuration WRITE setFaceListDuration NOTIFY faceListDurationChanged)
    Q_PROPERTY(bool showFPS READ showFPS WRITE setShowFPS NOTIFY showFPSChanged)
    Q_PROPERTY(bool showTotalInternetUsage READ showTotalInternetUsage WRITE setTotalInternetUsage NOTIFY totalInternetUsageChanged)


#ifdef CAM_QTMULTIMEDIA
    Q_PROPERTY(int selectedCamera READ selectedCamera WRITE setSelectedCamera NOTIFY selectedCameraChanged)
    Q_PROPERTY(int jpegQuality READ jpegQuality WRITE setJpegQuality NOTIFY jpegQualityChanged)

    int m_selectedCamera = 0;
    std::atomic<int> m_jpegQuality = 80;
#elif CAM_OMX
    Q_PROPERTY(int camWidth READ camWidth WRITE setCamWidth NOTIFY camWidthChanged)
    Q_PROPERTY(int camHeight READ camHeight WRITE setCamHeight NOTIFY camHeightChanged)
    Q_PROPERTY(int camDeviceNumber READ camDeviceNumber WRITE setCamDeviceNumber NOTIFY camDeviceNumberChanged)
    Q_PROPERTY(bool camHighSpeed READ camHighSpeed WRITE setCamHighSpeed NOTIFY camHighSpeedChanged)
    Q_PROPERTY(int camSensorMode READ camSensorMode WRITE setCamSensorMode NOTIFY camSensorModeChanged)
    Q_PROPERTY(bool camVerbose READ camVerbose WRITE setCamVerbose NOTIFY camVerboseChanged)
    Q_PROPERTY(int camFrameRate READ camFrameRate WRITE setCamFrameRate NOTIFY camFrameRateChanged)
    Q_PROPERTY(int camBrightness READ camBrightness WRITE setCamBrightness NOTIFY camBrightnessChanged)
    Q_PROPERTY(int camSharpness READ camSharpness WRITE setCamSharpness NOTIFY camSharpnessChanged)
    Q_PROPERTY(int camSaturation READ camSaturation WRITE setCamSaturation NOTIFY camSaturationChanged)
    Q_PROPERTY(int camContrast READ camContrast WRITE setCamContrast NOTIFY camContrastChanged)
    Q_PROPERTY(int camWhiteBalanceControl READ camWhiteBalanceControl WRITE setCamWhiteBalanceControl NOTIFY camWhiteBalanceControlChanged)
    Q_PROPERTY(int camExposureControl READ camExposureControl WRITE setCamExposureControl NOTIFY camExposureControlChanged)
    Q_PROPERTY(int camExposureMeteringMode READ camExposureMeteringMode WRITE setCamExposureMeteringMode NOTIFY camExposureMeteringModeChanged)
    Q_PROPERTY(int camExposureCompensation READ camExposureCompensation WRITE setCameraExposureCompensation NOTIFY camExposureCompensationChanged)
    Q_PROPERTY(int camISOSensitivity READ camISOSensitivity WRITE setCamISOSensitivity NOTIFY camISOSensitivityChanged)
    Q_PROPERTY(int camShutterSpeedUS READ camShutterSpeedUS WRITE setCamShutterSpeedUS NOTIFY camShutterSpeedUSChanged)
    Q_PROPERTY(int camImageFilter READ camImageFilter WRITE setCamImageFilter NOTIFY camImageFilterChanged)
    Q_PROPERTY(bool camFrameStabilisation READ camFrameStabilisation WRITE setCamFrameStabilisation NOTIFY camFrameStabilisationChanged)
    Q_PROPERTY(bool camMirrorH READ camMirrorH WRITE setCamMirrorH NOTIFY camMirrorHChanged)
    Q_PROPERTY(bool camMirrorV READ camMirrorV WRITE setCamMirrorV NOTIFY camMirrorVChanged)
    Q_PROPERTY(int camRotation READ camRotation WRITE setCamRotation NOTIFY camRotationChanged)
    Q_PROPERTY(bool camFocusOverlay READ camFocusOverlay WRITE setCamFocusOverlay NOTIFY camFocusOverlayChanged)

    Q_PROPERTY(int encBitrateKBPS READ encBitrateKBPS WRITE setEncBitrateKBPS NOTIFY encBitrateKBPSChanged)
    Q_PROPERTY(bool encVerbose READ encVerbose WRITE setEncVerbose NOTIFY encVerboseChanged)

    std::atomic<int> m_camWidth = 1280;
    std::atomic<int> m_camHeight = 720;
    std::atomic<int> m_camDeviceNumber = 0;
    std::atomic<bool> m_camHighSpeed = false;
    std::atomic<int> m_camSensorMode = 0; // https://picamera.readthedocs.io/en/release-1.13/fov.html#sensor-modes
    std::atomic<bool> m_camVerbose = false;
    std::atomic<int> m_camFrameRate = 15;
    std::atomic<int> m_camBrightness = 50;
    std::atomic<int> m_camSharpness = 0;
    std::atomic<int> m_camSaturation = 0;
    std::atomic<int> m_camContrast = 0;
    std::atomic<int> m_camWhiteBalanceControl = 1; // WhiteBalControlAuto
    std::atomic<int> m_camExposureControl = 1; // ExposureControlAuto
    std::atomic<int> m_camExposureMeteringMode = 0; // ExposureMeteringModeAverage
    std::atomic<int> m_camExposureCompensation = 0;
    std::atomic<int> m_camISOSensitivity = 0;
    std::atomic<int> m_camShutterSpeedUS = 0;
    std::atomic<int> m_camImageFilter = 0; // ImageFilterNone
    std::atomic<bool> m_camFrameStabilisation = true;
    std::atomic<bool> m_camMirrorH = false;
    std::atomic<bool> m_camMirrorV = false;
    std::atomic<int> m_camRotation = 0;
    std::atomic<bool> m_camFocusOverlay = false;

    std::atomic<int> m_encBitrateKBPS = 4096;
    std::atomic<bool> m_encVerbose = false;
#endif

    bool m_showFPS = true;

    bool m_showTotalInternetUsage = true;

    int m_objectListDuration = 3000;

    int m_faceListDuration = 3000;

    int m_labelPointSize = 12;


    QString m_accentColor {"darkred"};

    int m_lineWidth = 4;

public:
    inline explicit CameraSettings(QObject *parent, bool load = true) : BaseSettings(parent)
    {
        qRegisterMetaType<CameraSettings *>();

        if (!load)
            return;

        this->load(CameraSettings::staticMetaObject);
    }

    inline ~CameraSettings()
    {
        save();
    }

    Q_INVOKABLE void save()
    {
        BaseSettings::save(CameraSettings::staticMetaObject);
    }


    Q_INVOKABLE void reset()
    {
        CameraSettings nonLoadedInstance(this, false);
        BaseSettings::reset(CameraSettings::staticMetaObject, &nonLoadedInstance);
    }

#ifdef CAM_QTMULTIMEDIA
    int selectedCamera() const
    {
        return m_selectedCamera;
    }

    int jpegQuality() const
    {
        return m_jpegQuality;
    }

#elif CAM_OMX

    int camWidth() const
    {
        return m_camWidth;
    }

    int camHeight() const
    {
        return m_camHeight;
    }

    int camDeviceNumber() const
    {
        return m_camDeviceNumber;
    }

    bool camHighSpeed() const
    {
        return m_camHighSpeed;
    }

    int camSensorMode() const
    {
        return m_camSensorMode;
    }

    bool camVerbose() const
    {
        return m_camVerbose;
    }

    int camFrameRate() const
    {
        return m_camFrameRate;
    }

    int camBrightness() const
    {
        return m_camBrightness;
    }

    int camSharpness() const
    {
        return m_camSharpness;
    }

    int camSaturation() const
    {
        return m_camSaturation;
    }

    int camContrast() const
    {
        return m_camContrast;
    }

    int camWhiteBalanceControl() const
    {
        return m_camWhiteBalanceControl;
    }

    int camExposureControl() const
    {
        return m_camExposureControl;
    }

    int camISOSensitivity() const
    {
        return m_camISOSensitivity;
    }

    int camShutterSpeedUS() const
    {
        return m_camShutterSpeedUS;
    }

    int camImageFilter() const
    {
        return m_camImageFilter;
    }

    bool camFrameStabilisation() const
    {
        return m_camFrameStabilisation;
    }

    bool camMirrorH() const
    {
        return m_camMirrorH;
    }

    bool camMirrorV() const
    {
        return m_camMirrorV;
    }

    int camRotation() const
    {
        return m_camRotation;
    }

    bool camFocusOverlay() const
    {
        return m_camFocusOverlay;
    }

    int camExposureCompensation() const
    {
        return m_camExposureCompensation;
    }

    int encBitrateKBPS() const
    {
        return m_encBitrateKBPS;
    }

    bool encVerbose() const
    {
        return m_encVerbose;
    }

    int camExposureMeteringMode() const
    {
        return m_camExposureMeteringMode;
    }
#endif

    bool showFPS() const
    {
        return m_showFPS;
    }

    bool showTotalInternetUsage() const
    {
        return m_showTotalInternetUsage;
    }

    int objectListDuration() const
    {
        return m_objectListDuration;
    }

    int faceListDuration() const
    {
        return m_faceListDuration;
    }

    int labelPointSize() const
    {
        return m_labelPointSize;
    }

    QString accentColor() const
    {
        return m_accentColor;
    }

    int lineWidth() const
    {
        return m_lineWidth;
    }

public slots:

#ifdef CAM_QTMULTIMEDIA
    void setSelectedCamera(int selectedCamera)
    {
        if (m_selectedCamera == selectedCamera)
            return;

        m_selectedCamera = selectedCamera;
        emit selectedCameraChanged(m_selectedCamera);
    }

    void setJpegQuality(int jpegQuality)
    {
        if (m_jpegQuality == jpegQuality)
            return;

        m_jpegQuality = jpegQuality;
        emit jpegQualityChanged(m_jpegQuality);
    }

#elif CAM_OMX
    void setCamWidth(int camWidth)
    {
        if (m_camWidth == camWidth)
            return;

        m_camWidth = camWidth;
        emit camWidthChanged(m_camWidth);
    }

    void setCamHeight(int camHeight)
    {
        if (m_camHeight == camHeight)
            return;

        m_camHeight = camHeight;
        emit camHeightChanged(m_camHeight);
    }

    void setCamDeviceNumber(int camDeviceNumber)
    {
        if (m_camDeviceNumber == camDeviceNumber)
            return;

        m_camDeviceNumber = camDeviceNumber;
        emit camDeviceNumberChanged(m_camDeviceNumber);
    }

    void setCamHighSpeed(bool camHighSpeed)
    {
        if (m_camHighSpeed == camHighSpeed)
            return;

        m_camHighSpeed = camHighSpeed;
        emit camHighSpeedChanged(m_camHighSpeed);
    }

    void setCamSensorMode(int camSensorMode)
    {
        if (m_camSensorMode == camSensorMode)
            return;

        m_camSensorMode = camSensorMode;
        emit camSensorModeChanged(m_camSensorMode);
    }

    void setCamVerbose(bool camVerbose)
    {
        if (m_camVerbose == camVerbose)
            return;

        m_camVerbose = camVerbose;
        emit camVerboseChanged(m_camVerbose);
    }

    void setCamFrameRate(int camFrameRate)
    {
        if (m_camFrameRate == camFrameRate)
            return;

        m_camFrameRate = camFrameRate;
        emit camFrameRateChanged(m_camFrameRate);
    }

    void setCamBrightness(int camBrightness)
    {
        if (m_camBrightness == camBrightness)
            return;

        m_camBrightness = camBrightness;
        emit camBrightnessChanged(m_camBrightness);
    }

    void setCamSharpness(int camSharpness)
    {
        if (m_camSharpness == camSharpness)
            return;

        m_camSharpness = camSharpness;
        emit camSharpnessChanged(m_camSharpness);
    }

    void setCamSaturation(int camSaturation)
    {
        if (m_camSaturation == camSaturation)
            return;

        m_camSaturation = camSaturation;
        emit camSaturationChanged(m_camSaturation);
    }

    void setCamContrast(int camContrast)
    {
        if (m_camContrast == camContrast)
            return;

        m_camContrast = camContrast;
        emit camContrastChanged(m_camContrast);
    }

    void setCamWhiteBalanceControl(int camWhiteBalanceControl)
    {
        if (m_camWhiteBalanceControl == camWhiteBalanceControl)
            return;

        m_camWhiteBalanceControl = camWhiteBalanceControl;
        emit camWhiteBalanceControlChanged(m_camWhiteBalanceControl);
    }

    void setCamExposureControl(int camExposureControl)
    {
        if (m_camExposureControl == camExposureControl)
            return;

        m_camExposureControl = camExposureControl;
        emit camExposureControlChanged(m_camExposureControl);
    }

    void setCamISOSensitivity(int camISOSensitivity)
    {
        if (m_camISOSensitivity == camISOSensitivity)
            return;

        m_camISOSensitivity = camISOSensitivity;
        emit camISOSensitivityChanged(m_camISOSensitivity);
    }

    void setCamShutterSpeedUS(int camShutterSpeedUS)
    {
        if (m_camShutterSpeedUS == camShutterSpeedUS)
            return;

        m_camShutterSpeedUS = camShutterSpeedUS;
        emit camShutterSpeedUSChanged(m_camShutterSpeedUS);
    }

    void setCamImageFilter(int camImageFilter)
    {
        if (m_camImageFilter == camImageFilter)
            return;

        m_camImageFilter = camImageFilter;
        emit camImageFilterChanged(m_camImageFilter);
    }

    void setCamFrameStabilisation(bool camFrameStabilisation)
    {
        if (m_camFrameStabilisation == camFrameStabilisation)
            return;

        m_camFrameStabilisation = camFrameStabilisation;
        emit camFrameStabilisationChanged(m_camFrameStabilisation);
    }

    void setCamMirrorH(bool camMirrorH)
    {
        if (m_camMirrorH == camMirrorH)
            return;

        m_camMirrorH = camMirrorH;
        emit camMirrorHChanged(m_camMirrorH);
    }

    void setCamMirrorV(bool camMirrorV)
    {
        if (m_camMirrorV == camMirrorV)
            return;

        m_camMirrorV = camMirrorV;
        emit camMirrorVChanged(m_camMirrorV);
    }

    void setCamRotation(int camRotation)
    {
        if (m_camRotation == camRotation)
            return;

        m_camRotation = camRotation;
        emit camRotationChanged(m_camRotation);
    }

    void setCamFocusOverlay(bool camFocusOverlay)
    {
        if (m_camFocusOverlay == camFocusOverlay)
            return;

        m_camFocusOverlay = camFocusOverlay;
        emit camFocusOverlayChanged(m_camFocusOverlay);
    }

    void setCameraExposureCompensation(int camExposureCompensation)
    {
        if (m_camExposureCompensation == camExposureCompensation)
            return;

        m_camExposureCompensation = camExposureCompensation;
        emit camExposureCompensationChanged(m_camExposureCompensation);
    }

    void setEncBitrateKBPS(int encBitrateKBPS)
    {
        if (m_encBitrateKBPS == encBitrateKBPS)
            return;

        m_encBitrateKBPS = encBitrateKBPS;
        emit encBitrateKBPSChanged(m_encBitrateKBPS);
    }

    void setEncVerbose(bool encVerbose)
    {
        if (m_encVerbose == encVerbose)
            return;

        m_encVerbose = encVerbose;
        emit encVerboseChanged(m_encVerbose);
    }

    void setCamExposureMeteringMode(int camExposureMeteringMode)
    {
        if (m_camExposureMeteringMode == camExposureMeteringMode)
            return;

        m_camExposureMeteringMode = camExposureMeteringMode;
        emit camExposureMeteringModeChanged(m_camExposureMeteringMode);
    }
#endif

    void setShowFPS(bool showFPS)
    {
        if (m_showFPS == showFPS)
            return;

        m_showFPS = showFPS;
        emit showFPSChanged(m_showFPS);
    }

    void setTotalInternetUsage(bool showTotalInternetUsage)
    {
        if (m_showTotalInternetUsage == showTotalInternetUsage)
            return;

        m_showTotalInternetUsage = showTotalInternetUsage;
        emit totalInternetUsageChanged(m_showTotalInternetUsage);
    }

    void setObjectListDuration(int objectListDuration)
    {
        if (m_objectListDuration == objectListDuration)
            return;

        m_objectListDuration = objectListDuration;
        emit objectListDurationChanged(m_objectListDuration);
    }

    void setFaceListDuration(int faceListDuration)
    {
        if (m_faceListDuration == faceListDuration)
            return;

        m_faceListDuration = faceListDuration;
        emit faceListDurationChanged(m_faceListDuration);
    }

    void setLabelPointSize(int labelPointSize)
    {
        if (m_labelPointSize == labelPointSize)
            return;

        m_labelPointSize = labelPointSize;
        emit labelPointSizeChanged(m_labelPointSize);
    }

    void setAccentColor(QString accentColor)
    {
        if (m_accentColor == accentColor)
            return;

        m_accentColor = accentColor;
        emit accentColorChanged(m_accentColor);
    }

    void setLineWidth(int lineWidth)
    {
        if (m_lineWidth == lineWidth)
            return;

        m_lineWidth = lineWidth;
        emit lineWidthChanged(m_lineWidth);
    }

signals:

#ifdef CAM_QTMULTIMEDIA
    void selectedCameraChanged(int selectedCamera);
    void jpegQualityChanged(int jpegQuality);
#elif CAM_OMX
    void camWidthChanged(int camWidth);
    void camHeightChanged(int camHeight);
    void camDeviceNumberChanged(int camDeviceNumber);
    void camHighSpeedChanged(bool camHighSpeed);
    void camSensorModeChanged(int camSensorMode);
    void camVerboseChanged(bool camVerbose);
    void camFrameRateChanged(int camFrameRate);
    void camBrightnessChanged(int camBrightness);
    void camSharpnessChanged(int camSharpness);
    void camSaturationChanged(int camSaturation);
    void camContrastChanged(int camContrast);
    void camWhiteBalanceControlChanged(int camWhiteBalanceControl);
    void camExposureControlChanged(int camExposureControl);
    void camISOSensitivityChanged(int camISOSensitivity);
    void camShutterSpeedUSChanged(int camShutterSpeedUS);
    void camImageFilterChanged(int camImageFilter);
    void camFrameStabilisationChanged(bool camFrameStabilisation);
    void camMirrorHChanged(bool camMirrorH);
    void camMirrorVChanged(bool camMirrorV);
    void camRotationChanged(int camRotation);
    void camFocusOverlayChanged(bool camFocusOverlay);
    void camExposureCompensationChanged(int camExposureCompensation);
    void encBitrateKBPSChanged(int encBitrateKBPS);
    void encVerboseChanged(bool encVerbose);
    void camExposureMeteringModeChanged(int camExposureMeteringMode);
#endif
    void showFPSChanged(bool showFPS);
    void totalInternetUsageChanged(bool showTotalInternetUsage);
    void objectListDurationChanged(int objectListDuration);
    void faceListDurationChanged(int faceListDuration);
    void labelPointSizeChanged(int labelPointSize);
    void accentColorChanged(QString accentColor);
    void lineWidthChanged(int lineWidth);
};

#endif // CAMERASETTINGS_H
