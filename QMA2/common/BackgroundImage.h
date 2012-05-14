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

#ifndef BACKGROUNDIMAGE_H
#define BACKGROUNDIMAGE_H

#include <QtGui/QImage>
#include <QtOpenGL/QtOpenGL>
#include "TextureDrawHelper.h"

namespace internal {

class BackgroundImage {
public:
    BackgroundImage(const QSize &size)
        : m_backgroundDrawer(0),
          m_backgroundTexture(0),
          m_scaleImage(false)
    {
        m_backgroundDrawer = new TextureDrawHelper(size);
        m_backgroundDrawer->load();
    }
    ~BackgroundImage() {
        glDeleteTextures(1, &m_backgroundTexture);
        m_backgroundTexture = 0;
        delete m_backgroundDrawer;
        m_backgroundDrawer = 0;
    }

    void resize(const QSize &size) {
        if (m_scaleImage)
            m_backgroundDrawer->resize(m_backgroundImageSize);
        else
            m_backgroundDrawer->resize(size);
    }
    void setImage(const QImage &image, const QString &filename, QGLWidget *widget) {
        widget->deleteTexture(m_backgroundTexture);
        if (image.isNull()) {
            m_backgroundTexture = 0;
            m_backgroundImageFilename = "";
        }
        else {
            QGLContext::BindOptions options = QGLContext::LinearFilteringBindOption;
            m_backgroundTexture = widget->bindTexture(image, GL_TEXTURE_2D, GL_RGBA, options);
            m_backgroundImageSize = image.size();
            m_backgroundImageFilename = filename;
        }
        if (m_scaleImage)
            resize(QSize());
    }
    void draw() {
        QRectF rect;
        if (!m_scaleImage) {
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
    bool isScaleEnabled() const { return m_scaleImage; }
    void setScaleEnable(bool value) { m_scaleImage = value; }

private:
    TextureDrawHelper *m_backgroundDrawer;
    QSize m_backgroundImageSize;
    QPoint m_backgroundImagePosition;
    QString m_backgroundImageFilename;
    GLuint m_backgroundTexture;
    bool m_scaleImage;
};

}

#endif // BACKGROUNDIMAGE_H
