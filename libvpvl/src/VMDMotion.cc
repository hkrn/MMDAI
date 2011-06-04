#include "vpvl/vpvl.h"
#include "vpvl/internal/BoneKeyFrame.h"
#include "vpvl/internal/CameraKeyFrame.h"
#include "vpvl/internal/FaceKeyFrame.h"
#include "vpvl/internal/VMDMotion.h"
#include "vpvl/internal/util.h"

namespace vpvl
{

VMDMotion::VMDMotion(const char *data, size_t size)
    : m_data(data),
      m_size(size)
{
    memset(&m_name, 0, sizeof(m_name));
    memset(&m_result, 0, sizeof(m_result));
}

VMDMotion::~VMDMotion()
{
    memset(&m_name, 0, sizeof(m_name));
    memset(&m_result, 0, sizeof(m_result));
    m_data = 0;
}

bool VMDMotion::preparse()
{
    size_t rest = m_size;
    /* header + name */
    if (50 > rest)
        return false;

    char *ptr = const_cast<char *>(m_data);
    m_result.basePtr = ptr;

    if (strcmp(ptr, "Vocaloid Motion Data 0002") != 0)
        return false;
    ptr += 30;
    m_result.namePtr = ptr;
    ptr += 20;
    rest -= 50;

    /* bone key frame */
    size_t nBoneKeyFrames, nFaceKeyFrames, nCameraKeyFrames;
    if (!size32(ptr, rest, nBoneKeyFrames))
        return false;
    m_result.boneKeyFramePtr = ptr;
    if (!validateSize(ptr, BoneKeyFrame::stride(ptr), nBoneKeyFrames, rest))
        return false;
    m_result.boneKeyFrameCount = nBoneKeyFrames;

    /* face key frame */
    if (!size32(ptr, rest, nFaceKeyFrames))
        return false;
    m_result.faceKeyFramePtr = ptr;
    if (!validateSize(ptr, FaceKeyFrame::stride(ptr), nFaceKeyFrames, rest))
        return false;
    m_result.faceKeyFrameCount = nFaceKeyFrames;

    /* camera key frame */
    if (!size32(ptr, rest, nCameraKeyFrames))
        return false;
    m_result.cameraKeyFramePtr = ptr;
    if (!validateSize(ptr, CameraKeyFrame::stride(ptr), nCameraKeyFrames, rest))
        return false;
    m_result.cameraKeyFrameCount = nCameraKeyFrames;

    return true;
}

bool VMDMotion::parse()
{
    if (preparse()) {
        parseHeader();
        parseBoneFrames();
        parseFaceFrames();
        parseCameraFrames();
        parseLightFrames();
        parseSelfShadowFrames();
        return true;
    }
    return false;
}

void VMDMotion::parseHeader()
{
    stringCopySafe(m_name, m_result.namePtr, sizeof(m_name));
}

void VMDMotion::parseBoneFrames()
{
    m_boneMotion.read(m_result.boneKeyFramePtr, m_result.boneKeyFrameCount);
    m_boneMotion.sort();
}

void VMDMotion::parseFaceFrames()
{
    m_faceMotion.read(m_result.faceKeyFramePtr, m_result.faceKeyFrameCount);
    m_faceMotion.sort();
}

void VMDMotion::parseCameraFrames()
{
    m_cameraMotion.read(m_result.cameraKeyFramePtr, m_result.cameraKeyFrameCount);
    m_cameraMotion.sort();
}

void VMDMotion::parseLightFrames()
{
}

void VMDMotion::parseSelfShadowFrames()
{
}

}
