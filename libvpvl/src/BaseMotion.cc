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

BaseMotion::BaseMotion(float smearDefault) :
    m_lastIndex(0),
    m_lastLoopStartIndex(0),
    m_smearDefault(smearDefault),
    m_maxFrame(0.0f),
    m_currentFrame(0.0f),
    m_previousFrame(0.0f),
    m_lastLoopStartFrame(0.0f),
    m_blendRate(1.0f),
    m_smearIndex(smearDefault),
    m_ignoreSingleMotion(false),
    m_overrideFirst(false)
{
}

void BaseMotion::advance(float deltaFrame, bool &reached)
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

void BaseMotion::rewind(float target, float frameAt)
{
    m_currentFrame = m_previousFrame + frameAt - m_maxFrame + target;
    m_previousFrame = target;
    if (m_overrideFirst) {
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

void BaseMotion::reset()
{
    m_currentFrame = 0.0f;
    m_previousFrame = 0.0f;
    m_lastLoopStartFrame = 0.0f;
    m_blendRate = 1.0f;
    m_smearIndex = m_smearDefault;
}

void BaseMotion::setOverrideFirst(const btVector3 &center)
{
    takeSnap(center);
    m_overrideFirst = true;
    m_smearIndex = m_smearDefault;
}

}
