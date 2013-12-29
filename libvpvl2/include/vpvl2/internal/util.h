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

#ifdef VPVL2_OS_WINDOWS
#include <windows.h>
#endif

#define VPVL2_TRIGGER_PROPERTY_EVENTS(events, expr) do { \
    const int nevents = events.count(); \
    for (int i = 0; i < nevents; i++) { \
      PropertyEventListener *event = events[i]; \
      event->expr; \
    } \
  } while (0)

namespace vpvl2
{
namespace internal
{

static const int kCurrentVersion = VPVL2_VERSION;
static const char *const kCurrentVersionString = VPVL2_VERSION_STRING;

template<typename T>
static inline void deleteObject(T *&object)
{
    delete object;
    object = 0;
}

template<typename T>
static inline void deleteObjectArray(T *&object)
{
    delete[] object;
    object = 0;
}

static inline void zerofill(void *ptr, vsize size)
{
    VPVL2_DCHECK_NOTNULL(ptr);
    VPVL2_DCHECK_GT(size, vsize(0));
#if defined(_MSC_VER) && _MSC_VER < 1700
    SecureZeroMemory(ptr, size);
#else
    std::memset(ptr, 0, size);
#endif
}

static inline vsize estimateSize(const IString *string, IString::Codec codec)
{
    vsize value = sizeof(int32) + (string ? string->length(codec) : 0);
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

static inline void toggleFlag(int value, bool enable, uint16 &flags)
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

static inline void drainBytes(vsize size, uint8 *&ptr, vsize &rest)
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
static inline void getData(const uint8 *ptr, T &output)
{
#ifdef VPVL2_BUILD_IOS
    memcpy(&output, ptr, sizeof(output));
#else
    output = *reinterpret_cast<const T *>(ptr);
#endif
}

template<typename T>
static inline bool getTyped(uint8 *&ptr, vsize &rest, T &output)
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

static inline bool getText(uint8 *&ptr, vsize &rest, uint8 *&text, int32 &size)
{
    VPVL2_DCHECK_NOTNULL(ptr);
    if (!getTyped<int32>(ptr, rest, size) || vsize(size) > rest) {
        return false;
    }
    else {
        text = ptr;
        drainBytes(size, ptr, rest);
        return true;
    }
}

static inline bool validateSize(uint8 *&ptr, vsize stride, vsize size, vsize &rest)
{
    VPVL2_DCHECK_NOTNULL(ptr);
    vsize required = stride * size;
    if (required > rest) {
        return false;
    }
    else {
        drainBytes(required, ptr, rest);
        return true;
    }
}

static inline bool validateSize(uint8 *&ptr, vsize stride, vsize &rest)
{
    VPVL2_DCHECK_NOTNULL(ptr);
    return validateSize(ptr, 1, stride, rest);
}

static inline int readSignedIndex(uint8 *&ptr, vsize size)
{
    VPVL2_DCHECK_NOTNULL(ptr);
    int result = 0;
    switch (size) {
    case 1: {
        result = *reinterpret_cast<int8 *>(ptr);
        ptr += sizeof(int8);
        break;
    }
    case 2: {
        result = *reinterpret_cast<int16 *>(ptr);
        ptr += sizeof(int16);
        break;
    }
    case 4: {
        result = *reinterpret_cast<int *>(ptr);
        ptr += sizeof(int32);
        break;
    }
    default:
        break;
    }
    return result;
}

static inline int readUnsignedIndex(uint8 *&ptr, vsize size)
{
    VPVL2_DCHECK_NOTNULL(ptr);
    int result = 0;
    switch (size) {
    case 1: {
        result = *reinterpret_cast<uint8 *>(ptr);
        ptr += sizeof(uint8);
        break;
    }
    case 2: {
        result = *reinterpret_cast<uint16 *>(ptr);
        ptr += sizeof(uint16);
        break;
    }
    case 4: {
        result = *reinterpret_cast<int *>(ptr);
        ptr += sizeof(int32);
        break;
    }
    default:
        assert(0);
    }
    return result;
}

static void inline setPositionRaw(const float32 *input, Vector3 &output)
{
    VPVL2_DCHECK_NOTNULL(input);
    output.setValue(input[0], input[1], input[2]);
}

static void inline setPosition(const float32 *input, Vector3 &output)
{
    VPVL2_DCHECK_NOTNULL(input);
#ifdef VPVL2_COORDINATE_OPENGL
    output.setValue(input[0], input[1], -input[2]);
#else
    setPositionRaw(input, output);
#endif
}

static void inline getPosition(const Vector3 &input, float32 *output)
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

static void inline getPositionRaw(const Vector3 &input, float32 *output)
{
    VPVL2_DCHECK_NOTNULL(output);
    output[0] = input.x();
    output[1] = input.y();
    output[2] = input.z();
}

static void inline setRotation(const float32 *input, Quaternion &output)
{
    VPVL2_DCHECK_NOTNULL(input);
#ifdef VPVL2_COORDINATE_OPENGL
    output.setValue(input[0], -input[1], -input[2], input[3]);
#else
    output.setValue(input[0], input[1], input[2], input[3]);
#endif
}

static void inline setRotation2(const float32 *input, Quaternion &output)
{
    VPVL2_DCHECK_NOTNULL(input);
#ifdef VPVL2_COORDINATE_OPENGL
    output.setValue(-input[0], -input[1], input[2], input[3]);
#else
    output.setValue(input[0], input[1], input[2], input[3]);
#endif
}

static void inline getRotation(const Quaternion &input, float32 *output)
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

static void inline getRotation2(const Quaternion &input, float32 *output)
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

static void inline getColor(const Vector3 &input, float32 *output)
{
    VPVL2_DCHECK_NOTNULL(output);
    output[0] = input.x();
    output[1] = input.y();
    output[2] = input.z();
}

static void inline getColor(const Vector4 &input, float32 *output)
{
    VPVL2_DCHECK_NOTNULL(output);
    output[0] = input.x();
    output[1] = input.y();
    output[2] = input.z();
    output[3] = input.w();
}

static inline uint8 *copyBytes(uint8 *dst, const void *src, vsize max)
{
    VPVL2_DCHECK_NOTNULL(src);
    VPVL2_DCHECK_NOTNULL(dst);
    VPVL2_DCHECK_GT(max, vsize(0));
    uint8 *ptr = static_cast<uint8 *>(std::memcpy(dst, src, max));
    return ptr;
}

static inline void writeBytes(const void *src, vsize size, uint8 *&dst)
{
    VPVL2_DCHECK_NOTNULL(src);
    VPVL2_DCHECK_NOTNULL(dst);
    copyBytes(dst, src, size);
    dst += size;
}

static inline void writeSignedIndex(int value, vsize size, uint8 *&dst)
{
    VPVL2_DCHECK_NOTNULL(dst);
    switch (size) {
    case 1: {
        int8 v = value;
        writeBytes(&v, sizeof(v), dst);
        break;
    }
    case 2: {
        int16 v = value;
        writeBytes(&v, sizeof(v), dst);
        break;
    }
    case 4: {
        int32 v = value;
        writeBytes(&v, sizeof(v), dst);
        break;
    }
    default:
        assert(0);
    }
}

static inline void writeUnsignedIndex(int value, vsize size, uint8 *&dst)
{
    VPVL2_DCHECK_NOTNULL(dst);
    switch (size) {
    case 1: {
        uint8 v = value;
        writeBytes(&v, sizeof(v), dst);
        break;
    }
    case 2: {
        uint16 v = value;
        writeBytes(&v, sizeof(v), dst);
        break;
    }
    case 4: {
        int32 v = value;
        writeBytes(&v, sizeof(v), dst);
        break;
    }
    default:
        assert(0);
    }
}

static inline void writeString(const IString *string, IString::Codec codec, uint8 *&dst)
{
    VPVL2_DCHECK_NOTNULL(dst);
    int32 s = string ? int32(string->length(codec)) : 0;
    writeBytes(&s, sizeof(s), dst);
    if (s > 0) {
        writeBytes(string->toByteArray(), s, dst);
    }
}

static inline void writeStringAsByteArray(const IString *string, IString::Codec codec, const IEncoding *encodingRef, vsize bufsiz, uint8 *&dst)
{
    VPVL2_DCHECK_NOTNULL(encodingRef);
    VPVL2_DCHECK_NOTNULL(dst);
    VPVL2_DCHECK_GT(bufsiz, vsize(0));
    if (uint8 *bytes = encodingRef->toByteArray(string, codec)) {
        zerofill(dst, bufsiz);
        writeBytes(bytes, bufsiz, dst);
        encodingRef->disposeByteArray(bytes);
    }
}

__attribute__((format(printf, 3, 4)))
static inline void snprintf(char *buf, vsize size, const char *format, ...)
{
    VPVL2_DCHECK_NOTNULL(buf);
    VPVL2_DCHECK_NOTNULL(format);
    VPVL2_DCHECK_GT(size, vsize(0));
    std::va_list ap;
    va_start(ap, format);
#if defined(VPVL2_OS_WINDOWS) || defined(VPVL2_OS_ANDROID)
    ::vsnprintf(buf, size, format, ap);
#else
    std::vsnprintf(buf, size, format, ap);
#endif
    va_end(ap);
}

static inline int memcmp(const void *left, const void *right, size_t size)
{
#if defined(VPVL2_OS_WINDOWS)
    return ::memcmp(left, right, size);
#else
    return std::memcmp(left, right, size);
#endif
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
