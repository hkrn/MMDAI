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
        memset(m_linear, 0, sizeof(m_linear));
        memset(m_interpolationTable, 0, sizeof(m_interpolationTable));
    }
    ~BoneKeyFrame() {
        m_position.setZero();
        m_rotation.setValue(0.0f, 0.0f, 0.0f, 1.0f);
        for (int i = 0; i < 4; i++)
            delete m_interpolationTable[i];
        memset(m_name, 0, sizeof(m_name));
        memset(m_linear, 0, sizeof(m_linear));
        memset(m_interpolationTable, 0, sizeof(m_interpolationTable));
    }

    static const int kTableSize = 64;

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
        int8_t table[64];
        memcpy(table, ptr, sizeof(table));
        data += sizeof(table);

        m_index = static_cast<float>(index);
#ifdef VPVL_COORDINATE_OPENGL
        m_position.setValue(pos[0], pos[1], -pos[2]);
        m_rotation.setValue(-rot[0], -rot[1], rot[2], rot[3]);
#else
        m_position.setValue(pos[0], pos[1], pos[2]);
        m_rotation.setValue(rot[0], rot[1], rot[2], rot[3]);
#endif
        setInterpolationTable(table);
    }

    const char *name() const {
        return m_name;
    }
    float index() const {
        return m_index;
    }
    const btVector3 &position() const {
        return m_position;
    }
    const btQuaternion &rotation() const {
        return m_rotation;
    }
    const bool *linear() const {
        return m_linear;
    }
    const float *const *interpolationTable() const {
        return m_interpolationTable;
    }

private:
    void setInterpolationTable(const int8_t *table) {
        for (int i = 0; i < 4; i++)
            m_linear[i] = (table[0 + i] == table[4 + i] == table[8 + i] == table[12 + i]) ? true : false;
        for (int i = 0; i < 4; i++) {
            if (m_linear[i]) {
                m_interpolationTable[i] = 0;
                continue;
            }
            m_interpolationTable[i] = new float[kTableSize + 1];
            float x1 = table[i]      / 127.0f;
            float y1 = table[i +  4] / 127.0f;
            float x2 = table[i +  8] / 127.0f;
            float y2 = table[i + 12] / 127.0f;
            buildInterpolationTable(x1, x2, y1, y2, kTableSize, m_interpolationTable[i]);
        }
    }

    char m_name[15];
    float m_index;
    btVector3 m_position;
    btQuaternion m_rotation;
    bool m_linear[4];
    float *m_interpolationTable[4];
};

}

#endif
