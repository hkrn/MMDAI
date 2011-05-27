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

    static size_t stride(const char *data);

    void read(const char *data, BoneList *bones);
    void transformToBone();
    void setKinematic(bool value);

    const char *name() const {
        return m_name;
    }
    btRigidBody *body() const {
        return m_body;
    }

    void setName(const char *value) {
        stringCopySafe(m_name, value, sizeof(m_name));
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

typedef btAlignedObjectArray<RigidBody*> RigidBodyList;

} /* namespace vpvl */

#endif
