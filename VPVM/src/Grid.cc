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

#include "Common.h"
#include "Grid.h"
#include "ProjectProxy.h"

#include <QOpenGLFunctions>
#include <QMatrix4x4>

#include <vpvl2/vpvl2.h>
#include <vpvl2/extensions/XMLProject.h>
#include <vpvl2/extensions/qt/String.h>
#include <vpvl2/gl/ShaderProgram.h>
#include <vpvl2/gl/VertexBundle.h>
#include <vpvl2/gl/VertexBundleLayout.h>
#include <glm/gtc/type_ptr.hpp>

using namespace vpvl2;
using namespace vpvl2::gl;
using namespace vpvl2::extensions::qt;

class Grid::PrivateShaderProgram : public ShaderProgram {
public:
    enum VertexType {
        kPosition,
        kColor
    };

    PrivateShaderProgram(IApplicationContext::FunctionResolver *resolver)
        : ShaderProgram(resolver),
          m_modelViewProjectionMatrix(-1)
    {
    }
    ~PrivateShaderProgram() {
        m_modelViewProjectionMatrix = -1;
    }

    void addShaderFromFile(const QString &path, GLuint type) {
        QFile file(path);
        if (file.open(QFile::ReadOnly | QFile::Unbuffered)) {
            vsize size = file.size();
            uchar *address = file.map(0, size);
            String s(QString::fromUtf8(reinterpret_cast<const char *>(address), size));
            addShaderSource(&s, type);
            file.unmap(address);
        }
    }
    bool link() {
        bindAttribLocation(m_program, kPosition, "inPosition");
        bindAttribLocation(m_program, kColor, "inColor");
        bool ok = ShaderProgram::link();
        if (ok) {
            m_modelViewProjectionMatrix = getUniformLocation(m_program, "modelViewProjectionMatrix");
        }
        return ok;
    }
    void enableAttributes() {
        QOpenGLFunctions functions(QOpenGLContext::currentContext());
        functions.glEnableVertexAttribArray(kPosition);
        functions.glEnableVertexAttribArray(kColor);
    }
    void disableAttributes() {
        QOpenGLFunctions functions(QOpenGLContext::currentContext());
        functions.glDisableVertexAttribArray(kPosition);
        functions.glDisableVertexAttribArray(kColor);
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
        uniformMatrix4fv(m_modelViewProjectionMatrix, 1, GL_FALSE, m);
    }

private:
    GLint m_modelViewProjectionMatrix;
};

Grid::Grid(QObject *parent)
    : QObject(parent),
      m_parentProjectProxyRef(0),
      m_size(50.0, 50.0, 50.0, 5.0),
      m_lineColor(127, 127, 127),
      m_axisXColor(255, 0, 0),
      m_axisYColor(0, 255, 0),
      m_axisZColor(0, 0, 255),
      m_nindices(0),
      m_visible(true)
{
}

Grid::~Grid()
{
    m_parentProjectProxyRef = 0;
}

void Grid::load(vpvl2::IApplicationContext::FunctionResolver *resolver)
{
    m_program.reset(new PrivateShaderProgram(resolver)),
    m_bundle.reset(new VertexBundle(resolver)),
    m_layout.reset(new VertexBundleLayout(resolver)),
    m_program->create();
    m_program->addShaderFromFile(":shaders/gui/grid.vsh", ShaderProgram::kGL_VERTEX_SHADER);
    m_program->addShaderFromFile(":shaders/gui/grid.fsh", ShaderProgram::kGL_FRAGMENT_SHADER);
    if (m_program->link()) {
        // draw black grid
        Array<Vertex> vertices;
        Array<uint8> indices;
        Scalar width = m_size.x(), height = m_size.y(), depth = m_size.z(), gridSize = m_size.w();
        uint8 index = 0;
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
        m_bundle->create(VertexBundle::kVertexBuffer, 0, VertexBundle::kGL_STATIC_DRAW,
                         &vertices[0].position, sizeof(Vertex) * vertices.count());
        m_bundle->create(VertexBundle::kIndexBuffer, 0, VertexBundle::kGL_STATIC_DRAW,
                         &indices[0], sizeof(uint8) * indices.count());
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

void Grid::draw(const glm::mat4 &mvp)
{
    if (m_visible && m_program->isLinked()) {
        m_program->bind();
        const float *v = glm::value_ptr(mvp);
        QMatrix4x4 matrix;
        for (int i = 0; i < 16; i++) {
            matrix.data()[i] = v[i];
        }
        m_program->setUniformValues(matrix);
        bindVertexBundle(true);
        glDrawElements(GL_LINES, m_nindices, GL_UNSIGNED_BYTE, 0);
        releaseVertexBundle(true);
        m_program->unbind();
    }
}

void Grid::setProjectProxy(ProjectProxy *value)
{
    m_parentProjectProxyRef = value;
    setVisible(value->globalSetting("grid.visible", true).toBool());
}

QVector4D Grid::size() const
{
    return m_size;
}

void Grid::setSize(const QVector4D &value)
{
    if (value != m_size) {
        m_size = value;
        emit sizeChanged();
    }
}

QColor Grid::lineColor() const
{
    return m_lineColor;
}

void Grid::setLineColor(const QColor &value)
{
    if (value != m_lineColor) {
        m_lineColor = value;
        emit lineColorChanged();
    }
}

QColor Grid::axisXColor() const
{
    return m_axisXColor;
}

void Grid::setAxisXColor(const QColor &value)
{
    if (value != m_axisXColor) {
        m_axisXColor = value;
        emit axisXColorChanged();
    }
}

QColor Grid::axisYColor() const
{
    return m_axisYColor;
}

void Grid::setAxisYColor(const QColor &value)
{
    if (value != m_axisYColor) {
        m_axisYColor = value;
        emit axisYColorChanged();
    }
}

QColor Grid::axisZColor() const
{
    return m_axisZColor;
}

void Grid::setAxisZColor(const QColor &value)
{
    if (value != m_axisZColor) {
        m_axisZColor = value;
        emit axisZColorChanged();
    }
}

bool Grid::isVisible() const
{
    return m_visible;
}

void Grid::setVisible(bool value)
{
    Q_ASSERT(m_parentProjectProxyRef);
    if (value != m_visible) {
        m_parentProjectProxyRef->projectInstanceRef()->setGlobalSetting("grid.visible", value ? "true" : "false");
        m_visible = value;
        emit visibleChanged();
    }
}

void Grid::addLine(const Vector3 &from,
                   const Vector3 &to,
                   const QColor &color,
                   Array<Vertex> &vertices,
                   Array<uint8> &indices,
                   uint8 &index)
{
    vertices.append(Vertex(from, color));
    vertices.append(Vertex(to, color));
    indices.append(index++);
    indices.append(index++);
}

void Grid::bindVertexBundle(bool bundle)
{
    if (!bundle || !m_layout->bind()) {
        QOpenGLFunctions functions(QOpenGLContext::currentContext());
        m_bundle->bind(VertexBundle::kVertexBuffer, 0);
        static const Vertex v;
        functions.glVertexAttribPointer(PrivateShaderProgram::kPosition, 3, GL_FLOAT, GL_FALSE, sizeof(v), 0);
        const uint8 *offset = reinterpret_cast<const uint8 *>(
                    reinterpret_cast<const uint8 *>(&v.color)
                    - reinterpret_cast<const uint8 *>(&v.position));
        functions.glVertexAttribPointer(PrivateShaderProgram::kColor, 3, GL_FLOAT, GL_FALSE, sizeof(v), offset);
        m_program->enableAttributes();
        m_bundle->bind(VertexBundle::kIndexBuffer, 0);
    }
}

void Grid::releaseVertexBundle(bool bundle)
{
    if (!bundle || !m_layout->unbind()) {
        m_program->disableAttributes();
        m_bundle->unbind(VertexBundle::kVertexBuffer);
        m_bundle->unbind(VertexBundle::kIndexBuffer);
    }
}
