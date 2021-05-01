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
#ifndef STATISTICS_HPP
#define STATISTICS_HPP

#include <QObject>
#include <QMap>
#include <QVariant>
#include <QPointF>

#include <optional>

class StatisticsSettings;
class SettingObject;

namespace QtCharts
{
class QPieSeries;
class QLineSeries;
}

namespace StatisticsTypes
{
    template<typename T>
    struct DataType;

    template<>
    struct DataType< QtCharts::QLineSeries * const > {
        using type = QPointF;

        using ctnrType = QVector< type >;
    };

    template<>
    struct DataType< QtCharts::QPieSeries * const > {
        using type = QPair<QString, qreal>;

        using ctnrType = QVector< type >;
    };
}

class Statistics : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QList<SettingObject *> settingModel READ settingModel NOTIFY settingModelChanged)
    Q_PROPERTY(StatisticsSettings* settings READ settings CONSTANT)

public:
    enum SeriesType {
        RecognizedFacesPieSeries,
        RecognizedMaxPredictionObjectsPieSeries,
        TaggedFacesLineSeries,
        UntaggedFacesLineSeries,
        MaximumObjectPredictionRateLineSeries,
        InternetUsageSentCumulativeLineSeries,
        InternetUsageReceivedCumulativeLineSeries,
        InternetUsageSentLineSeries,
        InternetUsageReceivedLineSeries,
        FPSLineSeries
    };
    Q_ENUM(SeriesType);


    explicit Statistics(QObject *parent = nullptr);
    ~Statistics();

    Q_INVOKABLE bool registerSeries(SeriesType seriesType);
    Q_INVOKABLE bool deregisterSeries(SeriesType seriesType, bool _remove = true);

    Q_INVOKABLE bool attachSeries(QObject* object);
    Q_INVOKABLE bool detachSeries(QObject* object);

    StatisticsSettings* settings() const;
    QList<SettingObject *> settingModel() const;

    void addData(SeriesType seriesType, const QPointF& data);
    void incrementData(SeriesType seriesType, const QString& data);

signals:
    void settingModelChanged(QList<SettingObject *> settingModel);

private:
    void setupSettings();

    struct Series {
        void* data = nullptr;

        QObject* ptr = nullptr;

        bool isAttached() {
            return (ptr != nullptr);
        }
    };

    template<class T>
    QPair<QPointF, QPointF> minmax(T data);

    void loadData(QtCharts::QLineSeries* object,
        StatisticsTypes::DataType<decltype(object) const>::ctnrType * data);

    void loadData(QtCharts::QPieSeries* object,
        StatisticsTypes::DataType<decltype(object) const>::ctnrType * data);

    void unloadData(QtCharts::QLineSeries* object,
        StatisticsTypes::DataType<decltype(object) const>::ctnrType * data);

    void unloadData(QtCharts::QPieSeries* object,
        StatisticsTypes::DataType<decltype(object) const>::ctnrType * data);

    QMap<SeriesType, Series*> m_series;

    Series* getSeries(SeriesType seriesType);
    Series* getSeries(const QObject* object);

    QObject* getAverageSeries(QtCharts::QLineSeries* lineSeries);

    StatisticsSettings* m_settings;

    QList<SettingObject *> m_settingModel;

    std::optional<SeriesType> getSeriesType(const QObject* object);
};

#endif // STATISTICS_HPP
