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

#ifndef TEXTUREDRAWHELPER_H
#define TEXTUREDRAWHELPER_H

#include <QtOpenGL/QGLShaderProgram>
#include <QtOpenGL/QGLWidget>

namespace internal {

class TextureDrawHelper
{
public:
    TextureDrawHelper(const QSize &size)
        : m_size(size)
    {
    }
    ~TextureDrawHelper() {
    }

    void load() {
        m_program.addShaderFromSourceFile(QGLShader::Vertex, ":shaders/texture.vsh");
        m_program.addShaderFromSourceFile(QGLShader::Fragment, ":shaders/texture.fsh");
        m_program.link();
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
        if (!isAvailable())
            return;
        glDisable(GL_DEPTH_TEST);
        m_program.bind();
        QMatrix4x4 modelview, projection;
        projection.ortho(0.0, m_size.width(), 0.0, m_size.height(), -1.0, 1.0);
        modelview.translate(pos);
        m_program.setUniformValue("modelViewProjectionMatrix", projection * modelview);
        QGLFunctions func(QGLContext::currentContext());
        func.glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        m_program.setUniformValue("mainTexture", 0);
        QVector2D position[4], texcoord[4];
        setVertices2D(rect, position);
        m_program.setAttributeArray("inPosition", position);
        setVertices2D(QRectF(0.0, 0.0, 1.0, -1.0), texcoord);
        m_program.setAttributeArray("inTexCoord", texcoord);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        m_program.release();
        glEnable(GL_DEPTH_TEST);
    }
    const QSize &size() {
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

    QGLShaderProgram m_program;
    QSize m_size;
};

}

#endif // TEXTUREDRAWHELPER_H
