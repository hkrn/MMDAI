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

#pragma once
#ifndef VPVL2_INTERNAL_UTIL_H_
#define VPVL2_INTERNAL_UTIL_H_

#include "vpvl2/Common.h"
#include "vpvl2/IEncoding.h"
#include "vpvl2/IString.h"

#include <cstdio>
#include <cstring>
#include <cstdarg>

#ifndef __GNUC__
#define __attribute__(x)
#endif

#ifdef _MSC_VER
#include <windows.h>
#endif

namespace vpvl2
{
namespace internal
{

static const int kCurrentVersion = VPVL2_VERSION;
static const char *const kCurrentVersionString = VPVL2_VERSION_STRING;

static inline void zerofill(void *ptr, size_t size)
{
    VPVL2_DCHECK_NOTNULL(ptr);
    VPVL2_DCHECK_GT(size, size_t(0));
#if defined(_MSC_VER) && _MSC_VER < 1700
    SecureZeroMemory(ptr, size);
#else
    memset(ptr, 0, size);
#endif
}

static inline size_t estimateSize(const IString *string, IString::Codec codec)
{
    size_t value = sizeof(int32_t) + (string ? string->length(codec) : 0);
    return value;
}

static inline void setString(const IString *newValue, IString *&value)
{
    if (newValue && newValue != value) {
        delete value;
        value = newValue->clone();
    }
}

static inline void setStringDirect(IString *newValue, IString *&value)
{
    if (newValue && newValue != value) {
        delete value;
        value = newValue;
    }
}

static inline void toggleFlag(int value, bool enable, uint16_t &flags)
{
    if (enable) {
        flags |= value;
    }
    else {
        flags &= ~value;
    }
}

static inline bool hasFlagBits(int flags, int test)
{
    return (flags & test) == test;
}

template<typename T>
static inline bool checkBound(const T &value, const T &min, const T &max)
{
    return value >= min && value < max;
}

static inline const char *cstr(const IString *value, const char *defv)
{
    return value ? reinterpret_cast<const char *>(value->toByteArray()) : defv;
}

static inline void drainBytes(size_t size, uint8_t *&ptr, size_t &rest)
{
    if (rest >= size) {
        ptr += size;
        rest -= size;
    }
    else {
        VPVL2_LOG(ERROR, "Unexpected size required: size=" << size << " rest=" << rest);
    }
}

template<typename T>
static inline void getData(const uint8_t *ptr, T &output)
{
#ifdef VPVL2_BUILD_IOS
    memcpy(&output, ptr, sizeof(output));
#else
    output = *reinterpret_cast<const T *>(ptr);
#endif
}

template<typename T>
static inline bool getTyped(uint8_t *&ptr, size_t &rest, T &output)
{
    VPVL2_DCHECK_NOTNULL(ptr);
    if (sizeof(T) > rest) {
        return false;
    }
    else {
        getData(ptr, output);
        drainBytes(sizeof(T), ptr, rest);
        return true;
    }
}

static inline bool getText(uint8_t *&ptr, size_t &rest, uint8_t *&text, int32_t &size)
{
    VPVL2_DCHECK_NOTNULL(ptr);
    if (!getTyped<int32_t>(ptr, rest, size) || size_t(size) > rest) {
        return false;
    }
    else {
        text = ptr;
        drainBytes(size, ptr, rest);
        return true;
    }
}

static inline bool validateSize(uint8_t *&ptr, size_t stride, size_t size, size_t &rest)
{
    VPVL2_DCHECK_NOTNULL(ptr);
    size_t required = stride * size;
    if (required > rest) {
        return false;
    }
    else {
        drainBytes(required, ptr, rest);
        return true;
    }
}

static inline bool validateSize(uint8_t *&ptr, size_t stride, size_t &rest)
{
    VPVL2_DCHECK_NOTNULL(ptr);
    return validateSize(ptr, 1, stride, rest);
}

static inline int readSignedIndex(uint8_t *&ptr, size_t size)
{
    VPVL2_DCHECK_NOTNULL(ptr);
    int result = 0;
    switch (size) {
    case 1: {
        result = *reinterpret_cast<int8_t *>(ptr);
        ptr += sizeof(int8_t);
        break;
    }
    case 2: {
        result = *reinterpret_cast<int16_t *>(ptr);
        ptr += sizeof(int16_t);
        break;
    }
    case 4: {
        result = *reinterpret_cast<int *>(ptr);
        ptr += sizeof(int32_t);
        break;
    }
    default:
        break;
    }
    return result;
}

static inline int readUnsignedIndex(uint8_t *&ptr, size_t size)
{
    VPVL2_DCHECK_NOTNULL(ptr);
    int result = 0;
    switch (size) {
    case 1: {
        result = *reinterpret_cast<uint8_t *>(ptr);
        ptr += sizeof(uint8_t);
        break;
    }
    case 2: {
        result = *reinterpret_cast<uint16_t *>(ptr);
        ptr += sizeof(uint16_t);
        break;
    }
    case 4: {
        result = *reinterpret_cast<int *>(ptr);
        ptr += sizeof(int32_t);
        break;
    }
    default:
        assert(0);
    }
    return result;
}

static void inline setPosition(const float32_t *input, Vector3 &output)
{
    VPVL2_DCHECK_NOTNULL(input);
#ifdef VPVL2_COORDINATE_OPENGL
    output.setValue(input[0], input[1], -input[2]);
#else
    setPositionRaw(input, output);
#endif
}

static void inline setPositionRaw(const float32_t *input, Vector3 &output)
{
    VPVL2_DCHECK_NOTNULL(input);
    output.setValue(input[0], input[1], input[2]);
}

static void inline getPosition(const Vector3 &input, float32_t *output)
{
    VPVL2_DCHECK_NOTNULL(output);
    output[0] = input.x();
    output[1] = input.y();
#ifdef VPVL2_COORDINATE_OPENGL
    output[2] = -input.z();
#else
    output[2] = input.z();
#endif
}

static void inline getPositionRaw(const Vector3 &input, float32_t *output)
{
    VPVL2_DCHECK_NOTNULL(output);
    output[0] = input.x();
    output[1] = input.y();
    output[2] = input.z();
}

static void inline setRotation(const float32_t *input, Quaternion &output)
{
    VPVL2_DCHECK_NOTNULL(input);
#ifdef VPVL2_COORDINATE_OPENGL
    output.setValue(input[0], -input[1], -input[2], input[3]);
#else
    output.setValue(input[0], input[1], input[2], input[3]);
#endif
}

static void inline setRotation2(const float32_t *input, Quaternion &output)
{
    VPVL2_DCHECK_NOTNULL(input);
#ifdef VPVL2_COORDINATE_OPENGL
    output.setValue(-input[0], -input[1], input[2], input[3]);
#else
    output.setValue(input[0], input[1], input[2], input[3]);
#endif
}

static void inline getRotation(const Quaternion &input, float32_t *output)
{
    VPVL2_DCHECK_NOTNULL(output);
    output[0] = input.x();
#ifdef VPVL2_COORDINATE_OPENGL
    output[1] = -input.y();
    output[2] = -input.z();
#else
    output[1] = input.y();
    output[2] = input.z();
#endif
    output[3] = input.w();
}

static void inline getRotation2(const Quaternion &input, float32_t *output)
{
    VPVL2_DCHECK_NOTNULL(output);
#ifdef VPVL2_COORDINATE_OPENGL
    output[0] = -input.x();
    output[1] = -input.y();
#else
    output[0] = input.x();
    output[1] = input.y();
#endif
    output[2] = input.z();
    output[3] = input.w();
}

static void inline getColor(const Vector3 &input, float32_t *output)
{
    VPVL2_DCHECK_NOTNULL(output);
    output[0] = input.x();
    output[1] = input.y();
    output[2] = input.z();
}

static void inline getColor(const Vector4 &input, float32_t *output)
{
    VPVL2_DCHECK_NOTNULL(output);
    output[0] = input.x();
    output[1] = input.y();
    output[2] = input.z();
    output[3] = input.w();
}

static inline uint8_t *copyBytes(uint8_t *dst, const void *src, size_t max)
{
    VPVL2_DCHECK_NOTNULL(src);
    VPVL2_DCHECK_NOTNULL(dst);
    VPVL2_DCHECK_GT(max, size_t(0));
    uint8_t *ptr = static_cast<uint8_t *>(std::memcpy(dst, src, max));
    return ptr;
}

static inline void writeBytes(const void *src, size_t size, uint8_t *&dst)
{
    VPVL2_DCHECK_NOTNULL(src);
    VPVL2_DCHECK_NOTNULL(dst);
    copyBytes(dst, src, size);
    dst += size;
}

static inline void writeSignedIndex(int value, size_t size, uint8_t *&dst)
{
    VPVL2_DCHECK_NOTNULL(dst);
    switch (size) {
    case 1: {
        int8_t v = value;
        writeBytes(&v, sizeof(v), dst);
        break;
    }
    case 2: {
        int16_t v = value;
        writeBytes(&v, sizeof(v), dst);
        break;
    }
    case 4: {
        int32_t v = value;
        writeBytes(&v, sizeof(v), dst);
        break;
    }
    default:
        assert(0);
    }
}

static inline void writeUnsignedIndex(int value, size_t size, uint8_t *&dst)
{
    VPVL2_DCHECK_NOTNULL(dst);
    switch (size) {
    case 1: {
        uint8_t v = value;
        writeBytes(&v, sizeof(v), dst);
        break;
    }
    case 2: {
        uint16_t v = value;
        writeBytes(&v, sizeof(v), dst);
        break;
    }
    case 4: {
        int32_t v = value;
        writeBytes(&v, sizeof(v), dst);
        break;
    }
    default:
        assert(0);
    }
}

static inline void writeString(const IString *string, IString::Codec codec, uint8_t *&dst)
{
    VPVL2_DCHECK_NOTNULL(dst);
    int32_t s = string ? string->length(codec) : 0;
    writeBytes(&s, sizeof(s), dst);
    if (s > 0) {
        writeBytes(string->toByteArray(), s, dst);
    }
}

static inline void writeStringAsByteArray(const IString *string, IString::Codec codec, const IEncoding *encodingRef, size_t bufsiz, uint8_t *&dst)
{
    VPVL2_DCHECK_NOTNULL(encodingRef);
    VPVL2_DCHECK_NOTNULL(dst);
    VPVL2_DCHECK_GT(bufsiz, size_t(0));
    if (uint8_t *bytes = encodingRef->toByteArray(string, codec)) {
        zerofill(dst, bufsiz);
        writeBytes(bytes, bufsiz, dst);
        encodingRef->disposeByteArray(bytes);
    }
}

__attribute__((format(printf, 3, 4)))
static inline void snprintf(char *buf, size_t size, const char *format, ...)
{
    VPVL2_DCHECK_NOTNULL(buf);
    VPVL2_DCHECK_NOTNULL(format);
    VPVL2_DCHECK_GT(size, size_t(0));
    std::va_list ap;
    va_start(ap, format);
    std::vsnprintf(buf, size, format, ap);
    va_end(ap);
}

static inline void dump(const Vector3 &v)
{
    (void) v;
    VPVL2_DLOG(INFO, "Vector3(x=" << v.x() << ", y=" << v.y() << ", z=" << v.z() << ")");
}

static inline void dump(const Vector4 &v)
{
    (void) v;
    VPVL2_DLOG(INFO, "Vector4(x=" << v.x() << ", y=" << v.y() << ", z=" << v.z() << ", w=" << v.w() << ")");
}

static inline void dump(const Quaternion &v)
{
    (void) v;
    VPVL2_DLOG(INFO, "Quaternion(x=" << v.x() << ", y=" << v.y() << ", z=" << v.z() << ", w=" << v.w() << ")");
}

} /* namespace internal */
} /* namespace vpvl2 */

#endif
