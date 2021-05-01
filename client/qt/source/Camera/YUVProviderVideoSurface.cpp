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
#include "YUVProviderVideoSurface.hpp"

#include <QBuffer>
#include <QByteArray>

#ifdef TURBOJPEG_AVAILABLE
#include "turbojpeg.h"
#endif

#include "yuv2rgb/yuv_rgb.h"

#include "CameraSettings.hpp"
#include "common/YUVFrame.hpp"

YUVProviderVideoSurface::YUVProviderVideoSurface(QObject *parent, YUVFrame* frame, CameraSettings *settings)
    : QAbstractVideoSurface(parent),
    m_frame(frame),
    m_settings(settings)
{
    assert(m_frame);
    assert(m_settings);

#ifdef TURBOJPEG_AVAILABLE
    tjHandle = tjInitCompress();
#endif
}

YUVProviderVideoSurface::~YUVProviderVideoSurface()
{
#ifdef TURBOJPEG_AVAILABLE
    tjDestroy(tjHandle);
#endif

    cleanBuffer();
}

bool YUVProviderVideoSurface::present(const QVideoFrame &frame)
{
    assert(m_frame);

    if (frame.pixelFormat() != QVideoFrame::Format_YUV420P
        && frame.pixelFormat() != QVideoFrame::Format_RGB32)
    {
        setError(IncorrectFormatError);
        return false;
    }

    if (!m_frame)
        return false;

    m_frame->lockForWrite();

    QVideoFrame mapped = const_cast<QVideoFrame&>(frame);

    if(!mapped.isValid() || !mapped.map(QAbstractVideoBuffer::ReadOnly))
    {
        setError(ResourceError);
        return false;
    }

    if (m_frame->buffer && (m_frame->width != mapped.width() || m_frame->height != mapped.height()))
    {
        cleanBuffer();
    }

    if (!m_frame->buffer)
    {
        m_frame->width = mapped.width();
        m_frame->height = mapped.height();
        m_frame->buffer = new YUVFrame::BufferType[m_frame->size()];
    }

    if (frame.pixelFormat() == QVideoFrame::Format_RGB32)
    {
        // convert to YUV420
#ifdef _YUVRGB_SSE2_
        rgb32_yuv420_sseu
#else
        rgb32_yuv420_std
#endif
            (frame.width(),
            frame.height(),
            frame.bits(),
            frame.width() * 4,
            m_frame->y().second,
            m_frame->v().second,
            m_frame->u().second,
            frame.width(),
            (frame.width() + 1) / 2,
            YCbCrType::YCBCR_709);
    }
    else if (frame.pixelFormat() == QVideoFrame::Format_YUV420P)
    {
        memcpy(m_frame->buffer,
            mapped.bits(),
            static_cast<size_t>(mapped.mappedBytes()) > m_frame->size() ? m_frame->size() : mapped.mappedBytes());
    }
    else
    {
        assert(false);
    }

    m_frame->unlock();

    emit updated();

    // Encode JPEG

#ifdef TURBOJPEG_AVAILABLE
    unsigned char* jpegBuffer = nullptr;

    unsigned long bufSize;

    tjCompress2(tjHandle,
        mapped.bits(),
        mapped.width(),
        mapped.bytesPerLine(),
        mapped.height(),
        TJPF_BGRA,
        &jpegBuffer,
        &bufSize,
        TJSAMP_420,
        m_settings->jpegQuality(),
        TJFLAG_FASTDCT);

    if (jpegBuffer && bufSize > 0)
    {
        QByteArray bArray(bufSize, Qt::Initialization::Uninitialized);
        memcpy(bArray.data(), jpegBuffer, bufSize);
        emit jpegReady(bArray);

        tjFree(jpegBuffer);
    }

#else
    const QImage img(mapped.bits(),
        mapped.width(),
        mapped.height(),
        QVideoFrame::imageFormatFromPixelFormat(frame.pixelFormat()));
    QByteArray bArray;
    QBuffer buffer(&bArray);
    img.save(&buffer, "JPEG");
    emit jpegReady(bArray);

#endif

    mapped.unmap();

    return true;
}

void YUVProviderVideoSurface::cleanBuffer()
{
    assert(m_frame);

    if (m_frame)
    {
        m_frame->lockForWrite();
        if (m_frame->buffer)
        {
            delete[] static_cast<YUVFrame::BufferType>(m_frame->buffer);
            m_frame->buffer = nullptr;
        }
        m_frame->unlock();
    }
}

QList<QVideoFrame::PixelFormat> YUVProviderVideoSurface::supportedPixelFormats(QAbstractVideoBuffer::HandleType type) const
{
    QList<QVideoFrame::PixelFormat> ret;

    if (type == QAbstractVideoBuffer::NoHandle)
        return (ret << QVideoFrame::PixelFormat::Format_YUV420P << QVideoFrame::PixelFormat::Format_RGB32);
    else
    {
        return ret;
    }
}
