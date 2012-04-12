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

#include "vpvl2/vmd/BaseAnimation.h"
#include "vpvl2/vmd/BaseKeyframe.h"

namespace vpvl2
{
namespace vmd
{

BaseAnimation::BaseAnimation()
    : m_lastIndex(0),
      m_maxFrame(0.0f),
      m_currentFrame(0.0f),
      m_previousFrame(0.0f),
      m_enableAutomaticRefresh(true)
{
}

BaseAnimation::~BaseAnimation()
{
    m_frames.releaseAll();
    m_lastIndex = 0;
    m_maxFrame = 0.0f;
    m_currentFrame = 0.0f;
    m_previousFrame = 0.0f;
    m_enableAutomaticRefresh = true;
}

void BaseAnimation::advance(float deltaFrame)
{
    seek(m_currentFrame);
    m_currentFrame += deltaFrame;
}

void BaseAnimation::rewind(float target, float deltaFrame)
{
    m_currentFrame = m_previousFrame + deltaFrame - m_maxFrame + target;
    m_previousFrame = target;
}

void BaseAnimation::reset()
{
    m_currentFrame = 0.0f;
    m_previousFrame = 0.0f;
}

void BaseAnimation::addKeyframe(BaseKeyframe *frame)
{
    m_frames.add(frame);
    if (m_enableAutomaticRefresh)
        refresh();
}

void BaseAnimation::replaceKeyframe(BaseKeyframe *frame)
{
    deleteKeyframe(frame->frameIndex(), frame->name());
    addKeyframe(frame);
}

void BaseAnimation::deleteKeyframe(float frameIndex, const IString *value)
{
    const int nframes = m_frames.count();
    BaseKeyframe *frameToRemove = 0;
    for (int i = 0; i < nframes; i++) {
        BaseKeyframe *frame = m_frames[i];
        if (frame->frameIndex() == frameIndex && value->equals(frame->name())) {
            frameToRemove = frame;
            break;
        }
    }
    if (frameToRemove) {
        m_frames.remove(frameToRemove);
        delete frameToRemove;
        if (m_enableAutomaticRefresh)
            refresh();
    }
}

void BaseAnimation::deleteKeyframes(float frameIndex)
{
    const int nframes = m_frames.count();
    Array<BaseKeyframe *> framesToRemove;
    for (int i = 0; i < nframes; i++) {
        BaseKeyframe *frame = m_frames[i];
        if (frame->frameIndex() == frameIndex)
            framesToRemove.add(frame);
    }
    const int nFramesToRemove = framesToRemove.count();
    if (nFramesToRemove) {
        for (int i = 0; i < nFramesToRemove; i++) {
            BaseKeyframe *frame = framesToRemove[i];
            m_frames.remove(frame);
            delete frame;
        }
        if (m_enableAutomaticRefresh)
            refresh();
    }
}

}
}
