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

#ifndef INTERFACE_H
#define INTERFACE_H

#include <QObject>

#include "common/Log.hpp"

class SettingObject;
class InterfaceSettings;
class LogModel;

class Interface : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QList<SettingObject *> settingModel READ settingModel NOTIFY settingModelChanged)

    Q_PROPERTY(InterfaceSettings* settings READ settings CONSTANT)

    Q_PROPERTY(LogModel* logModel READ logModel CONSTANT)

    QList<SettingObject *> m_settingModel;
    InterfaceSettings* m_settings;
    LogModel* m_logModel;

    void setupSettings();

public:
    Q_INVOKABLE static QString readTextFile(const QUrl &fileName, bool convertLineBreak = true);

public:
    explicit Interface(QObject *parent);

    QList<SettingObject *> settingModel() const;
    InterfaceSettings* settings() const;
    LogModel* logModel() const;

signals:
    void settingModelChanged(QList<SettingObject *> settingModel);

    Q_INVOKABLE void log(const QVariant& str, Log::LogType type = Log::INFORMATION);

    void setLoggingToFile(bool enabled);
};

#endif // INTERFACE_H
