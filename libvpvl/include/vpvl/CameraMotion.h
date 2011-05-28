#ifndef VPVL_CAMERAMOTION_H_
#define VPVL_CAMERAMOTION_H_

#include "vpvl/BaseMotion.h"

namespace vpvl
{

class CameraKeyFrame;
typedef btAlignedObjectArray<CameraKeyFrame *> CameraKeyFrameList;

class CameraMotion : BaseMotion
{
public:
    CameraMotion();
    ~CameraMotion();

    void read(const char *data, uint32_t size);

    const CameraKeyFrameList &frames() const {
        return m_frames;
    }

private:
    void calculate();

    CameraKeyFrameList m_frames;
    btVector3 m_position;
    btVector3 m_angle;
    float m_distance;
    float m_fovy;
};

}

#endif

