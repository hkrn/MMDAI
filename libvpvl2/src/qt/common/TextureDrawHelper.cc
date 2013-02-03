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

#include "vpvl2/qt/TextureDrawHelper.h"

#include "vpvl2/vpvl2.h"
#include "vpvl2/extensions/gl/ShaderProgram.h"
#include "vpvl2/extensions/icu4c/String.h"
#include "vpvl2/extensions/gl/VertexBundle.h"
#include "vpvl2/extensions/gl/VertexBundleLayout.h"

#include <QVector2D>
#include <QMatrix4x4>

namespace vpvl2
{
namespace qt
{
using namespace extensions::icu4c;

class TextureDrawHelper::PrivateShaderProgram : public ShaderProgram {
public:
    enum VertexType {
        kPosition,
        kTexCoord
    };

    PrivateShaderProgram()
        : ShaderProgram(),
          m_modelViewProjectionMatrix(-1),
          m_mainTexture(-1)
    {
    }
    ~PrivateShaderProgram() {
        m_modelViewProjectionMatrix = -1;
        m_mainTexture = -1;
    }

    void addShaderFromFile(const QString &path, GLuint type) {
        QFile file(path);
        if (file.open(QFile::ReadOnly | QFile::Unbuffered)) {
            size_t size = file.size();
            uchar *address = file.map(0, size);
            String s(UnicodeString(reinterpret_cast<const char *>(address), size));
            addShaderSource(&s, type);
            file.unmap(address);
        }
    }
    bool link() {
        glBindAttribLocation(m_program, PrivateShaderProgram::kPosition, "inPosition");
        glBindAttribLocation(m_program, PrivateShaderProgram::kTexCoord, "inTexCoord");
        bool ok = ShaderProgram::link();
        if (ok) {
            m_modelViewProjectionMatrix = glGetUniformLocation(m_program, "modelViewProjectionMatrix");
            m_mainTexture = glGetUniformLocation(m_program, "mainTexture");
        }
        return ok;
    }
    void enableAttributes() {
        glEnableVertexAttribArray(PrivateShaderProgram::kPosition);
        glEnableVertexAttribArray(PrivateShaderProgram::kTexCoord);
    }
    void setUniformValues(const QMatrix4x4 &matrix, GLuint textureID) {
        GLfloat m[16] = { 0 };
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        const float *source = matrix.constData();
#else
        const qreal *source = matrix.constData();
#endif
        for (int i = 0; i < 16; i++) {
            m[i] = source[i];
        }
        glUniformMatrix4fv(m_modelViewProjectionMatrix, 1, GL_FALSE, m);
        glUniform1f(m_mainTexture, textureID);
    }

private:
    GLint m_modelViewProjectionMatrix;
    GLint m_mainTexture;
};

TextureDrawHelper::TextureDrawHelper(const QSize &size)
    : m_program(new PrivateShaderProgram()),
      m_bundle(new VertexBundle()),
      m_layout(new VertexBundleLayout()),
      m_size(size),
      m_linked(false)
{
}

TextureDrawHelper::~TextureDrawHelper()
{
    m_linked = false;
}

void TextureDrawHelper::load(const QDir &dir, const QRectF &baseTexCoord)
{
    m_program->create();
    m_program->addShaderFromFile(dir.absoluteFilePath("texture.vsh"), GL_VERTEX_SHADER);
    m_program->addShaderFromFile(dir.absoluteFilePath("texture.fsh"), GL_FRAGMENT_SHADER);
    m_linked = m_program->link();
    if (m_linked) {
        QVector2D positions[4], texcoord[4];
        setVertices2D(QRectF(0.0, 0.0, 1.0, -1.0), positions);
        m_bundle->create(VertexBundle::kVertexBuffer, PrivateShaderProgram::kPosition,
                         GL_DYNAMIC_DRAW, &positions[0], sizeof(positions));
        setVertices2D(baseTexCoord, texcoord);
        m_bundle->create(VertexBundle::kVertexBuffer, PrivateShaderProgram::kTexCoord,
                         GL_STATIC_DRAW, &texcoord[0], sizeof(texcoord));
        m_layout->create();
        m_layout->bind();
        bindVertexBundleLayout(false);
        m_program->enableAttributes();
        unbindVertexBundleLayout(false);
        m_layout->unbind();
        m_bundle->unbind(VertexBundle::kVertexBuffer);
    }
    else {
        qWarning("Cannot link program: %s", m_program->message());
    }
}

void TextureDrawHelper::resize(const QSize &size)
{
    m_size = size;
}

void TextureDrawHelper::draw(const QRectF &rect, intptr_t textureID)
{
    draw(rect, QVector3D(), textureID);
}

void TextureDrawHelper::draw(const QRect &rect, const QVector3D &pos, intptr_t textureID)
{
    draw(QRectF(rect), pos, textureID);
}

void TextureDrawHelper::draw(const QRectF &rect, const QVector3D &pos, intptr_t textureID)
{
    if (!isAvailable() || m_size.isEmpty() || !textureID)
        return;
    glDisable(GL_DEPTH_TEST);
    updateVertexBuffer(rect);
    QMatrix4x4 modelview, projection;
    modelview.translate(pos);
    projection.ortho(0.0, m_size.width(), 0.0, m_size.height(), -1.0, 1.0);
    m_program->bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    m_program->setUniformValues(projection * modelview, textureID);
    bindVertexBundleLayout(true);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    unbindVertexBundleLayout(true);
    m_program->unbind();
    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_DEPTH_TEST);
}

QSize TextureDrawHelper::size() const
{
    return m_size;
}

bool TextureDrawHelper::isAvailable() const
{
    return m_linked;
}

void TextureDrawHelper::setVertices2D(const QRectF &rect, QVector2D *vertices2D) const
{
    vertices2D[0].setX(rect.left());
    vertices2D[0].setY(rect.top());
    vertices2D[1].setX(rect.right());
    vertices2D[1].setY(rect.top());
    vertices2D[2].setX(rect.right());
    vertices2D[2].setY(rect.bottom());
    vertices2D[3].setX(rect.left());
    vertices2D[3].setY(rect.bottom());
}

void TextureDrawHelper::updateVertexBuffer(const QRectF &rect)
{
    QVector2D positions[4];
    setVertices2D(rect, positions);
    m_bundle->bind(VertexBundle::kVertexBuffer, PrivateShaderProgram::kPosition);
    m_bundle->write(VertexBundle::kVertexBuffer, 0, sizeof(positions), positions);
    m_bundle->unbind(VertexBundle::kVertexBuffer);
}

void TextureDrawHelper::bindVertexBundleLayout(bool bundle)
{
    if (!bundle || !m_layout->bind()) {
        m_bundle->bind(VertexBundle::kVertexBuffer, PrivateShaderProgram::kPosition);
        glVertexAttribPointer(PrivateShaderProgram::kPosition, 2, GL_FLOAT, GL_FALSE, 0, 0);
        m_bundle->bind(VertexBundle::kVertexBuffer, PrivateShaderProgram::kTexCoord);
        glVertexAttribPointer(PrivateShaderProgram::kTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
    }
}

void TextureDrawHelper::unbindVertexBundleLayout(bool bundle)
{
    if (!bundle || !m_layout->unbind()) {
        m_bundle->unbind(VertexBundle::kVertexBuffer);
    }
}

} /* namespace qt */
} /* namespace vpvl2 */
