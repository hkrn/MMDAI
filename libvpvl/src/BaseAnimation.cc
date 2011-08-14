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

BaseAnimation::BaseAnimation(float smearDefault) :
    m_lastIndex(0),
    m_lastLoopStartIndex(0),
    m_smearDefault(smearDefault),
    m_maxFrame(0.0f),
    m_currentFrame(0.0f),
    m_previousFrame(0.0f),
    m_lastLoopStartFrame(0.0f),
    m_blendRate(1.0f),
    m_smearIndex(smearDefault),
    m_ignoreSingleAnimation(false),
    m_overrideFirst(false),
    m_automaticRefresh(true)
{
}

void BaseAnimation::advance(float deltaFrame, bool &reached)
{
    seek(m_currentFrame);
    m_previousFrame = m_currentFrame;
    m_currentFrame += deltaFrame;
    if (m_currentFrame >= m_maxFrame) {
        m_currentFrame = m_maxFrame;
        reached = true;
    }
    else {
        reached = false;
    }
}

void BaseAnimation::rewind(float target, float deltaFrame)
{
    m_currentFrame = m_previousFrame + deltaFrame - m_maxFrame + target;
    m_previousFrame = target;
    if (m_overrideFirst) {
        // save current Animation state for loop
        takeSnap(internal::kZeroV);
        m_lastLoopStartFrame = target;
        if (m_maxFrame >= m_smearDefault) {
            m_smearIndex = m_smearDefault;
        }
        else {
            m_smearIndex -= m_maxFrame + 1.0f;
            btSetMax(m_smearIndex, 0.0f);
        }
    }
}

void BaseAnimation::reset()
{
    m_currentFrame = 0.0f;
    m_previousFrame = 0.0f;
    m_lastLoopStartFrame = 0.0f;
    m_blendRate = 1.0f;
    m_smearIndex = m_smearDefault;
}

void BaseAnimation::addFrame(BaseKeyFrame *frame)
{
    m_frames.add(frame);
    if (m_automaticRefresh)
        refresh();
}

void BaseAnimation::deleteFrame(int frameIndex, const uint8_t *value)
{
    const uint32_t nFrames = m_frames.count();
    const size_t len = strlen(reinterpret_cast<const char *>(value));
    BaseKeyFrame *frameToRemove = 0;
    for (uint32_t i = 0; i < nFrames; i++) {
        BaseKeyFrame *frame = m_frames[i];
        if (frame->frameIndex() == frameIndex && internal::stringEquals(value, frame->name(), len)) {
            frameToRemove = frame;
            break;
        }
    }
    if (frameToRemove) {
        m_frames.remove(frameToRemove);
        if (m_automaticRefresh)
            refresh();
    }
}

void BaseAnimation::deleteFrames(int frameIndex)
{
    const uint32_t nFrames = m_frames.count();
    BaseKeyFrameList framesToRemove;
    for (uint32_t i = 0; i < nFrames; i++) {
        BaseKeyFrame *frame = m_frames[i];
        if (frame->frameIndex() == frameIndex)
            framesToRemove.add(frame);
    }
    const uint32_t nFramesToRemove = framesToRemove.count();
    if (nFramesToRemove) {
        for (uint32_t i = 0; i < nFramesToRemove; i++)
            m_frames.remove(framesToRemove[i]);
        if (m_automaticRefresh)
            refresh();
    }
}

void BaseAnimation::setOverrideFirst(const btVector3 &center)
{
    takeSnap(center);
    m_overrideFirst = true;
    m_smearIndex = m_smearDefault;
}

}
