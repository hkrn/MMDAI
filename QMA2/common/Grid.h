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
#include <QtOpenGL/qglfunctions.h>

#include <vpvl/Scene.h>

namespace internal {

class Grid {
public:
    struct Vertex {
        vpvl::Vector3 position;
        vpvl::Vector3 color;
    };

    static const int kLimit = 50;

    Grid() : m_vbo(0), m_ibo(0), m_list(0), m_enabled(true) {}
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

    void initialize() {
        // draw black grid
        static const vpvl::Vector3 zero(0.0f, 0.0f, 0.0f);
        static const vpvl::Vector3 lineColor(0.5f, 0.5f, 0.5f);
        uint8_t index = 0;
        for (int x = -kLimit; x <= kLimit; x += 5)
            addLine(vpvl::Vector3(x, 0.0, -kLimit), vpvl::Vector3(x, 0.0, x == 0 ? 0.0 : kLimit), lineColor, index);
        for (int z = -kLimit; z <= kLimit; z += 5)
            addLine(vpvl::Vector3(-kLimit, 0.0f, z), vpvl::Vector3(z == 0 ? 0.0f : kLimit, 0.0f, z), lineColor, index);
        // X coordinate (red)
        addLine(zero, vpvl::Vector3(kLimit, 0.0f, 0.0f), vpvl::Vector3(1.0f, 0.0f, 0.0f), index);
        // Y coordinate (green)
        addLine(zero, vpvl::Vector3(0.0f, kLimit, 0.0f), vpvl::Vector3(0.0f, 1.0f, 0.0f), index);
        // Z coordinate (blue)
        addLine(zero, vpvl::Vector3(0.0f, 0.0f, kLimit), vpvl::Vector3(0.0f, 0.0f, 1.0f), index);
        m_list = glGenLists(1);
		QGLFunctions func(QGLContext::currentContext());
        func.glGenBuffers(1, &m_vbo);
        func.glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        func.glBufferData(GL_ARRAY_BUFFER, m_vertices.count() * sizeof(Vertex), &m_vertices[0].position, GL_STATIC_DRAW);
        func.glGenBuffers(1, &m_ibo);
        func.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
        func.glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.count() * sizeof(uint8_t), &m_indices[0], GL_STATIC_DRAW);
        // start compiling to render with list cache
        glNewList(m_list, GL_COMPILE);
        glDisable(GL_LIGHTING);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        func.glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glVertexPointer(3, GL_FLOAT, sizeof(Vertex), reinterpret_cast<GLvoid *>(0));
        glColorPointer(3, GL_FLOAT, sizeof(Vertex), reinterpret_cast<GLvoid *>(sizeof(vpvl::Vector3)));
        func.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
        glDrawElements(GL_LINES, m_indices.count(), GL_UNSIGNED_BYTE, 0);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        func.glBindBuffer(GL_ARRAY_BUFFER, 0);
        func.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glEnable(GL_LIGHTING);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        glEndList();
    }

    void draw(vpvl::Scene *scene) const {
        if (m_enabled) {
            float modelview[16], projection[16];
            glMatrixMode(GL_PROJECTION);
            scene->getProjectionMatrix(projection);
            glLoadMatrixf(projection);
            glMatrixMode(GL_MODELVIEW);
            scene->getModelViewMatrix(modelview);
            glLoadMatrixf(modelview);
            glCallList(m_list);
        }
    }

    bool isEnabled() const {
        return m_enabled;
    }

    void setEnable(bool value) {
        m_enabled = value;
    }

private:
    void addLine(const vpvl::Vector3 &from, const vpvl::Vector3 &to, const vpvl::Vector3 &color, uint8_t &index) {
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

    vpvl::Array<Vertex> m_vertices;
    vpvl::Array<uint8_t> m_indices;
    GLuint m_vbo;
    GLuint m_ibo;
    GLuint m_list;
    bool m_enabled;

    Q_DISABLE_COPY(Grid)
};

}

#endif // GRID_H
