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

#ifndef VPVM_INFOPANEL_H_
#define VPVM_INFOPANEL_H_

#include <QtCore/QtCore>
#include <QtOpenGL/QtOpenGL>
#include <vpvl2/qt/TextureDrawHelper.h>
#include "util.h"

namespace vpvm {

using namespace vpvl2::qt;

class InfoPanel
{
public:
    InfoPanel(const QSize &size, IRenderContext *renderContextRef)
        : m_helper(new TextureDrawHelper(size, renderContextRef)),
          m_rect(0, 0, 1024, 256),
          m_texture(m_rect.size(), QImage::Format_ARGB32_Premultiplied),
          m_font("System", 16),
          m_fontMetrics(m_font),
          m_selectedModelName(toQStringFromModel(static_cast<const IModel *>(0))),
          m_selectedBoneName(toQStringFromBone(static_cast<const IBone *>(0))),
          m_selectedMorphName(toQStringFromMorph(static_cast<const IMorph *>(0))),
          m_textureID(0),
          m_fps(0.0f),
          m_visible(true)
    {
    }
    ~InfoPanel() {
        deleteTexture();
    }

    void resize(const QSize &size) {
        m_helper->resize(size);
    }
    void load() {
        m_helper->load(QDir(":shaders"));
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
        painter.drawText(kModelPrefixWidth, height, m_selectedModelName);
        painter.setPen(Qt::blue);
        int boneHeightOffset = height * 2;
        painter.drawText(0, boneHeightOffset, "Bone: ");
        painter.setPen(Qt::red);
        painter.drawText(kModelPrefixWidth, boneHeightOffset, m_selectedBoneName);
        painter.setPen(Qt::blue);
        int morphHeightOffset = height * 3;
        painter.drawText(0, morphHeightOffset, "Morph: ");
        painter.setPen(Qt::red);
        painter.drawText(kModelPrefixWidth, morphHeightOffset, m_selectedMorphName);
        if (m_fps > 0.0f) {
            int fpsHeightOffset = height * 4;
            painter.setPen(Qt::black);
            painter.drawText(0, fpsHeightOffset, QString("FPS: %1").arg(m_fps));
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

    void setModel(const IModel *model) {
        m_selectedModelName = toQStringFromModel(model);
    }
    void setBones(const QList<IBone *> &bones, const QString &alterTextOnMultiple) {
        int nbones = bones.size();
        if (nbones > 1) {
            m_selectedBoneName = alterTextOnMultiple;
        }
        else if (nbones == 1) {
            m_selectedBoneName = toQStringFromBone(bones.first());
        }
        else {
            m_selectedBoneName = toQStringFromBone(static_cast<IBone *>(0));
        }
    }
    void setMorphs(const QList<IMorph *> &morphs, const QString &alterTextOnMultiple) {
        int nmorphs = morphs.size();
        if (nmorphs > 1) {
            m_selectedMorphName = alterTextOnMultiple;
        }
        else if (nmorphs == 1) {
            m_selectedMorphName = toQStringFromMorph(morphs.first());
        }
        else {
            m_selectedMorphName = toQStringFromMorph(static_cast<IMorph *>(0));
        }
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

    QScopedPointer<TextureDrawHelper> m_helper;
    QRect m_rect;
    QImage m_texture;
    QFont m_font;
    QFontMetrics m_fontMetrics;
    QString m_selectedModelName;
    QString m_selectedBoneName;
    QString m_selectedMorphName;
    QString m_morph;
    GLuint m_textureID;
    float m_fps;
    bool m_visible;

    Q_DISABLE_COPY(InfoPanel)
};

} /* namespace vpvm */

#endif // INFOPANEL_H
