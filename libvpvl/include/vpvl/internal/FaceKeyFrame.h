#ifndef VPVL_FACEKEYFRAME_H_
#define VPVL_FACEKEYFRAME_H_

#include "LinearMath/btAlignedObjectArray.h"
#include "vpvl/common.h"
#include "vpvl/internal/util.h"

namespace vpvl
{

class FaceKeyFrame
{
public:
    FaceKeyFrame() : m_index(0), m_weight(0.0f) {
        memset(m_name, 0, sizeof(m_name));
    }
    ~FaceKeyFrame() {
        memset(m_name, 0, sizeof(m_name));
    }

    static size_t stride(const char * /* data */) {
        return 15 + sizeof(uint32_t) + sizeof(float);
    }

    void read(const char *data) {
        char *ptr = const_cast<char *>(data);
        stringCopySafe(m_name, ptr, sizeof(m_name));
        ptr += sizeof(m_name);
        uint32_t index = *reinterpret_cast<uint32_t *>(ptr);
        ptr += sizeof(uint32_t);
        float weight = *reinterpret_cast<float *>(ptr);
        ptr += sizeof(float);

        m_index = static_cast<float>(index);
        m_weight = weight;
    }

    const char *name() const {
        return m_name;
    }
    float index() const {
        return m_index;
    }
    float weight() const {
        return m_weight;
    }

private:
    char m_name[15];
    float m_index;
    float m_weight;
};

}

#endif
