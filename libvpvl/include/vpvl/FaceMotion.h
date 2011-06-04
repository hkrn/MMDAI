#ifndef VPVL_FACEMOTION_H_
#define VPVL_FACEMOTION_H_

#include "vpvl/BaseMotion.h"

namespace vpvl
{

class Face;
class FaceKeyFrame;
typedef btAlignedObjectArray<FaceKeyFrame *> FaceKeyFrameList;

class FaceMotion
{
public:
    FaceMotion();
    ~FaceMotion();

    static const float kStartingMarginFrame;

    void read(const char *data, uint32_t size);
    void calculate(float frameAt);
    void sort();

    const FaceKeyFrameList &frames() const {
        return m_frames;
    }
    float weight() const {
        return m_weight;
    }
    float snapWeight() const {
        return m_weight;
    }
    void setSnapWeight(float value) {
        m_weight = value;
    }

private:
    Face *m_face;
    FaceKeyFrameList m_frames;
    uint32_t m_lastIndex;
    uint32_t m_lastLoopStartIndex;
    uint32_t m_noFaceSmearIndex;
    float m_weight;
    float m_snapWeight;
    bool m_overrideFirst;
};

}

#endif
