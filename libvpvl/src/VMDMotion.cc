#include "vpvl/vpvl.h"
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
    char *ptr = const_cast<char *>(m_result.boneKeyFramePtr);
    int nBoneKeyFrames = m_result.boneKeyFrameCount;
    m_boneKeyFrames.reserve(nBoneKeyFrames);
    for (int i = 0; i < nBoneKeyFrames; i++) {
        BoneKeyFrame *frame = new BoneKeyFrame();
        frame->read(ptr);
        ptr += BoneKeyFrame::stride(ptr);
        m_boneKeyFrames.push_back(frame);
    }
}

void VMDMotion::parseFaceFrames()
{
    char *ptr = const_cast<char *>(m_result.faceKeyFramePtr);
    int nFaceKeyFrames = m_result.faceKeyFrameCount;
    m_faceKeyFrames.reserve(nFaceKeyFrames);
    for (int i = 0; i < nFaceKeyFrames; i++) {
        FaceKeyFrame *frame = new FaceKeyFrame();
        frame->read(ptr);
        ptr += FaceKeyFrame::stride(ptr);
        m_faceKeyFrames.push_back(frame);
    }
}

void VMDMotion::parseCameraFrames()
{
    char *ptr = const_cast<char *>(m_result.cameraKeyFramePtr);
    int nCameraKeyFrames = m_result.cameraKeyFrameCount;
    m_cameraKeyFrames.reserve(nCameraKeyFrames);
    for (int i = 0; i < nCameraKeyFrames; i++) {
        CameraKeyFrame *frame = new CameraKeyFrame();
        frame->read(ptr);
        ptr += CameraKeyFrame::stride(ptr);
        m_cameraKeyFrames.push_back(frame);
    }
}

void VMDMotion::parseLightFrames()
{
}

void VMDMotion::parseSelfShadowFrames()
{
}

}
