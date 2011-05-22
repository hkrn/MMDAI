#include "vpvl/vpvl.h"

namespace vpvl
{

Face::Face()
    : m_type(kOther),
      m_weight(0.0f)
{
    memset(m_name, 0, sizeof(m_name));
}

Face::~Face()
{
    memset(m_name, 0, sizeof(m_name));
    m_vertices.clear();
    m_type = kOther;
    m_weight = 0.0f;
}

size_t Face::stride(const char *data)
{
    char *ptr = const_cast<char *>(data);
    size_t base = 20 + sizeof(uint32_t) + sizeof(uint8_t);
    ptr += base;
    int nvertices = *reinterpret_cast<int *>(ptr);
    return base + nvertices * sizeof(FaceVertex);
}

void Face::read(const char *data)
{
    char *ptr = const_cast<char *>(data);
    vpvlStringCopySafe(m_name, ptr, sizeof(m_name));
    ptr += sizeof(m_name);
    FaceType type = *reinterpret_cast<FaceType *>(ptr);
    ptr += sizeof(uint8_t);
    int nvertices = *reinterpret_cast<int *>(ptr);
    m_type = type;
    if (nvertices > 0) {
        m_vertices.reserve(nvertices);
        for (int i = 0; i < nvertices; i++) {
            m_vertices[i].id = *reinterpret_cast<uint32_t *>(ptr);
            ptr += sizeof(uint32_t);
            float pos[3];
            vpvlStringGetVector3(ptr, pos);
#ifdef VPVL_COORDINATE_OPENGL
            m_vertices[i].position.setValue(pos[0], pos[1], -pos[2]);
#else
            m_vertices[i].position.setValue(pos[0], pos[1], pos[2]);
#endif
        }
    }
}

void Face::convertIndices(const Face &base)
{
    int nvertices = m_vertices.size();
    int baseNVertices = base.m_vertices.size();
    if (m_type != kBase) {
        for (int i = 0; i < nvertices; i++) {
            int relID = m_vertices[i].id;
            if (relID >= baseNVertices)
                relID -= kMaxVertexID;
            m_vertices[i].id = base.m_vertices[relID].id;
        }
    }
    else {
        for (int i = 0; i < nvertices; i++) {
            if (m_vertices[i].id >= kMaxVertexID)
                m_vertices[i].id -= kMaxVertexID;
        }
    }
}

void Face::applyToVertices(VertexList &vertices)
{
    int nvertices = m_vertices.size();
    for (int i = 0; i < nvertices; i++)
        vertices[m_vertices[i].id].setPosition(m_vertices[i].position);
}

void Face::addToVertices(VertexList &vertices, float rate)
{
    int nvertices = m_vertices.size();
    for (int i = 0; i < nvertices; i++)
        vertices[m_vertices[i].id].setPosition(m_vertices[i].position * rate);
}

} /* namespace vpvl */
