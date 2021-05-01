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

#include "Log.hpp"

#include <QDateTime>
#include <QCoreApplication>

Log::Log(const QVariant &ctx, LogType type)
    : m_ctx(ctx),
    m_type(type)
{
    QDateTime dateTime = QDateTime::currentDateTime();
    m_timeStamp = dateTime.toString("dd-MM-yyyy / hh:mm:ss");
}

QVariant Log::ctx() const
{
    return m_ctx;
}

Log::LogType Log::type() const
{
    return m_type;
}

QString Log::typeStr() const
{
    switch (m_type)
    {
    case LogType::WARNING:
        return QCoreApplication::tr("WARNING");
    case LogType::ERROR:
        return QCoreApplication::tr("ERROR");
    case LogType::INFORMATION:
    default:
        return QCoreApplication::tr("INFORMATION");
    }
}

QString Log::timeStamp() const
{
    return m_timeStamp;
}
