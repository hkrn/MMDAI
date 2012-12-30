/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
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

class KeyframeTimeIndexPredication
{
public:
    bool operator()(const IKeyframe *left, const IKeyframe *right) const {
        const IKeyframe::LayerIndex &leftLayerIndex = left->layerIndex(),
                &rightLayerIndex = right->layerIndex();
        if (leftLayerIndex == rightLayerIndex) {
            return left->timeIndex() < right->timeIndex();
        }
        return leftLayerIndex < rightLayerIndex;
    }
};

class BaseSectionContext
{
public:
    typedef Array<IKeyframe *> KeyframeCollection;
    KeyframeCollection keyframes;
    BaseSectionContext()
        : m_lastIndex(0)
    {
    }
    virtual ~BaseSectionContext() {
        keyframes.releaseAll();
        m_lastIndex = 0;
    }

protected:
    void findKeyframeIndices(const IKeyframe::TimeIndex &seekIndex,
                             IKeyframe::TimeIndex &currentKeyframe,
                             int &fromIndex,
                             int &toIndex) const
    {
        const int nframes = keyframes.count();
        IKeyframe *lastKeyFrame = keyframes[nframes - 1];
        currentKeyframe = btMin(seekIndex, lastKeyFrame->timeIndex());
        // Find the next frame index bigger than the frame index of last key frame
        fromIndex = toIndex = 0;
        if (currentKeyframe >= keyframes[m_lastIndex]->timeIndex()) {
            for (int i = m_lastIndex; i < nframes; i++) {
                if (currentKeyframe <= keyframes[i]->timeIndex()) {
                    toIndex = i;
                    break;
                }
            }
        }
        else {
            for (int i = 0; i <= m_lastIndex && i < nframes; i++) {
                if (currentKeyframe <= keyframes[i]->timeIndex()) {
                    toIndex = i;
                    break;
                }
            }
        }
        if (toIndex >= nframes)
            toIndex = nframes - 1;
        fromIndex = toIndex <= 1 ? 0 : toIndex - 1;
        m_lastIndex = fromIndex;
    }
    static IKeyframe::SmoothPrecision calculateWeight(const IKeyframe::TimeIndex &currentTimeIndex,
                                                      const IKeyframe::TimeIndex &timeIndexFrom,
                                                      const IKeyframe::TimeIndex &timeIndexTo)
    {
        const IKeyframe::SmoothPrecision &value = (currentTimeIndex - timeIndexFrom) / (timeIndexTo - timeIndexFrom);
        return value;
    }
    static IKeyframe::SmoothPrecision calculateInterpolatedWeight(const Motion::InterpolationTable &t,
                                                                  const IKeyframe::SmoothPrecision &weight)
    {
        const Motion::InterpolationTable::Value &v = t.table;
        const uint16_t index = static_cast<int16_t>(weight * t.size);
        const IKeyframe::SmoothPrecision &value = v[index] + (v[index + 1] - v[index]) * (weight * t.size - index);
        return value;
    }
    static void interpolate(const Motion::InterpolationTable &t,
                            const Vector3 &from,
                            const Vector3 &to,
                            const IKeyframe::SmoothPrecision &weight,
                            int at,
                            IKeyframe::SmoothPrecision &value)
    {
        const IKeyframe::SmoothPrecision &valueFrom = from[at];
        const IKeyframe::SmoothPrecision &valueTo = to[at];
        if (t.linear) {
            value = internal::lerp(valueFrom, valueTo, weight);
        }
        else {
            const IKeyframe::SmoothPrecision &weight2 = calculateInterpolatedWeight(t, weight);
            value = internal::lerp(valueFrom, valueTo, weight2);
        }
    }

    mutable int m_lastIndex;

private:
    VPVL2_DISABLE_COPY_AND_ASSIGN(BaseSectionContext)
};

class VPVL2_API BaseSection
{
public:
    BaseSection(const Motion *motionRef)
        : m_motionRef(motionRef),
          m_nameListSectionRef(motionRef->nameListSection()),
          m_maxTimeIndex(0),
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
        m_maxTimeIndex = 0;
    }
    virtual void read(const uint8_t *data) = 0;
    virtual void seek(const IKeyframe::TimeIndex &timeIndex) = 0;
    virtual void write(uint8_t *data) const = 0;
    virtual size_t estimateSize() const = 0;
    virtual size_t countKeyframes() const = 0;
    virtual void addKeyframe(IKeyframe *keyframe) = 0;
    virtual void deleteKeyframe(IKeyframe *&keyframe) = 0;
    virtual void getKeyframes(const IKeyframe::TimeIndex &timeIndex,
                              const IKeyframe::LayerIndex &layerIndex,
                              Array<IKeyframe *> &keyframes) = 0;

    void advance(const IKeyframe::TimeIndex &deltaTimeIndex) {
        seek(m_currentTimeIndex);
        saveCurrentTimeIndex(m_currentTimeIndex + deltaTimeIndex);
    }

    const Motion *parentMotionRef() const { return m_motionRef; }
    IKeyframe::TimeIndex maxTimeIndex() const { return m_maxTimeIndex; }
    IKeyframe::TimeIndex currentTimeIndex() const { return m_currentTimeIndex; }
    IKeyframe::TimeIndex previousTimeIndex() const { return m_previousTimeIndex; }

protected:
    virtual void addKeyframe0(IKeyframe *keyframe, BaseSectionContext::KeyframeCollection &keyframes) {
        keyframes.add(keyframe);
        btSetMax(m_maxTimeIndex, keyframe->timeIndex());
    }
    void saveCurrentTimeIndex(const IKeyframe::TimeIndex &timeIndex) {
        m_previousTimeIndex = m_currentTimeIndex;
        m_currentTimeIndex = timeIndex;
    }

    const Motion *m_motionRef;
    NameListSection *m_nameListSectionRef;
    IKeyframe::TimeIndex m_maxTimeIndex;
    IKeyframe::TimeIndex m_currentTimeIndex;
    IKeyframe::TimeIndex m_previousTimeIndex;

private:
    VPVL2_DISABLE_COPY_AND_ASSIGN(BaseSection)
};

} /* namespace mvd */
} /* namespace vpvl2 */

#endif

