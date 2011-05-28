#ifndef VPVL_BONEMOTION_H_
#define VPVL_BONEMOTION_H_

#include "vpvl/BaseMotion.h"

namespace vpvl
{

class Bone;
class BoneKeyFrame;
typedef btAlignedObjectArray<BoneKeyFrame *> BoneKeyFrameList;

class BoneMotion
{
public:
    BoneMotion();
    ~BoneMotion();

    static const float kStartingMarginFrame;

    void read(const char *data, uint32_t size);

    const BoneKeyFrameList &frames() const {
        return m_frames;
    }

private:
    void calculate();
    void takeSnap(btVector3 &center);

    Bone *m_bone;
    BoneKeyFrameList m_frames;
    btVector3 m_position;
    btVector3 m_snapPosition;
    btQuaternion m_rotation;
    btQuaternion m_snapRotation;
};

}

#endif
