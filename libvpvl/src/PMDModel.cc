#include "vpvl/vpvl.h"
#include "PMDModel.h"

namespace vpvl
{

PMDModel::PMDModel()
{
    memset(&m_name, 0, sizeof(m_name));
    memset(&m_comment, 0, sizeof(m_comment));
    memset(&m_englishName, 0, sizeof(m_englishName));
    memset(&m_englishComment, 0, sizeof(m_englishComment));
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
