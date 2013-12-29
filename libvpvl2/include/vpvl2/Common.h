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
#ifndef VPVL2_COMMON_H_
#define VPVL2_COMMON_H_

#include "vpvl2/config.h"

#include <cstddef>
#include <LinearMath/btAlignedObjectArray.h>
#include <LinearMath/btHashMap.h>
#include <LinearMath/btQuaternion.h>
#include <LinearMath/btTransform.h>
#include <LinearMath/btVector3.h>

namespace vpvl2
{

typedef ::int8_t       int8;
typedef ::uint8_t     uint8;
typedef ::int16_t     int16;
typedef ::uint16_t   uint16;
typedef ::int32_t     int32;
typedef ::uint32_t   uint32;
typedef ::int64_t     int64;
typedef ::uint64_t   uint64;
typedef std::size_t   vsize;
typedef float       float32;
typedef double      float64;

typedef btScalar         Scalar;
typedef btVector3       Vector3;
typedef btVector4       Vector4;
typedef btQuadWord     QuadWord;
typedef btQuaternion Quaternion;
typedef btMatrix3x3   Matrix3x3;
typedef btTransform   Transform;
typedef btHashInt       HashInt;
typedef btHashPtr       HashPtr;
typedef Vector4           Color;

struct HashString : public btHashString {
    HashString() : btHashString(0) {}
    HashString(const char *value) : btHashString(value) {}
};

static const Vector3 kZeroV3 = Vector3(0, 0, 0);
static const Vector3 kUnitX = Vector3(1, 0, 0);
static const Vector3 kUnitY = Vector3(0, 1, 0);
static const Vector3 kUnitZ = Vector3(0, 0, 1);
static const Vector4 kZeroV4 = Vector4(0, 0, 0, 0);
static const Color kZeroC = Vector4(0, 0, 0, 1);

template<typename T>
class Array {
public:
    Array() {}
    virtual ~Array() {}

    inline void append(const T &item) {
        m_values.push_back(item);
    }
    inline const T &at(int i) const VPVL2_DECL_NOEXCEPT {
        return m_values.at(i);
    }
    inline T &at(int i) VPVL2_DECL_NOEXCEPT {
        return m_values.at(i);
    }
    inline void clear() {
        m_values.clear();
    }
    inline void copy(const Array &items) {
        m_values.copyFromArray(items.m_values);
    }
    inline int count() const VPVL2_DECL_NOEXCEPT {
        return m_values.size();
    }
    inline void releaseAll() {
        typedef char type_check_must_be_complete_this[sizeof(T) ? 1 : -1];
        (void) sizeof(type_check_must_be_complete_this);
        const int size = m_values.size();
        for (int i = 0; i < size; i++) {
            delete m_values[i];
        }
        m_values.clear();
    }
    inline void releaseArrayAll() {
        typedef char s[sizeof(T) ? 1 : -1]; (void) sizeof(s);
        const int size = m_values.size();
        for (int i = 0; i < size; i++) {
            delete[] m_values[i];
        }
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
    inline const T &operator[](int i) const VPVL2_DECL_NOEXCEPT {
        return m_values[i];
    }
    inline T &operator[](int i) VPVL2_DECL_NOEXCEPT {
        return m_values[i];
    }

private:
    btAlignedObjectArray<T> m_values;
    VPVL2_DISABLE_COPY_AND_ASSIGN(Array)
};

template<typename T>
class PointerArray : public Array<T *> {
public:
    PointerArray()
        : Array<T *>(),
          m_released(true)
    {
    }
    ~PointerArray() {
        VPVL2_DCHECK(m_released);
    }

    template<typename T2>
    inline T2 *append(T2 *item) {
        Array<T *>::append(item);
        m_released = false;
        return item;
    }
    inline void remove(T *item) {
        Array<T *>::remove(item);
        m_released = Array<T *>::count() == 0;
    }
    inline void removeAt(int index) {
        Array<T *>::removeAt(index);
        m_released = Array<T *>::count() == 0;
    }
    inline void releaseAll() {
        Array<T *>::releaseAll();
        m_released = true;
    }
    inline void releaseArrayAll() {
        Array<T *>::releaseArrayAll();
        m_released = true;
    }

private:
    bool m_released;
    VPVL2_DISABLE_COPY_AND_ASSIGN(PointerArray)
};

template<typename K, typename V>
class Hash {
public:
    Hash() {}
    ~Hash() {}

    inline void clear() {
        m_values.clear();
    }
    inline int count() const VPVL2_DECL_NOEXCEPT {
        return m_values.size();
    }
    inline const V *find(const K &key) const VPVL2_DECL_NOEXCEPT {
        return m_values.find(key);
    }
    inline void insert(const K &key, const V &value) {
        m_values.insert(key, value);
    }
    inline void releaseAll() {
        typedef char type_check_must_be_complete_this[sizeof(V) ? 1 : -1];
        (void) sizeof(type_check_must_be_complete_this);
        const int nNodes = m_values.size();
        for (int i = 0; i < nNodes; i++) {
            delete *m_values.getAtIndex(i);
        }
        m_values.clear();
    }
    inline void remove(const K &key) {
        m_values.remove(key);
    }
    inline V *value(int index) VPVL2_DECL_NOEXCEPT {
        return m_values.getAtIndex(index);
    }
    inline const V *value(int index) const VPVL2_DECL_NOEXCEPT {
        return m_values.getAtIndex(index);
    }
    inline V *operator[](const K &key) VPVL2_DECL_NOEXCEPT {
        return m_values[key];
    }

private:
    btHashMap<K, V> m_values;
};

template<typename K, typename V>
class PointerHash : public Hash<K, V *> {
public:
    PointerHash()
        : m_released(true)
    {
    }
    ~PointerHash() {
        VPVL2_DCHECK(m_released);
    }

    template<typename V2>
    inline V2 *insert(const K &key, V2 *value) {
        Hash<K, V *>::insert(key, value);
        m_released = false;
        return value;
    }
    inline void remove(const K &key) {
        Hash<K, V *>::remove(key);
        m_released = Hash<K, V *>::count() == 0;
    }
    inline void releaseAll() {
        Hash<K, V *>::releaseAll();
        m_released = true;
    }
    inline void releaseArrayAll() {
        Hash<K, V *>::releaseArrayAll();
        m_released = true;
    }

private:
    bool m_released;
    VPVL2_DISABLE_COPY_AND_ASSIGN(PointerHash)
};

/**
 * Get whether current library version is compatible with specified version.
 *
 * A first argument should be passed VPVL2_VERSION macro.
 *
 * @param version A version of compiled version (VPVL2_VERSION)
 * @return True if the version is compatible to work
 */
VPVL2_API bool isLibraryVersionCorrect(int version) VPVL2_DECL_NOEXCEPT;

/**
 * Get current version string of this library.
 *
 * @return A string of current version
 */
VPVL2_API const char *libraryVersionString() VPVL2_DECL_NOEXCEPT;

VPVL2_API const char *libraryCommitRevisionString() VPVL2_DECL_NOEXCEPT;

VPVL2_API void installLogger(const char *argv0, const char *logdir, int vlog);

VPVL2_API void uninstallLogger();

} /* namespace vpvl2 */

#endif
