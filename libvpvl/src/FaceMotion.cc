#include "vpvl/vpvl.h"
#include "vpvl/internal/FaceKeyFrame.h"

namespace vpvl
{

FaceMotion::FaceMotion()
    : m_face(0),
      m_weight(0.0f),
      m_snapWeight(0.0f)
{
}

FaceMotion::~FaceMotion()
{
    m_face = 0;
    m_weight = 0.0f;
    m_snapWeight = 0.0f;
}

void FaceMotion::read(const char *data, uint32_t size)
{
    char *ptr = const_cast<char *>(data);
    m_frames.reserve(size);
    for (uint32_t i = 0; i < size; i++) {
        FaceKeyFrame *frame = new FaceKeyFrame();
        frame->read(ptr);
        ptr += FaceKeyFrame::stride(ptr);
        m_frames.push_back(frame);
    }
}

void FaceMotion::calculate()
{
}

void FaceMotion::takeSnap(btVector3 &center)
{
}

}
