#ifndef VPVL_VMDMOTION_H_
#define VPVL_VMDMOTION_H_

#include "LinearMath/btHashMap.h"
#include "vpvl/BoneKeyFrame.h"
#include "vpvl/CameraKeyFrame.h"
#include "vpvl/FaceKeyFrame.h"

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
    const BoneKeyFrameList &boneKeyFrames() const {
        return m_boneKeyFrames;
    }
    const CameraKeyFrameList &cameraKeyFrames() const {
        return m_cameraKeyFrames;
    }
    const FaceKeyFrameList &faceKeyFrames() const {
        return m_faceKeyFrames;
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
    BoneKeyFrameList m_boneKeyFrames;
    CameraKeyFrameList m_cameraKeyFrames;
    FaceKeyFrameList m_faceKeyFrames;
    btHashMap<const char *, BoneKeyFrame *> m_name2bone;
    btHashMap<const char *, FaceKeyFrame *> m_name2face;
    const char *m_data;
    const size_t m_size;
};

}

#endif
