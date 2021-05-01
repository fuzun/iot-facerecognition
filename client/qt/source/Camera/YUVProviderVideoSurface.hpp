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
#ifndef YUVPROVIDERVIDEOSURFACE_H
#define YUVPROVIDERVIDEOSURFACE_H

#include <QAbstractVideoSurface>

struct YUVFrame;
class CameraSettings;

class YUVProviderVideoSurface : public QAbstractVideoSurface
{
    Q_OBJECT

    YUVFrame* m_frame = nullptr;

#ifdef TURBOJPEG_AVAILABLE
    void* tjHandle = NULL;
#endif

    CameraSettings *m_settings = nullptr;

public:
    explicit YUVProviderVideoSurface(QObject *parent = nullptr, YUVFrame* frame = nullptr, CameraSettings *settings = nullptr);
    ~YUVProviderVideoSurface();

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType type) const override;
    bool present(const QVideoFrame& frame) override;

private:
    void cleanBuffer();

signals:
    void updated();
    void jpegReady(const QByteArray& data);

};

#endif // YUVPROVIDERVIDEOSURFACE_H
