#include "vpvl/vpvl.h"
#include "vpvl/internal/util.h"

namespace vpvl
{

BoneKeyFrame::BoneKeyFrame()
{
}

BoneKeyFrame::~BoneKeyFrame()
{
}

size_t BoneKeyFrame::stride(const char * /* data */)
{
    return 15 + sizeof(uint32_t) + sizeof(float) * 7 + 64;
}

void BoneKeyFrame::read(const char *data)
{
    char *ptr = const_cast<char *>(data);
    stringCopySafe(m_name, ptr, sizeof(m_name));
    ptr += sizeof(m_name);
    uint32_t index = *reinterpret_cast<uint32_t *>(ptr);
    ptr += sizeof(uint32_t);
    float pos[3], rot[4];
    vector3(ptr, pos);
    vector4(ptr, rot);
    memcpy(m_interpolationTable, ptr, sizeof(m_interpolationTable));
    data += sizeof(m_interpolationTable);

    m_index = index;
#ifdef VPVL_COORDINATE_OPENGL
    m_position.setValue(pos[0], pos[1], -pos[2]);
    m_rotation.setValue(-rot[0], -rot[1], rot[2], rot[3]);
#else
    m_position.setValue(pos[0], pos[1], pos[2]);
    m_rotation.setValue(rot[0], rot[1], rot[2], rot[3]);
#endif
}

}
