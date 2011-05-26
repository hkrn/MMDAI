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

    static Bone *centerBone(btAlignedObjectArray<Bone*> *bones);
    static size_t stride(const char *data);

    void read(const char *data, btAlignedObjectArray<Bone*> *bones, Bone *rootBone);
    void computeOffset();
    void reset();
    void setMotionIndependency();
    void updateRotation();
    void updateTransform();
    void updateTransform(const btQuaternion &q);
    void getSkinTransform(btTransform &tr);

    const char *name() const {
        return m_name;
    }
    BoneType type() const {
        return m_type;
    }
    const btTransform &currentTransform() const {
        return m_currentTransform;
    }
    const btVector3 &offset() const {
        return m_offset;
    }
    const btVector3 &originPosition() const {
        return m_originPosition;
    }
    const btVector3 &currentPosition() const {
        return m_currentPosition;
    }
    const btQuaternion &currentRotation() const {
        return m_currentRotation;
    }
    bool isAngleXLimited() const {
        return m_angleXLimited;
    }
    bool isSimulated() const {
        return m_simulated;
    }
    bool hasMotionIndependency() const {
        return m_motionIndepent;
    }

    void setName(const char *value) {
        vpvlStringCopySafe(m_name, value, sizeof(m_name));
    }
    void setCurrentTransform(const btTransform &value) {
        m_currentTransform = value;
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
    btTransform m_currentTransform;
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

typedef btAlignedObjectArray<Bone*> BoneList;

} /* namespace vpvl */

#endif
