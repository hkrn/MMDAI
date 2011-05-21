#ifndef VPVL_RIGIDBODY_H_
#define VPVL_RIGIDBODY_H_

#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <LinearMath/btAlignedObjectArray.h>
#include "vpvl/Bone.h"

namespace vpvl
{

class RigidBody
{
public:
    RigidBody();
    ~RigidBody();

    size_t stride(const char *data);
    void read(const char *data, Bone *bone);
    void transformToBone();
    void setKinematic(bool value);

    const char *name() {
        return m_name;
    }
    btRigidBody *body() {
        return m_body;
    }

private:
    char m_name[20];
    Bone *m_bone;
    btCollisionShape *m_shape;
    btRigidBody *m_body;
    btMotionState *m_motionState;
    btTransform m_transform;
    btTransform m_invertedTransform;
    btMotionState *m_kinematicMotionState;
    unsigned short m_groupID;
    unsigned short m_groupMask;
    unsigned char m_type;
    bool m_noBone;
};

typedef btAlignedObjectArray<RigidBody> RigidBodyList;

} /* namespace vpvl */

#endif
