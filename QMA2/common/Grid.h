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

#ifndef GRID_H
#define GRID_H

#include <QtGlobal>
#include <QtOpenGL/QGLFunctions>
#include <QtOpenGL/QGLShaderProgram>
#include <vpvl2/Common.h>

namespace internal {

using namespace vpvl2;

class Grid {
public:
    struct Vertex {
        Vector3 position;
        Vector3 color;
    };

    Grid()
        : m_size(50.0, 50.0, 50.0, 5.0),
          m_lineColor(0.5, 0.5, 0.5),
          m_axisXColor(1.0, 0.0, 0.0),
          m_axisYColor(0.0, 1.0, 0.0),
          m_axisZColor(0.0, 0.0, 1.0),
          m_vbo(0),
          m_ibo(0),
          m_list(0)
    {
    }
    ~Grid() {
        QGLFunctions func(QGLContext::currentContext());
        if (m_vbo) {
            func.glDeleteBuffers(1, &m_vbo);
            m_vbo = 0;
        }
        if (m_ibo) {
            func.glDeleteBuffers(1, &m_ibo);
            m_ibo = 0;
        }
        if (m_list) {
            glDeleteLists(m_list, 1);
            m_list = 0;
        }
    }

    void load() {
        // draw black grid
        Scalar width = m_size.x(), height = m_size.y(), depth = m_size.z(), gridSize = m_size.w();
        uint8_t index = 0;
        for (Scalar x = -width; x <= width; x += gridSize)
            addLine(Vector3(x, 0.0, -width), Vector3(x, 0.0, x == 0 ? 0.0 : width), m_lineColor, index);
        for (Scalar z = -height; z <= height; z += gridSize)
            addLine(Vector3(-height, 0.0f, z), Vector3(z == 0 ? 0.0f : height, 0.0f, z), m_lineColor, index);
        // X coordinate (red)
        addLine(kZeroV3, Vector3(width, 0.0f, 0.0f), m_axisXColor, index);
        // Y coordinate (green)
        addLine(kZeroV3, Vector3(0.0f, height, 0.0f), m_axisYColor, index);
        // Z coordinate (blue)
        addLine(kZeroV3, Vector3(0.0f, 0.0f, depth), m_axisZColor, index);
        QGLFunctions func(QGLContext::currentContext());
        func.glGenBuffers(1, &m_vbo);
        func.glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        func.glBufferData(GL_ARRAY_BUFFER, m_vertices.count() * sizeof(Vertex), &m_vertices[0].position, GL_STATIC_DRAW);
        func.glGenBuffers(1, &m_ibo);
        func.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
        func.glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.count() * sizeof(uint8_t), &m_indices[0], GL_STATIC_DRAW);
        m_program.addShaderFromSourceFile(QGLShader::Vertex, ":shaders/grid.vsh");
        m_program.addShaderFromSourceFile(QGLShader::Fragment, ":shaders/grid.fsh");
        m_program.link();
    }

    void draw(Scene *scene, bool visible) {
        if (visible && m_program.isLinked()) {
            float matrix[16];
            m_program.bind();
            QGLFunctions func(QGLContext::currentContext());
            func.glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
            func.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
            scene->getModelViewMatrix(matrix);
            int modelViewMatrix = m_program.uniformLocation("modelViewMatrix");
            func.glUniformMatrix4fv(modelViewMatrix, 1, GL_FALSE, matrix);
            scene->getProjectionMatrix(matrix);
            int projectionMatrix = m_program.uniformLocation("projectionMatrix");
            func.glUniformMatrix4fv(projectionMatrix, 1, GL_FALSE, matrix);
            int inPosition = m_program.attributeLocation("inPosition");
            m_program.enableAttributeArray(inPosition);
            m_program.setAttributeBuffer(inPosition, GL_FLOAT, 0, 3, sizeof(Vertex));
            int inColor = m_program.attributeLocation("inColor");
            m_program.enableAttributeArray(inColor);
            m_program.setAttributeBuffer(inColor, GL_FLOAT, sizeof(Vector3), 3, sizeof(Vertex));
            glDrawElements(GL_LINES, m_indices.count(), GL_UNSIGNED_BYTE, 0);
            func.glBindBuffer(GL_ARRAY_BUFFER, 0);
            func.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            m_program.release();
        }
    }

    void setGridSize(const Vector4 &value) { m_size = value; }
    void setLineColor(const Vector3 &value) { m_lineColor = value; }
    void setAxisXColor(const Vector3 &value) { m_axisXColor = value; }
    void setAxisYColor(const Vector3 &value) { m_axisYColor = value; }
    void setAxisZColor(const Vector3 &value) { m_axisZColor = value; }

private:
    void addLine(const Vector3 &from, const Vector3 &to, const Vector3 &color, uint8_t &index) {
        Vertex f, t;
        f.position = from;
        f.color = color;
        t.position = to;
        t.color = color;
        m_vertices.add(f);
        m_vertices.add(t);
        m_indices.add(index++);
        m_indices.add(index++);
    }

    QGLShaderProgram m_program;
    Array<Vertex> m_vertices;
    Array<uint8_t> m_indices;
    Vector4 m_size;
    Vector3 m_lineColor;
    Vector3 m_axisXColor;
    Vector3 m_axisYColor;
    Vector3 m_axisZColor;
    GLuint m_vbo;
    GLuint m_ibo;
    GLuint m_list;

    Q_DISABLE_COPY(Grid)
};

}

#endif // GRID_H
