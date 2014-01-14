/**

 Copyright (c) 2009-2011  Nagoya Institute of Technology
                          Department of Computer Science
               2010-2014  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/MotionHelper.h"

#include "vpvl2/vmd/MorphAnimation.h"
#include "vpvl2/vmd/MorphKeyframe.h"

namespace vpvl2
{
namespace vmd
{

struct MorphAnimation::PrivateContext {
    IMorph *morph;
    Array<MorphKeyframe *> keyframes;
    IMorph::WeightPrecision weight;
    int lastIndex;

    bool isNull() const {
        if (keyframes.count() == 1) {
            const MorphKeyframe *keyframe = keyframes[0];
            return keyframe->weight() == 0.0f;
        }
        return false;
    }
};

MorphAnimation::MorphAnimation(IEncoding *encoding)
    : BaseAnimation(),
      m_encodingRef(encoding),
      m_modelRef(0),
      m_enableNullFrame(false)
{
}

MorphAnimation::~MorphAnimation()
{
    m_name2contexts.releaseAll();
    m_modelRef = 0;
}

void MorphAnimation::read(const uint8 *data, int size)
{
    uint8 *ptr = const_cast<uint8 *>(data);
    m_keyframes.reserve(size);
    for (int i = 0; i < size; i++) {
        MorphKeyframe *keyframe = m_keyframes.append(new MorphKeyframe(m_encodingRef));
        keyframe->read(ptr);
        ptr += keyframe->estimateSize();
    }
}

void MorphAnimation::seek(const IKeyframe::TimeIndex &timeIndexAt)
{
    if (!m_modelRef) {
        return;
    }
    const int ncontexts = m_name2contexts.count();
    for (int i = 0; i < ncontexts; i++) {
        PrivateContext *context = *m_name2contexts.value(i);
        if (m_enableNullFrame && context->isNull()) {
            continue;
        }
        calculateFrames(timeIndexAt, context);
        IMorph *morph = context->morph;
        morph->setWeight(context->weight);
    }
    m_previousTimeIndex = m_currentTimeIndex;
    m_currentTimeIndex = timeIndexAt;
}

void MorphAnimation::setParentModelRef(IModel *model)
{
    createPrivateContexts(model);
    m_modelRef = model;
}

void MorphAnimation::createPrivateContexts(IModel *model)
{
    if (!model) {
        return;
    }
    const int nkeyframes = m_keyframes.count();
    m_name2contexts.releaseAll();
    // Build internal node to find by name, not frame index
    for (int i = 0; i < nkeyframes; i++) {
        MorphKeyframe *keyframe = reinterpret_cast<MorphKeyframe *>(m_keyframes.at(i));
        const IString *name = keyframe->name();
        const HashString &key = name->toHashString();
        PrivateContext **ptr = m_name2contexts[key], *context;
        if (ptr) {
            context = *ptr;
            context->keyframes.append(keyframe);
        }
        else if (IMorph *morph = model->findMorphRef(name)) {
            PrivateContext *context = m_name2contexts.insert(key, new PrivateContext());
            context->keyframes.append(keyframe);
            context->morph = morph;
            context->lastIndex = 0;
            context->weight = 0.0f;
        }
    }
    // Sort frames from each internal nodes by frame index ascend
    const int ncontexts = m_name2contexts.count();
    for (int i = 0; i < ncontexts; i++) {
        PrivateContext *context = *m_name2contexts.value(i);
        Array<MorphKeyframe *> &keyframes = context->keyframes;
        keyframes.sort(internal::MotionHelper::KeyframeTimeIndexPredication());
        btSetMax(m_durationTimeIndex, keyframes[keyframes.count() - 1]->timeIndex());
    }
}

void MorphAnimation::reset()
{
    BaseAnimation::reset();
    const int ncontexts = m_name2contexts.count();
    for (int i = 0; i < ncontexts; i++) {
        PrivateContext *context = *m_name2contexts.value(i);
        context->lastIndex = 0;
    }
}

MorphKeyframe *MorphAnimation::findKeyframeAt(int i) const
{
    return internal::checkBound(i, 0, m_keyframes.count()) ? reinterpret_cast<MorphKeyframe *>(m_keyframes[i]) : 0;
}

MorphKeyframe *MorphAnimation::findKeyframe(const IKeyframe::TimeIndex &timeIndex, const IString *name) const
{
    if (!name) {
        return 0;
    }
    const HashString &key = name->toHashString();
    PrivateContext *const *ptr = m_name2contexts.find(key);
    if (ptr) {
        const PrivateContext *context = *ptr;
        const Array<MorphKeyframe *> &keyframes = context->keyframes;
        int index = findKeyframeIndex(timeIndex, keyframes);
        return index != -1 ? keyframes[index] : 0;
    }
    return 0;
}

void MorphAnimation::calculateFrames(const IKeyframe::TimeIndex &timeIndexAt, PrivateContext *context)
{
    const Array<MorphKeyframe *> &keyframes = context->keyframes;
    int fromIndex, toIndex;
    internal::MotionHelper::findKeyframeIndices(timeIndexAt, m_currentTimeIndex, context->lastIndex, fromIndex, toIndex, keyframes);
    const MorphKeyframe *keyframeFrom = keyframes.at(fromIndex), *keyframeTo = keyframes.at(toIndex);
    const IKeyframe::TimeIndex &timeIndexFrom = keyframeFrom->timeIndex(), timeIndexTo = keyframeTo->timeIndex();
    const IMorph::WeightPrecision &weightFrom = keyframeFrom->weight();
    const IMorph::WeightPrecision &weightTo = keyframeTo->weight();
    if (timeIndexFrom != timeIndexTo) {
        const IKeyframe::SmoothPrecision &w = (m_currentTimeIndex - timeIndexFrom) / (timeIndexTo - timeIndexFrom);
        context->weight = internal::MotionHelper::lerp(weightFrom, weightTo, w);
    }
    else {
        context->weight = weightFrom;
    }
    m_previousTimeIndex = m_currentTimeIndex;
    m_currentTimeIndex = timeIndexAt;
}

} /* namespace vmd */
} /* namespace vpvl2 */
