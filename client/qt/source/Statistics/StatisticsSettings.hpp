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
#ifndef STATISTICSSETTINGS_HPP
#define STATISTICSSETTINGS_HPP

#include "common/BaseSettings.hpp"

class StatisticsSettings : public BaseSettings
{
    Q_OBJECT

    Q_PROPERTY(int stdWidthXValueAxis READ stdWidthXValueAxis WRITE setStdWidthXValueAxis NOTIFY stdWidthXValueAxisChanged)
    Q_PROPERTY(int stdWidthXDateTimeAxis READ stdWidthXDateTimeAxis WRITE setStdWidthXDateTimeAxis NOTIFY stdWidthXDateTimeAxisChanged)

    Q_PROPERTY(bool useAntialiasing READ useAntialiasing WRITE setUseAntialiasing NOTIFY useAntiAliasingChanged)

    Q_PROPERTY(bool enableAnimations READ enableAnimations WRITE setEnableAnimations NOTIFY enableAnimationsChanged)

    Q_PROPERTY(bool loadRecognizedMaxPredictionObjectsPieSeries
            READ loadRecognizedMaxPredictionObjectsPieSeries
                WRITE setLoadRecognizedMaxPredictionObjectsPieSeries
                    NOTIFY loadRecognizedMaxPredictionObjectsPieSeriesChanged)

    Q_PROPERTY(bool loadRecognizedFacesPieSeries
            READ loadRecognizedFacesPieSeries
                WRITE setloadRecognizedFacesPieSeries
                    NOTIFY loadRecognizedFacesPieSeriesChanged)

    Q_PROPERTY(bool loadRecognizedFacesLineSeries
            READ loadRecognizedFacesLineSeries
                WRITE setloadRecognizedFacesLineSeries
                    NOTIFY loadRecognizedFacesLineSeriesChanged)

    Q_PROPERTY(bool loadMaxObjectPredictionRateLineSeries
            READ loadMaxObjectPredictionRateLineSeries
                WRITE setloadMaxObjectPredictionRateLineSeries
                    NOTIFY loadMaxObjectPredictionRateLineSeriesChanged)

    Q_PROPERTY(bool loadInternetUsageCumulativeChart
            READ loadInternetUsageCumulativeChart
                WRITE setloadInternetUsageCumulativeChart
                    NOTIFY loadInternetUsageCumulativeChartChanged)

    Q_PROPERTY(bool loadInternetUsageChart
            READ loadInternetUsageChart
                WRITE setloadInternetUsageChart
                    NOTIFY loadInternetUsageChartChanged)

    Q_PROPERTY(bool loadFPSChart
            READ loadFPSChart
                WRITE setloadFPSChart
                    NOTIFY loadFPSChartChanged)

    // normal width (max - 2 etc.)
    // opengl enabled

    bool m_loadRecognizedMaxPredictionObjectsPieSeries = true;

    bool m_loadRecognizedFacesPieSeries = true;

    bool m_loadRecognizedFacesLineSeries = true;

    bool m_loadMaxObjectPredictionRateLineSeries = true;

    bool m_loadInternetUsageCumulativeChart = true;

    bool m_loadInternetUsageChart = true;

    bool m_loadFPSChart = true;

    bool m_useAntialiasing = true;

    bool m_enableAnimations = false;

    int m_stdWidthXValueAxis = 3;

    int m_stdWidthXDateTimeAxis = 3; // seconds

public:
    inline explicit StatisticsSettings(QObject *parent = nullptr, bool load = true) : BaseSettings(parent)
    {
        qRegisterMetaType<StatisticsSettings*>();

        if (!load)
            return;

        this->load(StatisticsSettings::staticMetaObject);
    }

    inline ~StatisticsSettings()
    {
        save();
    }

    Q_INVOKABLE void save()
    {
        BaseSettings::save(StatisticsSettings::staticMetaObject);
    }

    Q_INVOKABLE void reset()
    {
        StatisticsSettings nonLoadedInstance(this, false);
        BaseSettings::reset(StatisticsSettings::staticMetaObject, &nonLoadedInstance);
    }

    bool loadRecognizedMaxPredictionObjectsPieSeries() const
    {
        return m_loadRecognizedMaxPredictionObjectsPieSeries;
    }
    bool loadRecognizedFacesPieSeries() const
    {
        return m_loadRecognizedFacesPieSeries;
    }

    bool loadRecognizedFacesLineSeries() const
    {
        return m_loadRecognizedFacesLineSeries;
    }

    bool loadMaxObjectPredictionRateLineSeries() const
    {
        return m_loadMaxObjectPredictionRateLineSeries;
    }

    bool loadInternetUsageCumulativeChart() const
    {
        return m_loadInternetUsageCumulativeChart;
    }

    bool loadInternetUsageChart() const
    {
        return m_loadInternetUsageChart;
    }

    bool loadFPSChart() const
    {
        return m_loadFPSChart;
    }

    bool useAntialiasing() const
    {
        return m_useAntialiasing;
    }

    bool enableAnimations() const
    {
        return m_enableAnimations;
    }

    int stdWidthXValueAxis() const
    {
        return m_stdWidthXValueAxis;
    }

    int stdWidthXDateTimeAxis() const
    {
        return m_stdWidthXDateTimeAxis;
    }

public slots:
    void setLoadRecognizedMaxPredictionObjectsPieSeries(bool loadRecognizedMaxPredictionObjectsPieSeries)
    {
        if (m_loadRecognizedMaxPredictionObjectsPieSeries == loadRecognizedMaxPredictionObjectsPieSeries)
            return;

        m_loadRecognizedMaxPredictionObjectsPieSeries = loadRecognizedMaxPredictionObjectsPieSeries;
        emit loadRecognizedMaxPredictionObjectsPieSeriesChanged(m_loadRecognizedMaxPredictionObjectsPieSeries);
    }
    void setloadRecognizedFacesPieSeries(bool loadRecognizedFacesPieSeries)
    {
        if (m_loadRecognizedFacesPieSeries == loadRecognizedFacesPieSeries)
            return;

        m_loadRecognizedFacesPieSeries = loadRecognizedFacesPieSeries;
        emit loadRecognizedFacesPieSeriesChanged(m_loadRecognizedFacesPieSeries);
    }

    void setloadRecognizedFacesLineSeries(bool loadRecognizedFacesLineSeries)
    {
        if (m_loadRecognizedFacesLineSeries == loadRecognizedFacesLineSeries)
            return;

        m_loadRecognizedFacesLineSeries = loadRecognizedFacesLineSeries;
        emit loadRecognizedFacesLineSeriesChanged(m_loadRecognizedFacesLineSeries);
    }

    void setloadMaxObjectPredictionRateLineSeries(bool loadMaxObjectPredictionRateLineSeries)
    {
        if (m_loadMaxObjectPredictionRateLineSeries == loadMaxObjectPredictionRateLineSeries)
            return;

        m_loadMaxObjectPredictionRateLineSeries = loadMaxObjectPredictionRateLineSeries;
        emit loadMaxObjectPredictionRateLineSeriesChanged(m_loadMaxObjectPredictionRateLineSeries);
    }

    void setloadInternetUsageCumulativeChart(bool loadInternetUsageCumulativeChart)
    {
        if (m_loadInternetUsageCumulativeChart == loadInternetUsageCumulativeChart)
            return;

        m_loadInternetUsageCumulativeChart = loadInternetUsageCumulativeChart;
        emit loadInternetUsageCumulativeChartChanged(m_loadInternetUsageCumulativeChart);
    }

    void setloadInternetUsageChart(bool loadInternetUsageChart)
    {
        if (m_loadInternetUsageChart == loadInternetUsageChart)
            return;

        m_loadInternetUsageChart = loadInternetUsageChart;
        emit loadInternetUsageChartChanged(m_loadInternetUsageChart);
    }

    void setloadFPSChart(bool loadFPSChart)
    {
        if (m_loadFPSChart == loadFPSChart)
            return;

        m_loadFPSChart = loadFPSChart;
        emit loadFPSChartChanged(m_loadFPSChart);
    }

    void setUseAntialiasing(bool useAntialiasing)
    {
        if (m_useAntialiasing == useAntialiasing)
            return;

        m_useAntialiasing = useAntialiasing;
        emit useAntiAliasingChanged(m_useAntialiasing);
    }

    void setEnableAnimations(bool enableAnimations)
    {
        if (m_enableAnimations == enableAnimations)
            return;

        m_enableAnimations = enableAnimations;
        emit enableAnimationsChanged(m_enableAnimations);
    }

    void setStdWidthXValueAxis(int stdWidthXValueAxis)
    {
        if (m_stdWidthXValueAxis == stdWidthXValueAxis)
            return;

        m_stdWidthXValueAxis = stdWidthXValueAxis;
        emit stdWidthXValueAxisChanged(m_stdWidthXValueAxis);
    }

    void setStdWidthXDateTimeAxis(int stdWidthXDateTimeAxis)
    {
        if (m_stdWidthXDateTimeAxis == stdWidthXDateTimeAxis)
            return;

        m_stdWidthXDateTimeAxis = stdWidthXDateTimeAxis;
        emit stdWidthXDateTimeAxisChanged(m_stdWidthXDateTimeAxis);
    }

signals:
    void loadRecognizedMaxPredictionObjectsPieSeriesChanged(bool loadRecognizedMaxPredictionObjectsPieSeries);
    void loadRecognizedFacesPieSeriesChanged(bool loadRecognizedFacesPieSeries);
    void loadRecognizedFacesLineSeriesChanged(bool loadRecognizedFacesLineSeries);
    void loadMaxObjectPredictionRateLineSeriesChanged(bool loadMaxObjectPredictionRateLineSeries);
    void loadInternetUsageCumulativeChartChanged(bool loadInternetUsageCumulativeChart);
    void loadInternetUsageChartChanged(bool loadInternetUsageChart);
    void loadFPSChartChanged(bool loadFPSChart);
    void useAntiAliasingChanged(bool useAntialiasing);
    void enableAnimationsChanged(bool enableAnimations);
    void stdWidthXValueAxisChanged(int stdWidthXValueAxis);
    void stdWidthXDateTimeAxisChanged(int stdWidthXDateTimeAxis);
};

#endif // STATISTICSSETTINGS_HPP
