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

namespace vpvl
{

const float FaceAnimation::kStartingMarginFrame = 6.0f;

struct FaceAnimationInternal {
    Face *face;
    FaceKeyFrameList keyFrames;
    float weight;
    float snapWeight;
    uint32_t lastIndex;
};

class FaceAnimationKeyFramePredication
{
public:
    bool operator()(const FaceKeyFrame *left, const FaceKeyFrame *right) {
        return left->frameIndex() < right->frameIndex();
    }
};

FaceAnimation::FaceAnimation()
    : BaseAnimation(kStartingMarginFrame),
      m_model(0)
{
}

FaceAnimation::~FaceAnimation()
{
    internal::clearArray(m_frames);
    internal::clearHash(m_name2node);
    m_model = 0;
}

void FaceAnimation::read(const uint8_t *data, uint32_t size)
{
    uint8_t *ptr = const_cast<uint8_t *>(data);
    m_frames.reserve(size);
    for (uint32_t i = 0; i < size; i++) {
        FaceKeyFrame *frame = new FaceKeyFrame();
        frame->read(ptr);
        ptr += FaceKeyFrame::stride();
        m_frames.push_back(frame);
    }
}

void FaceAnimation::seek(float frameAt)
{
    const uint32_t nNodes = m_name2node.size();
    for (uint32_t i = 0; i < nNodes; i++) {
        FaceAnimationInternal *node = *m_name2node.getAtIndex(i);
        if (m_ignoreSingleAnimation && node->keyFrames.size() <= 1)
            continue;
        calculateFrames(frameAt, node);
        Face *face = node->face;
        if (m_blendRate == 1.0f)
            face->setWeight(node->weight);
        else
            face->setWeight(face->weight() * (1.0f - m_blendRate) + node->weight * m_blendRate);
    }
}

void FaceAnimation::takeSnap(const btVector3 & /* center */)
{
    const uint32_t nNodes = m_name2node.size();
    for (uint32_t i = 0; i < nNodes; i++) {
        FaceAnimationInternal *node = *m_name2node.getAtIndex(i);
        const Face *face = node->face;
        node->snapWeight = face->weight();
    }
}

void FaceAnimation::attachModel(PMDModel *model)
{
    if (m_model)
        return;

    const uint32_t nFrames = m_frames.size();
    for (uint32_t i = 0; i < nFrames; i++) {
        FaceKeyFrame *frame = m_frames.at(i);
        btHashString name(reinterpret_cast<const char *>(frame->name()));
        FaceAnimationInternal **ptr = m_name2node.find(name), *node;
        if (ptr) {
            node = *ptr;
            node->keyFrames.push_back(frame);
        }
        else {
            Face *face = model->findFace(frame->name());
            if (face) {
                node = new FaceAnimationInternal();
                node->keyFrames.push_back(frame);
                node->face = face;
                node->lastIndex = 0;
                node->weight = 0.0f;
                node->snapWeight = 0.0f;
                m_name2node.insert(name, node);
            }
        }
    }

    const uint32_t nNodes = m_name2node.size();
    for (uint32_t i = 0; i < nNodes; i++) {
        FaceAnimationInternal *node = *m_name2node.getAtIndex(i);
        FaceKeyFrameList &frames = node->keyFrames;
        frames.quickSort(FaceAnimationKeyFramePredication());
        btSetMax(m_maxFrame, frames[frames.size() - 1]->frameIndex());
    }

    m_model = model;
}

void FaceAnimation::reset()
{
    BaseAnimation::reset();
    const uint32_t nNodes = m_name2node.size();
    for (uint32_t i = 0; i < nNodes; i++) {
        FaceAnimationInternal *node = *m_name2node.getAtIndex(i);
        node->lastIndex = 0;
    }
}

void FaceAnimation::calculateFrames(float frameAt, FaceAnimationInternal *node)
{
    FaceKeyFrameList &kframes = node->keyFrames;
    const uint32_t nFrames = kframes.size();
    FaceKeyFrame *lastKeyFrame = kframes.at(nFrames - 1);
    float currentFrame = frameAt;
    if (currentFrame > lastKeyFrame->frameIndex())
        currentFrame = lastKeyFrame->frameIndex();

    uint32_t k1 = 0, k2 = 0;
    if (currentFrame >= kframes.at(node->lastIndex)->frameIndex()) {
        for (uint32_t i = node->lastIndex; i < nFrames; i++) {
            if (currentFrame <= kframes.at(i)->frameIndex()) {
                k2 = i;
                break;
            }
        }
    }
    else {
        for (uint32_t i = 0; i <= node->lastIndex && i < nFrames; i++) {
            if (currentFrame <= m_frames.at(i)->frameIndex()) {
                k2 = i;
                break;
            }
        }
    }

    if (k2 >= nFrames)
        k2 = nFrames - 1;
    k1 = k2 <= 1 ? 0 : k2 - 1;
    node->lastIndex = k1;

    const FaceKeyFrame *keyFrameFrom = kframes.at(k1), *keyFrameTo = kframes.at(k2);
    float frameIndexFrom = keyFrameFrom->frameIndex(), frameIndexTo = keyFrameTo->frameIndex();
    float weightFrom = 0.0f, weightTo = 0.0f;
    if (m_overrideFirst && (k1 == 0 || frameIndexFrom <= m_lastLoopStartIndex)) {
        if (nFrames > 1 && frameIndexTo < m_lastLoopStartIndex + 60.0f) {
            frameIndexFrom = static_cast<float>(m_lastLoopStartIndex);
            weightFrom = node->snapWeight;
            weightTo = keyFrameTo->weight();
        }
        else if (frameAt - frameIndexFrom < m_smearIndex) {
            frameIndexFrom = static_cast<float>(m_lastLoopStartIndex);
            frameIndexTo = m_lastLoopStartIndex + m_smearIndex;
            currentFrame = frameAt;
            weightFrom = node->snapWeight;
            weightTo = keyFrameFrom->weight();
        }
        else if (nFrames > 1) {
            frameIndexFrom = m_lastLoopStartIndex + m_smearIndex;
            weightFrom = keyFrameFrom->weight();
            weightTo = keyFrameTo->weight();
        }
        else {
            weightFrom = keyFrameFrom->weight();
        }
    }
    else {
        weightFrom = keyFrameFrom->weight();
        weightTo = keyFrameTo->weight();
    }

    if (frameIndexFrom != frameIndexTo) {
        const float w = (currentFrame - frameIndexFrom) / (frameIndexTo - frameIndexFrom);
        node->weight = internal::lerp(weightFrom, weightTo, w);
    }
    else {
        node->weight = weightFrom;
    }
}

}
