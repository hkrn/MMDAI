/**

 Copyright (c) 2010-2013  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#include "VideoSurface.h"

#include <QtCore>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QVideoSurfaceFormat>

VideoSurface::VideoSurface(QMediaPlayer *playerRef, QObject *parent)
    : QAbstractVideoSurface(parent),
      m_createdThreadRef(QThread::currentThread()),
      m_playerRef(playerRef),
      m_textureHandle(0)
{
    connect(playerRef, &QMediaPlayer::mediaStatusChanged, this, &VideoSurface::handleMediaStatusChanged);
    playerRef->setVideoOutput(this);
}

VideoSurface::~VideoSurface()
{
    m_playerRef = 0;
}

QList<QVideoFrame::PixelFormat> VideoSurface::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
{
    switch (handleType) {
    case QAbstractVideoBuffer::NoHandle:
        return QList<QVideoFrame::PixelFormat>()
                << QVideoFrame::Format_ARGB32
                << QVideoFrame::Format_ARGB32_Premultiplied
                << QVideoFrame::Format_RGB32;
    default:
        return QList<QVideoFrame::PixelFormat>();
    }
}

bool VideoSurface::isFormatSupported(const QVideoSurfaceFormat &format) const
{
    const QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(format.pixelFormat());
    return imageFormat != QImage::Format_Invalid && !format.frameSize().isEmpty() && format.handleType() == QAbstractVideoBuffer::NoHandle;
}

bool VideoSurface::present(const QVideoFrame &frame)
{
    const QVideoSurfaceFormat &s = surfaceFormat();
    if (s.pixelFormat() != frame.pixelFormat() || s.frameSize() != frame.size()) {
        setError(IncorrectFormatError);
        stop();
        return false;
    }
    else {
        assignVideoFrame(frame);
        return true;
    }
}

bool VideoSurface::start(const QVideoSurfaceFormat &format)
{
    if (isFormatSupported(format)) {
        return QAbstractVideoSurface::start(format);
    }
    return false;
}

void VideoSurface::stop()
{
    assignVideoFrame(QVideoFrame());
    QAbstractVideoSurface::stop();
}

void VideoSurface::initialize()
{
    Q_ASSERT(m_createdThreadRef != QThread::currentThread());
    const QSize &size = surfaceFormat().frameSize();
    if (size.isValid() && !m_program) {
        m_program.reset(new QOpenGLShaderProgram());
        m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/gui/texture.vsh");
        m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/gui/texture.fsh");
        m_program->bindAttributeLocation("inPosition", 0);
        m_program->bindAttributeLocation("inTexCoord", 1);
        m_program->link();
        Q_ASSERT(m_program->isLinked());
        QVarLengthArray<QVector2D> positions;
        /* left is for inPosition, right is for inTexCoord */
        positions.append(QVector2D(-1, -1)); positions.append(QVector2D(0, 1));
        positions.append(QVector2D(1, -1));  positions.append(QVector2D(1, 1));
        positions.append(QVector2D(-1, 1));  positions.append(QVector2D(0, 0));
        positions.append(QVector2D(1, 1));   positions.append(QVector2D(1, 0));
        allocateBuffer(positions.data(), positions.size() * sizeof(QVector2D), m_vbo);
        m_vao.reset(new QOpenGLVertexArrayObject());
        if (m_vao->create()) {
            m_vao->bind();
            m_program->enableAttributeArray(0);
            m_program->enableAttributeArray(1);
            bindAttributeBuffers();
            m_vao->release();
        }
        glGenTextures(1, &m_textureHandle);
        glBindTexture(GL_TEXTURE_2D, m_textureHandle);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#if defined(QT_OPENGL_ES_2)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.width(), size.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
#else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.width(), size.height(), 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, 0);
#endif
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void VideoSurface::release()
{
    Q_ASSERT(m_createdThreadRef != QThread::currentThread());
    m_program.reset();
    m_vao.reset();
    m_vbo.reset();
    glDeleteTextures(1, &m_textureHandle);
}

void VideoSurface::renderVideoFrame()
{
    Q_ASSERT(m_createdThreadRef != QThread::currentThread());
    QVideoFrame localVideoFrame;
    {
        QMutexLocker locker(&m_videoFrameLock); Q_UNUSED(locker);
        localVideoFrame = m_videoFrame;
    }
    const QSize &size = localVideoFrame.size();
    if (m_program && localVideoFrame.isValid() && !size.isEmpty()) {
        if (localVideoFrame.map(QAbstractVideoBuffer::ReadOnly)) {
            bindProgram();
            glBindTexture(GL_TEXTURE_2D, m_textureHandle);
#if defined(QT_OPENGL_ES_2)
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.width(), size.height(), GL_RGBA, GL_UNSIGNED_BYTE, localVideoFrame.bits());
#else
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.width(), size.height(), GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, localVideoFrame.bits());
#endif
            m_program->setUniformValue("mainTexture", 0);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glBindTexture(GL_TEXTURE_2D, 0);
            releaseProgram();
            localVideoFrame.unmap();
        }
    }
}

void VideoSurface::handleMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (status == QMediaPlayer::LoadedMedia) {
        m_playerRef->stop();
        m_playerRef->pause();
    }
}

void VideoSurface::allocateBuffer(const void *data, size_t size, QScopedPointer<QOpenGLBuffer> &buffer)
{
    buffer.reset(new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer));
    buffer->create();
    buffer->bind();
    buffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
    buffer->allocate(data, size);
    buffer->release();
}

void VideoSurface::assignVideoFrame(const QVideoFrame &value)
{
    QMutexLocker locker(&m_videoFrameLock); Q_UNUSED(locker);
    m_videoFrame = value;
}

void VideoSurface::bindAttributeBuffers()
{
    static const size_t kStride = sizeof(QVector2D) * 2;
    m_vbo->bind();
    m_program->setAttributeBuffer(0, GL_FLOAT, 0, 2, kStride);
    m_program->setAttributeBuffer(1, GL_FLOAT, sizeof(QVector2D), 2, kStride);
}

void VideoSurface::bindProgram()
{
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    m_program->bind();
    if (m_vao->isCreated()) {
        m_vao->bind();
    }
    else {
        m_program->enableAttributeArray(0);
        m_program->enableAttributeArray(1);
        bindAttributeBuffers();
    }
}

void VideoSurface::releaseProgram()
{
    if (m_vao->isCreated()) {
        m_vao->release();
    }
    else {
        m_program->disableAttributeArray(0);
        m_program->disableAttributeArray(1);
        m_vbo->release();
    }
    m_program->release();
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}
