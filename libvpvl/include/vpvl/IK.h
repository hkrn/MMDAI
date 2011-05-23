#ifndef VPVL_IK_H_
#define VPVL_IK_H_

#include "vpvl/Bone.h"

namespace vpvl
{

class IK
{
public:
    IK();
    ~IK();

    static const float kPi;
    static const float kMinDistance;
    static const float kMinAngle;
    static const float kMinAxis;
    static const float kMinRotationSum;
    static const float kMinRotation;
    static size_t totalSize(const char *data, size_t n);
    static size_t stride(const char *data);

    void read(const char *data, BoneList *bones);
    void solve();

    bool isSimulated() const {
        return m_bones.size() > 0 ? m_bones[0]->isSimulated() : false;
    }

private:
    Bone *m_destination;
    Bone *m_target;
    BoneList m_bones;
    uint16_t m_iteration;
    float m_angleConstraint;
};

typedef btAlignedObjectArray<IK*> IKList;

} /* namespace vpvl */

#endif
