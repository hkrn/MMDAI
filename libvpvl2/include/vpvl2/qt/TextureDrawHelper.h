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

#ifndef VPVL2_QT_TEXTUREDRAWHELPER_H_
#define VPVL2_QT_TEXTUREDRAWHELPER_H_

#include <QtOpenGL/QGLBuffer>
#include <QtOpenGL/QGLFunctions>
#include <QtOpenGL/QGLShaderProgram>
#include <vpvl2/IModel.h>

#include "vpvl2/qt/VertexBundle.h"

namespace vpvl2
{
namespace qt
{

class TextureDrawHelper : protected QGLFunctions
{
public:
    TextureDrawHelper(const QSize &size)
        : QGLFunctions(),
          m_dvbo(QGLBuffer::VertexBuffer),
          m_svbo(QGLBuffer::VertexBuffer),
          m_size(size)
    {
    }
    ~TextureDrawHelper() {
    }

    void load(const QDir &dir, const QRectF &baseTexCoord = QRectF(0.0, 0.0, 1.0, -1.0)) {
        initializeGLFunctions();
        m_bundle.initialize(QGLContext::currentContext());
        m_program.addShaderFromSourceFile(QGLShader::Vertex, dir.absoluteFilePath("texture.vsh"));
        m_program.addShaderFromSourceFile(QGLShader::Fragment, dir.absoluteFilePath("texture.fsh"));
        m_program.bindAttributeLocation("inPosition", IModel::IBuffer::kVertexStride);
        m_program.bindAttributeLocation("inTexCoord", IModel::IBuffer::kTextureCoordStride);
        m_program.link();
        m_dvbo.setUsagePattern(QGLBuffer::DynamicDraw);
        m_dvbo.create();
        m_dvbo.bind();
        QVector2D positions[4];
        setVertices2D(QRectF(0.0, 0.0, 1.0, -1.0), positions);
        m_dvbo.allocate(&positions[0], sizeof(positions));
        m_dvbo.release();
        m_svbo.setUsagePattern(QGLBuffer::StaticDraw);
        m_svbo.create();
        m_svbo.bind();
        QVector2D texcoord[4];
        setVertices2D(baseTexCoord, texcoord);
        m_svbo.allocate(&texcoord[0], sizeof(texcoord));
        m_svbo.release();
        m_bundle.create();
        m_bundle.bind();
        bindVertexBundle(false);
        m_program.enableAttributeArray(IModel::IBuffer::kVertexStride);
        m_program.enableAttributeArray(IModel::IBuffer::kTextureCoordStride);
        m_bundle.release();
        releaseVertexBundle(false);
    }
    void resize(const QSize &size) {
        m_size = size;
    }
    void draw(const QRectF &rect, GLuint textureID) {
        draw(rect, QVector3D(), textureID);
    }
    void draw(const QRect &rect, const QVector3D &pos, GLuint textureID) {
        draw(QRectF(rect), pos, textureID);
    }
    void draw(const QRectF &rect, const QVector3D &pos, GLuint textureID) {
        if (!isAvailable() || m_size.isEmpty() || textureID == 0)
            return;
        glDisable(GL_DEPTH_TEST);
        QMatrix4x4 modelview, projection;
        projection.ortho(0.0, m_size.width(), 0.0, m_size.height(), -1.0, 1.0);
        modelview.translate(pos);
        updateVertexBuffer(rect);
        m_program.bind();
        m_program.setUniformValue("modelViewProjectionMatrix", projection * modelview);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        m_program.setUniformValue("mainTexture", 0);
        bindVertexBundle(true);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        releaseVertexBundle(true);
        m_program.release();
        glEnable(GL_DEPTH_TEST);
    }
    QSize size() {
        return m_size;
    }
    bool isAvailable() const {
        return m_program.isLinked();
    }

private:
    void setVertices2D(const QRectF &rect, QVector2D *vertices2D) const {
        vertices2D[0].setX(rect.left());
        vertices2D[0].setY(rect.top());
        vertices2D[1].setX(rect.right());
        vertices2D[1].setY(rect.top());
        vertices2D[2].setX(rect.right());
        vertices2D[2].setY(rect.bottom());
        vertices2D[3].setX(rect.left());
        vertices2D[3].setY(rect.bottom());
    }
    void updateVertexBuffer(const QRectF &rect) {
        QVector2D positions[4];
        setVertices2D(rect, positions);
        m_dvbo.bind();
        m_dvbo.write(0, &positions[0], sizeof(positions));
        m_dvbo.release();
    }
    void bindVertexBundle(bool bundle) {
        if (!bundle || !m_bundle.bind()) {
            m_dvbo.bind();
            m_program.setAttributeBuffer(IModel::IBuffer::kVertexStride, GL_FLOAT, 0, 2);
            m_svbo.bind();
            m_program.setAttributeBuffer(IModel::IBuffer::kTextureCoordStride, GL_FLOAT, 0, 2);
        }
    }
    void releaseVertexBundle(bool bundle) {
        if (!bundle || !m_bundle.release()) {
            m_dvbo.release();
            m_svbo.release();
        }
    }

    VertexBundle m_bundle;
    QGLShaderProgram m_program;
    QGLBuffer m_dvbo;
    QGLBuffer m_svbo;
    QSize m_size;
};

} /* namespace qt */
} /* namespace vpvl2 */

#endif // VPVL2_QT_TEXTUREDRAWHELPER_H
