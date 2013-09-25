/**

 Copyright (c) 2010-2013  hkrn

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
#ifndef VPVL2_INTERNAL_MOTIONHELPER_H_
#define VPVL2_INTERNAL_MOTIONHELPER_H_

#include "vpvl2/Common.h"
#include "vpvl2/IKeyframe.h"
#include "vpvl2/internal/Keyframe.h"
#include "vpvl2/internal/util.h"

namespace vpvl2
{
namespace internal
{

class MotionHelper VPVL2_DECL_FINAL {
public:
    class KeyframeTimeIndexPredication VPVL2_DECL_FINAL {
    public:
        bool operator()(const IKeyframe *left, const IKeyframe *right) const VPVL2_DECL_NOEXCEPT {
            const IKeyframe::LayerIndex &leftLayerIndex = left->layerIndex(),
                    &rightLayerIndex = right->layerIndex();
            if (leftLayerIndex == rightLayerIndex) {
                return left->timeIndex() < right->timeIndex();
            }
            return leftLayerIndex < rightLayerIndex;
        }
    };

    template<typename T>
    static void findKeyframeIndices(const IKeyframe::TimeIndex &seekIndex,
                                    IKeyframe::TimeIndex &currentKeyframe,
                                    int &lastIndex,
                                    int &fromIndex,
                                    int &toIndex,
                                    const Array<T *> &keyframes) VPVL2_DECL_NOEXCEPT
    {
        const int nframes = keyframes.count();
        IKeyframe *lastKeyFrame = keyframes[nframes - 1];
        currentKeyframe = btMin(seekIndex, lastKeyFrame->timeIndex());
        // Find the next frame index bigger than the frame index of last key frame
        fromIndex = toIndex = 0;
        if (currentKeyframe >= keyframes[lastIndex]->timeIndex()) {
            for (int i = lastIndex; i < nframes; i++) {
                if (currentKeyframe <= keyframes[i]->timeIndex()) {
                    toIndex = i;
                    break;
                }
            }
        }
        else {
            for (int i = 0; i <= lastIndex && i < nframes; i++) {
                if (currentKeyframe <= keyframes[i]->timeIndex()) {
                    toIndex = i;
                    break;
                }
            }
        }
        if (toIndex >= nframes) {
            toIndex = nframes - 1;
        }
        fromIndex = toIndex <= 1 ? 0 : toIndex - 1;
        lastIndex = fromIndex;
    }
    template<typename TMotion>
    static inline bool isReachedToDuration(const TMotion &motion, const IKeyframe::TimeIndex &atEnd) VPVL2_DECL_NOEXCEPT
    {
        return motion.duration() > 0 ? motion.currentTimeIndex() >= atEnd : true;
    }
    static inline IKeyframe::SmoothPrecision lerp(const IKeyframe::SmoothPrecision &x,
                                                  const IKeyframe::SmoothPrecision &y,
                                                  const IKeyframe::SmoothPrecision &t) VPVL2_DECL_NOEXCEPT
    {
        return x + (y - x) * t;
    }
    static inline IKeyframe::SmoothPrecision calculateWeight(const IKeyframe::TimeIndex &currentTimeIndex,
                                                             const IKeyframe::TimeIndex &timeIndexFrom,
                                                             const IKeyframe::TimeIndex &timeIndexTo) VPVL2_DECL_NOEXCEPT
    {
        const IKeyframe::SmoothPrecision &value = (currentTimeIndex - timeIndexFrom) / (timeIndexTo - timeIndexFrom);
        return value;
    }
    static inline IKeyframe::SmoothPrecision calculateInterpolatedWeight(const InterpolationTable &t,
                                                                         const IKeyframe::SmoothPrecision &weight) VPVL2_DECL_NOEXCEPT
    {
        const internal::InterpolationTable::Value &v = t.table;
        const uint16 index = static_cast<int16>(weight * t.size);
        const IKeyframe::SmoothPrecision &value = v[index] + (v[index + 1] - v[index]) * (weight * t.size - index);
        return value;
    }
    static inline void interpolate(const InterpolationTable &t,
                                   const Vector3 &from,
                                   const Vector3 &to,
                                   const IKeyframe::SmoothPrecision &weight,
                                   int at,
                                   IKeyframe::SmoothPrecision &value) VPVL2_DECL_NOEXCEPT
    {
        const IKeyframe::SmoothPrecision &valueFrom = from[at];
        const IKeyframe::SmoothPrecision &valueTo = to[at];
        if (t.linear) {
            value = lerp(valueFrom, valueTo, weight);
        }
        else {
            const IKeyframe::SmoothPrecision &weight2 = calculateInterpolatedWeight(t, weight);
            value = lerp(valueFrom, valueTo, weight2);
        }
    }

private:
    VPVL2_MAKE_STATIC_CLASS(MotionHelper)
};

} /* namespace internal */
} /* namespace vpvl2 */

#endif
