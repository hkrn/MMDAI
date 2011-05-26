#include "vpvl/vpvl.h"

namespace vpvl
{

FaceKeyFrame::FaceKeyFrame()
{
}

FaceKeyFrame::~FaceKeyFrame()
{
}

size_t FaceKeyFrame::stride(const char * /* data */)
{
    return 15 + sizeof(uint32_t) + sizeof(float);
}

void FaceKeyFrame::read(const char *data)
{
    char *ptr = const_cast<char *>(data);
    vpvlStringCopySafe(m_name, ptr, sizeof(m_name));
    ptr += sizeof(m_name);
    uint32_t index = *reinterpret_cast<uint32_t *>(ptr);
    ptr += sizeof(uint32_t);
    float weight = *reinterpret_cast<float *>(ptr);
    ptr += sizeof(float);

    m_index = index;
    m_weight = weight;
}

}
