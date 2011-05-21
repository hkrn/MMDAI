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

    size_t stride(const char *data);
    void read(const char *data, BoneList *bones);
    void solve();

    bool isSimulated() {
        return m_bones[0]->isSimulated();
    }

private:
    Bone *m_destination;
    Bone *m_target;
    btAlignedObjectArray<Bone *> m_bones;
    uint16_t m_iteration;
    float m_angleConstraint;
};

} /* namespace vpvl */

