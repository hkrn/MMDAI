#include "vpvl/vpvl.h"
#include "vpvl/internal/CameraKeyFrame.h"

namespace vpvl
{

class CameraMotionKeyFramePredication
{
public:
    bool operator()(const CameraKeyFrame *left, const CameraKeyFrame *right) {
        return left->index() - right->index();
    }
};

CameraMotion::CameraMotion()
    : m_position(0.0f, 0.0f, 0.0f),
      m_angle(0.0f, 0.0f, 0.0f),
      m_distance(0.0f),
      m_fovy(0.0f)
{
}

CameraMotion::~CameraMotion()
{
    m_position.setZero();
    m_angle.setZero();
    m_distance = 0.0f;
    m_fovy = 0.0f;
}

void CameraMotion::read(const char *data, uint32_t size)
{
    char *ptr = const_cast<char *>(data);
    m_frames.reserve(size);
    for (uint32_t i = 0; i < size; i++) {
        CameraKeyFrame *frame = new CameraKeyFrame();
        frame->read(ptr);
        ptr += CameraKeyFrame::stride(ptr);
        m_frames.push_back(frame);
    }
}

void CameraMotion::sort()
{
    m_frames.quickSort(CameraMotionKeyFramePredication());
}

}
