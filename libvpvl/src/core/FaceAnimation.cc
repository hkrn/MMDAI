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

struct InternalFaceKeyFrameList {
    Face *face;
    FaceKeyFrameList keyFrames;
    float weight;
    int lastIndex;

    bool isNull() const {
        if (keyFrames.count() == 1) {
            const FaceKeyframe *keyFrame = keyFrames[0];
            return keyFrame->weight() == 0.0f;
        }
        return false;
    }
};

class FaceAnimationKeyFramePredication
{
public:
    bool operator()(const BaseKeyframe *left, const BaseKeyframe *right) {
        return left->frameIndex() < right->frameIndex();
    }
};

FaceAnimation::FaceAnimation()
    : BaseAnimation(),
      m_model(0),
      m_enableNullFrame(false)
{
}

FaceAnimation::~FaceAnimation()
{
    m_name2keyframes.releaseAll();
    m_model = 0;
}

void FaceAnimation::read(const uint8_t *data, int size)
{
    uint8_t *ptr = const_cast<uint8_t *>(data);
    m_frames.reserve(size);
    for (int i = 0; i < size; i++) {
        FaceKeyframe *frame = new FaceKeyframe();
        frame->read(ptr);
        ptr += frame->stride();
        m_frames.add(frame);
    }
}

void FaceAnimation::seek(float frameAt)
{
    const int nnodes = m_name2keyframes.count();
    for (int i = 0; i < nnodes; i++) {
        InternalFaceKeyFrameList *keyFrames = *m_name2keyframes.value(i);
        if (m_enableNullFrame && keyFrames->isNull())
            continue;
        calculateFrames(frameAt, keyFrames);
        Face *face = keyFrames->face;
        face->setWeight(keyFrames->weight);
    }
    m_previousFrame = m_currentFrame;
    m_currentFrame = frameAt;
}

void FaceAnimation::attachModel(PMDModel *model)
{
    if (!m_model) {
        buildInternalNodes(model);
        m_model = model;
    }
}

void FaceAnimation::refresh()
{
    if (m_model) {
        m_name2keyframes.releaseAll();
        buildInternalNodes(m_model);
    }
}

void FaceAnimation::buildInternalNodes(vpvl::PMDModel *model)
{
    const int nframes = m_frames.count();
    // Build internal node to find by name, not frame index
    for (int i = 0; i < nframes; i++) {
        FaceKeyframe *frame = static_cast<FaceKeyframe *>(m_frames.at(i));
        HashString name(reinterpret_cast<const char *>(frame->name()));
        InternalFaceKeyFrameList **ptr = m_name2keyframes[name], *node;
        if (ptr) {
            node = *ptr;
            node->keyFrames.add(frame);
        }
        else {
            Face *face = model->findFace(frame->name());
            if (face) {
                node = new InternalFaceKeyFrameList();
                node->keyFrames.add(frame);
                node->face = face;
                node->lastIndex = 0;
                node->weight = 0.0f;
                m_name2keyframes.insert(name, node);
            }
        }
    }
    // Sort frames from each internal nodes by frame index ascend
    const int nnodes = m_name2keyframes.count();
    for (int i = 0; i < nnodes; i++) {
        InternalFaceKeyFrameList *keyFrames = *m_name2keyframes.value(i);
        FaceKeyFrameList &frames = keyFrames->keyFrames;
        frames.sort(FaceAnimationKeyFramePredication());
        btSetMax(m_maxFrame, frames[frames.count() - 1]->frameIndex());
    }
}

void FaceAnimation::reset()
{
    BaseAnimation::reset();
    const int nnodes = m_name2keyframes.count();
    for (int i = 0; i < nnodes; i++) {
        InternalFaceKeyFrameList *node = *m_name2keyframes.value(i);
        node->lastIndex = 0;
    }
}

void FaceAnimation::calculateFrames(float frameAt, InternalFaceKeyFrameList *keyFrames)
{
    FaceKeyFrameList &kframes = keyFrames->keyFrames;
    const int nframes = kframes.count();
    FaceKeyframe *lastKeyFrame = kframes.at(nframes - 1);
    float currentFrame = btMin(frameAt, lastKeyFrame->frameIndex());
    // Find the next frame index bigger than the frame index of last key frame
    int k1 = 0, k2 = 0, lastIndex = keyFrames->lastIndex;
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
    keyFrames->lastIndex = k1;

    const FaceKeyframe *keyFrameFrom = kframes.at(k1), *keyFrameTo = kframes.at(k2);
    float frameIndexFrom = keyFrameFrom->frameIndex(), frameIndexTo = keyFrameTo->frameIndex();
    float weightFrom = keyFrameFrom->weight();
    float weightTo = keyFrameTo->weight();

    if (frameIndexFrom != frameIndexTo) {
        const float w = (currentFrame - frameIndexFrom) / (frameIndexTo - frameIndexFrom);
        keyFrames->weight = internal::lerp(weightFrom, weightTo, w);
    }
    else {
        keyFrames->weight = weightFrom;
    }
    m_previousFrame = m_currentFrame;
    m_currentFrame = frameAt;
}

}
