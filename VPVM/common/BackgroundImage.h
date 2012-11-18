/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#ifndef VPVM_BACKGROUNDIMAGE_H_
#define VPVM_BACKGROUNDIMAGE_H_

#include <QtGui/QImage>
#include <QtOpenGL/QtOpenGL>
#include <vpvl2/qt/TextureDrawHelper.h>

namespace vpvm {

using namespace vpvl2::qt;

class BackgroundImage {
public:
    BackgroundImage(const QSize &size)
        : m_backgroundDrawer(new TextureDrawHelper(size)),
          m_backgroundTexture(0),
          m_uniformImage(false)
    {
        m_backgroundDrawer->load();
    }
    ~BackgroundImage() {
        glDeleteTextures(1, &m_backgroundTexture);
        m_backgroundTexture = 0;
    }

    void resize(const QSize &size) {
        if (m_uniformImage)
            m_backgroundDrawer->resize(m_backgroundImageSize);
        else
            m_backgroundDrawer->resize(size);
    }
    void setImage(const QString &filename) {
        QGLContext *context = const_cast<QGLContext *>(QGLContext::currentContext());
        context->deleteTexture(m_backgroundTexture);
        QFileInfo info(filename);
        const QString &suffix = info.suffix().toLower();
        QStringList movies;
        movies << "mng";
#ifdef Q_OS_MACX
        movies << "avi" << "mp4" << "m4v" << "mov";
#endif
        if (movies.contains(suffix)) {
            m_movie.setFileName(filename);
            m_movie.jumpToFrame(0);
            m_backgroundImageFilename = filename;
            generateTextureFromImage(m_movie.currentImage(), context);
        }
        else {
            QImage image(filename);
            if (image.isNull()) {
                m_backgroundImageFilename = "";
                m_backgroundTexture = 0;
            }
            else {
                m_backgroundImageFilename = filename;
                generateTextureFromImage(image, context);
            }
        }
        if (m_uniformImage)
            resize(QSize());
    }
    void setFrameIndex(int value) {
        if (m_movie.isValid()) {
            int frameIndex = qBound(0, value, m_movie.frameCount() - 1);
            if (m_movie.jumpToFrame(frameIndex)) {
                QGLContext *context = const_cast<QGLContext *>(QGLContext::currentContext());
                context->deleteTexture(m_backgroundTexture);
                generateTextureFromImage(m_movie.currentImage(), context);
            }
        }
    }
    void draw() {
        QRectF rect;
        if (!m_uniformImage) {
            const QSize &sceneSize = m_backgroundDrawer->size();
            const qreal &centerX = sceneSize.width() * 0.5 - m_backgroundImageSize.width() * 0.5;
            const qreal &centerY = sceneSize.height() * 0.5 - m_backgroundImageSize.height() * 0.5;
            rect.setTopLeft(m_backgroundImagePosition + QPointF(centerX, centerY));
        }
        rect.setSize(m_backgroundImageSize);
        m_backgroundDrawer->draw(rect, m_backgroundTexture);
    }

    const QSize &imageSize() const { return m_backgroundImageSize; }
    const QString &imageFilename() const { return m_backgroundImageFilename; }
    const QPoint &imagePosition() const { return m_backgroundImagePosition; }
    void setImagePosition(const QPoint &value) { m_backgroundImagePosition = value; }
    bool isUniformEnabled() const { return m_uniformImage; }
    void setUniformEnable(bool value) { m_uniformImage = value; }

private:
    void generateTextureFromImage(const QImage &image, QGLContext *context) {
        QGLContext::BindOptions options = QGLContext::LinearFilteringBindOption;
        m_backgroundTexture = context->bindTexture(image, GL_TEXTURE_2D, GL_RGBA, options);
        m_backgroundImageSize = image.size();
    }

    QScopedPointer<TextureDrawHelper> m_backgroundDrawer;
    QMovie m_movie;
    QSize m_backgroundImageSize;
    QPoint m_backgroundImagePosition;
    QString m_backgroundImageFilename;
    GLuint m_backgroundTexture;
    bool m_uniformImage;
};

} /* namespace vpvm */

#endif // BACKGROUNDIMAGE_H
