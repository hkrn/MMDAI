#include "vpvl/vpvl.h"

namespace vpvl
{

CameraKeyFrame::CameraKeyFrame()
{
}

CameraKeyFrame::~CameraKeyFrame()
{
}

size_t CameraKeyFrame::stride(const char * /* data */)
{
    return sizeof(uint32_t) + sizeof(float) * 7 + 24 + sizeof(uint32_t) + sizeof(uint8_t);
}

void CameraKeyFrame::read(const char *data)
{
    char *ptr = const_cast<char *>(data);
    uint32_t index = *reinterpret_cast<uint32_t *>(ptr);
    ptr += sizeof(uint32_t);
    float distance = *reinterpret_cast<float *>(ptr);
    ptr += sizeof(float);
    float pos[3], angle[3];
    vpvlStringGetVector3(ptr, pos);
    vpvlStringGetVector3(ptr, angle);
    memcpy(m_interpolationTable, ptr, sizeof(m_interpolationTable));
    ptr += sizeof(m_interpolationTable);
    uint32_t fovy = *reinterpret_cast<uint32_t *>(ptr);
    ptr += sizeof(uint32_t);
    uint8_t noPerspective = *reinterpret_cast<uint8_t *>(ptr);
    ptr += sizeof(uint8_t);

    m_index = index;
    m_distance = distance;
    m_fovy = static_cast<float>(fovy);
    m_noPerspective = noPerspective == 1;
#ifdef VPVL_COORDINATE_OPENGL
    m_position.setValue(pos[0], pos[1], -pos[2]);
    m_angle.setValue(-vpvlMathDegree(angle[0]), -vpvlMathDegree(angle[1]), vpvlMathDegree(angle[2]));
#else
    m_position.setValue(pos[0], pos[1], pos[2]);
    m_angle.setValue(vpvlMathDegree(angle[0]), vpvlMathDegree(angle[1]), vpvlMathDegree(angle[2]));
#endif
}

}
