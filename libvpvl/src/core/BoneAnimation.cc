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

struct InternalKeyFrameList {
    Bone *bone;
    BoneKeyFrameList keyFrames;
    Vector3 position;
    Quaternion rotation;
    int lastIndex;

    bool isNull() const {
        if (keyFrames.count() == 1) {
            const BoneKeyframe *keyFrame = keyFrames[0];
            return keyFrame->position() == internal::kZeroV &&
                    keyFrame->rotation() == internal::kZeroV;
        }
        return false;
    }
};

class BoneAnimationKeyFramePredication
{
public:
    bool operator()(const BoneKeyframe *left, const BoneKeyframe *right) {
        return left->frameIndex() < right->frameIndex();
    }
};

float BoneAnimation::weightValue(const BoneKeyframe *keyFrame, float w, int at)
{
    const uint16_t index = static_cast<int16_t>(w * BoneKeyframe::kTableSize);
    const float *v = keyFrame->interpolationTable()[at];
    return v[index] + (v[index + 1] - v[index]) * (w * BoneKeyframe::kTableSize - index);
}

void BoneAnimation::lerpVector3(const BoneKeyframe *keyFrame,
                                const Vector3 &from,
                                const Vector3 &to,
                                float w,
                                int at,
                                float &value)
{
    const float valueFrom = static_cast<const Scalar *>(from)[at];
    const float valueTo = static_cast<const Scalar *>(to)[at];
    if (keyFrame->linear()[at]) {
        value = internal::lerp(valueFrom, valueTo, w);
    }
    else {
        const float w2 = weightValue(keyFrame, w, at);
        value = internal::lerp(valueFrom, valueTo, w2);
    }
}

BoneAnimation::BoneAnimation()
    : BaseAnimation(),
      m_model(0),
      m_hasCenterBoneAnimation(false),
      m_enableNullFrame(false)
{
}

BoneAnimation::~BoneAnimation()
{
    m_name2keyframes.releaseAll();
    m_model = 0;
    m_hasCenterBoneAnimation = false;
}

void BoneAnimation::read(const uint8_t *data, int size)
{
    uint8_t *ptr = const_cast<uint8_t *>(data);
    m_frames.reserve(size);
    for (int i = 0; i < size; i++) {
        BoneKeyframe *frame = new BoneKeyframe();
        frame->read(ptr);
        ptr += frame->stride();
        m_frames.add(frame);
    }
}

void BoneAnimation::seek(float frameAt)
{
    const int nnodes = m_name2keyframes.count();
    for (int i = 0; i < nnodes; i++) {
        InternalKeyFrameList *keyFrames = *m_name2keyframes.value(i);
        if (m_enableNullFrame && keyFrames->isNull())
            continue;
        calculateFrames(frameAt, keyFrames);
        Bone *bone = keyFrames->bone;
        bone->setPosition(keyFrames->position);
        bone->setRotation(keyFrames->rotation);
    }
    m_previousFrame = m_currentFrame;
    m_currentFrame = frameAt;
}

void BoneAnimation::attachModel(PMDModel *model)
{
    if (!m_model) {
        buildInternalKeyFrameList(model);
        m_model = model;
    }
}

void BoneAnimation::refresh()
{
    if (m_model) {
        m_name2keyframes.releaseAll();
        buildInternalKeyFrameList(m_model);
    }
}

void BoneAnimation::buildInternalKeyFrameList(vpvl::PMDModel *model)
{
    const int nframes = m_frames.count();
    const uint8_t *centerBoneName = Bone::centerBoneName();
    const size_t len = strlen(reinterpret_cast<const char *>(centerBoneName));
    // Build internal node to find by name, not frame index
    for (int i = 0; i < nframes; i++) {
        BoneKeyframe *frame = static_cast<BoneKeyframe *>(m_frames.at(i));
        HashString name(reinterpret_cast<const char *>(frame->name()));
        InternalKeyFrameList **ptr = m_name2keyframes[name], *node;
        if (ptr) {
            node = *ptr;
            node->keyFrames.add(frame);
            if (internal::stringEquals(frame->name(), centerBoneName, len))
                m_hasCenterBoneAnimation = true;
        }
        else {
            Bone *bone = model->findBone(frame->name());
            if (bone) {
                node = new InternalKeyFrameList();
                node->keyFrames.add(frame);
                node->bone = bone;
                node->lastIndex = 0;
                node->position.setZero();
                node->rotation.setValue(0.0f, 0.0f, 0.0f, 1.0f);
                m_name2keyframes.insert(name, node);
            }
        }
    }
    // Sort frames from each internal nodes by frame index ascend
    const int nnodes = m_name2keyframes.count();
    for (int i = 0; i < nnodes; i++) {
        InternalKeyFrameList *node = *m_name2keyframes.value(i);
        BoneKeyFrameList &frames = node->keyFrames;
        frames.sort(BoneAnimationKeyFramePredication());
        btSetMax(m_maxFrame, frames[frames.count() - 1]->frameIndex());
    }
}

void BoneAnimation::calculateFrames(float frameAt, InternalKeyFrameList *keyFrames)
{
    BoneKeyFrameList &kframes = keyFrames->keyFrames;
    const int nframes = kframes.count();
    BoneKeyframe *lastKeyFrame = kframes[nframes - 1];
    float currentFrame = btMin(frameAt, lastKeyFrame->frameIndex());
    // Find the next frame index bigger than the frame index of last key frame
    int k1 = 0, k2 = 0, lastIndex = keyFrames->lastIndex;
    if (currentFrame >= kframes[lastIndex]->frameIndex()) {
        for (int i = lastIndex; i < nframes; i++) {
            if (currentFrame <= kframes[i]->frameIndex()) {
                k2 = i;
                break;
            }
        }
    }
    else {
        for (int i = 0; i <= lastIndex && i < nframes; i++) {
            if (currentFrame <= kframes[i]->frameIndex()) {
                k2 = i;
                break;
            }
        }
    }

    if (k2 >= nframes)
        k2 = nframes - 1;
    k1 = k2 <= 1 ? 0 : k2 - 1;
    keyFrames->lastIndex = k1;

    const BoneKeyframe *keyFrameFrom = kframes.at(k1), *keyFrameTo = kframes.at(k2);
    float frameIndexFrom = keyFrameFrom->frameIndex(), frameIndexTo = keyFrameTo->frameIndex();
    BoneKeyframe *keyFrameForInterpolation = const_cast<BoneKeyframe *>(keyFrameTo);
    const Vector3 &positionFrom = keyFrameFrom->position();
    const Quaternion &rotationFrom = keyFrameFrom->rotation();
    const Vector3 &positionTo = keyFrameTo->position();
    const Quaternion &rotationTo = keyFrameTo->rotation();

    if (frameIndexFrom != frameIndexTo) {
        if (currentFrame <= frameIndexFrom) {
            keyFrames->position = positionFrom;
            keyFrames->rotation = rotationFrom;
        }
        else if (currentFrame >= frameIndexTo) {
            keyFrames->position = positionTo;
            keyFrames->rotation = rotationTo;
        }
        else {
            const float w = (currentFrame - frameIndexFrom) / (frameIndexTo - frameIndexFrom);
            float x = 0, y = 0, z = 0;
            lerpVector3(keyFrameForInterpolation, positionFrom, positionTo, w, 0, x);
            lerpVector3(keyFrameForInterpolation, positionFrom, positionTo, w, 1, y);
            lerpVector3(keyFrameForInterpolation, positionFrom, positionTo, w, 2, z);
            keyFrames->position.setValue(x, y, z);
            if (keyFrameForInterpolation->linear()[3]) {
                keyFrames->rotation = rotationFrom.slerp(rotationTo, w);
            }
            else {
                const float w2 = weightValue(keyFrameForInterpolation, w, 3);
                keyFrames->rotation = rotationFrom.slerp(rotationTo, w2);
            }
        }
    }
    else {
        keyFrames->position = positionFrom;
        keyFrames->rotation = rotationFrom;
    }
}

void BoneAnimation::reset()
{
    BaseAnimation::reset();
    const int nnodes = m_name2keyframes.count();
    for (int i = 0; i < nnodes; i++) {
        InternalKeyFrameList *keyframes = *m_name2keyframes.value(i);
        keyframes->lastIndex = 0;
    }
}

}
