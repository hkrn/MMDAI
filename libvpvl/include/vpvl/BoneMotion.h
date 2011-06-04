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
    void calculate(float frameAt);
    void sort();

    const BoneKeyFrameList &frames() const {
        return m_frames;
    }
    const btVector3 &position() const {
        return m_position;
    }
    const btQuaternion &rotation() const {
        return m_rotation;
    }
    const btVector3 &snapPosition() const {
        return m_snapPosition;
    }
    const btQuaternion &snapRotation() const {
        return m_snapRotation;
    }
    const void setSnapPosition(const btVector3 &value) {
        m_snapPosition = value;
    }
    const void setSnapRotation(const btQuaternion &value) {
        m_snapRotation = value;
    }

private:
    static void lerpPosition(const BoneKeyFrame *keyFrame,
                             const btVector3 &from,
                             const btVector3 &to,
                             float w,
                             uint32_t at,
                             float &value);
    void takeSnap(btVector3 &center);

    Bone *m_bone;
    BoneKeyFrameList m_frames;
    btVector3 m_position;
    btVector3 m_snapPosition;
    btQuaternion m_rotation;
    btQuaternion m_snapRotation;
    uint32_t m_lastIndex;
    uint32_t m_lastLoopStartIndex;
    uint32_t m_noBoneSmearIndex;
    bool m_overrideFirst;
};

}

#endif
