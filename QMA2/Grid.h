#ifndef GRID_H
#define GRID_H

#include <GL/glew.h>
#include <LinearMath/btAlignedObjectArray.h>
#include <LinearMath/btVector3.h>

class Grid {
public:
    static const int kLimit = 50;

    Grid() : m_vbo(0), m_cbo(0), m_ibo(0), m_list(0) {}
    ~Grid() {
        m_vertices.clear();
        m_colors.clear();
        m_indices.clear();
        if (m_vbo) {
            glDeleteBuffers(1, &m_vbo);
            m_vbo = 0;
        }
        if (m_cbo) {
            glDeleteBuffers(1, &m_cbo);
            m_cbo = 0;
        }
        if (m_ibo) {
            glDeleteBuffers(1, &m_ibo);
            m_ibo = 0;
        }
        if (m_list) {
            glDeleteLists(m_list, 1);
            m_list = 0;
        }
    }

    void initialize() {
        // draw black grid
        btVector3 lineColor(0.5f, 0.5f, 0.5f);
        uint16_t index = 0;
        for (int x = -kLimit; x <= kLimit; x += 5)
            addLine(btVector3(x, 0.0, -kLimit), btVector3(x, 0.0, x == 0 ? 0.0 : kLimit), lineColor, index);
        for (int z = -kLimit; z <= kLimit; z += 5)
            addLine(btVector3(-kLimit, 0.0f, z), btVector3(z == 0 ? 0.0f : kLimit, 0.0f, z), lineColor, index);
        // X coordinate (red)
        addLine(btVector3(0.0f, 0.0f, 0.0f), btVector3(kLimit, 0.0f, 0.0f), btVector3(1.0f, 0.0f, 0.0f), index);
        // Y coordinate (green)
        addLine(btVector3(0.0f, 0.0f, 0.0f), btVector3(0.0f, kLimit, 0.0f), btVector3(0.0f, 1.0f, 0.0f), index);
        // Z coordinate (blue)
        addLine(btVector3(0.0f, 0.0f, 0.0f), btVector3(0.0f, 0.0f, kLimit), btVector3(0.0f, 0.0f, 1.0f), index);
        m_list = glGenLists(1);
        glGenBuffers(1, &m_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(btVector3), &m_vertices[0], GL_STATIC_DRAW);
        glGenBuffers(1, &m_cbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_cbo);
        glBufferData(GL_ARRAY_BUFFER, m_colors.size() * sizeof(btVector3), &m_colors[0], GL_STATIC_DRAW);
        glGenBuffers(1, &m_ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(uint16_t), &m_indices[0], GL_STATIC_DRAW);
        // start compiling to render with list cache
        glNewList(m_list, GL_COMPILE);
        glDisable(GL_LIGHTING);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glVertexPointer(3, GL_FLOAT, sizeof(btVector3), 0);
        glBindBuffer(GL_ARRAY_BUFFER, m_cbo);
        glColorPointer(3, GL_FLOAT, sizeof(btVector3), 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
        glDrawElements(GL_LINES, m_indices.size(), GL_UNSIGNED_SHORT, 0);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glEnable(GL_LIGHTING);
        glEndList();
    }

    void draw() const {
        glCallList(m_list);
    }

private:
    void addLine(const btVector3 &from, const btVector3 &to, const btVector3 &color, uint16_t &index) {
        m_vertices.push_back(from);
        m_vertices.push_back(to);
        m_colors.push_back(color);
        m_colors.push_back(color);
        m_indices.push_back(index);
        index++;
        m_indices.push_back(index);
        index++;
    }

    btAlignedObjectArray<btVector3> m_vertices;
    btAlignedObjectArray<btVector3> m_colors;
    btAlignedObjectArray<uint16_t> m_indices;
    GLuint m_vbo;
    GLuint m_cbo;
    GLuint m_ibo;
    GLuint m_list;
};

#endif // GRID_H
