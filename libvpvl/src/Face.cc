#include "vpvl/vpvl.h"
#include "vpvl/internal/util.h"

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
    m_type = kOther;
    m_weight = 0.0f;
}

// FIXME: boundary check
size_t Face::totalSize(const char *data, size_t n)
{
    size_t size = 0;
    char *ptr = const_cast<char *>(data);
    for (size_t i = 0; i < n; i++) {
        ptr += 20;
        uint32_t nvertices = *reinterpret_cast<uint32_t *>(ptr);
        size_t rest = sizeof(uint32_t) + sizeof(uint8_t) + nvertices * (sizeof(uint32_t) + sizeof(float) * 3);
        size += 20 + rest;
        ptr += rest;
    }
    return size;
}

size_t Face::stride(const char *data)
{
    char *ptr = const_cast<char *>(data);
    size_t base = 20;
    ptr += base;
    int nvertices = *reinterpret_cast<int *>(ptr);
    return base + sizeof(uint32_t) + sizeof(uint8_t) + nvertices * (sizeof(uint32_t) + sizeof(float) * 3);
}

void Face::read(const char *data)
{
    char *ptr = const_cast<char *>(data);
    stringCopySafe(m_name, ptr, sizeof(m_name));
    ptr += sizeof(m_name);
    uint32_t nvertices = *reinterpret_cast<uint32_t *>(ptr);
    ptr += sizeof(uint32_t);
    FaceType type = static_cast<FaceType>(*reinterpret_cast<uint8_t *>(ptr));
    ptr += sizeof(uint8_t);
    m_type = type;
    if (nvertices > 0) {
        for (uint32_t i = 0; i < nvertices; i++) {
            FaceVertex *vertex = new FaceVertex();
            vertex->id = *reinterpret_cast<uint32_t *>(ptr);
            ptr += sizeof(uint32_t);
            float pos[3];
            vector3(ptr, pos);
#ifdef VPVL_COORDINATE_OPENGL
            vertex->position.setValue(pos[0], pos[1], -pos[2]);
#else
            vertex->position.setValue(pos[0], pos[1], pos[2]);
#endif
            m_vertices.push_back(vertex);
        }
    }
}

void Face::convertIndices(const Face *base)
{
    uint32_t nvertices = m_vertices.size();
    uint32_t baseNVertices = base->m_vertices.size();
    if (m_type != kBase) {
        for (uint32_t i = 0; i < nvertices; i++) {
            uint32_t relID = m_vertices[i]->id;
            if (relID >= baseNVertices)
                relID -= kMaxVertexID;
            m_vertices[i]->id = base->m_vertices[relID]->id;
        }
    }
    else {
        for (uint32_t i = 0; i < nvertices; i++) {
            if (m_vertices[i]->id >= kMaxVertexID)
                m_vertices[i]->id -= kMaxVertexID;
        }
    }
}

void Face::applyToVertices(VertexList &vertices)
{
    uint32_t nvertices = m_vertices.size();
    for (uint32_t i = 0; i < nvertices; i++)
        vertices[m_vertices[i]->id]->setPosition(m_vertices[i]->position);
}

void Face::addToVertices(VertexList &vertices, float rate)
{
    uint32_t nvertices = m_vertices.size();
    for (uint32_t i = 0; i < nvertices; i++)
        vertices[m_vertices[i]->id]->setPosition(m_vertices[i]->position * rate);
}

} /* namespace vpvl */
