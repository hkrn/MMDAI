#ifndef VPVL_FACEKEYFRAME_H_
#define VPVL_FACEKEYFRAME_H_

#include "LinearMath/btAlignedObjectArray.h"
#include "vpvl/common.h"

namespace vpvl
{

class FaceKeyFrame
{
public:
    FaceKeyFrame();
    ~FaceKeyFrame();

    static size_t stride(const char *data);
    void read(const char *data);

    const char *name() const {
        return m_name;
    }
    uint32_t index() const {
        return m_index;
    }
    float weight() const {
        return m_weight;
    }

private:
    char m_name[15];
    uint32_t m_index;
    float m_weight;
};

typedef btAlignedObjectArray<FaceKeyFrame *> FaceKeyFrameList;

}

#endif
