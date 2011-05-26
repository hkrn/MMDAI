#ifndef VPVL_CAMERAKEYFRAME_H_
#define VPVL_CAMERAKEYFRAME_H_

#include "LinearMath/btAlignedObjectArray.h"
#include "LinearMath/btVector3.h"
#include "vpvl/common.h"

namespace vpvl
{

class CameraKeyFrame
{
public:
    CameraKeyFrame();
    ~CameraKeyFrame();

    static size_t stride(const char *data);
    void read(const char *data);

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

typedef btAlignedObjectArray<CameraKeyFrame *> CameraKeyFrameList;

}

#endif
