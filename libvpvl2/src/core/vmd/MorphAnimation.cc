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

namespace vpvl2
{
namespace vmd
{

struct InternalMorphKeyFrameList {
    IMorph *morph;
    Array<MorphKeyframe *> keyframes;
    float weight;
    int lastIndex;

    bool isNull() const {
        if (keyframes.count() == 1) {
            const MorphKeyframe *keyFrame = keyframes[0];
            return keyFrame->weight() == 0.0f;
        }
        return false;
    }
};

class MorphAnimationKeyFramePredication
{
public:
    bool operator()(const BaseKeyframe *left, const BaseKeyframe *right) {
        return left->frameIndex() < right->frameIndex();
    }
};

MorphAnimation::MorphAnimation(IEncoding *encoding)
    : BaseAnimation(),
      m_encoding(encoding),
      m_model(0),
      m_enableNullFrame(false)
{
}

MorphAnimation::~MorphAnimation()
{
    m_name2keyframes.releaseAll();
    m_model = 0;
}

void MorphAnimation::read(const uint8_t *data, int size)
{
    uint8_t *ptr = const_cast<uint8_t *>(data);
    m_frames.reserve(size);
    for (int i = 0; i < size; i++) {
        MorphKeyframe *frame = new MorphKeyframe(m_encoding);
        frame->read(ptr);
        ptr += frame->stride();
        m_frames.add(frame);
    }
}

void MorphAnimation::seek(float frameAt)
{
    const int nnodes = m_name2keyframes.count();
    m_model->resetVertices();
    for (int i = 0; i < nnodes; i++) {
        InternalMorphKeyFrameList *frames = *m_name2keyframes.value(i);
        if (m_enableNullFrame && frames->isNull())
            continue;
        calculateFrames(frameAt, frames);
        IMorph *morph = frames->morph;
        morph->setWeight(frames->weight);
    }
    m_previousFrame = m_currentFrame;
    m_currentFrame = frameAt;
}

void MorphAnimation::setParentModel(IModel *model)
{
    if (!m_model) {
        buildInternalNodes(model);
        m_model = model;
    }
}

void MorphAnimation::refresh()
{
    if (m_model) {
        m_name2keyframes.releaseAll();
        buildInternalNodes(m_model);
    }
}

void MorphAnimation::buildInternalNodes(IModel *model)
{
    const int nframes = m_frames.count();
    // Build internal node to find by name, not frame index
    for (int i = 0; i < nframes; i++) {
        MorphKeyframe *frame = static_cast<MorphKeyframe *>(m_frames.at(i));
        const IString *name = frame->name();
        const HashString &key = name->toHashString();
        InternalMorphKeyFrameList **ptr = m_name2keyframes[key], *node;
        if (ptr) {
            node = *ptr;
            node->keyframes.add(frame);
        }
        else {
            IMorph *morph = model->findMorph(name);
            if (morph) {
                node = new InternalMorphKeyFrameList();
                node->keyframes.add(frame);
                node->morph = morph;
                node->lastIndex = 0;
                node->weight = 0.0f;
                m_name2keyframes.insert(key, node);
            }
        }
    }
    // Sort frames from each internal nodes by frame index ascend
    const int nnodes = m_name2keyframes.count();
    for (int i = 0; i < nnodes; i++) {
        InternalMorphKeyFrameList *keyframes = *m_name2keyframes.value(i);
        Array<MorphKeyframe *> &frames = keyframes->keyframes;
        frames.sort(MorphAnimationKeyFramePredication());
        btSetMax(m_maxFrame, frames[frames.count() - 1]->frameIndex());
    }
}

void MorphAnimation::reset()
{
    BaseAnimation::reset();
    const int nnodes = m_name2keyframes.count();
    for (int i = 0; i < nnodes; i++) {
        InternalMorphKeyFrameList *node = *m_name2keyframes.value(i);
        node->lastIndex = 0;
    }
}

MorphKeyframe *MorphAnimation::frameAt(int i) const
{
    return static_cast<MorphKeyframe *>(m_frames[i]);
}

void MorphAnimation::calculateFrames(float frameAt, InternalMorphKeyFrameList *keyframes)
{
    Array<MorphKeyframe *> &kframes = keyframes->keyframes;
    const int nframes = kframes.count();
    MorphKeyframe *lastKeyFrame = kframes.at(nframes - 1);
    float currentFrame = btMin(frameAt, lastKeyFrame->frameIndex());
    // Find the next frame index bigger than the frame index of last key frame
    int k1 = 0, k2 = 0, lastIndex = keyframes->lastIndex;
    if (currentFrame >= kframes.at(lastIndex)->frameIndex()) {
        for (int i = lastIndex; i < nframes; i++) {
            if (currentFrame <= kframes.at(i)->frameIndex()) {
                k2 = i;
                break;
            }
        }
    }
    else {
        for (int i = 0; i <= lastIndex && i < nframes; i++) {
            if (currentFrame <= m_frames.at(i)->frameIndex()) {
                k2 = i;
                break;
            }
        }
    }

    if (k2 >= nframes)
        k2 = nframes - 1;
    k1 = k2 <= 1 ? 0 : k2 - 1;
    keyframes->lastIndex = k1;

    const MorphKeyframe *keyFrameFrom = kframes.at(k1), *keyFrameTo = kframes.at(k2);
    float frameIndexFrom = keyFrameFrom->frameIndex(), frameIndexTo = keyFrameTo->frameIndex();
    float weightFrom = keyFrameFrom->weight();
    float weightTo = keyFrameTo->weight();

    if (frameIndexFrom != frameIndexTo) {
        const float w = (currentFrame - frameIndexFrom) / (frameIndexTo - frameIndexFrom);
        keyframes->weight = internal::lerp(weightFrom, weightTo, w);
    }
    else {
        keyframes->weight = weightFrom;
    }
    m_previousFrame = m_currentFrame;
    m_currentFrame = frameAt;
}

}
}
