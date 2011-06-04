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
#include "vpvl/internal/FaceKeyFrame.h"

namespace vpvl
{

const float FaceMotion::kStartingMarginFrame = 6.0f;

class FaceMotionKeyFramePredication
{
public:
    bool operator()(const FaceKeyFrame *left, const FaceKeyFrame *right) {
        return left->index() - right->index();
    }
};

FaceMotion::FaceMotion()
    : m_face(0),
      m_lastIndex(0),
      m_lastLoopStartIndex(0),
      m_noFaceSmearIndex(0),
      m_weight(0.0f),
      m_snapWeight(0.0f),
      m_overrideFirst(false)
{
}

FaceMotion::~FaceMotion()
{
    m_face = 0;
    m_weight = 0.0f;
    m_snapWeight = 0.0f;
}

void FaceMotion::read(const char *data, uint32_t size)
{
    char *ptr = const_cast<char *>(data);
    m_frames.reserve(size);
    for (uint32_t i = 0; i < size; i++) {
        FaceKeyFrame *frame = new FaceKeyFrame();
        frame->read(ptr);
        ptr += FaceKeyFrame::stride(ptr);
        m_frames.push_back(frame);
    }
}

void FaceMotion::calculate(float frameAt)
{
    uint32_t nFrames = m_frames.size();
    FaceKeyFrame *lastKeyFrame = m_frames.at(nFrames - 1);
    float currentFrame = frameAt;
    if (currentFrame > lastKeyFrame->index())
        currentFrame = lastKeyFrame->index();

    uint32_t k1 = 0, k2 = 0;
    if (currentFrame >= m_frames.at(m_lastIndex)->index()) {
        for (uint32_t i = m_lastIndex; i < nFrames; i++) {
            if (currentFrame <= m_frames.at(i)->index()) {
                k2 = i;
                break;
            }
        }
    }
    else {
        for (uint32_t i = 0; i <= m_lastIndex && i < nFrames; i++) {
            if (currentFrame <= m_frames.at(i)->index()) {
                k2 = i;
                break;
            }
        }
    }

    if (k2 >= nFrames)
        k2 = nFrames - 1;
    k1 = k2 <= 1 ? 0 : k2 - 1;
    m_lastIndex = k1;

    const FaceKeyFrame *keyFrameFrom = m_frames.at(k1), *keyFrameTo = m_frames.at(k2);
    float timeFrom = keyFrameFrom->index();
    float timeTo = keyFrameTo->index();
    float weightFrom = 0.0f, weightTo = 0.0f;
    if (m_overrideFirst &&(k1 == 0 || timeFrom <= m_lastIndex <= m_lastLoopStartIndex)) {
        if (nFrames > 1 && timeTo < m_lastLoopStartIndex + 60.0f) {
            timeFrom = m_lastLoopStartIndex;
            weightFrom = m_snapWeight;
            weightTo = keyFrameTo->weight();
        }
        else if (frameAt - timeFrom < m_noFaceSmearIndex) {
            timeFrom = m_lastLoopStartIndex;
            timeTo = m_lastLoopStartIndex + m_noFaceSmearIndex;
            currentFrame = frameAt;
            weightFrom = m_snapWeight;
            weightTo = keyFrameFrom->weight();
        }
        else if (nFrames > 1) {
            timeFrom = m_lastLoopStartIndex + m_noFaceSmearIndex;
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

    if (timeFrom != timeTo) {
        float w = (currentFrame - timeFrom) / (timeTo - timeFrom);
        m_weight = weightFrom * (1.0f - w) + weightTo * w;
    }
    else {
        m_weight = weightFrom;
    }
}

void FaceMotion::sort()
{
    m_frames.quickSort(FaceMotionKeyFramePredication());
}

}
