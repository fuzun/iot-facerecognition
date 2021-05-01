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

#ifndef YUVFRAME_H
#define YUVFRAME_H

#include "ThreadSafeBuffer.hpp"

#include <QPair>
#include <QSize>
#include <QMetaType>

// YUV420
struct YUVFrame : public ThreadSafeBuffer
{
    using BufferType = unsigned char *;
    using layer = QPair<QSize, BufferType>;

    int width = 0, height = 0;

    inline layer y() const
    {
        return { QSize{ width, height }, static_cast<BufferType>(buffer) };
    }

    inline layer u() const
    {
        return { QSize{ width / 2, height / 2 }, static_cast<BufferType>(buffer) + width * height };
    }

    inline layer v() const
    {
        return { QSize{ width / 2, height / 2 }, static_cast<BufferType>(buffer) + width * height * 5 / 4};
    }

    inline size_t size() const
    {
        return width * height * 3 / 2;
    }
};

Q_DECLARE_METATYPE(YUVFrame *);

#endif // YUVFRAME_H
