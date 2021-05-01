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

#include "LogModel.hpp"

#include "common/Log.hpp"

LogModel::LogModel(QObject *parent) : QAbstractListModel(parent)
{

}

QHash<int, QByteArray> LogModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TimeStampRole] = "timeStamp";
    roles[TypeRole] = "type";
    roles[ContextRole] = "context";
    return roles;
}

int LogModel::count() const
{
    return m_count;
}

void LogModel::add(const Log &log)
{
    beginInsertRows(QModelIndex(), 0, 0);
    m_data.push_back(log);
    endInsertRows();

    m_count++;
    emit countChanged(m_count);
}

QVariant LogModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    const Log& log = m_data[rowCount() - index.row() - 1];

    switch (role)
    {
    case TimeStampRole:
        return QVariant(log.timeStamp());
    case TypeRole:
        return QVariant(log.type());
    case ContextRole:
        return QVariant(log.ctx());
    default:
        return QVariant();
    }
}

int LogModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_data.size();
}

