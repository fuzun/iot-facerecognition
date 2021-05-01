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

#ifndef SETTINGOBJECT_H
#define SETTINGOBJECT_H

#include <QObject>
#include <QVariantMap>
#include <QMetaProperty>
#include <QPair>


class SettingObject : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(QVariantMap ctx READ ctx CONSTANT)
    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)

    Q_PROPERTY(QString keyType MEMBER keyType CONSTANT)
    Q_PROPERTY(QString keyDropDownCtx MEMBER keyDropDownCtx CONSTANT)
    Q_PROPERTY(QString keyMin MEMBER keyMin CONSTANT)
    Q_PROPERTY(QString keyMax MEMBER keyMax CONSTANT)

public:
    inline static const QString keyType {"type"};
    inline static const QString keyDropDownCtx {"model"};
    inline static const QString keyMin {"min"};
    inline static const QString keyMax {"max"};
    inline static const QString keyText {"text"};

    enum SettingType {
        TextField,
        TextFieldInt,
        Button,
        ToggleButton,
        DropDown,
        Slider
    };
    Q_ENUM(SettingType)

    struct Setting {
        QString description;
        QVariantMap ctx;
        QPair<QObject*, QMetaProperty> property { nullptr, QMetaProperty() };
        std::function<void(const QVariant& value)> slot = nullptr;
    };

    static inline QPair<QObject*, QMetaProperty> makePropertyPair(QObject* parent, const char* propertyName)
    {
        assert(parent->property(propertyName).isValid());

        return qMakePair(parent,
            parent->metaObject()->property(parent->metaObject()->indexOfProperty(propertyName)));
    }

    static inline void newSetting(QList<SettingObject*>& list, QObject* parent, const Setting& setting)
    {
        list.push_back(new SettingObject(parent, setting));
    }


private:
    Setting m_setting;

public:
    inline explicit SettingObject(QObject *parent, const Setting& setting)
        : QObject(parent),
        m_setting(setting)
    {
        if (m_setting.property.second.isValid())
        {
            if (m_setting.property.second.hasNotifySignal())
            {
                connect(m_setting.property.first,
                    m_setting.property.second.notifySignal(),
                    this,
                    SettingObject::staticMetaObject.property(SettingObject::staticMetaObject.indexOfProperty("value"))
                        .notifySignal());
            }
        }
    }

    inline QString description() const
    {
        return m_setting.description;
    }

    inline QVariant value() const
    {
        if (m_setting.property.second.isValid())
            return m_setting.property.second.read(m_setting.property.first);
        else
            return QVariant();
    }

    inline QVariantMap ctx() const
    {
        return m_setting.ctx;
    }

    inline void invokeSlot()
    {
        if (m_setting.slot)
            m_setting.slot(value());
    }

public slots:
    inline void setValue(const QVariant& value)
    {
        if (this->value() == value)
            return;

        if (m_setting.slot)
        {
            m_setting.slot(value);
            return;
        }
        else if (!m_setting.property.second.isWritable())
            return;

        if ( m_setting.property.second.write(m_setting.property.first, value) )
            emit valueChanged();
    }

signals:
    void valueChanged();

};

#endif // SETTINGOBJECT_H
