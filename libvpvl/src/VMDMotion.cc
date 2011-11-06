/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn                                    */
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

namespace
{
const uint8_t *kSignature = reinterpret_cast<const uint8_t *>("Vocaloid Motion Data 0002");
}

namespace vpvl
{

const float VMDMotion::kDefaultLoopAtFrame = 0.0f;
const float VMDMotion::kDefaultPriority = 0.0f;

VMDMotion::VMDMotion()
    : m_model(0),
      m_status(kRunning),
      m_error(kNoError),
      m_onEnd(2),
      m_loopAt(kDefaultLoopAtFrame),
      m_priority(kDefaultPriority),
      m_endingBoneBlend(0.0f),
      m_endingFaceBlend(0.0f),
      m_endingBoneBlendFrames(20.0f),
      m_endingFaceBlendFrames(5.0f),
      m_motionBlendRate(1.0f),
      m_beginningNonControlledBlend(0.0f),
      m_active(false),
      m_enableSmooth(false),
      m_enableRelocation(false),
      m_ignoreStatic(false)
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
    if (memcmp(ptr, kSignature, sizeof(kSignature)) != 0) {
        m_error = kInvalidSignatureError;
        return false;
    }
    ptr += kSignatureSize;
    info.namePtr = ptr;
    ptr += kNameSize;
    rest -= kSignatureSize + kNameSize;

    // Bone key frame
    size_t nBoneKeyFrames, nFaceKeyFrames, nCameraKeyFrames;
    if (!internal::size32(ptr, rest, nBoneKeyFrames)) {
        m_error = kBoneKeyFramesSizeError;
        return false;
    }
    info.boneKeyFramePtr = ptr;
    if (!internal::validateSize(ptr, BoneKeyFrame::strideSize(), nBoneKeyFrames, rest)) {
        m_error = kBoneKeyFramesError;
        return false;
    }
    info.boneKeyFrameCount = nBoneKeyFrames;

    // Face key frame
    if (!internal::size32(ptr, rest, nFaceKeyFrames)) {
        m_error = kFaceKeyFramesSizeError;
        return false;
    }
    info.faceKeyFramePtr = ptr;
    if (!internal::validateSize(ptr, FaceKeyFrame::strideSize(), nFaceKeyFrames, rest)) {
        m_error = kFaceKeyFramesError;
        return false;
    }
    info.faceKeyFrameCount = nFaceKeyFrames;

    // Camera key frame
    if (!internal::size32(ptr, rest, nCameraKeyFrames)) {
        m_error = kCameraKeyFramesSizeError;
        return false;
    }
    info.cameraKeyFramePtr = ptr;
    if (!internal::validateSize(ptr, CameraKeyFrame::strideSize(), nCameraKeyFrames, rest)) {
        m_error = kCameraKeyFramesError;
        return false;
    }
    info.cameraKeyFrameCount = nCameraKeyFrames;

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
            + m_boneMotion.countKeyFrames() * BoneKeyFrame::strideSize()
            + m_faceMotion.countKeyFrames() * FaceKeyFrame::strideSize()
            + m_cameraMotion.countKeyFrames() * CameraKeyFrame::strideSize();
}

void VMDMotion::save(uint8_t *data) const
{
    internal::copyBytes(data, kSignature, kSignatureSize);
    data += kSignatureSize;
    internal::copyBytes(data, m_name, sizeof(m_name));
    data += kNameSize;
    int nBoneFrames = m_boneMotion.countKeyFrames();
    internal::copyBytes(data, reinterpret_cast<uint8_t *>(&nBoneFrames), sizeof(nBoneFrames));
    data += sizeof(nBoneFrames);
    for (int i = 0; i < nBoneFrames; i++) {
        BoneKeyFrame *frame = m_boneMotion.frameAt(i);
        frame->write(data);
        data += BoneKeyFrame::strideSize();
    }
    int nFaceFrames = m_faceMotion.countKeyFrames();
    internal::copyBytes(data, reinterpret_cast<uint8_t *>(&nFaceFrames), sizeof(nFaceFrames));
    data += sizeof(nFaceFrames);
    for (int i = 0; i < nFaceFrames; i++) {
        FaceKeyFrame *frame = m_faceMotion.frameAt(i);
        frame->write(data);
        data += FaceKeyFrame::strideSize();
    }
    int nCameraFrames = m_cameraMotion.countKeyFrames();
    internal::copyBytes(data, reinterpret_cast<uint8_t *>(&nCameraFrames), sizeof(nCameraFrames));
    data += sizeof(nCameraFrames);
    for (int i = 0; i < nCameraFrames; i++) {
        CameraKeyFrame *frame = m_cameraMotion.frameAt(i);
        frame->write(data);
        data += CameraKeyFrame::strideSize();
    }
    int empty = 0;
    internal::copyBytes(data, reinterpret_cast<uint8_t *>(&empty), sizeof(empty));
    data += sizeof(empty);
    internal::copyBytes(data, reinterpret_cast<uint8_t *>(&empty), sizeof(empty));
    data += sizeof(empty);
}

void VMDMotion::attachModel(PMDModel *model)
{
    if (m_model)
        return;
    m_model = model;
    m_active = true;
    m_endingBoneBlend = 0.0f;
    m_endingFaceBlend = 0.0f;
    m_boneMotion.attachModel(model);
    m_faceMotion.attachModel(model);
    if (m_enableSmooth) {
        // The model is relocated to the specified offset and save the current motion state.
        if (m_enableRelocation && m_boneMotion.hasCenterBoneAnimation()) {
            Bone *root = model->mutableRootBone();
            Bone *center = Bone::centerBone(&model->bones());
            Transform transform = root->localTransform().inverse();
            Vector3 position = transform * center->localTransform().getOrigin();
            Vector3 centerPosition = center->originPosition();
            Vector3 offset = position - centerPosition;
            offset.setY(0.0f);
            m_boneMotion.setOverrideFirst(offset);
            m_faceMotion.setOverrideFirst(offset);
            root->setOffset(root->offset() + offset);
            root->updateTransform();
        }
        // Save the current motion state for loop
        else {
            m_boneMotion.setOverrideFirst(internal::kZeroV);
            m_faceMotion.setOverrideFirst(internal::kZeroV);
        }
    }
}

void VMDMotion::seek(float frameIndex)
{
    m_boneMotion.setBlendRate(m_boneMotion.blendRate());
    m_faceMotion.setBlendRate(1.0f);
    m_boneMotion.seek(frameIndex);
    m_faceMotion.seek(frameIndex);
}

void VMDMotion::advance(float deltaFrame)
{
    if (m_beginningNonControlledBlend > 0.0f) {
        m_beginningNonControlledBlend -= deltaFrame;
        btSetMax(m_beginningNonControlledBlend, 0.0f);
        m_model->smearAllBonesToDefault(m_beginningNonControlledBlend / 10.0f);
        m_model->smearAllFacesToDefault(m_beginningNonControlledBlend / 10.0f);
    }
    if (deltaFrame == 0.0f) {
        m_boneMotion.advance(deltaFrame);
        m_faceMotion.advance(deltaFrame);
    }
    else if (m_active) {
        // Started gracefully finish
        if (m_endingBoneBlend != 0.0f || m_endingFaceBlend != 0.0f) {
            m_boneMotion.setBlendRate(m_motionBlendRate * m_endingBoneBlend / m_endingBoneBlendFrames);
            m_faceMotion.setBlendRate(m_endingFaceBlend / m_endingFaceBlendFrames);
            m_boneMotion.advance(deltaFrame);
            m_faceMotion.advance(deltaFrame);
            m_endingBoneBlend -= deltaFrame;
            m_endingFaceBlend -= deltaFrame;
            btSetMax(m_endingBoneBlend, 0.0f);
            btSetMax(m_endingFaceBlend, 0.0f);
            // The motion's blend rate is zero (finish), it should be marked as end
            if (m_endingBoneBlend == 0.0f || m_endingFaceBlend == 0.0f) {
                m_active = false;
                m_status = kStopped;
            }
        }
        else {
            // The motion is active and continue to advance
            m_boneMotion.setBlendRate(m_boneMotion.blendRate());
            m_faceMotion.setBlendRate(1.0f);
            m_boneMotion.advance(deltaFrame);
            m_faceMotion.advance(deltaFrame);
            if (m_boneMotion.currentIndex() >= m_boneMotion.maxIndex() &&
                    m_faceMotion.currentIndex() >= m_faceMotion.maxIndex()) {
                switch (m_onEnd) {
                case 0: // none
                    break;
                case 1: // loop
                    if (m_boneMotion.maxIndex() != 0.0f) {
                        m_boneMotion.rewind(m_loopAt, deltaFrame);
                        m_status = kLooped;
                    }
                    if (m_faceMotion.maxIndex() != 0.0f) {
                        m_faceMotion.rewind(m_loopAt, deltaFrame);
                        m_status = kLooped;
                    }
                    break;
                case 2: // gracefully finish
                    if (m_enableSmooth) {
                        m_endingBoneBlend = m_endingBoneBlendFrames;
                        m_endingFaceBlend = m_endingFaceBlendFrames;
                    }
                    else {
                        m_active = false;
                        m_status = kStopped;
                    }
                    break;
                }
            }
        }
    }
}

void VMDMotion::reset()
{
    m_boneMotion.seek(0.0f);
    m_faceMotion.seek(0.0f);
    m_boneMotion.reset();
    m_faceMotion.reset();
    m_active = true;
    m_status = kRunning;
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

bool VMDMotion::isFull() const
{
    return m_boneMotion.isIgnoreOneKeyFrame() && m_faceMotion.isIgnoreOneKeyFrame();
}

void VMDMotion::setFull(bool value)
{
    bool ignoreOneKeyFrame = value ? false : true;
    m_boneMotion.setIgnoreOneKeyFrame(ignoreOneKeyFrame);
    m_faceMotion.setIgnoreOneKeyFrame(ignoreOneKeyFrame);
    m_beginningNonControlledBlend = ignoreOneKeyFrame ? 0.0f : 10.0f;
}

void VMDMotion::parseHeader(const DataInfo &info)
{
    copyBytesSafe(m_name, info.namePtr, sizeof(m_name));
}

void VMDMotion::parseBoneFrames(const DataInfo &info)
{
    m_boneMotion.read(info.boneKeyFramePtr, info.boneKeyFrameCount);
}

void VMDMotion::parseFaceFrames(const DataInfo &info)
{
    m_faceMotion.read(info.faceKeyFramePtr, info.faceKeyFrameCount);
}

void VMDMotion::parseCameraFrames(const DataInfo &info)
{
    m_cameraMotion.read(info.cameraKeyFramePtr, info.cameraKeyFrameCount);
}

void VMDMotion::parseLightFrames(const DataInfo & /* info */)
{
}

void VMDMotion::parseSelfShadowFrames(const DataInfo & /* info */)
{
}

void VMDMotion::release()
{
    internal::zerofill(&m_name, sizeof(m_name));
    m_model = 0;
    m_status = kRunning;
    m_onEnd = 0;
    m_loopAt = kDefaultLoopAtFrame;
    m_priority = kDefaultPriority;
    m_endingBoneBlend = 0.0f;
    m_endingFaceBlend = 0.0f;
    m_endingBoneBlendFrames = 20.0f;
    m_endingFaceBlendFrames = 5.0f;
    m_motionBlendRate = 1.0f;
    m_beginningNonControlledBlend = 0.0f;
    m_active = false;
    m_enableSmooth = false;
    m_enableRelocation = false;
    m_ignoreStatic = false;
}

}
