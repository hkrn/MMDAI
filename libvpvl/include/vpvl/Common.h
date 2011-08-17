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

#ifndef VPVL_COMMON_H_
#define VPVL_COMMON_H_

#include "vpvl/config.h"

#if defined(WIN32) && !(defined(__MINGW__) || defined(__MINGW32__))
typedef char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
#else
#include <stdint.h>
#endif

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include <LinearMath/btAlignedObjectArray.h>
#include <LinearMath/btHashMap.h>
#include <LinearMath/btQuaternion.h>
#include <LinearMath/btTransform.h>
#include <LinearMath/btVector3.h>

#if defined (WIN32)
  #if defined(vpvl_EXPORTS)
    #define VPVL_EXPORT __declspec(dllexport)
  #else
    #define VPVL_EXPORT __declspec(dllimport)
  #endif /* MyLibrary_EXPORTS */
#else /* defined (_WIN32) */
 #define VPVL_EXPORT
#endif

#define VPVL_DISABLE_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName &); \
    void operator=(const TypeName &);

namespace vpvl
{

template<typename T>
class Array
{
public:
    Array() {
    }
    ~Array() {
    }

    void add(const T &item) {
        m_values.push_back(item);
    }
    const T &at(uint32_t i) const {
        return m_values.at(i);
    }
    T &at(uint32_t i) {
        return m_values.at(i);
    }
    void clear() {
        uint32_t size = m_values.size();
        for (uint32_t i = 0; i < size; i++)
            delete m_values[i];
        m_values.clear();
    }
    void clearArray() {
        uint32_t size = m_values.size();
        for (uint32_t i = 0; i < size; i++)
            delete[] m_values[i];
        m_values.clear();
    }
    void copy(const Array &items) {
        m_values.copyFromArray(items.m_values);
    }
    uint32_t count() const {
        return m_values.size();
    }
    void remove(const T &item) {
        m_values.remove(item);
    }
    void reserve(uint32_t size) {
        m_values.reserve(size);
    }
    template <typename Comparable>
    void sort(Comparable comparable) {
        m_values.quickSort(comparable);
    }
    const T &operator[](uint32_t i) const {
        return m_values[i];
    }
    T &operator[](uint32_t i) {
        return m_values[i];
    }

private:
    btAlignedObjectArray<T> m_values;
    VPVL_DISABLE_COPY_AND_ASSIGN(Array)
};

template<typename K, typename V>
class Hash
{
public:
    Hash() {
    }
    ~Hash() {
    }

    void clear() {
        uint32_t nNodes = m_values.size();
        for (uint32_t i = 0; i < nNodes; i++)
            delete *m_values.getAtIndex(i);
        m_values.clear();
    }
    uint32_t count() const {
        return m_values.size();
    }
    const V *find(const K &key) const {
        return m_values.find(key);
    }
    void insert(const K &key, const V &value) {
        m_values.insert(key, value);
    }
    const V *value(uint32_t index) const {
        return m_values.getAtIndex(index);
    }
    V *operator[](const K &key) {
        return m_values[key];
    }

private:
    btHashMap<K, V> m_values;
};
typedef btHashString HashString;

VPVL_EXPORT static const float kPI = 3.14159265358979323846f;

/**
 * Get whether current library version is compatible with specified version.
 *
 * A first argument should be passed VPVL_VERSION macro.
 *
 * @param A version of compiled version (VPVL_VERSION)
 * @return True if the version is compatible to work
 */
VPVL_EXPORT bool isLibraryVersionCorrect(int version);

/**
 * Get current version string of this library.
 *
 * @return A string of current version
 */
VPVL_EXPORT const char *libraryVersionString();

/**
 * Convert degree to radian.
 *
 * @param A value of degree
 * @return A value of converted radian from degree
 */
VPVL_EXPORT inline float radian(float value)
{
    return value * static_cast<float>(kPI / 180.0f);
}

/**
 * Convert radian to degree.
 *
 * @param A value of radian
 * @return A value of converted degree from radian
 */
VPVL_EXPORT inline float degree(float value)
{
    return value * static_cast<float>(180.0f / kPI);
}

/**
 * Copy string with terminated character like strlcpy.
 *
 * This function aims to copy stack based string from pointer based string.
 *
 * @param A value of string to be copied
 * @param A value of string to copy
 * @param A length of string to copy
 * @return A pointer of string to be copied
 */
VPVL_EXPORT inline uint8_t *copyBytesSafe(uint8_t *dst, const uint8_t *src, size_t max)
{
    assert(dst != NULL && src != NULL && max > 0);
    size_t len = max - 1;
    uint8_t *ptr = static_cast<uint8_t *>(memcpy(dst, src, len));
    dst[len] = '\0';
    return ptr;
}

}

#endif
