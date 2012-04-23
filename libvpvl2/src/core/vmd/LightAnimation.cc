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

#include "vpvl2/vmd/LightAnimation.h"
#include "vpvl2/vmd/LightKeyframe.h"

namespace vpvl2
{
namespace vmd
{

class LightAnimationKeyFramePredication
{
public:
    bool operator()(const IKeyframe *left, const IKeyframe *right) {
        return left->frameIndex() < right->frameIndex();
    }
};

LightAnimation::LightAnimation()
    : BaseAnimation(),
      m_color(0.0f, 0.0f, 0.0f),
      m_direction(0.0f, 0.0f, 0.0f)
{
}

LightAnimation::~LightAnimation()
{
    m_color.setZero();
    m_direction.setZero();
}

void LightAnimation::read(const uint8_t *data, int size)
{
    if (size > 0) {
        uint8_t *ptr = const_cast<uint8_t *>(data);
        m_frames.reserve(size);
        for (int i = 0; i < size; i++) {
            LightKeyframe *frame = new LightKeyframe();
            frame->read(ptr);
            ptr += frame->estimateSize();
            m_frames.add(frame);
        }
        m_frames.sort(LightAnimationKeyFramePredication());
        m_maxFrame = m_frames[size - 1]->frameIndex();
    }
}

void LightAnimation::seek(float frameAt)
{
    const int nframes = m_frames.count();
    LightKeyframe *lastKeyFrame = reinterpret_cast<LightKeyframe *>(m_frames[nframes - 1]);
    float currentFrame = btMin(frameAt, lastKeyFrame->frameIndex());
    // Find the next frame index bigger than the frame index of last key frame
    int k1 = 0, k2 = 0;
    if (currentFrame >= m_frames[m_lastIndex]->frameIndex()) {
        for (int i = m_lastIndex; i < nframes; i++) {
            if (currentFrame <= m_frames[i]->frameIndex()) {
                k2 = i;
                break;
            }
        }
    }
    else {
        for (int i = 0; i <= m_lastIndex && i < nframes; i++) {
            if (currentFrame <= m_frames[i]->frameIndex()) {
                k2 = i;
                break;
            }
        }
    }

    if (k2 >= nframes)
        k2 = nframes - 1;
    k1 = k2 <= 1 ? 0 : k2 - 1;
    m_lastIndex = k1;

    const LightKeyframe *keyFrameFrom = this->frameAt(k1), *keyFrameTo = this->frameAt(k2);
    float frameIndexFrom = keyFrameFrom->frameIndex(), frameIndexTo = keyFrameTo->frameIndex();
    const Vector3 &colorFrom = keyFrameFrom->color(), &directionFrom = keyFrameFrom->direction();
    const Vector3 &colorTo = keyFrameTo->color(), &directionTo = keyFrameTo->direction();

    if (frameIndexFrom != frameIndexTo) {
        const float w = (currentFrame - frameIndexFrom) / (frameIndexTo - frameIndexFrom);
        m_color.setInterpolate3(colorFrom, colorTo, w);
        m_direction.setInterpolate3(directionFrom, directionTo, w);
    }
    else {
        m_color = colorFrom;
        m_direction = directionFrom;
    }
    m_previousFrame = m_currentFrame;
    m_currentFrame = frameAt;
}

void LightAnimation::reset()
{
    BaseAnimation::reset();
}

LightKeyframe *LightAnimation::frameAt(int i) const
{
    return reinterpret_cast<LightKeyframe *>(m_frames[i]);
}

}
}
