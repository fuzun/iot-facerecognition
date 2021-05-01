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

#ifndef INTERFACESETTINGS_H
#define INTERFACESETTINGS_H

#include "common/BaseSettings.hpp"

class InterfaceSettings : public BaseSettings
{
    Q_OBJECT

    Q_PROPERTY(bool darkMode READ darkMode WRITE setDarkMode NOTIFY darkModeChanged)
    Q_PROPERTY(bool logToFile READ logToFile WRITE setLogToFile NOTIFY logToFileChanged)
    Q_PROPERTY(bool testMode READ testMode WRITE setTestMode NOTIFY testModeChanged)

    bool m_darkMode = true;
    bool m_logToFile = true;
    bool m_testMode = false;

public:
    inline explicit InterfaceSettings(QObject *parent = nullptr, bool load = true) : BaseSettings(parent)
    {
        qRegisterMetaType<InterfaceSettings *>();

        if (!load)
            return;

        this->load(InterfaceSettings::staticMetaObject);
    }

    inline ~InterfaceSettings()
    {
        save();
    }

    Q_INVOKABLE void save()
    {
        BaseSettings::save(InterfaceSettings::staticMetaObject);
    }

    Q_INVOKABLE void reset()
    {
        InterfaceSettings nonLoadedInstance(this, false);
        BaseSettings::reset(InterfaceSettings::staticMetaObject, &nonLoadedInstance);
    }

    inline bool darkMode() const
    {
        return m_darkMode;
    }

    inline bool logToFile() const
    {
        return m_logToFile;
    }

    inline bool testMode() const
    {
        return m_testMode;
    }

public slots:
    inline void setDarkMode(bool darkMode)
    {
        if (m_darkMode == darkMode)
            return;

        m_darkMode = darkMode;
        emit darkModeChanged(m_darkMode);
    }

    inline void setLogToFile(bool logToFile)
    {
        if (m_logToFile == logToFile)
            return;

        m_logToFile = logToFile;
        emit logToFileChanged(m_logToFile);
    }

    inline void setTestMode(bool testMode)
    {
        if (m_testMode == testMode)
            return;

        m_testMode = testMode;
        emit testModeChanged(m_testMode);
    }

signals:
    void darkModeChanged(bool darkMode);
    void logToFileChanged(bool logToFile);
    void testModeChanged(bool testMode);
};

#endif // INTERFACESETTINGS_H
