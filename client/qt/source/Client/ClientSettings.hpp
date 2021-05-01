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

#ifndef CLIENTSETTINGS_H
#define CLIENTSETTINGS_H

#include "common/BaseSettings.hpp"

class ClientSettings : public BaseSettings
{
    Q_OBJECT

    Q_PROPERTY(bool objectDetectionEnabled READ objectDetectionEnabled WRITE setObjectDetectionEnabled NOTIFY objectDetectionEnabledChanged)
    Q_PROPERTY(size_t labelCount READ labelCount WRITE setLabelCount NOTIFY labelCountChanged)
    Q_PROPERTY(QString clientName READ clientName WRITE setClientName NOTIFY clientNameChanged)
    Q_PROPERTY(bool deterministicObjectDetection READ deterministicObjectDetection WRITE setDeterministicObjectDetection NOTIFY deterministicObjectDetectionChanged)
    Q_PROPERTY(bool faceRecognitionEnabled READ faceRecognitionEnabled WRITE setFaceRecognitionEnabled NOTIFY faceRecognitionEnabledChanged)
    Q_PROPERTY(bool ignoreSSLErrors READ ignoreSSLErrors WRITE setIgnoreSSLErrors NOTIFY ignoreSSLErrorsChanged)

    bool m_objectDetectionEnabled = true;
    size_t m_labelCount = 5;
    QString m_clientName {"Default"};
    bool m_deterministicObjectDetection = false;
    bool m_faceRecognitionEnabled = true;
    bool m_ignoreSSLErrors = false;

public:
    inline explicit ClientSettings(QObject *parent = nullptr, bool load = true) : BaseSettings(parent)
    {
        qRegisterMetaType<ClientSettings*>();

        if (!load)
            return;

        this->load(ClientSettings::staticMetaObject);
    }

    inline ~ClientSettings()
    {
        save();
    }

    Q_INVOKABLE void save()
    {
        BaseSettings::save(ClientSettings::staticMetaObject);
    }

    Q_INVOKABLE void reset()
    {
        ClientSettings nonLoadedInstance(this, false);
        BaseSettings::reset(ClientSettings::staticMetaObject, &nonLoadedInstance);
    }

    inline bool objectDetectionEnabled() const
    {
        return m_objectDetectionEnabled;
    }

    inline size_t labelCount() const
    {
        return m_labelCount;
    }

    inline QString clientName() const
    {
        return m_clientName;
    }

    inline bool deterministicObjectDetection() const
    {
        return m_deterministicObjectDetection;
    }

    inline bool faceRecognitionEnabled() const
    {
        return m_faceRecognitionEnabled;
    }

    inline bool ignoreSSLErrors() const
    {
        return m_ignoreSSLErrors;
    }

public slots:
    inline void setObjectDetectionEnabled(bool objectDetectionEnabled)
    {
        if (m_objectDetectionEnabled == objectDetectionEnabled)
            return;

        m_objectDetectionEnabled = objectDetectionEnabled;
        emit objectDetectionEnabledChanged(m_objectDetectionEnabled);
    }

    inline void setLabelCount(size_t labelCount)
    {
        if (m_labelCount == labelCount)
            return;

        m_labelCount = labelCount;
        emit labelCountChanged(m_labelCount);
    }

    inline void setClientName(const QString& clientName)
    {
        if (m_clientName == clientName)
            return;

        m_clientName = clientName;
        emit clientNameChanged(m_clientName);
    }

    inline void setDeterministicObjectDetection(bool deterministicObjectDetection)
    {
        if (m_deterministicObjectDetection == deterministicObjectDetection)
            return;

        m_deterministicObjectDetection = deterministicObjectDetection;
        emit deterministicObjectDetectionChanged(m_deterministicObjectDetection);
    }

    inline void setFaceRecognitionEnabled(bool faceRecognitionEnabled)
    {
        if (m_faceRecognitionEnabled == faceRecognitionEnabled)
            return;

        m_faceRecognitionEnabled = faceRecognitionEnabled;
        emit faceRecognitionEnabledChanged(m_faceRecognitionEnabled);
    }

    inline void setIgnoreSSLErrors(bool ignoreSSLErrors)
    {
        if (m_ignoreSSLErrors == ignoreSSLErrors)
            return;

        m_ignoreSSLErrors = ignoreSSLErrors;
        emit ignoreSSLErrorsChanged(m_ignoreSSLErrors);
    }

signals:
    void objectDetectionEnabledChanged(bool objectDetectionEnabled);
    void labelCountChanged(size_t labelCount);
    void clientNameChanged(QString clientName);
    void deterministicObjectDetectionChanged(bool deterministicObjectDetection);
    void faceRecognitionEnabledChanged(bool faceRecognitionEnabled);
    void ignoreSSLErrorsChanged(bool ignoreSSLErrors);
};

#endif // CLIENTSETTINGS_H
