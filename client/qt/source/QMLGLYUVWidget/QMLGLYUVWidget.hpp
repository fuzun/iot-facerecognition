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

/*
 * This file contains portions from "Scene Graph - OpenGL Under QML" article of official Qt Framework docs.
 * This file contains portions from "SampleYUVRenderer" project of MasterAler.
*/

#ifndef QMLGLYUVWIDGET_H
#define QMLGLYUVWIDGET_H

#include <QQuickItem>
#include <QOpenGLFunctions>

// Microsoft related:
#ifdef interface
#undef interface
#endif
#ifdef ERROR
#undef ERROR
#endif

struct YUVFrame;
class QOpenGLTexture;

class QMLGLYUVRenderer : public QObject, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit QMLGLYUVRenderer(QObject* parent);
    ~QMLGLYUVRenderer();

    inline void setViewportSize(const QSize &size) { m_viewportSize = size; }
    inline void setWindow(QQuickWindow *window) { m_window = window; }
    inline void setFrame(YUVFrame *frame) { m_frame = frame; };

public slots:
    void init();
    void paint();

private:
    QSize m_viewportSize;
    class QOpenGLShaderProgram *m_program = nullptr;
    QQuickWindow *m_window = nullptr;

    YUVFrame *m_frame = nullptr;

    QOpenGLTexture *m_textureY = nullptr;
    QOpenGLTexture *m_textureU = nullptr;
    QOpenGLTexture *m_textureV = nullptr;

    GLuint id_y = 0, id_u = 0, id_v = 0;

    int textureUniformY = 0, textureUniformU = 0, textureUniformV = 0;
};

class QMLGLYUVWidget : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(YUVFrame* frame READ frame WRITE setFrame NOTIFY frameChanged)

public:
    QMLGLYUVWidget();

    YUVFrame* frame() const;

signals:
    void frameChanged(YUVFrame* frame);

public slots:
    void sync();
    void cleanup();
    void setFrame(YUVFrame* frame);

private slots:
    void handleWindowChanged(QQuickWindow *win);

private:
    void releaseResources() override;

    QMLGLYUVRenderer *m_renderer = nullptr;
    YUVFrame* m_frame = nullptr;
};

#endif // QMLGLYUVWIDGET_H
