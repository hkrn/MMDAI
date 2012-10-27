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

#ifndef VPVM_GRID_H_
#define VPVM_GRID_H_

#include <QtGlobal>
#include <QtOpenGL/QGLFunctions>
#include <QtOpenGL/QGLShaderProgram>
#include <vpvl2/Common.h>
#include <vpvl2/Scene.h>

#include "SceneLoader.h"
#include "VertexBundle.h"

namespace vpvm {

using namespace vpvl2;

class Grid {
public:
    Grid()
        : m_size(50.0, 50.0, 50.0, 5.0),
          m_lineColor(0.5, 0.5, 0.5),
          m_axisXColor(1.0, 0.0, 0.0),
          m_axisYColor(0.0, 1.0, 0.0),
          m_axisZColor(0.0, 0.0, 1.0),
          m_vbo(QGLBuffer::VertexBuffer),
          m_ibo(QGLBuffer::IndexBuffer),
          m_nindices(0)
    {
    }
    ~Grid() {
    }

    void load() {
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
        m_program.addShaderFromSourceFile(QGLShader::Vertex, ":shaders/grid.vsh");
        m_program.addShaderFromSourceFile(QGLShader::Fragment, ":shaders/grid.fsh");
        m_program.link();
        m_vbo.setUsagePattern(QGLBuffer::StaticDraw);
        m_vbo.create();
        m_vbo.bind();
        m_vbo.allocate(&vertices[0].position, sizeof(Vertex) * vertices.count());
        m_vbo.release();
        m_ibo.setUsagePattern(QGLBuffer::StaticDraw);
        m_ibo.create();
        m_ibo.bind();
        m_ibo.allocate(&indices[0], sizeof(uint8_t) * indices.count());
        m_ibo.release();
        m_bundle.initialize(QGLContext::currentContext());
        m_bundle.create();
        m_bundle.bind();
        bindVertexBundle(false);
        m_program.enableAttributeArray("inPosition");
        m_program.enableAttributeArray("inColor");
        m_bundle.release();
        releaseVertexBundle(false);
        m_nindices = index;
    }

    void draw(const SceneLoader *loader, bool visible) {
        if (visible && m_program.isLinked()) {
            m_program.bind();
            QMatrix4x4 world, view, projection;
            loader->getCameraMatrices(world, view, projection);
            m_program.setUniformValue("modelViewProjectionMatrix", projection * view * world);
            bindVertexBundle(true);
            glDrawElements(GL_LINES, m_nindices, GL_UNSIGNED_BYTE, 0);
            releaseVertexBundle(true);
            m_program.release();
        }
    }

    void setGridSize(const Vector4 &value) { m_size = value; }
    void setLineColor(const Vector3 &value) { m_lineColor = value; }
    void setAxisXColor(const Vector3 &value) { m_axisXColor = value; }
    void setAxisYColor(const Vector3 &value) { m_axisYColor = value; }
    void setAxisZColor(const Vector3 &value) { m_axisZColor = value; }

private:
    struct Vertex {
        Vertex() {}
        Vertex(const Vector3 &p, const Vector3 &c)
            : position(p),
              color(c)
        {
        }
        Vector3 position;
        Vector3 color;
    };
    void addLine(const Vector3 &from,
                 const Vector3 &to,
                 const Vector3 &color,
                 Array<Vertex> &vertices,
                 Array<uint8_t> &indices,
                 uint8_t &index)
    {
        vertices.add(Vertex(from, color));
        vertices.add(Vertex(to, color));
        indices.add(index++);
        indices.add(index++);
    }
    void bindVertexBundle(bool bundle) {
        if (!bundle || !m_bundle.bind()) {
            m_vbo.bind();
            m_program.setAttributeBuffer("inPosition", GL_FLOAT, 0, 3, sizeof(Vertex));
            static const Vertex v;
            const size_t offset = reinterpret_cast<const uint8_t *>(&v.color)
                    - reinterpret_cast<const uint8_t *>(&v.position);
            m_program.setAttributeBuffer("inColor", GL_FLOAT, offset, 3, sizeof(Vertex));
            m_ibo.bind();
        }
    }
    void releaseVertexBundle(bool bundle) {
        if (!bundle || !m_bundle.release()) {
            m_vbo.release();
            m_ibo.release();
        }
    }

    QGLShaderProgram m_program;
    VertexBundle m_bundle;
    Vector4 m_size;
    Vector3 m_lineColor;
    Vector3 m_axisXColor;
    Vector3 m_axisYColor;
    Vector3 m_axisZColor;
    QGLBuffer m_vbo;
    QGLBuffer m_ibo;
    int m_nindices;

    Q_DISABLE_COPY(Grid)
};

} /* namespace vpvm */

#endif // GRID_H
