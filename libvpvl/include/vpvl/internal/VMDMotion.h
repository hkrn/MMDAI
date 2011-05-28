#ifndef VPVL_VMDMOTION_H_
#define VPVL_VMDMOTION_H_

#include "LinearMath/btHashMap.h"
#include "vpvl/BoneMotion.h"
#include "vpvl/CameraMotion.h"
#include "vpvl/FaceMotion.h"

namespace vpvl
{

struct VMDMotionDataInfo
{
    const char *basePtr;
    const char *namePtr;
    const char *boneKeyFramePtr;
    size_t boneKeyFrameCount;
    const char *faceKeyFramePtr;
    size_t faceKeyFrameCount;
    const char *cameraKeyFramePtr;
    size_t cameraKeyFrameCount;
    const char *lightKeyFramePtr;
    size_t lightKeyFrameCount;
    const char *selfShadowKeyFramePtr;
    size_t selfShadowKeyFrameCount;
};

class VMDMotion
{
public:
    VMDMotion(const char *data, size_t size);
    ~VMDMotion();

    bool preparse();
    bool parse();

    const char *name() const {
        return m_name;
    }
    const char *data() const {
        return m_data;
    }
    size_t size() const {
        return m_size;
    }
    const BoneMotion &bone() const {
        return m_boneMotion;
    }
    const CameraMotion &camera() const {
        return m_cameraMotion;
    }
    const FaceMotion &face() const {
        return m_faceMotion;
    }
    const VMDMotionDataInfo &result() const {
        return m_result;
    }

private:
    void parseHeader();
    void parseBoneFrames();
    void parseFaceFrames();
    void parseCameraFrames();
    void parseLightFrames();
    void parseSelfShadowFrames();

    char m_name[20];
    VMDMotionDataInfo m_result;
    BoneMotion m_boneMotion;
    CameraMotion m_cameraMotion;
    FaceMotion m_faceMotion;
    const char *m_data;
    const size_t m_size;
};

}

#endif
