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

#include "Grid.h"

#include <vpvl2/vpvl2.h>
#include <vpvl2/extensions/gl/ShaderProgram.h>
#include <vpvl2/extensions/gl/VertexBundle.h>
#include <vpvl2/extensions/gl/VertexBundleLayout.h>
#include <glm/gtc/type_ptr.hpp>

namespace vpvm {

using namespace vpvl2;
using namespace vpvl2::extensions::icu4c;
using namespace vpvl2::extensions::gl;
using namespace vpvl2::qt;

class Grid::PrivateShaderProgram : public ShaderProgram {
public:
    enum VertexType {
        kPosition,
        kColor
    };

    PrivateShaderProgram()
        : ShaderProgram(),
          m_modelViewProjectionMatrix(-1)
    {
    }
    ~PrivateShaderProgram() {
        m_modelViewProjectionMatrix = -1;
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
        glBindAttribLocation(m_program, kPosition, "inPosition");
        glBindAttribLocation(m_program, kColor, "inColor");
        bool ok = ShaderProgram::link();
        if (ok) {
            m_modelViewProjectionMatrix = glGetUniformLocation(m_program, "modelViewProjectionMatrix");
        }
        return ok;
    }
    void enableAttributes() {
        glEnableVertexAttribArray(kPosition);
        glEnableVertexAttribArray(kColor);
    }
    void setUniformValues(const QMatrix4x4 &matrix) {
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
    }

private:
    GLint m_modelViewProjectionMatrix;
};

Grid::Grid()
    : m_program(new PrivateShaderProgram()),
      m_bundle(new VertexBundle()),
      m_layout(new VertexBundleLayout()),
      m_size(50.0, 50.0, 50.0, 5.0),
      m_lineColor(0.5, 0.5, 0.5),
      m_axisXColor(1.0, 0.0, 0.0),
      m_axisYColor(0.0, 1.0, 0.0),
      m_axisZColor(0.0, 0.0, 1.0),
      m_nindices(0)
{
}

Grid::~Grid()
{
}

void Grid::load()
{
    m_program->create();
    m_program->addShaderFromFile(":shaders/gui/grid.vsh", GL_VERTEX_SHADER);
    m_program->addShaderFromFile(":shaders/gui/grid.fsh", GL_FRAGMENT_SHADER);
    if (m_program->link()) {
        // draw black grid
        Array<Vertex> vertices;
        Array<uint8_t> indices;
        Scalar width = m_size.x(), height = m_size.y(), depth = m_size.z(), gridSize = m_size.w();
        uint8_t index = 0;
        for (Scalar x = -width; x <= width; x += gridSize) {
            Vector3 from(x, 0.0, -width), to(x, 0.0, x == 0 ? 0.0 : width);
            addLine(from, to, m_lineColor, vertices, indices, index);
        }
        for (Scalar z = -height; z <= height; z += gridSize) {
            Vector3 from(-height, 0.0f, z), to(z == 0 ? 0.0f : height, 0.0f, z);
            addLine(from, to, m_lineColor, vertices, indices, index);
        }
        // X coordinate (red)
        addLine(kZeroV3, Vector3(width, 0.0f, 0.0f), m_axisXColor, vertices, indices, index);
        // Y coordinate (green)
        addLine(kZeroV3, Vector3(0.0f, height, 0.0f), m_axisYColor, vertices, indices, index);
        // Z coordinate (blue)
        addLine(kZeroV3, Vector3(0.0f, 0.0f, depth), m_axisZColor, vertices, indices, index);
        m_bundle->create(VertexBundle::kVertexBuffer, 0, GL_STATIC_DRAW,
                         &vertices[0].position, sizeof(Vertex) * vertices.count());
        m_bundle->create(VertexBundle::kIndexBuffer, 0, GL_STATIC_DRAW,
                         &indices[0], sizeof(uint8_t) * indices.count());
        m_layout->create();
        m_layout->bind();
        bindVertexBundle(false);
        m_program->enableAttributes();
        m_layout->unbind();
        releaseVertexBundle(false);
        m_nindices = index;
    }
    else {
        qWarning("Cannot link grid program: %s", m_program->message());
    }
}

void Grid::draw(const SceneLoader *loader, bool visible)
{
    if (visible && m_program->isLinked()) {
        m_program->bind();
        glm::mat4 world, view, projection;
        loader->getCameraMatrices(world, view, projection);
        const float *v = glm::value_ptr(projection * view * world);
        QMatrix4x4 matrix;
        for (int i = 0; i < 16; i++)
            matrix.data()[i] = v[i];
        m_program->setUniformValues(matrix);
        bindVertexBundle(true);
        glDrawElements(GL_LINES, m_nindices, GL_UNSIGNED_BYTE, 0);
        releaseVertexBundle(true);
        m_program->unbind();
    }
}

void Grid::addLine(const Vector3 &from,
                   const Vector3 &to,
                   const Vector3 &color,
                   Array<Vertex> &vertices,
                   Array<uint8_t> &indices,
                   uint8_t &index)
{
    vertices.append(Vertex(from, color));
    vertices.append(Vertex(to, color));
    indices.append(index++);
    indices.append(index++);
}

void Grid::bindVertexBundle(bool bundle)
{
    if (!bundle || !m_layout->bind()) {
        m_bundle->bind(VertexBundle::kVertexBuffer, 0);
        glVertexAttribPointer(PrivateShaderProgram::kPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
        static const Vertex v;
        const uint8_t *offset = reinterpret_cast<const uint8_t *>(
                    reinterpret_cast<const uint8_t *>(&v.color)
                    - reinterpret_cast<const uint8_t *>(&v.position));
        glVertexAttribPointer(PrivateShaderProgram::kColor, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offset);
        m_bundle->bind(VertexBundle::kIndexBuffer, 0);
    }
}

void Grid::releaseVertexBundle(bool bundle)
{
    if (!bundle || !m_layout->unbind()) {
        m_bundle->unbind(VertexBundle::kVertexBuffer);
        m_bundle->unbind(VertexBundle::kIndexBuffer);
    }
}

} /* namespace vpvm */
