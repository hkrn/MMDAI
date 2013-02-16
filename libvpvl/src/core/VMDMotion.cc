/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2012  hkrn                                    */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#include "vpvl/vpvl.h"
#include "vpvl/internal/util.h"


namespace vpvl
{

const uint8_t *VMDMotion::kSignature = reinterpret_cast<const uint8_t *>("Vocaloid Motion Data 0002");

VMDMotion::VMDMotion()
    : m_model(0),
      m_error(kNoError),
      m_active(false)
{
    internal::zerofill(&m_name, sizeof(m_name));
}

VMDMotion::~VMDMotion()
{
    release();
}

bool VMDMotion::preparse(const uint8_t *data, size_t size, DataInfo &info)
{
    size_t rest = size;
    // Header(30) + Name(20)
    if (kSignatureSize + kNameSize > rest) {
        m_error = kInvalidHeaderError;
        return false;
    }

    uint8_t *ptr = const_cast<uint8_t *>(data);
    info.basePtr = ptr;

    // Check the signature is valid
    if (memcmp(ptr, kSignature, sizeof(kSignature) - 1) != 0) {
        m_error = kInvalidSignatureError;
        return false;
    }
    ptr += kSignatureSize;
    info.namePtr = ptr;
    ptr += kNameSize;
    rest -= kSignatureSize + kNameSize;

    // Bone key frame
    size_t nBoneKeyFrames, nFaceKeyFrames, nCameraKeyFrames, nLightKeyFrames;
    if (!internal::size32(ptr, rest, nBoneKeyFrames)) {
        m_error = kBoneKeyFramesSizeError;
        return false;
    }
    info.boneKeyframePtr = ptr;
    if (!internal::validateSize(ptr, BoneKeyframe::strideSize(), nBoneKeyFrames, rest)) {
        m_error = kBoneKeyFramesError;
        return false;
    }
    info.boneKeyframeCount = nBoneKeyFrames;

    // Face key frame
    if (!internal::size32(ptr, rest, nFaceKeyFrames)) {
        m_error = kFaceKeyFramesSizeError;
        return false;
    }
    info.faceKeyframePtr = ptr;
    if (!internal::validateSize(ptr, FaceKeyframe::strideSize(), nFaceKeyFrames, rest)) {
        m_error = kFaceKeyFramesError;
        return false;
    }
    info.faceKeyframeCount = nFaceKeyFrames;

    // Camera key frame
    if (!internal::size32(ptr, rest, nCameraKeyFrames)) {
        m_error = kCameraKeyFramesSizeError;
        return false;
    }
    info.cameraKeyframePtr = ptr;
    size_t cameraKeyframeStrideSize = CameraKeyframe::strideSize();
    if (!internal::validateSize(ptr, cameraKeyframeStrideSize, nCameraKeyFrames, rest)) {
        m_error = kCameraKeyFramesError;
        return false;
    }
    info.cameraKeyframeCount = nCameraKeyFrames;

    // workaround for no camera keyframe
    if (nCameraKeyFrames == 0 && rest > cameraKeyframeStrideSize)
        internal::validateSize(ptr, cameraKeyframeStrideSize, 1, rest);

    // Light key frame
    if (!internal::size32(ptr, rest, nLightKeyFrames)) {
        m_error = kLightKeyFramesSizeError;
        return false;
    }
    info.lightKeyframePtr = ptr;
    if (!internal::validateSize(ptr, LightKeyframe::strideSize(), nLightKeyFrames, rest)) {
        m_error = kCameraKeyFramesError;
        return false;
    }
    info.lightKeyframeCount = nLightKeyFrames;

    return true;
}

bool VMDMotion::load(const uint8_t *data, size_t size)
{
    DataInfo info;
    internal::zerofill(&info, sizeof(info));
    if (preparse(data, size, info)) {
        release();
        parseHeader(info);
        parseBoneFrames(info);
        parseFaceFrames(info);
        parseCameraFrames(info);
        parseLightFrames(info);
        parseSelfShadowFrames(info);
        return true;
    }
    return false;
}

size_t VMDMotion::estimateSize()
{
    /*
     * header[30]
     * name[20]
     * bone size
     * face size
     * camera size
     * light size (empty)
     * selfshadow size (empty)
     */
    return kSignatureSize + kNameSize + sizeof(int) * 5
            + m_boneMotion.countKeyframes() * BoneKeyframe::strideSize()
            + m_faceMotion.countKeyframes() * FaceKeyframe::strideSize()
            + m_cameraMotion.countKeyframes() * CameraKeyframe::strideSize()
            + m_lightMotion.countKeyframes() * LightKeyframe::strideSize();
}

void VMDMotion::save(uint8_t *data) const
{
    internal::copyBytes(data, kSignature, kSignatureSize);
    data += kSignatureSize;
    internal::copyBytes(data, m_name, sizeof(m_name));
    data += kNameSize;
    int nBoneFrames = m_boneMotion.countKeyframes();
    internal::copyBytes(data, reinterpret_cast<uint8_t *>(&nBoneFrames), sizeof(nBoneFrames));
    data += sizeof(nBoneFrames);
    for (int i = 0; i < nBoneFrames; i++) {
        BoneKeyframe *frame = m_boneMotion.frameAt(i);
        frame->write(data);
        data += BoneKeyframe::strideSize();
    }
    int nFaceFrames = m_faceMotion.countKeyframes();
    internal::copyBytes(data, reinterpret_cast<uint8_t *>(&nFaceFrames), sizeof(nFaceFrames));
    data += sizeof(nFaceFrames);
    for (int i = 0; i < nFaceFrames; i++) {
        FaceKeyframe *frame = m_faceMotion.frameAt(i);
        frame->write(data);
        data += FaceKeyframe::strideSize();
    }
    int nCameraFrames = m_cameraMotion.countKeyframes();
    internal::copyBytes(data, reinterpret_cast<uint8_t *>(&nCameraFrames), sizeof(nCameraFrames));
    data += sizeof(nCameraFrames);
    for (int i = 0; i < nCameraFrames; i++) {
        CameraKeyframe *frame = m_cameraMotion.frameAt(i);
        frame->write(data);
        data += CameraKeyframe::strideSize();
    }
    int nLightFrames = m_lightMotion.countKeyframes();
    internal::copyBytes(data, reinterpret_cast<uint8_t *>(&nLightFrames), sizeof(nLightFrames));
    data += sizeof(nLightFrames);
    for (int i = 0; i < nLightFrames; i++) {
        LightKeyframe *frame = m_lightMotion.frameAt(i);
        frame->write(data);
        data += LightKeyframe::strideSize();
    }
    int empty = 0;
    internal::copyBytes(data, reinterpret_cast<uint8_t *>(&empty), sizeof(empty));
    data += sizeof(empty);
}

void VMDMotion::attachModel(PMDModel *model)
{
    if (!model || m_model)
        return;
    m_model = model;
    m_active = true;
    m_boneMotion.attachModel(model);
    m_faceMotion.attachModel(model);
}

void VMDMotion::detachModel(PMDModel *model)
{
    if (!model || !m_model)
        return;
    m_model = 0;
    m_active = false;
}

void VMDMotion::seek(float frameIndex)
{
    m_boneMotion.seek(frameIndex);
    m_faceMotion.seek(frameIndex);
    m_active = maxFrameIndex() > frameIndex;
}

void VMDMotion::advance(float deltaFrame)
{
    if (deltaFrame == 0.0f) {
        m_boneMotion.advance(deltaFrame);
        m_faceMotion.advance(deltaFrame);
    }
    else if (m_active) {
        // The motion is active and continue to advance
        m_boneMotion.advance(deltaFrame);
        m_faceMotion.advance(deltaFrame);
        if (m_boneMotion.currentIndex() >= m_boneMotion.maxIndex() &&
                m_faceMotion.currentIndex() >= m_faceMotion.maxIndex()) {
            m_active = false;
        }
    }
}

void VMDMotion::reload()
{
    m_boneMotion.refresh();
    m_faceMotion.refresh();
    reset();
}

void VMDMotion::reset()
{
    m_boneMotion.seek(0.0f);
    m_faceMotion.seek(0.0f);
    m_boneMotion.reset();
    m_faceMotion.reset();
    m_active = true;
}

float VMDMotion::maxFrameIndex() const
{
    return btMax(m_boneMotion.maxIndex(), m_faceMotion.maxIndex());
}

bool VMDMotion::isReachedTo(float atEnd) const
{
    // force inactive motion is reached
    return !m_active || (m_boneMotion.currentIndex() >= atEnd && m_faceMotion.currentIndex() >= atEnd);
}

bool VMDMotion::isNullFrameEnabled() const
{
    return m_boneMotion.isNullFrameEnabled() && m_faceMotion.isNullFrameEnabled();
}

void VMDMotion::setNullFrameEnable(bool value)
{
    m_boneMotion.setNullFrameEnable(value);
    m_faceMotion.setNullFrameEnable(value);
}

void VMDMotion::parseHeader(const DataInfo &info)
{
    copyBytesSafe(m_name, info.namePtr, sizeof(m_name));
}

void VMDMotion::parseBoneFrames(const DataInfo &info)
{
    m_boneMotion.read(info.boneKeyframePtr, info.boneKeyframeCount);
}

void VMDMotion::parseFaceFrames(const DataInfo &info)
{
    m_faceMotion.read(info.faceKeyframePtr, info.faceKeyframeCount);
}

void VMDMotion::parseCameraFrames(const DataInfo &info)
{
    m_cameraMotion.read(info.cameraKeyframePtr, info.cameraKeyframeCount);
}

void VMDMotion::parseLightFrames(const DataInfo &info)
{
    m_lightMotion.read(info.lightKeyframePtr, info.lightKeyframeCount);
}

void VMDMotion::parseSelfShadowFrames(const DataInfo & /* info */)
{
}

void VMDMotion::release()
{
    internal::zerofill(&m_name, sizeof(m_name));
    m_model = 0;
    m_active = false;
}

}
