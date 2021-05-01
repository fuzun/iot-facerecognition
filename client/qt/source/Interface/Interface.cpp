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

#include "Interface.hpp"

#include <QUrl>
#include <QFile>
#include <QQmlEngine>

#include "common/SettingObject.hpp"
#include "InterfaceSettings.hpp"
#include "LogModel.hpp"

#include "common/common.hpp"

Interface::Interface(QObject *parent)
    : QObject(parent),
    m_settings(new InterfaceSettings(this)),
    m_logModel(new LogModel(this))
{
    qRegisterMetaType<QList<SettingObject *>>();
    qRegisterMetaType<LogModel *>();

    qmlRegisterUncreatableType<InterfaceSettings>("com.iotfacerecognition.settings", 1, 0, "InterfaceSettings", "This type is uncreatable!");

    connect(this, &Interface::log, this, [this](const QVariant& str, Log::LogType type = Log::INFORMATION) {
        m_logModel->add(Log(str, type));
        }, Qt::QueuedConnection);

    connect(m_settings, &InterfaceSettings::logToFileChanged, this, &Interface::setLoggingToFile);

    setupSettings();
}

QList<SettingObject *> Interface::settingModel() const
{
    return m_settingModel;
}

InterfaceSettings *Interface::settings() const
{
    return m_settings;
}

LogModel *Interface::logModel() const
{
    return m_logModel;
}

void Interface::setupSettings()
{
    using SettingType = SettingObject::SettingType;

    static const auto& newSetting = SettingObject::newSetting;
    static const auto& makePropPair = SettingObject::makePropertyPair;

    newSetting(m_settingModel, this,
        {
            tr("Dark Mode"),
            {{SettingObject::keyType, SettingType::ToggleButton}},
            makePropPair(m_settings, "darkMode")
        } );

    newSetting(m_settingModel, this,
        {
            tr("Test Mode (throw exception on log::error)"),
            {{SettingObject::keyType, SettingType::ToggleButton}},
            makePropPair(m_settings, "testMode")
        } );

    newSetting(m_settingModel, this,
        {
            tr("Enable logging to file? (log.txt)"),
            {{SettingObject::keyType, SettingType::ToggleButton}},
            makePropPair(m_settings, "logToFile")
        });

    emit settingModelChanged(m_settingModel);
}

QString Interface::readTextFile(const QUrl &fileName, bool convertLineBreak)
{
    if (!fileName.isValid())
        return QString();

    QFile file(fileName.toString());

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();

    QString ret = file.readAll();

    if (convertLineBreak)
        return ret.replace("\r", "").replace("\n", "<br/>");
    else
        return ret;
}
