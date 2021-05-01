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
#ifndef BASESETTINGS_H
#define BASESETTINGS_H

#include <QObject>
#include <QSettings>
#include <QMetaObject>
#include <QMetaProperty>

class BaseSettings : public QObject
{
    Q_OBJECT

    QSettings settings;

protected:
    explicit BaseSettings(QObject *parent = nullptr) : QObject(parent) { }

    void load(const QMetaObject& metaObject)
    {
        settings.beginGroup(metaObject.className());

        for (int i = metaObject.propertyOffset(); i < metaObject.propertyCount(); ++i)
        {
            QMetaProperty p = metaObject.property(i);
            p.write(this, settings.value(p.name(), p.read(this)));
        }
    }

    void save(const QMetaObject& metaObject)
    {
        for (int i = metaObject.propertyOffset(); i < metaObject.propertyCount(); ++i)
        {
            QMetaProperty p = metaObject.property(i);
            settings.setValue(p.name(), p.read(this));
        }
    }

    void reset(const QMetaObject& oldMetaObject, const QObject* newObject)
    {
        if (!newObject)
            return;

        for (int i = oldMetaObject.propertyOffset(); i < oldMetaObject.propertyCount(); ++i)
        {
            QMetaProperty p = oldMetaObject.property(i);
            p.write(this, newObject->metaObject()->property(i).read(newObject));
        }
    }
};

#endif // BASESETTINGS_H
