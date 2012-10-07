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

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/util.h"

#include "vpvl2/IBoneKeyframe.h"
#include "vpvl2/vmd/BoneAnimation.h"
#include "vpvl2/vmd/BoneKeyframe.h"

namespace
{

using namespace vpvl2;
using namespace vpvl2::vmd;

class BoneAnimationKeyframePredication
{
public:
    bool operator()(const IBoneKeyframe *left, const IBoneKeyframe *right) const {
        return left->timeIndex() < right->timeIndex();
    }
};

}

namespace vpvl2
{
namespace vmd
{

struct BoneAnimation::PrivateContext {
    IBone *bone;
    Array<BoneKeyframe *> keyframes;
    Vector3 position;
    Quaternion rotation;
    int lastIndex;

    bool isNull() const {
        if (keyframes.count() == 1) {
            const IBoneKeyframe *keyframe = keyframes[0];
            return keyframe->position() == kZeroV3 &&
                    keyframe->rotation() == Quaternion::getIdentity();
        }
        return false;
    }
};

IKeyframe::SmoothPrecision BoneAnimation::weightValue(const BoneKeyframe *keyframe,
                                                      const IKeyframe::SmoothPrecision &w,
                                                      int at)
{
    const uint16_t index = static_cast<int16_t>(w * BoneKeyframe::kTableSize);
    const IKeyframe::SmoothPrecision *v = keyframe->interpolationTable()[at];
    return v[index] + (v[index + 1] - v[index]) * (w * BoneKeyframe::kTableSize - index);
}

void BoneAnimation::lerpVector3(const BoneKeyframe *keyframe,
                                const Vector3 &from,
                                const Vector3 &to,
                                const IKeyframe::SmoothPrecision &w,
                                int at,
                                IKeyframe::SmoothPrecision &value)
{
    const IKeyframe::SmoothPrecision &valueFrom = from[at];
    const IKeyframe::SmoothPrecision &valueTo = to[at];
    if (keyframe->linear()[at]) {
        value = internal::lerp(valueFrom, valueTo, w);
    }
    else {
        const IKeyframe::SmoothPrecision &w2 = weightValue(keyframe, w, at);
        value = internal::lerp(valueFrom, valueTo, w2);
    }
}

BoneAnimation::BoneAnimation(IEncoding *encoding)
    : BaseAnimation(),
      m_encodingRef(encoding),
      m_modelRef(0),
      m_enableNullFrame(false)
{
}

BoneAnimation::~BoneAnimation()
{
    m_name2contexts.releaseAll();
    m_modelRef = 0;
}

void BoneAnimation::read(const uint8_t *data, int size)
{
    uint8_t *ptr = const_cast<uint8_t *>(data);
    m_keyframes.reserve(size);
    for (int i = 0; i < size; i++) {
        BoneKeyframe *frame = new BoneKeyframe(m_encodingRef);
        m_keyframes.add(frame);
        frame->read(ptr);
        ptr += frame->estimateSize();
    }
}

void BoneAnimation::seek(const IKeyframe::TimeIndex &frameAt)
{
    if (!m_modelRef)
        return;
    const int ncontexts = m_name2contexts.count();
    for (int i = 0; i < ncontexts; i++) {
        PrivateContext *keyframes = *m_name2contexts.value(i);
        if (m_enableNullFrame && keyframes->isNull())
            continue;
        calculateKeyframes(frameAt, keyframes);
        IBone *bone = keyframes->bone;
        bone->setLocalPosition(keyframes->position);
        bone->setRotation(keyframes->rotation);
    }
    m_previousTimeIndex = m_currentTimeIndex;
    m_currentTimeIndex = frameAt;
}

void BoneAnimation::setParentModel(IModel *model)
{
    createPrivateContexts(model);
    m_modelRef = model;
}

BoneKeyframe *BoneAnimation::frameAt(int i) const
{
    return internal::checkBound(i, 0, m_keyframes.count()) ? reinterpret_cast<BoneKeyframe *>(m_keyframes[i]) : 0;
}

BoneKeyframe *BoneAnimation::findKeyframe(const IKeyframe::TimeIndex &timeIndex, const IString *name) const
{
    if (!name)
        return 0;
    const HashString &key = name->toHashString();
    PrivateContext *const *ptr = m_name2contexts.find(key);
    if (ptr) {
        const PrivateContext *context = *ptr;
        const Array<BoneKeyframe *> &keyframes = context->keyframes;
        int index = findKeyframeIndex(timeIndex, keyframes);
        return index != -1 ? keyframes[index] : 0;
    }
    return 0;
}

void BoneAnimation::createPrivateContexts(IModel *model)
{
    if (!model)
        return;
    const int nkeyframes = m_keyframes.count();
    m_name2contexts.releaseAll();
    // Build internal node to find by name, not frame index
    for (int i = 0; i < nkeyframes; i++) {
        BoneKeyframe *keyframe = reinterpret_cast<BoneKeyframe *>(m_keyframes.at(i));
        const IString *name = keyframe->name();
        const HashString &key = name->toHashString();
        PrivateContext **ptr = m_name2contexts[key], *context;
        if (ptr) {
            context = *ptr;
            context->keyframes.add(keyframe);
        }
        else {
            IBone *bone = model->findBone(name);
            if (bone) {
                context = new PrivateContext();
                context->keyframes.add(keyframe);
                context->bone = bone;
                context->lastIndex = 0;
                context->position.setZero();
                context->rotation.setValue(0.0f, 0.0f, 0.0f, 1.0f);
                m_name2contexts.insert(key, context);
            }
        }
    }
    // Sort frames from each internal nodes by frame index ascend
    const int ncontexts = m_name2contexts.count();
    for (int i = 0; i < ncontexts; i++) {
        PrivateContext *context = *m_name2contexts.value(i);
        Array<BoneKeyframe *> &frames = context->keyframes;
        frames.sort(BoneAnimationKeyframePredication());
        btSetMax(m_maxTimeIndex, frames[frames.count() - 1]->timeIndex());
    }
}

void BoneAnimation::calculateKeyframes(const IKeyframe::TimeIndex &timeIndexAt, PrivateContext *context)
{
    Array<BoneKeyframe *> &keyframes = context->keyframes;
    const int nkeyframes = keyframes.count();
    IBoneKeyframe *lastKeyFrame = keyframes[nkeyframes - 1];
    const IKeyframe::TimeIndex &currentTimeIndex = btMin(timeIndexAt, lastKeyFrame->timeIndex());
    // Find the next frame index bigger than the frame index of last key frame
    int k1 = 0, k2 = 0, lastIndex = context->lastIndex;
    if (currentTimeIndex >= keyframes[lastIndex]->timeIndex()) {
        for (int i = lastIndex; i < nkeyframes; i++) {
            if (currentTimeIndex <= keyframes[i]->timeIndex()) {
                k2 = i;
                break;
            }
        }
    }
    else {
        for (int i = 0; i <= lastIndex && i < nkeyframes; i++) {
            if (currentTimeIndex <= keyframes[i]->timeIndex()) {
                k2 = i;
                break;
            }
        }
    }

    if (k2 >= nkeyframes)
        k2 = nkeyframes - 1;
    k1 = k2 <= 1 ? 0 : k2 - 1;
    context->lastIndex = k1;

    const BoneKeyframe *keyframeFrom = keyframes.at(k1),
            *keyframeTo = keyframes.at(k2);
    const IKeyframe::TimeIndex &timeIndexFrom = keyframeFrom->timeIndex(), timeIndexTo = keyframeTo->timeIndex();
    BoneKeyframe *keyframeForInterpolation = const_cast<BoneKeyframe *>(keyframeTo);
    const Vector3 &positionFrom = keyframeFrom->position();
    const Quaternion &rotationFrom = keyframeFrom->rotation();
    const Vector3 &positionTo = keyframeTo->position();
    const Quaternion &rotationTo = keyframeTo->rotation();

    if (timeIndexFrom != timeIndexTo) {
        if (currentTimeIndex <= timeIndexFrom) {
            context->position = positionFrom;
            context->rotation = rotationFrom;
        }
        else if (currentTimeIndex >= timeIndexTo) {
            context->position = positionTo;
            context->rotation = rotationTo;
        }
        else {
            const IKeyframe::SmoothPrecision &w = (currentTimeIndex - timeIndexFrom) / (timeIndexTo - timeIndexFrom);
            IKeyframe::SmoothPrecision x = 0, y = 0, z = 0;
            lerpVector3(keyframeForInterpolation, positionFrom, positionTo, w, 0, x);
            lerpVector3(keyframeForInterpolation, positionFrom, positionTo, w, 1, y);
            lerpVector3(keyframeForInterpolation, positionFrom, positionTo, w, 2, z);
            context->position.setValue(x, y, z);
            if (keyframeForInterpolation->linear()[3]) {
                context->rotation = rotationFrom.slerp(rotationTo, w);
            }
            else {
                const IKeyframe::SmoothPrecision &w2 = weightValue(keyframeForInterpolation, w, 3);
                context->rotation = rotationFrom.slerp(rotationTo, w2);
            }
        }
    }
    else {
        context->position = positionFrom;
        context->rotation = rotationFrom;
    }
}

void BoneAnimation::reset()
{
    BaseAnimation::reset();
    const int ncontexts = m_name2contexts.count();
    for (int i = 0; i < ncontexts; i++) {
        PrivateContext *context = *m_name2contexts.value(i);
        context->lastIndex = 0;
    }
}

}
}
