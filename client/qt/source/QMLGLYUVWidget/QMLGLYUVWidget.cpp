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


#include "QMLGLYUVWidget.hpp"

#include <QOpenGLShaderProgram>
#include <QSGRendererInterface>
#include <QQuickWindow>
#include <QOpenGLTexture>
#include <QRunnable>

#include "common/YUVFrame.hpp"

#define ATTRIB_VERTEX 0
#define ATTRIB_TEXTURE 1

#ifndef GL_UNPACK_ROW_LENGTH
#ifndef GL_UNPACK_ROW_LENGTH_EXT
#error "GL_UNPACK_ROW_LENGTH or GL_UNPACK_ROW_LENGTH_EXT is not defined!"
#endif
#define GL_UNPACK_ROW_LENGTH GL_UNPACK_ROW_LENGTH_EXT
#endif

#ifndef GL_UNPACK_SKIP_PIXELS
#ifndef GL_UNPACK_SKIP_PIXELS_EXT
#error "GL_UNPACK_SKIP_PIXELS or GL_UNPACK_SKIP_PIXELS_EXT is not defined!"
#endif
#define GL_UNPACK_SKIP_PIXELS GL_UNPACK_SKIP_PIXELS_EXT
#endif

#ifndef GL_UNPACK_SKIP_ROWS
#ifndef GL_UNPACK_SKIP_ROWS_EXT
#error "GL_UNPACK_SKIP_ROWS or GL_UNPACK_SKIP_ROWS_EXT is not defined!"
#endif
#define GL_UNPACK_SKIP_ROWS GL_UNPACK_SKIP_ROWS_EXT
#endif

QMLGLYUVRenderer::QMLGLYUVRenderer(QObject* parent) : QObject(parent)
{

}

QMLGLYUVRenderer::~QMLGLYUVRenderer()
{
    delete m_textureY;
    delete m_textureU;
    delete m_textureV;

    delete m_program;
}

void QMLGLYUVRenderer::init()
{    
    if (!m_program)
    {
        QSGRendererInterface *rif = m_window->rendererInterface();
        Q_ASSERT(rif->graphicsApi() == QSGRendererInterface::OpenGL || rif->graphicsApi() == QSGRendererInterface::OpenGLRhi);

        initializeOpenGLFunctions();

        m_program = new QOpenGLShaderProgram();
        m_program->addCacheableShaderFromSourceCode(QOpenGLShader::Vertex,
                                                    "attribute vec4 vertexIn; \
                                                    attribute vec2 textureIn; \
                                                    varying vec2 textureOut;  \
                                                    void main(void)           \
                                                    {                         \
                                                        gl_Position = vertexIn; \
                                                        textureOut = textureIn; \
                                                    }");
#ifdef QT_OPENGL_ES_2
        m_program->addCacheableShaderFromSourceCode(QOpenGLShader::Fragment,
                                                    "precision mediump float; \
                                                    varying vec2 textureOut; \
                                                    uniform sampler2D tex_y; \
                                                    uniform sampler2D tex_u; \
                                                    uniform sampler2D tex_v; \
                                                    void main(void) \
                                                    { \
                                                        vec3 yuv; \
                                                        vec3 rgb; \
                                                        yuv.x = texture2D(tex_y, textureOut).r; \
                                                        yuv.y = texture2D(tex_u, textureOut).r - 0.5; \
                                                        yuv.z = texture2D(tex_v, textureOut).r - 0.5; \
                                                        rgb = mat3( 1,       1,         1, \
                                                                    0,       -0.39465,  2.03211, \
                                                                    1.13983, -0.58060,  0) * yuv; \
                                                        gl_FragColor = vec4(rgb, 1); \
                                                    }");
#else
        m_program->addCacheableShaderFromSourceCode(QOpenGLShader::Fragment,
                                                    "varying vec2 textureOut; \
                                                    uniform sampler2D tex_y; \
                                                    uniform sampler2D tex_u; \
                                                    uniform sampler2D tex_v; \
                                                    void main(void) \
                                                    { \
                                                        vec3 yuv; \
                                                        vec3 rgb; \
                                                        yuv.x = texture2D(tex_y, textureOut).r; \
                                                        yuv.y = texture2D(tex_u, textureOut).r - 0.5; \
                                                        yuv.z = texture2D(tex_v, textureOut).r - 0.5; \
                                                        rgb = mat3( 1,       1,         1, \
                                                                    0,       -0.39465,  2.03211, \
                                                                    1.13983, -0.58060,  0) * yuv; \
                                                        gl_FragColor = vec4(rgb, 1); \
                                                    }");
#endif
        m_program->bindAttributeLocation("vertexIn", ATTRIB_VERTEX);
        m_program->bindAttributeLocation("textureIn", ATTRIB_TEXTURE);

        m_program->link();

        textureUniformY = m_program->uniformLocation("tex_y");
        textureUniformU = m_program->uniformLocation("tex_u");
        textureUniformV = m_program->uniformLocation("tex_v");

        if(!m_textureY)
        {
            m_textureY = new QOpenGLTexture(QOpenGLTexture::Target2D);
            m_textureY->create();
        }

        if(!m_textureU)
        {
            m_textureU = new QOpenGLTexture(QOpenGLTexture::Target2D);
            m_textureU->create();
        }

        if(!m_textureV)
        {
            m_textureV = new QOpenGLTexture(QOpenGLTexture::Target2D);
            m_textureV->create();
        }

        id_y = m_textureY->textureId();
        id_u = m_textureU->textureId();
        id_v = m_textureV->textureId();
    }
}

void QMLGLYUVRenderer::paint()
{
    if (!m_frame || !m_frame->tryLockForRead(250))
        return;

    if (!m_frame->buffer)
    {
        m_frame->unlock();
        return;
    }

    init();

    m_window->beginExternalCommands();

    m_program->bind();

    m_program->enableAttributeArray(ATTRIB_VERTEX);
    m_program->enableAttributeArray(ATTRIB_TEXTURE);

    static const GLfloat vertexVertices[] = {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        -1.0f, 1.0f,
        1.0f, 1.0f,
    };

    static const GLfloat textureVertices[] = {
        0.0f,  1.0f,
        1.0f,  1.0f,
        0.0f,  0.0f,
        1.0f,  0.0f,
    };

    m_program->setAttributeArray(ATTRIB_VERTEX, GL_FLOAT, vertexVertices,2, 0);
    m_program->setAttributeArray(ATTRIB_TEXTURE, GL_FLOAT, textureVertices,2, 0);

    glViewport(0, 0, m_viewportSize.width(), m_viewportSize.height());

    glDisable(GL_DEPTH_TEST);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id_y);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, m_frame->y().first.width(), m_frame->y().first.height(), 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_frame->y().second);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, id_u);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, m_frame->u().first.width(), m_frame->u().first.height(), 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_frame->u().second);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, id_v);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, m_frame->v().first.width(), m_frame->v().first.height(), 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_frame->v().second);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glUniform1i(textureUniformY, 0);
    glUniform1i(textureUniformU, 1);
    glUniform1i(textureUniformV, 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    m_program->disableAttributeArray(ATTRIB_TEXTURE);
    m_program->disableAttributeArray(ATTRIB_VERTEX);

    m_program->release();

    m_window->resetOpenGLState();
    m_window->endExternalCommands();

    m_frame->unlock();
}

QMLGLYUVWidget::QMLGLYUVWidget()
{
    connect(this, &QQuickItem::windowChanged, this, &QMLGLYUVWidget::handleWindowChanged);
}

YUVFrame *QMLGLYUVWidget::frame() const
{
    return m_frame;
}

void QMLGLYUVWidget::sync()
{
    window()->setClearBeforeRendering(false);

    if (!m_renderer)
    {
        m_renderer = new QMLGLYUVRenderer(nullptr);
        connect(window(), &QQuickWindow::beforeRendering, m_renderer, &QMLGLYUVRenderer::paint, Qt::DirectConnection);
    }

    m_renderer->setViewportSize(window()->size() * window()->devicePixelRatio());
    m_renderer->setFrame(m_frame);
    m_renderer->setWindow(window());
}

void QMLGLYUVWidget::cleanup()
{
    delete m_renderer;
    m_renderer = nullptr;
}

void QMLGLYUVWidget::setFrame(YUVFrame *frame)
{
    if (m_frame == frame)
        return;

    m_frame = frame;
    emit frameChanged(m_frame);

    if (window())
        window()->update();
}

void QMLGLYUVWidget::handleWindowChanged(QQuickWindow *win)
{
    if (win)
    {
        connect(win, &QQuickWindow::beforeSynchronizing, this, &QMLGLYUVWidget::sync, Qt::DirectConnection);
        connect(win, &QQuickWindow::sceneGraphInvalidated, this, &QMLGLYUVWidget::cleanup, Qt::DirectConnection);

        win->setColor(Qt::black);
    }
}

class CleanupJob : public QRunnable
{
public:
    explicit CleanupJob(QMLGLYUVRenderer *renderer) : m_renderer(renderer) { }
    void run() override { delete m_renderer; }
private:
    QMLGLYUVRenderer *m_renderer;
};

void QMLGLYUVWidget::releaseResources()
{
    window()->scheduleRenderJob(new CleanupJob(m_renderer), QQuickWindow::BeforeSynchronizingStage);
    m_renderer = nullptr;
}
