#include "vpvl/vpvl.h"

namespace vpvl
{

Vertex::Vertex()
    : m_position(0.0f, 0.0f, 0.0f),
      m_normal(0.0f, 0.0f, 0.0f),
      m_u(0.0f),
      m_v(0.0f),
      m_bone1(0),
      m_bone2(0),
      m_weight(0.0f),
      m_edge(false)
{
}

Vertex::~Vertex()
{
    m_position.setZero();
    m_normal.setZero();
    m_u = 0.0f;
    m_v = 0.0f;
    m_bone1 = 0;
    m_bone2 = 0;
    m_weight = 0.0f;
    m_edge = false;
}

size_t Vertex::stride(const char * /* data */)
{
    return sizeof(float) * 7 + sizeof(int16_t) * 2 + sizeof(uint8_t);
}

void Vertex::read(const char *data)
{
    char *ptr = const_cast<char *>(data);
    float pos[3], normal[3];
    vpvlStringGetVector3(ptr, pos);
    vpvlStringGetVector3(ptr, normal);
    float u = *reinterpret_cast<float *>(ptr);
    ptr += sizeof(float);
    float v = *reinterpret_cast<float *>(ptr);
    ptr += sizeof(float);
    int16_t bone1 = *reinterpret_cast<int16_t *>(ptr);
    ptr += sizeof(int16_t);
    int16_t bone2 = *reinterpret_cast<int16_t *>(ptr);
    ptr += sizeof(int16_t);
    float weight = *reinterpret_cast<float *>(ptr);
    ptr += sizeof(float);
    uint8_t edge = *reinterpret_cast<uint8_t *>(ptr);
    ptr += sizeof(uint8_t);

#ifdef VPVL_COORDINATE_OPENGL
    m_position.setValue(pos[0], pos[1], -pos[2]);
#else
    m_position.setValue(pos[0], pos[1], pos[2]);
#endif
    m_normal.setValue(normal[0], normal[1], normal[2]);
    m_u = u;
    m_v = v;
    m_bone1 = bone1;
    m_bone2 = bone2;
    m_weight = weight * 0.01f;
    m_edge = edge == 0;
}

}
