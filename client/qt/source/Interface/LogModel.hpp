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

#ifndef LOGMODEL_H
#define LOGMODEL_H

#include <QAbstractListModel>

#include "common/Log.hpp"

class LogModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged)

    int m_count = 0;

    QVector<Log> m_data;

public:
    enum Roles
    {
        TimeStampRole = Qt::UserRole,
        TypeRole,
        ContextRole
    };

    explicit LogModel(QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;

    int count() const;

    void add(const Log& log);

    QVariant data(const QModelIndex &index, int role) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

signals:
    void countChanged(int count);
};

#endif // LOGMODEL_H
