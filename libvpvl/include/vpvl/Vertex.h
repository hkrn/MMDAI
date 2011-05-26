#ifndef VPVL_VERTEX_H_
#define VPVL_VERTEX_H_

#include <LinearMath/btAlignedObjectArray.h>
#include <LinearMath/btVector3.h>
#include "vpvl/common.h"

namespace vpvl
{

class Vertex
{
public:
    Vertex();
    ~Vertex();

    static size_t stride(const char *data);

    void read(const char *data);

    const btVector3 &position() const {
        return m_position;
    }
    const btVector3 &normal() const {
        return m_normal;
    }
    float u() const {
        return m_u;
    }
    float v() const {
        return m_v;
    }
    int16_t bone1() const {
        return m_bone1;
    }
    int16_t bone2() const {
        return m_bone2;
    }
    float weight() const {
        return m_weight;
    }
    bool isEdgeEnabled() const {
        return m_edge;
    }

    void setPosition(const btVector3 &value) {
        m_position = value;
    }
    void setNormal(const btVector3 &value) {
        m_normal = value;
    }
    void setU(float value) {
        m_u = value;
    }
    void setV(float value) {
        m_v = value;
    }
    void setWeight(float value) {
        m_weight = value;
    }
    void setEdgeEnable(bool value) {
        m_edge = value;
    }

private:
    btVector3 m_position;
    btVector3 m_normal;
    float m_u;
    float m_v;
    int16_t m_bone1;
    int16_t m_bone2;
    float m_weight;
    bool m_edge;
};

typedef btAlignedObjectArray<Vertex*> VertexList;

} /* namespace vpvl */

#endif
