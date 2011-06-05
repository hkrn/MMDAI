/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2010  Nagoya Institute of Technology          */
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
#include "vpvl/internal/PMDModel.h"

namespace vpvl
{

const float BoneMotion::kStartingMarginFrame = 20.0f;

struct BoneMotionInternal {
    Bone *bone;
    BoneKeyFrameList keyFrames;
    btVector3 position;
    btVector3 snapPosition;
    btQuaternion rotation;
    btQuaternion snapRotation;
    uint32_t lastIndex;
};

class BoneMotionKeyFramePredication
{
public:
    bool operator()(const BoneKeyFrame *left, const BoneKeyFrame *right) {
        return left->index() < right->index();
    }
};

void BoneMotion::lerpPosition(const BoneKeyFrame *keyFrame,
                              const btVector3 &from,
                              const btVector3 &to,
                              float w,
                              uint32_t at,
                              float &value)
{
    const float valueFrom = static_cast<const btScalar *>(from)[at];
    const float valueTo = static_cast<const btScalar *>(to)[at];
    if (keyFrame->linear()[at]) {
        value = valueFrom * (1.0f - w) + valueTo * w;
    }
    else {
        const uint16_t index = static_cast<int16_t>(w * BoneKeyFrame::kTableSize);
        const float *v = keyFrame->interpolationTable()[at];
        const float w2 = v[index] + (v[index + 1] - v[index]) * (w * BoneKeyFrame::kTableSize - index);
        value = valueFrom * (1.0f - w2) + valueTo * w2;
    }
}

BoneMotion::BoneMotion()
    : BaseMotion(kStartingMarginFrame),
      m_bone(0),
      m_hasCenterBoneMotion(false)
{
}

BoneMotion::~BoneMotion()
{
}

void BoneMotion::read(const uint8_t *data, uint32_t size)
{
    uint8_t *ptr = const_cast<uint8_t *>(data);
    m_frames.reserve(size);
    for (uint32_t i = 0; i < size; i++) {
        BoneKeyFrame *frame = new BoneKeyFrame();
        frame->read(ptr);
        ptr += BoneKeyFrame::stride(ptr);
        m_frames.push_back(frame);
    }
}

void BoneMotion::seek(float frameAt)
{
    uint32_t nNodes = m_name2node.size();
    for (uint32_t i = 0; i < nNodes; i++) {
        BoneMotionInternal *node = *m_name2node.getAtIndex(i);
        if (m_ignoreSingleMotion && node->keyFrames.size() <= 1)
            continue;
        calculateFrames(frameAt, node);
        Bone *bone = node->bone;
        if (m_blendRate == 1.0f) {
            bone->setCurrentPosition(node->position);
            bone->setCurrentRotation(node->rotation);
        }
        else {
            bone->setCurrentPosition(bone->currentPosition().lerp(node->position, m_blendRate));
            bone->setCurrentRotation(bone->currentRotation().slerp(node->rotation, m_blendRate));
        }
    }
}

void BoneMotion::takeSnap(const btVector3 &center)
{
    uint32_t nNodes = m_name2node.size();
    for (uint32_t i = 0; i < nNodes; i++) {
        BoneMotionInternal *node = *m_name2node.getAtIndex(i);
        Bone *bone = node->bone;
        node->snapPosition = bone->currentPosition();
        if (bone->hasMotionIndependency())
            node->snapPosition -= center;
        node->snapRotation = bone->currentRotation();
    }
}

void BoneMotion::build(PMDModel *model)
{
    uint32_t nFrames = m_frames.size();
    const uint8_t *centerBoneName = Bone::centerBoneName();
    size_t len = strlen(reinterpret_cast<const char *>(centerBoneName));
    for (uint32_t i = 0; i < nFrames; i++) {
        BoneKeyFrame *frame = m_frames.at(i);
        btHashString name(reinterpret_cast<const char *>(frame->name()));
        BoneMotionInternal **ptr = m_name2node.find(name), *node;
        if (ptr) {
            node = *ptr;
            node->keyFrames.push_back(frame);
            if (internal::stringEquals(frame->name(), centerBoneName, len))
                m_hasCenterBoneMotion = true;
        }
        else {
            Bone *bone = model->findBone(frame->name());
            if (bone) {
                node = new BoneMotionInternal();
                node->keyFrames.push_back(frame);
                node->bone = bone;
                node->lastIndex = 0;
                node->position.setZero();
                node->rotation.setValue(0.0f, 0.0f, 0.0f, 1.0f);
                node->snapPosition.setZero();
                node->snapRotation.setValue(0.0f, 0.0f, 0.0f, 1.0f);
                m_name2node.insert(name, node);
            }
        }
    }

    uint32_t nNodes = m_name2node.size();
    for (uint32_t i = 0; i < nNodes; i++) {
        BoneMotionInternal *node = *m_name2node.getAtIndex(i);
        BoneKeyFrameList &frames = node->keyFrames;
        frames.quickSort(BoneMotionKeyFramePredication());
        btSetMax(m_maxFrame, frames[frames.size() - 1]->index());
    }
}

void BoneMotion::calculateFrames(float frameAt, BoneMotionInternal *node)
{
    BoneKeyFrameList &kframes = node->keyFrames;
    uint32_t nFrames = kframes.size();
    BoneKeyFrame *lastKeyFrame = kframes[nFrames - 1];
    float currentFrame = frameAt;
    if (currentFrame > lastKeyFrame->index())
        currentFrame = lastKeyFrame->index();

    uint32_t k1 = 0, k2 = 0, lastIndex = node->lastIndex;
    if (currentFrame >= kframes[lastIndex]->index()) {
        for (uint32_t i = lastIndex; i < nFrames; i++) {
            if (currentFrame <= kframes[i]->index()) {
                k2 = i;
                break;
            }
        }
    }
    else {
        for (uint32_t i = 0; i <= lastIndex && i < nFrames; i++) {
            if (currentFrame <= kframes[i]->index()) {
                k2 = i;
                break;
            }
        }
    }

    if (k2 >= nFrames)
        k2 = nFrames - 1;
    k1 = k2 <= 1 ? 0 : k2 - 1;
    node->lastIndex = k1;

    const BoneKeyFrame *keyFrameFrom = kframes.at(k1), *keyFrameTo = kframes.at(k2);
    float timeFrom = keyFrameFrom->index();
    float timeTo = keyFrameTo->index();
    BoneKeyFrame *keyFrameForInterpolation = const_cast<BoneKeyFrame *>(keyFrameTo);
    btVector3 positionFrom(0.0f, 0.0f, 0.0f), positionTo(0.0f, 0.0f, 0.0f);
    btQuaternion rotationFrom(0.0f, 0.0f, 0.0f, 1.0f), rotationTo(0.0f, 0.0f, 0.0f, 1.0f);
    if (m_overrideFirst && (k1 == 0 || timeFrom <= m_lastLoopStartIndex)) {
        if (nFrames > 1 && timeTo < m_lastLoopStartIndex + 60.0f) {
            timeFrom = m_lastLoopStartIndex;
            positionFrom = node->snapPosition;
            rotationFrom = node->snapRotation;
            positionTo = keyFrameTo->position();
            rotationTo = keyFrameTo->rotation();
        }
        else if (frameAt - timeFrom < m_smearIndex) {
            timeFrom = m_lastLoopStartIndex;
            timeTo = m_lastLoopStartIndex + m_smearIndex;
            currentFrame = frameAt;
            positionFrom = node->snapPosition;
            rotationFrom = node->snapRotation;
            positionTo = keyFrameFrom->position();
            rotationTo = keyFrameFrom->rotation();
            keyFrameForInterpolation = const_cast<BoneKeyFrame *>(keyFrameFrom);
        }
        else if (nFrames > 1) {
            timeFrom = m_lastLoopStartIndex + m_smearIndex;
            currentFrame = frameAt;
            positionFrom = keyFrameFrom->position();
            rotationFrom = keyFrameFrom->rotation();
            positionTo = keyFrameTo->position();
            rotationTo = keyFrameTo->rotation();
        }
        else {
            positionFrom = keyFrameFrom->position();
            rotationFrom = keyFrameFrom->rotation();
        }
    }
    else {
        positionFrom = keyFrameFrom->position();
        rotationFrom = keyFrameFrom->rotation();
        positionTo = keyFrameTo->position();
        rotationTo = keyFrameTo->rotation();
    }

    if (timeFrom != timeTo) {
        if (currentFrame <= timeFrom) {
            node->position = positionFrom;
            node->rotation = rotationFrom;
        }
        else if (currentFrame >= timeTo) {
            node->position = positionTo;
            node->rotation = rotationTo;
        }
        else {
            float w = (currentFrame - timeFrom) / (timeTo - timeFrom);
            float x = 0, y = 0, z = 0;
            lerpPosition(keyFrameForInterpolation, positionFrom, positionTo, w, 0, x);
            lerpPosition(keyFrameForInterpolation, positionFrom, positionTo, w, 1, y);
            lerpPosition(keyFrameForInterpolation, positionFrom, positionTo, w, 2, z);
            node->position.setValue(x, y, z);
            if (keyFrameForInterpolation->linear()[3]) {
                node->rotation = rotationFrom.slerp(rotationTo, w);
            }
            else {
                const float *v = keyFrameForInterpolation->interpolationTable()[3];
                const int16_t index = static_cast<int16_t>(w * BoneKeyFrame::kTableSize);
                const float w2 = v[index] + (v[index + 1] - v[index]) * (w * BoneKeyFrame::kTableSize - index);
                node->rotation = rotationFrom.slerp(rotationTo, w2);
            }
        }
    }
    else {
        node->position = positionFrom;
        node->rotation = rotationFrom;
    }
}

void BoneMotion::reset()
{
    BaseMotion::reset();
    uint32_t nNodes = m_name2node.size();
    for (uint32_t i = 0; i < nNodes; i++) {
        BoneMotionInternal *node = *m_name2node.getAtIndex(i);
        node->lastIndex = 0;
    }
}

}
