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

namespace vpvl
{

const float VMDMotion::kDefaultLoopAtFrame = 0.0f;
const float VMDMotion::kDefaultPriority = 0.0f;

VMDMotion::VMDMotion()
    : m_model(0),
      m_error(kNoError),
      m_status(kRunning),
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
      m_enableSmooth(true),
      m_enableRelocation(true),
      m_ignoreStatic(false)
{
    internal::zerofill(&m_name, sizeof(m_name));
}

VMDMotion::~VMDMotion()
{
    release();
}

bool VMDMotion::preparse(const uint8_t *data, size_t size, VMDMotionDataInfo &info)
{
    size_t rest = size;
    // Header(30) + Name(20)
    if (50 > rest) {
        m_error = kInvalidHeaderError;
        return false;
    }

    uint8_t *ptr = const_cast<uint8_t *>(data);
    info.basePtr = ptr;

    // Check the signature is valid
    static const uint8_t header[] = "Vocaloid Motion Data 0002";
    if (memcmp(ptr, header, sizeof(header)) != 0) {
        m_error = kInvalidSignatureError;
        return false;
    }
    ptr += 30;
    info.namePtr = ptr;
    ptr += 20;
    rest -= 50;

    // Bone key frame
    size_t nBoneKeyFrames, nFaceKeyFrames, nCameraKeyFrames;
    if (!internal::size32(ptr, rest, nBoneKeyFrames)) {
        m_error = kBoneKeyFramesSizeError;
        return false;
    }
    info.boneKeyFramePtr = ptr;
    if (!internal::validateSize(ptr, BoneKeyFrame::stride(), nBoneKeyFrames, rest)) {
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
    if (!internal::validateSize(ptr, FaceKeyFrame::stride(), nFaceKeyFrames, rest)) {
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
    if (!internal::validateSize(ptr, CameraKeyFrame::stride(), nCameraKeyFrames, rest)) {
        m_error = kCameraKeyFramesError;
        return false;
    }
    info.cameraKeyFrameCount = nCameraKeyFrames;

    return true;
}

bool VMDMotion::load(const uint8_t *data, size_t size)
{
    VMDMotionDataInfo info;
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
    return 70 + m_boneMotion.frames().size() * BoneKeyFrame::stride()
            + m_faceMotion.frames().size() * FaceKeyFrame::stride()
            + m_cameraMotion.frames().size() * CameraKeyFrame::stride();
}

void VMDMotion::save(uint8_t *data)
{
    internal::copyBytes(data, reinterpret_cast<const uint8_t *>("Vocaloid Motion Data 0002"), 30);
    data += 30;
    internal::copyBytes(data, m_name, sizeof(m_name));
    data += sizeof(m_name);
    BoneKeyFrameList boneFrames = m_boneMotion.frames();
    uint32_t nBoneFrames = boneFrames.size();
    internal::copyBytes(data, reinterpret_cast<uint8_t *>(&nBoneFrames), sizeof(nBoneFrames));
    data += sizeof(nBoneFrames);
    for (uint32_t i = 0; i < nBoneFrames; i++) {
        BoneKeyFrame *frame = boneFrames[i];
        frame->write(data);
        data += BoneKeyFrame::stride();
    }
    FaceKeyFrameList faceFrames = m_faceMotion.frames();
    uint32_t nFaceFrames = faceFrames.size();
    internal::copyBytes(data, reinterpret_cast<uint8_t *>(&nFaceFrames), sizeof(nFaceFrames));
    data += sizeof(nFaceFrames);
    for (uint32_t i = 0; i < nFaceFrames; i++) {
        FaceKeyFrame *frame = faceFrames[i];
        frame->write(data);
        data += FaceKeyFrame::stride();
    }
    CameraKeyFrameList cameraFrames = m_cameraMotion.frames();
    uint32_t nCameraFrames = cameraFrames.size();
    internal::copyBytes(data, reinterpret_cast<uint8_t *>(&nCameraFrames), sizeof(nCameraFrames));
    data += sizeof(nCameraFrames);
    for (uint32_t i = 0; i < nCameraFrames; i++) {
        CameraKeyFrame *frame = cameraFrames[i];
        frame->write(data);
        data += CameraKeyFrame::stride();
    }
    uint32_t empty = 0;
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
        if (m_enableRelocation && m_boneMotion.hasCenterBoneMotion()) {
            Bone *root = model->mutableRootBone();
            Bone *center = Bone::centerBone(&model->bones());
            btTransform transform = root->transform().inverse();
            btVector3 position = transform * center->transform().getOrigin();
            btVector3 centerPosition = center->originPosition();
            btVector3 offset = position - centerPosition;
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
    if (!m_ignoreStatic)
        m_beginningNonControlledBlend = 10.0f;
}

void VMDMotion::seek(float frameIndex)
{
    m_boneMotion.setBlendRate(m_boneMotion.blendRate());
    m_faceMotion.setBlendRate(1.0f);
    m_boneMotion.seek(frameIndex);
    m_faceMotion.seek(frameIndex);
}

void VMDMotion::update(float deltaFrame)
{
    if (m_beginningNonControlledBlend > 0.0f) {
        m_beginningNonControlledBlend -= deltaFrame;
        btSetMax(m_beginningNonControlledBlend, 0.0f);
    }
    if (m_active) {
        // Started gracefully finish
        if (m_endingBoneBlend != 0.0f || m_endingFaceBlend != 0.0f) {
            bool reached = false;
            m_boneMotion.setBlendRate(m_motionBlendRate * m_endingBoneBlend / m_endingBoneBlendFrames);
            m_faceMotion.setBlendRate(m_endingFaceBlend / m_endingFaceBlendFrames);
            m_boneMotion.advance(deltaFrame, reached);
            m_faceMotion.advance(deltaFrame, reached);
            m_endingBoneBlend -= deltaFrame;
            m_endingFaceBlend -= deltaFrame;
            btSetMax(m_endingBoneBlend, 0.0f);
            btSetMax(m_endingFaceBlend, 0.0f);
            // The motion's blend rate is zero (finish), it should be marked as end
            if (m_endingBoneBlend == 0.0f || m_endingFaceBlend == 0.0f) {
                m_active = false;
                m_status = kDeleted;
            }
        }
        else {
            // The motion is active and continue to advance
            bool boneReached = false;
            bool faceReached = false;
            m_boneMotion.setBlendRate(m_boneMotion.blendRate());
            m_faceMotion.setBlendRate(1.0f);
            m_boneMotion.advance(deltaFrame, boneReached);
            m_faceMotion.advance(deltaFrame, faceReached);
            if (boneReached && faceReached) {
                switch (m_onEnd) {
                case 0: // none
                    break;
                case 1: // loop
                    if (m_boneMotion.maxIndex() != 0.0f && m_faceMotion.maxIndex() != 0.0f) {
                        m_boneMotion.rewind(m_loopAt, deltaFrame);
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
                        m_status = kDeleted;
                    }
                    break;
                }
            }
        }
    }
}

void VMDMotion::parseHeader(const VMDMotionDataInfo &info)
{
    copyBytesSafe(m_name, info.namePtr, sizeof(m_name));
}

void VMDMotion::parseBoneFrames(const VMDMotionDataInfo &info)
{
    m_boneMotion.read(info.boneKeyFramePtr, info.boneKeyFrameCount);
}

void VMDMotion::parseFaceFrames(const VMDMotionDataInfo &info)
{
    m_faceMotion.read(info.faceKeyFramePtr, info.faceKeyFrameCount);
}

void VMDMotion::parseCameraFrames(const VMDMotionDataInfo &info)
{
    m_cameraMotion.read(info.cameraKeyFramePtr, info.cameraKeyFrameCount);
}

void VMDMotion::parseLightFrames(const VMDMotionDataInfo & /* info */)
{
}

void VMDMotion::parseSelfShadowFrames(const VMDMotionDataInfo & /* info */)
{
}

void VMDMotion::release()
{
    internal::zerofill(&m_name, sizeof(m_name));
    m_model = 0;
    m_status = kRunning;
    m_onEnd = 2;
    m_loopAt = kDefaultLoopAtFrame;
    m_priority = kDefaultPriority;
    m_endingBoneBlend = 0.0f;
    m_endingFaceBlend = 0.0f;
    m_endingBoneBlendFrames = 20.0f;
    m_endingFaceBlendFrames = 5.0f;
    m_motionBlendRate = 1.0f;
    m_beginningNonControlledBlend = 0.0f;
    m_active = false;
    m_enableSmooth = true;
    m_enableRelocation = true;
    m_ignoreStatic = false;
}

}
