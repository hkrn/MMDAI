#ifndef VPVL_BASEMOTION_H_
#define VPVL_BASEMOTION_H_

#include "LinearMath/btAlignedObjectArray.h"
#include "LinearMath/btQuaternion.h"
#include "LinearMath/btVector3.h"
#include "vpvl/common.h"

namespace vpvl
{

class BaseMotion
{
public:
    virtual ~BaseMotion() {}

    float blendRate() const {
        return m_blendRate;
    }
    void setBlendRate(float value) {
        m_blendRate = value;
    }

protected:
    uint32_t m_lastKey;
    float m_blendRate;
    float m_smearFrame;
    bool m_overrideFirst;
};

}

#endif

