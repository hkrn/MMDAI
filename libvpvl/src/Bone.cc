#include "vpvl/Bone.h"

namespace vpvl
{

Bone::Bone()
    : m_type(kUnknown),
      m_originPosition(0.0f, 0.0f, 0.0f),
      m_currentPosition(0.0f, 0.0f, 0.0f),
      m_offset(0.0f, 0.0f, 0.0f),
      m_currentRotation(0.0f, 0.0f, 0.0f, 1.0f),
      m_rotateCoef(0.0f),
      m_parentBone(0),
      m_childBone(0),
      m_targetBone(0),
      m_parentIsRoot(false),
      m_angleXLimited(false),
      m_simulated(false),
      m_motionIndepent(false)
{
    m_transform.setIdentity();
    m_transformMoveToOrigin.setIdentity();
}

Bone::~Bone()
{
    memset(m_name, 0, sizeof(m_name));
    m_type = kUnknown;
    m_transform.setIdentity();
    m_transformMoveToOrigin.setIdentity();
    m_originPosition.setZero();
    m_currentPosition.setZero();
    m_offset.setZero();
    m_currentRotation.setValue(0.0f, 0.0f, 0.0f, 1.0f);
    m_rotateCoef = 0.0f;
    m_parentBone = 0;
    m_childBone = 0;
    m_targetBone = 0;
    m_parentIsRoot = false;
    m_angleXLimited = false;
    m_simulated = false;
    m_motionIndepent = false;
}

size_t Bone::stride(const char * /* data */)
{
    return sizeof(m_name) + sizeof(int16_t)
            + sizeof(int16_t) + sizeof(int8_t)
            + sizeof(int16_t) + (sizeof(float) * 3);
}

void Bone::read(const char *data, btAlignedObjectArray<Bone> &bones)
{
    char *ptr = const_cast<char *>(data);
    vpvlStringCopySafe(m_name, ptr, sizeof(m_name));
    ptr += sizeof(m_name);
    int16_t parentBoneID = *reinterpret_cast<int16_t *>(ptr);
    ptr += sizeof(int16_t);
    int16_t childBoneID = *reinterpret_cast<int16_t *>(ptr);
    ptr += sizeof(int16_t);
    BoneType type = *reinterpret_cast<BoneType *>(ptr);
    ptr += sizeof(int8_t);
    int16_t targetBoneID = *reinterpret_cast<int16_t *>(ptr);
    ptr += sizeof(int16_t);
    float pos[3];
    vpvlStringGetVector3(ptr, pos);

    int nbones = bones.size();
    if (parentBoneID != -1 && parentBoneID < nbones) {
        m_parentBone = &bones[parentBoneID];
        m_parentIsRoot = false;
    }
    else if (bones.size() > 0) {
        m_parentBone = &bones[0];
        m_parentIsRoot = true;
    }

    if (childBoneID != -1 && childBoneID < nbones)
        m_childBone = &bones[childBoneID];

    if ((type == kUnderIK || m_type == kUnderRotate) && targetBoneID > 0 && targetBoneID < nbones)
        m_targetBone = &bones[targetBoneID];
    else if (type == kFollowRotate)
        m_rotateCoef = targetBoneID * 0.01f;

#ifdef VPVL_COORDINATE_OPENGL
    m_originPosition.setValue(pos[0], pos[1], -pos[2]);
#else
    m_originPosition.setValue(pos[0], pos[1], pos[2]);
#endif
    m_transform.setOrigin(m_originPosition);
    m_transformMoveToOrigin.setOrigin(-m_originPosition);
}

void Bone::computeOffset()
{
    m_offset = m_parentBone ? m_originPosition - m_parentBone->m_originPosition : m_originPosition;
}

void Bone::reset()
{
    m_currentPosition.setZero();
    m_currentRotation = btQuaternion(0.0f, 0.0f, 0.0f, 1.0f);
    m_transform.setIdentity();
    m_transform.setOrigin(m_originPosition);
}

void Bone::setMotionIndependency()
{
}

void Bone::updateRotation()
{
    btQuaternion q;
    const btQuaternion zero(0.0f, 0.0f, 0.0f, 1.0f);
    switch (m_type) {
    case kUnderRotate:
        q = m_currentRotation * m_targetBone->m_currentRotation;
        updateTransform(q);
        break;
    case kFollowRotate:
        q = m_currentRotation * zero.slerp(m_childBone->m_currentRotation, m_rotateCoef);
        updateTransform(q);
        break;
    default:
        break;
    }
}

void Bone::updateTransform()
{
    updateTransform(m_currentRotation);
}

void Bone::updateTransform(btQuaternion &q)
{
    m_transform.setOrigin(m_currentPosition + m_offset);
    m_transform.setRotation(q);
    if (m_parentBone)
        m_transform = m_parentBone->m_transform * m_transform;
}

void Bone::getSkinTransform(btTransform &tr)
{
    tr = m_transform * m_transformMoveToOrigin;
}

} /* namespace vpvl */
