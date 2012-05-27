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

#ifndef INFOPANEL_H
#define INFOPANEL_H

#include <QtCore/QtCore>
#include <QtOpenGL/QtOpenGL>
#include "TextureDrawHelper.h"
#include "util.h"

namespace internal {

class InfoPanel
{
public:
    InfoPanel(const QSize &size)
        : m_helper(0),
          m_rect(0, 0, 256, 256),
          m_texture(m_rect.size(), QImage::Format_ARGB32_Premultiplied),
          m_font("System", 16),
          m_fontMetrics(m_font),
          m_textureID(0),
          m_fps(0.0f),
          m_visible(true)
    {
        m_helper = new TextureDrawHelper(size);
    }
    ~InfoPanel() {
        delete m_helper;
        m_helper = 0;
        deleteTexture();
    }

    void resize(const QSize &size) {
        m_helper->resize(size);
    }
    void load() {
        m_helper->load();
    }
    void update() {
        if (!m_helper->isAvailable())
            return;
        int height = m_fontMetrics.height();
        m_texture.fill(0);
        QPainter painter(&m_texture);
        painter.setFont(m_font);
        painter.setRenderHint(QPainter::TextAntialiasing);
        static const QString kModelPrefix("Model: ");
        static const int kModelPrefixWidth = m_fontMetrics.width(kModelPrefix);
        painter.setPen(Qt::blue);
        painter.drawText(0, height, kModelPrefix);
        painter.setPen(Qt::red);
        painter.drawText(kModelPrefixWidth, height, m_model);
        painter.setPen(Qt::blue);
        painter.drawText(0, height * 2, "Bone: ");
        painter.setPen(Qt::red);
        painter.drawText(kModelPrefixWidth, height * 2, m_bone);
        if (m_fps > 0.0f) {
            painter.setPen(Qt::black);
            painter.drawText(0, height * 3, QString("FPS: %1").arg(m_fps));
        }
        painter.end();
        deleteTexture();
        QGLContext *context = const_cast<QGLContext *>(QGLContext::currentContext());
        m_textureID = context->bindTexture(QGLWidget::convertToGLFormat(m_texture.rgbSwapped()));
    }
    void draw() {
        if (!m_visible)
            return;
        glDisable(GL_DEPTH_TEST);
        m_helper->draw(m_rect, QVector3D(0, m_helper->size().height() - m_rect.height(), 0), m_textureID);
        glEnable(GL_DEPTH_TEST);
    }

    void setModel(IModel *model) {
        m_model = internal::toQStringFromModel(model);
    }
    void setBones(const QList<IBone *> &bones, const QString &alterTextOnMultiple) {
        if (bones.count() > 1)
            m_bone = alterTextOnMultiple;
        else if (bones.count() == 1)
            m_bone = internal::toQStringFromBone(bones.first());
        else
            m_bone = internal::toQStringFromBone(static_cast<IBone *>(0));
    }
    void setFPS(float value) {
        m_fps = value;
    }
    void setVisible(bool value) {
        m_visible = value;
    }

private:
    void deleteTexture() {
        if (m_textureID) {
            QGLContext *context = const_cast<QGLContext *>(QGLContext::currentContext());
            context->deleteTexture(m_textureID);
            m_textureID = 0;
        }
    }

    TextureDrawHelper *m_helper;
    QRect m_rect;
    QImage m_texture;
    QFont m_font;
    QFontMetrics m_fontMetrics;
    QString m_model;
    QString m_bone;
    GLuint m_textureID;
    float m_fps;
    bool m_visible;

    Q_DISABLE_COPY(InfoPanel)
};

}

#endif // INFOPANEL_H
