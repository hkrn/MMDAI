#ifndef VPVL_FACEMOTION_H_
#define VPVL_FACEMOTION_H_

#include "vpvl/BaseMotion.h"

namespace vpvl
{

class Face;
class FaceKeyFrame;
typedef btAlignedObjectArray<FaceKeyFrame *> FaceKeyFrameList;

class FaceMotion : BaseMotion
{
public:
    FaceMotion();
    ~FaceMotion();

    static const float kStartingMarginFrame;

    void read(const char *data, uint32_t size);

    const FaceKeyFrameList &frames() const {
        return m_frames;
    }

private:
    void calculate();
    void takeSnap(btVector3 &center);

    Face *m_face;
    FaceKeyFrameList m_frames;
    float m_weight;
    float m_snapWeight;
};

}

#endif

