/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
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

#include "BackgroundImage.h"

#include <vpvl2/extensions/gl/CommonMacros.h>
#include <vpvl2/qt/TextureDrawHelper.h>

#include <QImage>

namespace vpvm {

using namespace vpvl2;
using namespace vpvl2::qt;

BackgroundImage::BackgroundImage(const QSize &size)
    : m_backgroundDrawer(new TextureDrawHelper(size)),
      m_backgroundTexture(0),
      m_uniformImage(false)
{
    m_backgroundDrawer->load();
}

BackgroundImage::~BackgroundImage()
{
    release();
}

void BackgroundImage::resize(const QSize &size)
{
    if (m_uniformImage)
        m_backgroundDrawer->resize(m_backgroundImageSize);
    else
        m_backgroundDrawer->resize(size);
}

void BackgroundImage::setImage(const QString &filename)
{
    release();
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
        generateTextureFromImage(m_movie.currentImage());
    }
    else {
        QImage image(filename);
        if (image.isNull()) {
            m_backgroundImageFilename = "";
            m_backgroundTexture = 0;
        }
        else {
            m_backgroundImageFilename = filename;
            generateTextureFromImage(image);
        }
    }
    if (m_uniformImage)
        resize(QSize());
}

void BackgroundImage::setTimeIndex(int value)
{
    if (m_movie.isValid()) {
        int timeIndex = qBound(0, value, m_movie.frameCount() - 1);
        if (m_movie.jumpToFrame(timeIndex)) {
            release();
            generateTextureFromImage(m_movie.currentImage());
        }
    }
}

void BackgroundImage::draw()
{
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

void BackgroundImage::release()
{
    glDeleteTextures(1, reinterpret_cast<GLuint *>(&m_backgroundTexture));
    m_backgroundTexture = 0;
}

void BackgroundImage::generateTextureFromImage(const QImage &image)
{
    glGenTextures(1, reinterpret_cast<GLuint *>(&m_backgroundTexture));
    glBindTexture(GL_TEXTURE_2D, m_backgroundTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image.constBits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    m_backgroundImageSize = image.size();
}

} /* namespace vpvm */
