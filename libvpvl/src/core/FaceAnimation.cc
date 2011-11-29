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

struct FaceAnimationInternal {
    Face *face;
    FaceKeyFrameList keyFrames;
    float weight;
    int lastIndex;
};

class FaceAnimationKeyFramePredication
{
public:
    bool operator()(const BaseKeyFrame *left, const BaseKeyFrame *right) {
        return left->frameIndex() < right->frameIndex();
    }
};

FaceAnimation::FaceAnimation()
    : BaseAnimation(),
      m_model(0)
{
}

FaceAnimation::~FaceAnimation()
{
    m_name2node.releaseAll();
    m_model = 0;
}

void FaceAnimation::read(const uint8_t *data, int size)
{
    uint8_t *ptr = const_cast<uint8_t *>(data);
    m_frames.reserve(size);
    for (int i = 0; i < size; i++) {
        FaceKeyFrame *frame = new FaceKeyFrame();
        frame->read(ptr);
        ptr += frame->stride();
        m_frames.add(frame);
    }
}

void FaceAnimation::seek(float frameAt)
{
    const int nnodes = m_name2node.count();
    for (int i = 0; i < nnodes; i++) {
        FaceAnimationInternal *node = *m_name2node.value(i);
        calculateFrames(frameAt, node);
        Face *face = node->face;
        face->setWeight(node->weight);
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
        m_name2node.releaseAll();
        buildInternalNodes(m_model);
    }
}

void FaceAnimation::buildInternalNodes(vpvl::PMDModel *model)
{
    const int nframes = m_frames.count();
    // Build internal node to find by name, not frame index
    for (int i = 0; i < nframes; i++) {
        FaceKeyFrame *frame = static_cast<FaceKeyFrame *>(m_frames.at(i));
        HashString name(reinterpret_cast<const char *>(frame->name()));
        FaceAnimationInternal **ptr = m_name2node[name], *node;
        if (ptr) {
            node = *ptr;
            node->keyFrames.add(frame);
        }
        else {
            Face *face = model->findFace(frame->name());
            if (face) {
                node = new FaceAnimationInternal();
                node->keyFrames.add(frame);
                node->face = face;
                node->lastIndex = 0;
                node->weight = 0.0f;
                m_name2node.insert(name, node);
            }
        }
    }
    // Sort frames from each internal nodes by frame index ascend
    const int nnodes = m_name2node.count();
    for (int i = 0; i < nnodes; i++) {
        FaceAnimationInternal *node = *m_name2node.value(i);
        FaceKeyFrameList &frames = node->keyFrames;
        frames.sort(FaceAnimationKeyFramePredication());
        btSetMax(m_maxFrame, frames[frames.count() - 1]->frameIndex());
    }
}

void FaceAnimation::reset()
{
    BaseAnimation::reset();
    const int nnodes = m_name2node.count();
    for (int i = 0; i < nnodes; i++) {
        FaceAnimationInternal *node = *m_name2node.value(i);
        node->lastIndex = 0;
    }
}

void FaceAnimation::calculateFrames(float frameAt, FaceAnimationInternal *node)
{
    FaceKeyFrameList &kframes = node->keyFrames;
    const int nframes = kframes.count();
    FaceKeyFrame *lastKeyFrame = kframes.at(nframes - 1);
    float currentFrame = btMin(frameAt, lastKeyFrame->frameIndex());
    // Find the next frame index bigger than the frame index of last key frame
    int k1 = 0, k2 = 0, lastIndex = node->lastIndex;
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
    node->lastIndex = k1;

    const FaceKeyFrame *keyFrameFrom = kframes.at(k1), *keyFrameTo = kframes.at(k2);
    float frameIndexFrom = keyFrameFrom->frameIndex(), frameIndexTo = keyFrameTo->frameIndex();
    float weightFrom = keyFrameFrom->weight();
    float weightTo = keyFrameTo->weight();

    if (frameIndexFrom != frameIndexTo) {
        const float w = (currentFrame - frameIndexFrom) / (frameIndexTo - frameIndexFrom);
        node->weight = internal::lerp(weightFrom, weightTo, w);
    }
    else {
        node->weight = weightFrom;
    }
    m_previousFrame = m_currentFrame;
    m_currentFrame = frameAt;
}

}
