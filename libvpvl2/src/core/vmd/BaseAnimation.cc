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
    : m_lastTimeIndex(0),
      m_maxTimeIndex(0),
      m_currentTimeIndex(0),
      m_previousTimeIndex(0)
{
}

BaseAnimation::~BaseAnimation()
{
    m_keyframes.releaseAll();
    m_lastTimeIndex = 0;
    m_maxTimeIndex = 0.0f;
    m_currentTimeIndex = 0.0f;
    m_previousTimeIndex = 0.0f;
}

void BaseAnimation::advance(const IKeyframe::TimeIndex &deltaTimeIndex)
{
    seek(m_currentTimeIndex);
    m_currentTimeIndex += deltaTimeIndex;
}

void BaseAnimation::rewind(const IKeyframe::TimeIndex &target, const IKeyframe::TimeIndex &deltaTimeIndex)
{
    m_currentTimeIndex = m_previousTimeIndex + deltaTimeIndex - m_maxTimeIndex + target;
    m_previousTimeIndex = target;
}

void BaseAnimation::reset()
{
    m_currentTimeIndex = 0.0f;
    m_previousTimeIndex = 0.0f;
}

void BaseAnimation::addKeyframe(IKeyframe *keyframe)
{
    m_keyframes.add(keyframe);
}

void BaseAnimation::deleteKeyframe(IKeyframe *&keyframe)
{
    m_keyframes.remove(keyframe);
    delete keyframe;
    keyframe = 0;
}

void BaseAnimation::getKeyframes(const IKeyframe::TimeIndex &timeIndex, Array<IKeyframe *> &keyframes)
{
    const int nkeyframes = m_keyframes.count();
    for (int i = 0; i < nkeyframes; i++) {
        IKeyframe *keyframe = m_keyframes[i];
        if (keyframe->timeIndex() == timeIndex)
            keyframes.add(keyframe);
    }
}

}
}
