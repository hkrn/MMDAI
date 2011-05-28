#ifndef VPVL_BONEKEYFRAME_H_
#define VPVL_BONEKEYFRAME_H_

#include "LinearMath/btAlignedObjectArray.h"
#include "LinearMath/btQuaternion.h"
#include "LinearMath/btVector3.h"
#include "vpvl/common.h"
#include "vpvl/internal/util.h"

namespace vpvl
{

class BoneKeyFrame
{
public:
    BoneKeyFrame()
        : m_index(0),
          m_position(0.0f, 0.0f, 0.0f),
          m_rotation(0.0f, 0.0f, 0.0f, 1.0f) {
        memset(m_name, 0, sizeof(m_name));
        memset(m_interpolationTable, 0, sizeof(m_interpolationTable));
    }
    ~BoneKeyFrame() {
        m_position.setZero();
        m_rotation.setValue(0.0f, 0.0f, 0.0f, 1.0f);
        memset(m_name, 0, sizeof(m_name));
        memset(m_interpolationTable, 0, sizeof(m_interpolationTable));
    }

    static size_t stride(const char * /* data */) {
        return 15 + sizeof(uint32_t) + sizeof(float) * 7 + 64;
    }

    void read(const char *data) {
        char *ptr = const_cast<char *>(data);
        stringCopySafe(m_name, ptr, sizeof(m_name));
        ptr += sizeof(m_name);
        uint32_t index = *reinterpret_cast<uint32_t *>(ptr);
        ptr += sizeof(uint32_t);
        float pos[3], rot[4];
        vector3(ptr, pos);
        vector4(ptr, rot);
        memcpy(m_interpolationTable, ptr, sizeof(m_interpolationTable));
        data += sizeof(m_interpolationTable);

        m_index = index;
#ifdef VPVL_COORDINATE_OPENGL
        m_position.setValue(pos[0], pos[1], -pos[2]);
        m_rotation.setValue(-rot[0], -rot[1], rot[2], rot[3]);
#else
        m_position.setValue(pos[0], pos[1], pos[2]);
        m_rotation.setValue(rot[0], rot[1], rot[2], rot[3]);
#endif
    }

    const char *name() const {
        return m_name;
    }
    uint32_t index() const {
        return m_index;
    }
    const btVector3 &position() const {
        return m_position;
    }
    const btQuaternion &rotation() const {
        return m_rotation;
    }
    const uint8_t *interpolationTable() const {
        return m_interpolationTable;
    }

private:
    void buildInterpolationTable() {
    }

    char m_name[15];
    uint32_t m_index;
    btVector3 m_position;
    btQuaternion m_rotation;
    uint8_t m_interpolationTable[64];
};

}

#endif
