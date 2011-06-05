/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2010  Nagoya Institute of Technology          */
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

#ifndef VPVL_INTERNAL_UTIL_H_
#define VPVL_INTERNAL_UTIL_H_

#include "LinearMath/btVector3.h"
#include "LinearMath/btQuaternion.h"
#include <string.h>

namespace vpvl
{
namespace internal
{

static const btVector3 kZeroV = btVector3(0.0f, 0.0f, 0.0f);
static const btQuaternion kZeroQ = btQuaternion(0.0f, 0.0f, 0.0f, 1.0f);

static inline void vectorN(uint8_t *&ptr, float *values, int n)
{
    for (int i = 0; i < n; i++) {
        values[i] = *reinterpret_cast<float *>(ptr);
        ptr += sizeof(float);
    }
}

static inline float spline1(const float t, const float p1, const float p2)
{
    return ((1 + 3 * p1 - 3 * p2) * t * t * t + (3 * p2 - 6 * p1) * t * t + 3 * p1 * t);
}

static inline float spline2(const float t, const float p1, const float p2)
{
    return ((3 + 9 * p1 - 9 * p2) * t * t + (6 * p2 - 12 * p1) * t + 3 * p1);
}

inline void vector3(uint8_t *&ptr, float *values)
{
    vectorN(ptr, values, 3);
}

inline void vector4(uint8_t *&ptr, float *values)
{
    vectorN(ptr, values, 4);
}

inline bool size8(uint8_t *&ptr, size_t &rest, size_t &size)
{
    if (sizeof(uint8_t) > rest)
        return false;
    size = *reinterpret_cast<uint8_t *>(ptr);
    ptr += sizeof(uint8_t);
    rest -= sizeof(uint8_t);
    return true;
}

inline bool size16(uint8_t *&ptr, size_t &rest, size_t &size)
{
    if (sizeof(uint16_t) > rest)
        return false;
    size = *reinterpret_cast<uint16_t *>(ptr);
    ptr += sizeof(uint16_t);
    rest -= sizeof(uint16_t);
    return true;
}

inline bool size32(uint8_t *&ptr, size_t &rest, size_t &size)
{
    if (sizeof(uint32_t) > rest)
        return false;
    size = *reinterpret_cast<uint32_t *>(ptr);
    ptr += sizeof(uint32_t);
    rest -= sizeof(uint32_t);
    return true;
}

inline bool validateSize(uint8_t *&ptr, size_t stride, size_t size, size_t &rest)
{
    size_t required = stride * size;
    if (required > rest)
        return false;
    ptr += required;
    rest -= required;
    return true;
}

inline void buildInterpolationTable(float x1, float x2, float y1, float y2, int size, float *&table)
{
    for (int i = 0; i < size; i++) {
        const float in = static_cast<double>(i) / size;
        float t = in;
        while (1) {
            const float v = spline1(t, x1, x2) - in;
            if (fabs(v) < 0.0001f)
                break;
            const float tt = spline2(t, x1, x2);
            if (tt == 0.0f)
                break;
            t -= v / tt;
        }
        table[i] = spline1(t, y1, y2);
    }
    table[size] = 1.0f;
}

inline bool stringEquals(const uint8_t *s1, const uint8_t *s2, size_t max)
{
    assert(s1 != NULL && s2 != NULL && max > 0);
    return memcmp(s1, s2, max) == 0;
}

}
}

#define VPVL_DISABLE_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName &); \
    void operator=(const TypeName &);

#endif
