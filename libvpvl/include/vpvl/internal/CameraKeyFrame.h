#ifndef VPVL_CAMERAKEYFRAME_H_
#define VPVL_CAMERAKEYFRAME_H_

#include "LinearMath/btAlignedObjectArray.h"
#include "LinearMath/btVector3.h"
#include "vpvl/common.h"
#include "vpvl/internal/util.h"

namespace vpvl
{

class CameraKeyFrame
{
public:
    CameraKeyFrame()
        : m_index(0),
          m_distance(0.0f),
          m_fovy(0.0f),
          m_position(0.0f, 0.0f, 0.0f),
          m_angle(0.0f, 0.0f, 0.0f),
          m_noPerspective(false)
    {
        memset(m_interpolationTable, 0, sizeof(m_interpolationTable));
    }
    ~CameraKeyFrame() {
        m_index = 0;
        m_distance = 0.0f;
        m_fovy = 0.0f;
        m_position.setZero();
        m_angle.setZero();
        m_noPerspective = false;
        memset(m_interpolationTable, 0, sizeof(m_interpolationTable));
    }

    static size_t stride(const char * /* data */) {
        return sizeof(uint32_t) + sizeof(float) * 7 + 24 + sizeof(uint32_t) + sizeof(uint8_t);
    }

    void read(const char *data) {
        char *ptr = const_cast<char *>(data);
        uint32_t index = *reinterpret_cast<uint32_t *>(ptr);
        ptr += sizeof(uint32_t);
        float distance = *reinterpret_cast<float *>(ptr);
        ptr += sizeof(float);
        float pos[3], angle[3];
        vector3(ptr, pos);
        vector3(ptr, angle);
        memcpy(m_interpolationTable, ptr, sizeof(m_interpolationTable));
        ptr += sizeof(m_interpolationTable);
        uint32_t fovy = *reinterpret_cast<uint32_t *>(ptr);
        ptr += sizeof(uint32_t);
        uint8_t noPerspective = *reinterpret_cast<uint8_t *>(ptr);
        ptr += sizeof(uint8_t);

        m_index = index;
        m_distance = distance;
        m_fovy = static_cast<float>(fovy);
        m_noPerspective = noPerspective == 1;
    #ifdef VPVL_COORDINATE_OPENGL
        m_position.setValue(pos[0], pos[1], -pos[2]);
        m_angle.setValue(-degree(angle[0]), -degree(angle[1]), degree(angle[2]));
    #else
        m_position.setValue(pos[0], pos[1], pos[2]);
        m_angle.setValue(degree(angle[0]), degree(angle[1]), degree(angle[2]));
    #endif
    }

    uint32_t index() const {
        return m_index;
    }
    float distance() const {
        return m_distance;
    }
    float fovy() const {
        return m_fovy;
    }
    const btVector3 &position() const {
        return m_position;
    }
    const btVector3 &angle() const {
        return m_angle;
    }
    const uint8_t *interpolationTable() const {
        return m_interpolationTable;
    }

private:
    void buildInterpolationTable();

    uint32_t m_index;
    float m_distance;
    float m_fovy;
    btVector3 m_position;
    btVector3 m_angle;
    bool m_noPerspective;
    uint8_t m_interpolationTable[24];
};

}

#endif
