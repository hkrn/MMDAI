#ifndef VPVL_BONEKEYFRAME_H_
#define VPVL_BONEKEYFRAME_H_

#include "LinearMath/btAlignedObjectArray.h"
#include "LinearMath/btQuaternion.h"
#include "LinearMath/btVector3.h"
#include "vpvl/common.h"

namespace vpvl
{

class BoneKeyFrame
{
public:
    BoneKeyFrame();
    ~BoneKeyFrame();

    static size_t stride(const char *data);
    void read(const char *data);

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
    void buildInterpolationTable();

    char m_name[15];
    uint32_t m_index;
    btVector3 m_position;
    btQuaternion m_rotation;
    uint8_t m_interpolationTable[64];
};

typedef btAlignedObjectArray<BoneKeyFrame *> BoneKeyFrameList;

}

#endif
