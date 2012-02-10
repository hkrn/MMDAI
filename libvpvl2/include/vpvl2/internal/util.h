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

#ifndef VPVL2_INTERNAL_UTIL_H_
#define VPVL2_INTERNAL_UTIL_H_

#include "vpvl2/config.h"
#include "vpvl2/Common.h"
#include <string.h>

#if defined(WIN32)
#include <windows.h>
#endif

namespace vpvl2
{
namespace internal
{

static const int kCurrentVersion = VPVL2_VERSION;
static const char *const kCurrentVersionString = VPVL2_VERSION_STRING;

static inline float spline1(const float t, const float p1, const float p2)
{
    return ((1 + 3 * p1 - 3 * p2) * t * t * t + (3 * p2 - 6 * p1) * t * t + 3 * p1 * t);
}

static inline float spline2(const float t, const float p1, const float p2)
{
    return ((3 + 9 * p1 - 9 * p2) * t * t + (6 * p2 - 12 * p1) * t + 3 * p1);
}

inline float lerp(float x, float y, float t)
{
    return x * (1.0f - t) + y * t;
}

inline uint8_t *copyBytes(uint8_t *dst, const uint8_t *src, size_t max)
{
    assert(dst != NULL && src != NULL && max > 0);
    uint8_t *ptr = static_cast<uint8_t *>(memcpy(dst, src, max));
    return ptr;
}

static inline void drain(size_t size, uint8_t *&ptr, size_t &rest)
{
    ptr += size;
    rest -= size;
}

static inline bool size8(uint8_t *&ptr, size_t &rest, size_t &size)
{
    assert(ptr);
    if (sizeof(uint8_t) > rest)
        return false;
    size = *reinterpret_cast<uint8_t *>(ptr);
    drain(sizeof(uint8_t), ptr, rest);
    return true;
}

static inline bool size16(uint8_t *&ptr, size_t &rest, size_t &size)
{
    assert(ptr);
    if (sizeof(uint16_t) > rest)
        return false;
    size = *reinterpret_cast<uint16_t *>(ptr);
    drain(sizeof(uint16_t), ptr, rest);
    return true;
}

static inline bool size32(uint8_t *&ptr, size_t &rest, size_t &size)
{
    assert(ptr);
    if (sizeof(int) > rest)
        return false;
    size = *reinterpret_cast<int *>(ptr);
    drain(sizeof(int), ptr, rest);
    return true;
}

static inline bool sizeText(uint8_t *&ptr, size_t &rest, uint8_t *&text, size_t &size)
{
    assert(ptr);
    if (!internal::size32(ptr, rest, size) || size > rest)
        return false;
    text = ptr;
    internal::drain(size, ptr, rest);
    return true;
}

static inline bool validateSize(uint8_t *&ptr, size_t stride, size_t size, size_t &rest)
{
    assert(ptr);
    size_t required = stride * size;
    if (required > rest)
        return false;
    internal::drain(size, ptr, rest);
    return true;
}

static inline bool validateSize(uint8_t *&ptr, size_t stride, size_t &rest)
{
    return validateSize(ptr, 1, stride, rest);
}

static int variantIndex(uint8_t *&ptr, size_t size)
{
    int result = 0;
    switch (size) {
    case 1:
        result = *reinterpret_cast<int8_t *>(ptr);
        ptr += sizeof(int8_t);
        break;
    case 2:
        result = *reinterpret_cast<int16_t *>(ptr);
        ptr += sizeof(int16_t);
        break;
    case 4:
        result = *reinterpret_cast<int *>(ptr);
        ptr += sizeof(int);
        break;
    default:
        assert(0);
    }
    return result;
}

static int variantIndexUnsigned(uint8_t *&ptr, size_t size)
{
    int result = 0;
    switch (size) {
    case 1:
        result = *reinterpret_cast<uint8_t *>(ptr);
        ptr += sizeof(uint8_t);
        break;
    case 2:
        result = *reinterpret_cast<uint16_t *>(ptr);
        ptr += sizeof(uint16_t);
        break;
    case 4:
        result = *reinterpret_cast<int *>(ptr);
        ptr += sizeof(int);
        break;
    default:
        assert(0);
    }
    return result;
}

static void inline setPosition(const float *input, Vector3 &output)
{
#ifdef VPVL2_COORDINATE_OPENGL
    output.setValue(input[0], input[1], -input[2]);
#else
    output.setValue(input[0], input[1], input[2]);
#endif
}

static void inline setRotation(const float *input, Quaternion &output)
{
#ifdef VPVL2_COORDINATE_OPENGL
    output.setValue(input[0], -input[1], -input[2], input[3]);
#else
    output.setValue(input[0], input[1], input[2], input[3]);
#endif
}

static void buildInterpolationTable(float x1, float x2, float y1, float y2, int size, float *&table)
{
    assert(table && size > 0);
    for (int i = 0; i < size; i++) {
        const float in = static_cast<const float>(i) / size;
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

static inline bool stringEquals(const uint8_t *s1, const uint8_t *s2, size_t max)
{
    assert(s1 && s2);
    return strncmp(reinterpret_cast<const char *>(s1), reinterpret_cast<const char *>(s2), max) == 0;
}

static inline bool stringEquals(const char *s1, const char *s2, size_t max)
{
    assert(s1 && s2);
    return strncmp(s1, s2, max) == 0;
}

static inline char *stringToken(char *str, const char *delim, char **ptr)
{
    assert(delim);
#if defined(__MINGW32__)
    return strtok(str, delim);
#elif defined(WIN32)
    return strtok_s(str, delim, ptr);
#else
    return strtok_r(str, delim, ptr);
#endif
}

static inline int stringToInt(const char *str)
{
    assert(str);
    return atoi(str);
}

static inline float stringToFloat(const char *str)
{
    assert(str);
    char *p = 0;
#if defined(WIN32)
    return static_cast<float>(strtod(str, &p));
#else
    return strtof(str, &p);
#endif
}

static inline void zerofill(void *ptr, size_t size)
{
    assert(ptr && size > 0);
#if defined(WIN32) && !defined(__MINGW32__)
    SecureZeroMemory(ptr, size);
#else
    memset(ptr, 0, size);
#endif
}

static inline int snprintf(uint8_t *buffer, size_t size, const char *format, ...)
{
    assert(buffer && size > 0);
    va_list ap;
    va_start(ap, format);
    int ret = vsnprintf(reinterpret_cast<char *>(buffer), size, format, ap);
    va_end(ap);
    return ret;
}

}
}

#endif

