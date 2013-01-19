/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2013  hkrn                                    */
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
#ifndef VPVL2_COMMON_H_
#define VPVL2_COMMON_H_

#include "vpvl2/config.h"

#include <LinearMath/btAlignedObjectArray.h>
#include <LinearMath/btHashMap.h>
#include <LinearMath/btQuaternion.h>
#include <LinearMath/btTransform.h>
#include <LinearMath/btVector3.h>

namespace vpvl2
{

typedef btQuadWord QuadWord;
typedef btQuaternion Quaternion;
typedef btScalar Scalar;
typedef btMatrix3x3 Matrix3x3;
typedef btTransform Transform;
typedef btVector3 Vector3;
typedef btVector4 Vector4;
typedef btHashInt HashInt;
typedef btHashPtr HashPtr;
typedef Vector4 Color;

struct HashString : public btHashString {
    HashString() : btHashString(0) {}
    HashString(const char *value) : btHashString(value) {}
};

static const float kPI = 3.14159265358979323846f;
static const Vector3 kZeroV3 = Vector3(0, 0, 0);
static const Vector4 kZeroV4 = Vector4(0, 0, 0, 0);
static const Color kZeroC = Vector4(0, 0, 0, 1);

template<typename T>
class Array
{
public:
    Array() {
    }
    ~Array() {
    }

    inline void add(const T &item) {
        m_values.push_back(item);
    }
    inline const T &at(int i) const {
        return m_values.at(i);
    }
    inline T &at(int i) {
        return m_values.at(i);
    }
    inline void clear() {
        m_values.clear();
    }
    inline void copy(const Array &items) {
        m_values.copyFromArray(items.m_values);
    }
    inline int count() const {
        return m_values.size();
    }
    inline void releaseAll() {
        const int size = m_values.size();
        for (int i = 0; i < size; i++)
            delete m_values[i];
        m_values.clear();
    }
    inline void releaseArrayAll() {
        const int size = m_values.size();
        for (int i = 0; i < size; i++)
            delete[] m_values[i];
        m_values.clear();
    }
    inline void remove(const T &item) {
        m_values.remove(item);
    }
    inline void removeAt(int index) {
        m_values.swap(index, m_values.size() - 1);
        m_values.pop_back();
    }
    inline void reserve(int size) {
        m_values.reserve(size);
    }
    inline void resize(int size) {
        m_values.resize(size);
    }
    template <typename Comparable>
    inline void sort(Comparable comparable) {
        m_values.quickSort(comparable);
    }
    inline const T &operator[](int i) const {
        return m_values[i];
    }
    inline T &operator[](int i) {
        return m_values[i];
    }

private:
    btAlignedObjectArray<T> m_values;
    VPVL2_DISABLE_COPY_AND_ASSIGN(Array)
};

template<typename K, typename V>
class Hash
{
public:
    Hash() {
    }
    ~Hash() {
    }

    inline void clear() {
        m_values.clear();
    }
    inline int count() const {
        return m_values.size();
    }
    inline const V *find(const K &key) const {
        return m_values.find(key);
    }
    inline void insert(const K &key, const V &value) {
        m_values.insert(key, value);
    }
    inline void releaseAll() {
        const int nNodes = m_values.size();
        for (int i = 0; i < nNodes; i++)
            delete *m_values.getAtIndex(i);
        m_values.clear();
    }
    inline void remove(const K &key) {
        m_values.remove(key);
    }
    inline const V *value(int index) const {
        return m_values.getAtIndex(index);
    }
    inline V *operator[](const K &key) {
        return m_values[key];
    }

private:
    btHashMap<K, V> m_values;
};

/**
 * Get whether current library version is compatible with specified version.
 *
 * A first argument should be passed VPVL2_VERSION macro.
 *
 * @param version A version of compiled version (VPVL2_VERSION)
 * @return True if the version is compatible to work
 */
VPVL2_API bool isLibraryVersionCorrect(int version);

/**
 * Get current version string of this library.
 *
 * @return A string of current version
 */
VPVL2_API const char *libraryVersionString();

/**
 * Convert degree to radian.
 *
 * @param value A value of degree
 * @return A value of converted radian from degree
 */
VPVL2_API inline float radian(float value)
{
    return value * static_cast<float>(kPI / 180.0f);
}

/**
 * Convert radian to degree.
 *
 * @param value A value of radian
 * @return A value of converted degree from radian
 */
VPVL2_API inline float degree(float value)
{
    return value * static_cast<float>(180.0f / kPI);
}

} /* namespace vpvl2 */

#endif
