#include "vpvl/vpvl.h"
#include "vpvl/internal/FaceKeyFrame.h"

namespace vpvl
{

const float FaceMotion::kStartingMarginFrame = 6.0f;

class FaceMotionKeyFramePredication
{
public:
    bool operator()(const FaceKeyFrame *left, const FaceKeyFrame *right) {
        return left->index() - right->index();
    }
};

FaceMotion::FaceMotion()
    : m_face(0),
      m_lastIndex(0),
      m_lastLoopStartIndex(0),
      m_noFaceSmearIndex(0),
      m_weight(0.0f),
      m_snapWeight(0.0f),
      m_overrideFirst(false)
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

void FaceMotion::calculate(float frameAt)
{
    uint32_t nFrames = m_frames.size();
    FaceKeyFrame *lastKeyFrame = m_frames.at(nFrames - 1);
    float currentFrame = frameAt;
    if (currentFrame > lastKeyFrame->index())
        currentFrame = lastKeyFrame->index();

    uint32_t k1 = 0, k2 = 0;
    if (currentFrame >= m_frames.at(m_lastIndex)->index()) {
        for (uint32_t i = m_lastIndex; i < nFrames; i++) {
            if (currentFrame <= m_frames.at(i)->index()) {
                k2 = i;
                break;
            }
        }
    }
    else {
        for (uint32_t i = 0; i <= m_lastIndex && i < nFrames; i++) {
            if (currentFrame <= m_frames.at(i)->index()) {
                k2 = i;
                break;
            }
        }
    }

    if (k2 >= nFrames)
        k2 = nFrames - 1;
    k1 = k2 <= 1 ? 0 : k2 - 1;
    m_lastIndex = k1;

    const FaceKeyFrame *keyFrameFrom = m_frames.at(k1), *keyFrameTo = m_frames.at(k2);
    float timeFrom = keyFrameFrom->index();
    float timeTo = keyFrameTo->index();
    float weightFrom = 0.0f, weightTo = 0.0f;
    if (m_overrideFirst &&(k1 == 0 || timeFrom <= m_lastIndex <= m_lastLoopStartIndex)) {
        if (nFrames > 1 && timeTo < m_lastLoopStartIndex + 60.0f) {
            timeFrom = m_lastLoopStartIndex;
            weightFrom = m_snapWeight;
            weightTo = keyFrameTo->weight();
        }
        else if (frameAt - timeFrom < m_noFaceSmearIndex) {
            timeFrom = m_lastLoopStartIndex;
            timeTo = m_lastLoopStartIndex + m_noFaceSmearIndex;
            currentFrame = frameAt;
            weightFrom = m_snapWeight;
            weightTo = keyFrameFrom->weight();
        }
        else if (nFrames > 1) {
            timeFrom = m_lastLoopStartIndex + m_noFaceSmearIndex;
            weightFrom = keyFrameFrom->weight();
            weightTo = keyFrameTo->weight();
        }
        else {
            weightFrom = keyFrameFrom->weight();
        }
    }
    else {
        weightFrom = keyFrameFrom->weight();
        weightTo = keyFrameTo->weight();
    }

    if (timeFrom != timeTo) {
        float w = (currentFrame - timeFrom) / (timeTo - timeFrom);
        m_weight = weightFrom * (1.0f - w) + weightTo * w;
    }
    else {
        m_weight = weightFrom;
    }
}

void FaceMotion::sort()
{
    m_frames.quickSort(FaceMotionKeyFramePredication());
}

}
