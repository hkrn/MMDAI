/**

 Copyright (c) 2010-2014  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#pragma once
#ifndef VPVL2_MVD_BASESECTION_H_
#define VPVL2_MVD_BASESECTION_H_

#include "vpvl2/mvd/Motion.h"
#include "vpvl2/internal/util.h"

namespace vpvl2
{

class IEncoding;

namespace mvd
{

class NameListSection;

class BaseAnimationTrack
{
public:
    typedef PointerArray<IKeyframe> KeyframeCollection;
    KeyframeCollection keyframes;
    BaseAnimationTrack()
        : m_lastIndex(0)
    {
    }
    virtual ~BaseAnimationTrack() {
        keyframes.releaseAll();
        m_lastIndex = 0;
    }

protected:
    mutable int m_lastIndex;

private:
    VPVL2_DISABLE_COPY_AND_ASSIGN(BaseAnimationTrack)
};

class VPVL2_API BaseSection
{
public:
    BaseSection(const Motion *motionRef)
        : m_motionRef(motionRef),
          m_nameListSectionRef(motionRef->nameListSection()),
          m_durationTimeIndex(0),
          m_currentTimeIndex(0),
          m_previousTimeIndex(0)
    {
    }
    virtual ~BaseSection() {
        release();
    }

    virtual void release() {
        m_nameListSectionRef = 0;
        m_currentTimeIndex = 0;
        m_previousTimeIndex = 0;
        m_durationTimeIndex = 0;
    }
    virtual void read(const uint8 *data) = 0;
    virtual void seek(const IKeyframe::TimeIndex &timeIndex) = 0;
    virtual void write(uint8 *data) const = 0;
    virtual vsize estimateSize() const = 0;
    virtual vsize countKeyframes() const = 0;
    virtual void update() = 0;
    virtual void addKeyframe(IKeyframe *keyframe) = 0;
    virtual void removeKeyframe(IKeyframe *keyframe) = 0;
    virtual void deleteKeyframe(IKeyframe *&keyframe) = 0;
    virtual void getKeyframes(const IKeyframe::TimeIndex &timeIndex,
                              const IKeyframe::LayerIndex &layerIndex,
                              Array<IKeyframe *> &keyframes) const = 0;
    virtual void getAllKeyframes(Array<IKeyframe *> &value) const = 0;
    virtual void setAllKeyframes(const Array<IKeyframe *> &value) = 0;

    void advance(const IKeyframe::TimeIndex &deltaTimeIndex) {
        seek(m_currentTimeIndex);
        saveCurrentTimeIndex(m_currentTimeIndex + deltaTimeIndex);
    }

    const Motion *parentMotionRef() const { return m_motionRef; }
    IKeyframe::TimeIndex duration() const { return m_durationTimeIndex; }
    IKeyframe::TimeIndex currentTimeIndex() const { return m_currentTimeIndex; }
    IKeyframe::TimeIndex previousTimeIndex() const { return m_previousTimeIndex; }

protected:
    virtual void setDuration(IKeyframe *keyframe) {
        btSetMax(m_durationTimeIndex, keyframe->timeIndex());
    }
    void saveCurrentTimeIndex(const IKeyframe::TimeIndex &timeIndex) {
        m_previousTimeIndex = m_currentTimeIndex;
        m_currentTimeIndex = timeIndex;
    }

    const Motion *m_motionRef;
    NameListSection *m_nameListSectionRef;
    IKeyframe::TimeIndex m_durationTimeIndex;
    IKeyframe::TimeIndex m_currentTimeIndex;
    IKeyframe::TimeIndex m_previousTimeIndex;

private:
    VPVL2_DISABLE_COPY_AND_ASSIGN(BaseSection)
};

} /* namespace mvd */
} /* namespace vpvl2 */

#endif
