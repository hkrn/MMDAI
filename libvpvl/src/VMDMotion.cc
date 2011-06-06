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
#include "vpvl/internal/BoneKeyFrame.h"
#include "vpvl/internal/CameraKeyFrame.h"
#include "vpvl/internal/FaceKeyFrame.h"
#include "vpvl/internal/PMDModel.h"
#include "vpvl/internal/VMDMotion.h"
#include "vpvl/internal/util.h"

namespace vpvl
{

const float VMDMotion::kDefaultLoopAtFrame = 0.0f;
const float VMDMotion::kDefaultPriority = 0.0f;

VMDMotion::VMDMotion(const uint8_t *data, size_t size)
    : m_model(0),
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
      m_ignoreStatic(false),
      m_data(data),
      m_size(size)
{
    memset(&m_name, 0, sizeof(m_name));
    memset(&m_result, 0, sizeof(m_result));
}

VMDMotion::~VMDMotion()
{
    memset(&m_name, 0, sizeof(m_name));
    memset(&m_result, 0, sizeof(m_result));
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
    m_data = 0;
}

bool VMDMotion::preparse()
{
    size_t rest = m_size;
    /* header + name */
    if (50 > rest)
        return false;

    uint8_t *ptr = const_cast<uint8_t *>(m_data);
    m_result.basePtr = ptr;

    static const uint8_t header[] = "Vocaloid Motion Data 0002";
    if (memcmp(ptr, header, sizeof(header)) != 0)
        return false;
    ptr += 30;
    m_result.namePtr = ptr;
    ptr += 20;
    rest -= 50;

    /* bone key frame */
    size_t nBoneKeyFrames, nFaceKeyFrames, nCameraKeyFrames;
    if (!internal::size32(ptr, rest, nBoneKeyFrames))
        return false;
    m_result.boneKeyFramePtr = ptr;
    if (!internal::validateSize(ptr, BoneKeyFrame::stride(ptr), nBoneKeyFrames, rest))
        return false;
    m_result.boneKeyFrameCount = nBoneKeyFrames;

    /* face key frame */
    if (!internal::size32(ptr, rest, nFaceKeyFrames))
        return false;
    m_result.faceKeyFramePtr = ptr;
    if (!internal::validateSize(ptr, FaceKeyFrame::stride(ptr), nFaceKeyFrames, rest))
        return false;
    m_result.faceKeyFrameCount = nFaceKeyFrames;

    /* camera key frame */
    if (!internal::size32(ptr, rest, nCameraKeyFrames))
        return false;
    m_result.cameraKeyFramePtr = ptr;
    if (!internal::validateSize(ptr, CameraKeyFrame::stride(ptr), nCameraKeyFrames, rest))
        return false;
    m_result.cameraKeyFrameCount = nCameraKeyFrames;

    return true;
}

bool VMDMotion::load()
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
        if (m_enableRelocation && m_boneMotion.hasCenterBoneMotion()) {
            Bone *root = model->mutableRootBone();
            Bone *center = Bone::centerBone(&model->bones());
            btTransform transform = root->currentTransform().inverse();
            btVector3 position = transform * center->currentTransform().getOrigin();
            btVector3 centerPosition = center->originPosition();
            btVector3 offset = position - centerPosition;
            offset.setY(0.0f);
            m_boneMotion.setOverrideFirst(offset);
            m_faceMotion.setOverrideFirst(offset);
            root->setOffset(root->offset() + offset);
            root->updateTransform();
        }
        else {
            m_boneMotion.setOverrideFirst(internal::kZeroV);
            m_faceMotion.setOverrideFirst(internal::kZeroV);
        }
    }
    if (!m_ignoreStatic)
        m_beginningNonControlledBlend = 10.0f;
}

void VMDMotion::update(float frameAt)
{
    if (m_beginningNonControlledBlend > 0.0f) {
        m_beginningNonControlledBlend -= frameAt;
        btSetMax(m_beginningNonControlledBlend, 0.0f);
    }
    if (m_active) {
        if (m_endingBoneBlend != 0.0f || m_endingFaceBlend != 0.0f) {
            bool reached = false;
            m_boneMotion.setBlendRate(m_motionBlendRate * m_endingBoneBlend / m_endingBoneBlendFrames);
            m_faceMotion.setBlendRate(m_endingFaceBlend / m_endingFaceBlendFrames);
            m_boneMotion.advance(frameAt, reached);
            m_faceMotion.advance(frameAt, reached);
            m_endingBoneBlend -= frameAt;
            m_endingFaceBlend -= frameAt;
            btSetMax(m_endingBoneBlend, 0.0f);
            btSetMax(m_endingFaceBlend, 0.0f);
            if (m_endingBoneBlend == 0.0f || m_endingFaceBlend == 0.0f) {
                m_active = false;
                m_status = kDeleted;
            }
        }
        else {
            bool boneReached = false;
            bool faceReached = false;
            m_boneMotion.setBlendRate(m_boneMotion.blendRate());
            m_faceMotion.setBlendRate(1.0f);
            m_boneMotion.advance(frameAt, boneReached);
            m_faceMotion.advance(frameAt, faceReached);
            if (boneReached && faceReached) {
                switch (m_onEnd) {
                case 0:
                    break;
                case 1:
                    if (m_boneMotion.maxIndex() != 0.0f && m_faceMotion.maxIndex() != 0.0f) {
                        m_boneMotion.rewind(m_loopAt, frameAt);
                        m_faceMotion.rewind(m_loopAt, frameAt);
                        m_status = kLooped;
                    }
                    break;
                case 2:
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

void VMDMotion::parseHeader()
{
    copyBytesSafe(m_name, m_result.namePtr, sizeof(m_name));
}

void VMDMotion::parseBoneFrames()
{
    m_boneMotion.read(m_result.boneKeyFramePtr, m_result.boneKeyFrameCount);
}

void VMDMotion::parseFaceFrames()
{
    m_faceMotion.read(m_result.faceKeyFramePtr, m_result.faceKeyFrameCount);
}

void VMDMotion::parseCameraFrames()
{
    m_cameraMotion.read(m_result.cameraKeyFramePtr, m_result.cameraKeyFrameCount);
}

void VMDMotion::parseLightFrames()
{
}

void VMDMotion::parseSelfShadowFrames()
{
}

}
