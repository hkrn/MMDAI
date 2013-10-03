/**

 Copyright (c) 2009-2011  Nagoya Institute of Technology
                          Department of Computer Science
               2010-2013  hkrn

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

#ifndef VPVL2_INTERNAL_KEYFRAME_H_
#define VPVL2_INTERNAL_KEYFRAME_H_

#include "vpvl2/IKeyframe.h"

namespace vpvl2
{
namespace internal
{

#pragma pack(push, 1)

struct Interpolation VPVL2_DECL_FINAL {
    uint8 x;
    uint8 y;
};

struct InterpolationPair VPVL2_DECL_FINAL {
    Interpolation first;
    Interpolation second;
};

#pragma pack(pop)

struct InterpolationTable VPVL2_DECL_FINAL {
    typedef Array<IKeyframe::SmoothPrecision> Value;
    Value table;
    QuadWord parameter;
    bool linear;
    int size;
    InterpolationTable()
        : parameter(defaultParameter()),
          linear(true),
          size(0)
    {
    }
    ~InterpolationTable() {
        parameter = defaultParameter();
        linear = true;
        size = 0;
    }
    void getInterpolationPair(InterpolationPair &pair) const VPVL2_DECL_NOEXCEPT {
        pair.first.x = uint8(parameter.x());
        pair.first.y = uint8(parameter.y());
        pair.second.x = uint8(parameter.z());
        pair.second.y = uint8(parameter.w());
    }
    void build(const QuadWord &value, int s) {
        if (!btFuzzyZero(value.x() - value.y()) || !btFuzzyZero(value.z() - value.w())) {
            table.resize(s + 1);
            const IKeyframe::SmoothPrecision &x1 = value.x() / 127.0f, &x2 = value.z() / 127.0f;
            const IKeyframe::SmoothPrecision &y1 = value.y() / 127.0f, &y2 = value.w() / 127.0f;
            IKeyframe::SmoothPrecision *ptr = &table[0];
            build(x1, x2, y1, y2, s, ptr);
            linear = false;
        }
        else {
            table.clear();
            linear = true;
        }
        parameter = value;
        size = s;
    }
    void reset() {
        table.clear();
        linear = true;
        parameter = defaultParameter();
    }
    static inline QuadWord defaultParameter() VPVL2_DECL_NOEXCEPT {
        return QuadWord(20, 20, 107, 107);
    }
    static inline QuadWord toQuadWord(const InterpolationPair &pair) VPVL2_DECL_NOEXCEPT {
        return QuadWord(pair.first.x, pair.first.y, pair.second.x, pair.second.y);
    }
    static inline IKeyframe::SmoothPrecision spline1(const IKeyframe::SmoothPrecision &t,
                                                     const IKeyframe::SmoothPrecision &p1,
                                                     const IKeyframe::SmoothPrecision &p2) VPVL2_DECL_NOEXCEPT
    {
        return ((1 + 3 * p1 - 3 * p2) * t * t * t + (3 * p2 - 6 * p1) * t * t + 3 * p1 * t);
    }
    static inline IKeyframe::SmoothPrecision spline2(const IKeyframe::SmoothPrecision &t,
                                                     const IKeyframe::SmoothPrecision &p1,
                                                     const IKeyframe::SmoothPrecision &p2) VPVL2_DECL_NOEXCEPT
    {
        return ((3 + 9 * p1 - 9 * p2) * t * t + (6 * p2 - 12 * p1) * t + 3 * p1);
    }
    static void build(const IKeyframe::SmoothPrecision &x1,
                      const IKeyframe::SmoothPrecision &x2,
                      const IKeyframe::SmoothPrecision &y1,
                      const IKeyframe::SmoothPrecision &y2,
                      int size,
                      IKeyframe::SmoothPrecision *&table) VPVL2_DECL_NOEXCEPT
    {
        VPVL2_DCHECK_NOTNULL(table);
        VPVL2_DCHECK_GT(size, int(0));
        for (int i = 0; i < size; i++) {
            const IKeyframe::SmoothPrecision &in = IKeyframe::SmoothPrecision(i) / size;
            IKeyframe::SmoothPrecision t = in;
            while (1) {
                const IKeyframe::SmoothPrecision &v = spline1(t, x1, x2) - in;
                if (btFabs(btScalar(v)) < 0.0001f) {
                    break;
                }
                const IKeyframe::SmoothPrecision &tt = spline2(t, x1, x2);
                if (btFuzzyZero(btScalar(tt))) {
                    break;
                }
                t -= v / tt;
            }
            table[i] = spline1(t, y1, y2);
        }
        table[size] = 1;
    }
};

} /* namespace internal */
} /* namespace vpvl2 */

#define VPVL2_KEYFRAME_INITIALIZE_FIELDS() \
    m_namePtr(0), \
    m_timeIndex(0), \
    m_layerIndex(0)

#define VPVL2_KEYFRAME_DESTROY_FIELDS() \
    delete m_namePtr; \
    m_namePtr = 0; \
    m_timeIndex = 0; \
    m_layerIndex = 0; \

#define VPVL2_KEYFRAME_DEFINE_METHODS() \
    const IString *name() const { return m_namePtr; } \
    IKeyframe::TimeIndex timeIndex() const { return m_timeIndex; } \
    IKeyframe::LayerIndex layerIndex() const { return m_layerIndex; } \
    void setTimeIndex(const IKeyframe::TimeIndex &value) { m_timeIndex = value; } \
    void setLayerIndex(const IKeyframe::LayerIndex &value) { m_layerIndex = value; }

#define VPVL2_KEYFRAME_DEFINE_FIELDS() \
    IString *m_namePtr; \
    IKeyframe::TimeIndex m_timeIndex; \
    IKeyframe::LayerIndex m_layerIndex;

#endif
