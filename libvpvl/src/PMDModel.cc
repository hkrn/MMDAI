#include "vpvl/vpvl.h"
#include "vpvl/internal/PMDModel.h"

namespace vpvl
{

PMDModel::PMDModel()
{
    memset(&m_name, 0, sizeof(m_name));
    memset(&m_comment, 0, sizeof(m_comment));
    memset(&m_englishName, 0, sizeof(m_englishName));
    memset(&m_englishComment, 0, sizeof(m_englishComment));
    m_rootBone.setCurrentRotation(btQuaternion(0.0f, 0.0f, 0.0f, 1.0f));
    m_rootBone.updateTransform();
}

PMDModel::~PMDModel()
{
    memset(&m_name, 0, sizeof(m_name));
    memset(&m_comment, 0, sizeof(m_comment));
    memset(&m_englishName, 0, sizeof(m_englishName));
    memset(&m_englishComment, 0, sizeof(m_englishComment));
    m_vertices.clear();
    m_indices.clear();
    m_bones.clear();
    m_IKs.clear();
    m_faces.clear();
    m_rigidBodies.clear();
    m_constraints.clear();
}

}
