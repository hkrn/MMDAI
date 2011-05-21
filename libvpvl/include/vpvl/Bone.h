#ifndef VPVL_BONE_H_
#define VPVL_BONE_H_

#include <LinearMath/btAlignedObjectArray.h>
#include <LinearMath/btQuaternion.h>
#include <LinearMath/btTransform.h>
#include <LinearMath/btVector3.h>
#include "vpvl/common.h"

namespace vpvl
{

enum BoneType {
    kRotate,
    kRotateAndMove,
    kIKDestination,
    kUnknown,
    kUnderIK,
    kUnderRotate,
    kIKTarget,
    kInvisible,
    kTwist,
    kFollowRotate
};

class Bone
{
public:
    Bone();
    ~Bone();

    size_t stride();
    void read(const char *data, btAlignedObjectArray<Bone> &bones);
    void computeOffset();
    void reset();
    void setMotionIndependency();
    void updateRotation();
    void updateTransform();
    void updateTransform(btQuaternion &q);
    void getSkinTransform(btTransform &tr);

    const char *name() {
        return m_name;
    }
    int type() {
        return m_type;
    }
    const btTransform transform() {
        return m_transform;
    }
    const btVector3 offset() {
        return m_offset;
    }
    const btVector3 originPosition() {
        return m_originPosition;
    }
    const btVector3 currentPosition() {
        return m_currentPosition;
    }
    const btQuaternion currentRotation() {
        return m_currentRotation;
    }
    bool isAngleXLimited() {
        return m_angleXLimited;
    }
    bool isSimulated() {
        return m_simulated;
    }
    bool hasMotionIndependency() {
        return m_motionIndepent;
    }

    void setTransform(const btTransform &value) {
        m_transform = value;
    }
    void setOffset(const btVector3 &value) {
        m_offset = value;
    }
    void setCurrentPosition(const btVector3 &value) {
        m_currentPosition = value;
    }
    void setCurrentRotation(const btQuaternion &value) {
        m_currentRotation = value;
    }
    void setSimulated(bool value) {
        m_simulated = value;
    }

private:
    char m_name[20];
    BoneType m_type;
    btTransform m_transform;
    btTransform m_transformMoveToOrigin;
    btVector3 m_originPosition;
    btVector3 m_currentPosition;
    btVector3 m_offset;
    btQuaternion m_currentRotation;
    float m_rotateCoef;
    Bone *m_parentBone;
    Bone *m_childBone;
    Bone *m_targetBone;
    bool m_parentIsRoot;
    bool m_angleXLimited;
    bool m_simulated;
    bool m_motionIndepent;
};

typedef btAlignedObjectArray<Bone> BoneList;

} /* namespace vpvl */

#endif
