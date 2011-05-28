#include "vpvl/vpvl.h"
#include "vpvl/internal/BoneKeyFrame.h"

namespace vpvl
{

BoneMotion::BoneMotion()
    : m_bone(0),
      m_position(0.0f, 0.0f, 0.0f),
      m_snapPosition(0.0f, 0.0f, 0.0f),
      m_rotation(0.0f, 0.0f, 0.0f, 1.0f),
      m_snapRotation(0.0f, 0.0f, 0.0f, 1.0f)
{
}

BoneMotion::~BoneMotion()
{
    m_position.setZero();
    m_snapPosition.setZero();
    m_rotation.setValue(0.0f, 0.0f, 0.0f, 1.0f);
    m_snapRotation.setValue(0.0f, 0.0f, 0.0f, 1.0f);
}

void BoneMotion::read(const char *data, uint32_t size)
{
    char *ptr = const_cast<char *>(data);
    m_frames.reserve(size);
    for (uint32_t i = 0; i < size; i++) {
        BoneKeyFrame *frame = new BoneKeyFrame();
        frame->read(ptr);
        ptr += BoneKeyFrame::stride(ptr);
        m_frames.push_back(frame);
    }
}

void BoneMotion::calculate()
{
}

void BoneMotion::takeSnap(btVector3 &center)
{
}

}
