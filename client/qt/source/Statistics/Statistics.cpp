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
#include "Statistics.hpp"

#include <QLineSeries>
#include <QBarSet>
#include <QPieSeries>
#include <QBarSeries>
#include <QQmlEngine>

#include "common/common.hpp"
#include "StatisticsSettings.hpp"
#include "common/SettingObject.hpp"

using namespace QtCharts;
using namespace StatisticsTypes;

Statistics::Statistics(QObject *parent) :
    QObject(parent),
    m_settings(new StatisticsSettings(this))
{
    qmlRegisterUncreatableType<Statistics>("com.iotfacerecognition.statistics", 1, 0, "Statistics", "");
    qmlRegisterUncreatableType<StatisticsSettings>("com.iotfacerecognition.settings", 1, 0, "StatisticsSettings", "");

    setupSettings();
}

Statistics::~Statistics()
{
    // Deregister all:
    {
        auto it = std::begin(m_series);
        while (it != std::end(m_series))
        {
            [&]() {
                if (!it.value()->ptr)
                    return;

                bool ret = deregisterSeries(it.key(), false);
                assert(ret);
                it = m_series.erase(it);
            }();

            ++it;
        }
    }
}

bool Statistics::registerSeries(SeriesType seriesType)
{
    if (m_series.contains(seriesType))
        return false;

    Series* series = new Series;

    m_series.insert(seriesType, series);

    return true;
}

bool Statistics::deregisterSeries(SeriesType seriesType, bool _remove)
{
    if (!m_series.contains(seriesType))
        return false;

    const auto ptr = m_series[seriesType]->ptr;

    if (ptr)
        detachSeries(ptr);

    if (const auto castedPtr = dynamic_cast<QLineSeries*>(ptr))
    {
        delete static_cast< DataType<decltype(castedPtr)>::ctnrType * >( m_series[seriesType]->data );
    }
    else if (const auto castedPtr = dynamic_cast<QPieSeries*>(ptr))
    {
        delete static_cast< DataType<decltype(castedPtr)>::ctnrType * >( m_series[seriesType]->data );
    }
    else
    {
        assert(false);
        return false;
    }

    delete m_series[seriesType];
    m_series[seriesType] = nullptr;

    if (_remove)
    {
        m_series.remove(seriesType);
    }

    return true;
}

bool Statistics::attachSeries(QObject *object)
{
    auto series = getSeries(object);

    const auto name = getSeriesType(object);
    if (!name)
        return false;

    if (!series)
    {
        if (registerSeries(name.value()))
        {
            series = getSeries(object);

            if (!series)
                return false;
        }
        else
            return false;
    }

    assert(series->ptr == nullptr); // must detach first

    bool result = [&]() {
        const auto attach = [&](const auto ptr) {
            auto data = static_cast< typename DataType<decltype(ptr) const>::ctnrType * >(series->data);

            if (!data)
            {
                data = new typename DataType<decltype(ptr) const>::ctnrType;
                series->data = data;
            }
            //else if (!data->isEmpty())
            //{
                // load the data into the series
                loadData(ptr, data);
            //}

            return true;
        };

        if (const auto ptr = dynamic_cast<QLineSeries*>(object))
        {
            return attach(ptr);
        }
        else if (const auto ptr = dynamic_cast<QPieSeries*>(object))
        {
            return attach(ptr);
        }
        else
        {
            assert(false);
            return false;
        }
    }();

    if (result)
        series->ptr = object;

    return result;
}

bool Statistics::detachSeries(QObject *object)
{
    const auto series = getSeries(object);

    if (!series)
        return false;

    bool result = [&]() -> bool {
        const auto detach = [&](const auto ptr) {
            const auto data = static_cast< typename DataType<decltype(ptr) const>::ctnrType * >(series->data);

            assert(data);

            unloadData(ptr, data);

            return true;
        };

        if (const auto ptr = dynamic_cast<QLineSeries*>(object))
        {
            return detach(ptr);
        }
        else if (const auto ptr = dynamic_cast<QPieSeries*>(object))
        {
            return detach(ptr);
        }
        else
        {
            assert(false);
            return false;
        }
    }();

    if (result)
        series->ptr = nullptr;

    return result;
}

StatisticsSettings *Statistics::settings() const
{
    return m_settings;
}

QList<SettingObject *> Statistics::settingModel() const
{
    return m_settingModel;
}

void Statistics::addData(SeriesType seriesType, const QPointF &data)
{
    const auto series = getSeries(seriesType);

    if (!series)
        return;

    const auto ptr = dynamic_cast<QLineSeries*>(series->ptr);

    if (ptr)
    {
        const auto adjust = [ptr, data] (const char* propName, bool less) {
            QVariant var = ptr->property(propName);

            if (!var.isValid() || !var.canConvert(QVariant::PointF))
            {
                ptr->setProperty(propName, data);
            }
            else
            {
                QPointF point = var.toPointF();

                if (((less && (data.x() < point.x())) ||
                        (!less && (data.x() > point.x()))))
                    point.setX(data.x());

                if (((less && (data.y() < point.y())) ||
                        (!less && (data.y() > point.y()))))
                    point.setY(data.y());

                ptr->setProperty(propName, point);
            }
        };

        adjust("min", true);
        adjust("max", false);

        ptr->append(data);

        if (const auto seriesAverage = getAverageSeries(ptr))
        {
            auto averageCalculate = [] (auto average, auto value, auto size) {
                return (average + ( (value - average) / (size) ));
            };

            if (const auto ptr1 = dynamic_cast<QBarSet*>(seriesAverage))
            {
                qreal oldAverage = 0;
                const auto count = ptr1->count();

                if (ptr1->count() >= 1)
                    oldAverage = ptr1->at(count - 1);

                ptr1->append(averageCalculate(oldAverage, data.y(), count + 1));
            }

            else if (const auto ptr2 =  dynamic_cast<QLineSeries*>(seriesAverage))
            {
                qreal oldAverage = 0;
                const auto count = ptr2->count();

                if (ptr2->count() >= 1)
                    oldAverage = ptr2->pointsVector().constLast().y();

                ptr2->append(data.x(), averageCalculate(oldAverage, data.y(), count + 1));
            }
        }
    }
    else
    {
        typename DataType<QLineSeries*const>::ctnrType * ctn;

        if (!series->data)
        {
            ctn = new typename DataType<decltype(ptr)>::ctnrType;
            series->data = ctn;
        }
        else
            ctn = static_cast< decltype(ctn) >(series->data);

        ctn->append(data);
    }
}

void Statistics::incrementData(SeriesType seriesType, const QString& data)
{
    const auto series = getSeries(seriesType);

    if (!series)
        return;

    const auto ptr = dynamic_cast<QPieSeries*>(series->ptr);

    if (ptr)
    {
        [&]() {
            for (auto&& i : ptr->slices())
            {
                if (i->label() == data)
                {
                    i->setValue(i->value() + 1);
                    return;
                }
            }

            const auto slice = ptr->append(data, 1);
            slice->setLabelVisible(true);
        }();
    }
    else
    {
        typename DataType<decltype (ptr)>::ctnrType * ctn;

        if (!series->data)
        {
            ctn = new typename DataType<decltype(ptr)>::ctnrType;
            series->data = ctn;
        }
        else
            ctn = static_cast< decltype(ctn) >(series->data);

        [&]() {
            for (auto& i : *ctn)
            {
                if (i.first == data)
                {
                    i = {i.first, i.second + 1};
                    return;
                }
            }

            ctn->append({data, 1});
        }();
    }
}

Statistics::Series *Statistics::getSeries(SeriesType seriesType)
{
    if (!m_series.contains(seriesType))
        return nullptr;

    return m_series[seriesType];
}

QObject *Statistics::getAverageSeries(QLineSeries *lineSeries)
{
    const auto propAvgSeries = lineSeries->property("averageSeries");

    if (!propAvgSeries.isValid())
        return nullptr;

    return qvariant_cast<QObject *>(propAvgSeries);
}

std::optional<Statistics::SeriesType> Statistics::getSeriesType(const QObject *object)
{
    QVariant var = object->property("type");

    if (var.isValid() && var.canConvert(QVariant::Int))
        return static_cast<SeriesType>(var.toInt());
    else
        return std::nullopt;
}

Statistics::Series *Statistics::getSeries(const QObject *object)
{
    if (!object)
        return nullptr;

    if (const auto name = getSeriesType(object))
        return getSeries(name.value());
    else
        return nullptr;
}

void Statistics::setupSettings()
{
    using SettingType = SettingObject::SettingType;

    static const auto& newSetting = SettingObject::newSetting;
    static const auto& makePropPair = SettingObject::makePropertyPair;

    newSetting(m_settingModel, this,
        {
            tr("Standard width of X ValueAxis (in terms of units):"),
            {
                {SettingObject::keyType, SettingType::TextFieldInt},
                {SettingObject::keyMin, 1}
            },
            makePropPair(m_settings, "stdWidthXValueAxis")
        });

    newSetting(m_settingModel, this,
        {
            tr("Standard width of X DateTimeAxis (in seconds):"),
            {
                {SettingObject::keyType, SettingType::TextFieldInt},
                {SettingObject::keyMin, 1}
            },
            makePropPair(m_settings, "stdWidthXDateTimeAxis")
        });

    newSetting(m_settingModel, this,
        {
            tr("Enable antialiasing (may impact performance)?"),
            {
                {SettingObject::keyType, SettingType::ToggleButton}
            },
            makePropPair(m_settings, "useAntialiasing")
        });

    newSetting(m_settingModel, this,
        {
            tr("Enable antialiasing (may impact performance)?"),
            {
                {SettingObject::keyType, SettingType::ToggleButton}
            },
            makePropPair(m_settings, "useAntialiasing")
        });

    newSetting(m_settingModel, this,
        {
            tr("Enable animations?"),
            {
                {SettingObject::keyType, SettingType::ToggleButton}
            },
            makePropPair(m_settings, "enableAnimations")
        });

    newSetting(m_settingModel, this,
        {
            tr("Load 'FPS' line chart?"),
            {
                {SettingObject::keyType, SettingType::ToggleButton}
            },
            makePropPair(m_settings, "loadFPSChart")
        });


    newSetting(m_settingModel, this,
        {
            tr("Load 'Recognized Objects with Maximum Prediction' pie chart?"),
            {
                {SettingObject::keyType, SettingType::ToggleButton}
            },
            makePropPair(m_settings, "loadRecognizedMaxPredictionObjectsPieSeries")
        });

    newSetting(m_settingModel, this,
        {
            tr("Load 'Recognized Faces' pie chart?"),
            {
                {SettingObject::keyType, SettingType::ToggleButton}
            },
            makePropPair(m_settings, "loadRecognizedFacesPieSeries")
        });

    newSetting(m_settingModel, this,
        {
            tr("Load 'Recognized Faces' line chart?"),
            {
                {SettingObject::keyType, SettingType::ToggleButton}
            },
            makePropPair(m_settings, "loadRecognizedFacesLineSeries")
        });

    newSetting(m_settingModel, this,
        {
            tr("Load 'Maximum Object Prediction Rate' line chart?"),
            {
                {SettingObject::keyType, SettingType::ToggleButton}
            },
            makePropPair(m_settings, "loadMaxObjectPredictionRateLineSeries")
        });

    newSetting(m_settingModel, this,
        {
            tr("Load 'Cumulative Internet Usage' line chart?"),
            {
                {SettingObject::keyType, SettingType::ToggleButton}
            },
            makePropPair(m_settings, "loadInternetUsageCumulativeChart")
        });

    newSetting(m_settingModel, this,
        {
            tr("Load 'Internet Usage' line chart?"),
            {
                {SettingObject::keyType, SettingType::ToggleButton}
            },
            makePropPair(m_settings, "loadInternetUsageChart")
        });

    newSetting(m_settingModel, this,
        {
            tr("Load 'FPS' line chart?"),
            {
                {SettingObject::keyType, SettingType::ToggleButton}
            },
            makePropPair(m_settings, "loadFPSChart")
        });

    emit settingModelChanged(m_settingModel);
}

void Statistics::loadData(QtCharts::QLineSeries* object, StatisticsTypes::DataType<decltype(object) const>::ctnrType * data)
{
    const auto minmax = this->minmax(data);

    object->setProperty("min", minmax.first);
    object->setProperty("max", minmax.second);

    const auto averageSeries = getAverageSeries(object);

    if (averageSeries)
    {
        const auto ptr1 = dynamic_cast<QBarSet*>(averageSeries);
        const auto ptr2 = dynamic_cast<QLineSeries*>(averageSeries);

        const auto averageName = QString("%1 (Average)").arg(object->name());

        if (ptr2)
            ptr2->setName(averageName);
        else if (ptr1)
            ptr1->setLabel(averageName);

        qreal sumY = 0;

        int ctr = 1;
        for (const auto& i : std::as_const(*data))
        {
            sumY += i.y();

            const auto y = sumY / ctr;

            if (ptr1)
                ptr1->append(y);
            else if (ptr2)
                ptr2->append(i.x(), y);

            ++ctr;
        }
    }

    object->replace(std::move(*data));
}

void Statistics::loadData(QPieSeries *object, StatisticsTypes::DataType<decltype(object) const>::ctnrType * data)
{
    object->clear();

    for (const auto& i : std::as_const(*data))
    {
        const auto ptrSlice = object->append(i.first, i.second);
        ptrSlice->setLabelVisible(true);
    }
}

void Statistics::unloadData(QLineSeries *object, StatisticsTypes::DataType<decltype(object) const>::ctnrType * data)
{
    *data = object->pointsVector();
}

void Statistics::unloadData(QPieSeries *object, StatisticsTypes::DataType<decltype(object) const>::ctnrType * data)
{
    data->clear();

    for (auto&& i : object->slices())
    {
        data->append({i->label(), i->value()});
    }
}

template<class T>
QPair<QPointF, QPointF> Statistics::minmax(T data)
{
    static const auto xCompare = [](const QPointF& a, const QPointF& b) {
        return a.x() < b.x();
    };

    static const auto yCompare = [](const QPointF& a, const QPointF& b) {
        return a.y() < b.y();
    };

    const auto x_minmax = std::minmax_element(data->begin(), data->end(), xCompare);
    const auto y_minmax = std::minmax_element(data->begin(), data->end(), yCompare);

    QPointF min { x_minmax.first->x(), y_minmax.first->y() };
    QPointF max { x_minmax.second->x(), y_minmax.second->y() };

    return {min, max};
}
