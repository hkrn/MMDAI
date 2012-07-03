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

namespace
{

using namespace vpvl2;
using namespace vpvl2::vmd;

class LightAnimationKeyFramePredication
{
public:
    bool operator()(const IKeyframe *left, const IKeyframe *right) const {
        return left->frameIndex() < right->frameIndex();
    }
};

}

namespace vpvl2
{
namespace vmd
{

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
        m_keyframes.reserve(size);
        for (int i = 0; i < size; i++) {
            LightKeyframe *frame = new LightKeyframe();
            m_keyframes.add(frame);
            frame->read(ptr);
            ptr += frame->estimateSize();
        }
        update();
    }
}

void LightAnimation::seek(const IKeyframe::Index &frameAt)
{
    const int nframes = m_keyframes.count();
    LightKeyframe *lastKeyFrame = reinterpret_cast<LightKeyframe *>(m_keyframes[nframes - 1]);
    const IKeyframe::Index &currentFrame = btMin(frameAt, lastKeyFrame->frameIndex());
    // Find the next frame index bigger than the frame index of last key frame
    int k1 = 0, k2 = 0;
    if (currentFrame >= m_keyframes[m_lastIndex]->frameIndex()) {
        for (int i = m_lastIndex; i < nframes; i++) {
            if (currentFrame <= m_keyframes[i]->frameIndex()) {
                k2 = i;
                break;
            }
        }
    }
    else {
        for (int i = 0; i <= m_lastIndex && i < nframes; i++) {
            if (currentFrame <= m_keyframes[i]->frameIndex()) {
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
    const IKeyframe::Index &frameIndexFrom = keyFrameFrom->frameIndex(), frameIndexTo = keyFrameTo->frameIndex();
    const Vector3 &colorFrom = keyFrameFrom->color(), &directionFrom = keyFrameFrom->direction();
    const Vector3 &colorTo = keyFrameTo->color(), &directionTo = keyFrameTo->direction();

    if (frameIndexFrom != frameIndexTo) {
        const IKeyframe::SmoothPrecision &w = (currentFrame - frameIndexFrom) / (frameIndexTo - frameIndexFrom);
        m_color.setInterpolate3(colorFrom, colorTo, w);
        m_direction.setInterpolate3(directionFrom, directionTo, w);
    }
    else {
        m_color = colorFrom;
        m_direction = directionFrom;
    }
    m_previousFrameIndex = m_currentFrameIndex;
    m_currentFrameIndex = frameAt;
}

void LightAnimation::update()
{
    int nkeyframes = m_keyframes.count();
    if (nkeyframes > 0) {
        m_keyframes.sort(LightAnimationKeyFramePredication());
        m_maxFrameIndex = m_keyframes[m_keyframes.count() - 1]->frameIndex();
    }
    else {
        m_maxFrameIndex = 0;
    }
}

LightKeyframe *LightAnimation::findKeyframe(int frameIndex) const
{
    int index = findKeyframeIndex(frameIndex, m_keyframes);
    return index != -1 ? reinterpret_cast<LightKeyframe *>(m_keyframes[index]) : 0;
}

LightKeyframe *LightAnimation::frameAt(int i) const
{
    return i >= 0 && i < m_keyframes.count() ? reinterpret_cast<LightKeyframe *>(m_keyframes[i]) : 0;
}

}
}
