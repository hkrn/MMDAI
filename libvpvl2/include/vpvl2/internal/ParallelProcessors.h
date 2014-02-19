/**

 Copyright (c) 2010-2014  hkrn

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
#ifndef VPVL2_INTERNAL_PARALLELPROCESSORS_H_
#define VPVL2_INTERNAL_PARALLELPROCESSORS_H_

#include <vpvl2/Common.h>
#include <vpvl2/IMaterial.h>

#ifdef VPVL2_LINK_INTEL_TBB
#include <tbb/tbb.h>
#endif

namespace vpvl2
{
namespace VPVL2_VERSION_NS
{
namespace internal
{

static const Vector3 kAabbMin = Vector3(SIMD_INFINITY, SIMD_INFINITY, SIMD_INFINITY);
static const Vector3 kAabbMax = Vector3(-SIMD_INFINITY, -SIMD_INFINITY, -SIMD_INFINITY);

static inline bool GreaterOMP(const Vector3 &left, const Vector3 &right) VPVL2_DECL_NOEXCEPT
{
    return left.x() < right.x() ||  left.y() < right.y() || left.z() < right.z();
}

static inline bool LessOMP(const Vector3 &left, const Vector3 &right) VPVL2_DECL_NOEXCEPT
{
    return left.x() > right.x() ||  left.y() > right.y() || left.z() > right.z();
}

template<typename TModel, typename TVertex, typename TUnit>
class ParallelSkinningVertexProcessor VPVL2_DECL_FINAL {
public:
    ParallelSkinningVertexProcessor(const TModel *modelRef,
                                    const Array<TVertex *> *verticesRef,
                                    const Vector3 &cameraPosition,
                                    void *address)
        : m_verticesRef(verticesRef),
          m_edgeScaleFactor(modelRef->edgeScaleFactor(cameraPosition)),
          m_bufferPtr(static_cast<TUnit *>(address))
    {
    }
    ~ParallelSkinningVertexProcessor() {
        m_verticesRef = 0;
        m_bufferPtr = 0;
    }

    inline void performTransform(int i, Vector3 &position) const {
        const TVertex *vertex = m_verticesRef->at(i);
        const IMaterial *material = vertex->materialRef();
        const float materialEdgeSize = material->edgeSize() * m_edgeScaleFactor;
        TUnit &v = m_bufferPtr[i];
        v.performTransform(vertex, materialEdgeSize, position);
    }
#ifdef VPVL2_LINK_INTEL_TBB
    void operator()(const tbb::blocked_range<int> &range) const {
        Vector3 position;
        for (int i = range.begin(); i != range.end(); ++i) {
            performTransform(i, position);
        }
    }
#endif /* VPVL2_LINK_INTEL_TBB */
    void execute(bool enableParallel) {
        const int nvertices = m_verticesRef->count();
#if defined(VPVL2_LINK_INTEL_TBB)
        if (enableParallel) {
            static tbb::affinity_partitioner affinityPartitioner;
            tbb::parallel_for(tbb::blocked_range<int>(0, nvertices), *this, affinityPartitioner);
        }
        else {
#else
        {
            (void) enableParallel;
#endif
            Vector3 position;
#ifdef VPVL2_ENABLE_OPENMP
#pragma omp parallel for
#endif
            for (int i = 0; i < nvertices; ++i) {
                performTransform(i, position);
            }
        }
    }

private:
    const Array<TVertex *> *m_verticesRef;
    const IVertex::EdgeSizePrecision m_edgeScaleFactor;
    TUnit *m_bufferPtr;
};

template<typename TModel, typename TVertex, typename TUnit>
class ParallelBindPoseVertexProcessor VPVL2_DECL_FINAL {
public:
    ParallelBindPoseVertexProcessor(const Array<TVertex *> *verticesRef,
                                    void *address)
        : m_verticesRef(verticesRef),
          m_bufferPtr(static_cast<TUnit *>(address))
    {
    }
    ~ParallelBindPoseVertexProcessor() {
        m_verticesRef = 0;
        m_bufferPtr = 0;
    }

    inline void performTransform(int index) const VPVL2_DECL_NOEXCEPT {
        const TVertex *vertex = m_verticesRef->at(index);
        TUnit &v = m_bufferPtr[index];
        v.initialize(vertex);
    }
#ifdef VPVL2_LINK_INTEL_TBB
    void operator()(const tbb::blocked_range<int> &range) const {
        for (int i = range.begin(); i != range.end(); ++i) {
            performTransform(i);
        }
    }
#endif /* VPVL2_LINK_INTEL_TBB */
    void execute(bool enableParallel) {
        const int nvertices = m_verticesRef->count();
#if defined(VPVL2_LINK_INTEL_TBB)
        if (enableParallel) {
            static tbb::affinity_partitioner affinityPartitioner;
            tbb::parallel_for(tbb::blocked_range<int>(0, nvertices), *this, affinityPartitioner);
        }
        else {
#else
        {
            (void) enableParallel;
#endif
#ifdef VPVL2_ENABLE_OPENMP
#pragma omp parallel for
#endif
            for (int i = 0; i < nvertices; ++i) {
                performTransform(i);
            }
        }
    }

private:
    const Array<TVertex *> *m_verticesRef;
    TUnit *m_bufferPtr;
};

template<typename TModel, typename TVertex, typename TUnit>
class ParallelVertexMorphProcessor VPVL2_DECL_FINAL {
public:
    ParallelVertexMorphProcessor(const Array<TVertex *> *verticesRef,
                                 void *address)
        : m_verticesRef(verticesRef),
          m_bufferPtr(static_cast<TUnit *>(address))
    {
    }
    ~ParallelVertexMorphProcessor() {
        m_verticesRef = 0;
        m_bufferPtr = 0;
    }

    inline void performTransform(int index) const VPVL2_DECL_NOEXCEPT {
        const TVertex *vertex = m_verticesRef->at(index);
        TUnit &v = m_bufferPtr[index];
        v.setPosition(vertex);
    }
#ifdef VPVL2_LINK_INTEL_TBB
    void operator()(const tbb::blocked_range<int> &range) const {
        for (int i = range.begin(); i != range.end(); ++i) {
            performTransform(i);
        }
    }
#endif /* VPVL2_LINK_INTEL_TBB */
    void execute(bool enableParallel) {
        const int nvertices = m_verticesRef->count();
#if defined(VPVL2_LINK_INTEL_TBB)
        if (enableParallel) {
            static tbb::affinity_partitioner affinityPartitioner;
            tbb::parallel_for(tbb::blocked_range<int>(0, nvertices), *this, affinityPartitioner);
        }
        else {
#else
        {
            (void) enableParallel;
#endif
#ifdef VPVL2_ENABLE_OPENMP
#pragma omp parallel for
#endif
            for (int i = 0; i < nvertices; ++i) {
                performTransform(i);
            }
        }
    }

private:
    const Array<TVertex *> *m_verticesRef;
    TUnit *m_bufferPtr;
};

template<typename TVertex>
class ParallelResetVertexProcessor VPVL2_DECL_FINAL {
public:
    ParallelResetVertexProcessor(const Array<TVertex *> *verticesRef)
        : m_verticesRef(verticesRef)
    {
    }
    ~ParallelResetVertexProcessor() {
        m_verticesRef = 0;
    }

    inline void performTransform(int index) const VPVL2_DECL_NOEXCEPT {
        TVertex *vertex = m_verticesRef->at(index);
        vertex->reset();
    }
#ifdef VPVL2_LINK_INTEL_TBB
    void operator()(const tbb::blocked_range<int> &range) const {
        for (int i = range.begin(); i != range.end(); ++i) {
            performTransform(i);
        }
    }
#endif /* VPVL2_LINK_INTEL_TBB */

    void execute() {
        const int nvertices = m_verticesRef->count();
#if defined(VPVL2_LINK_INTEL_TBB)
        static tbb::affinity_partitioner affinityPartitioner;
        tbb::parallel_for(tbb::blocked_range<int>(0, nvertices), *this, affinityPartitioner);
#else
#ifdef VPVL2_ENABLE_OPENMP
#pragma omp parallel for
#endif
        for (int i = 0; i < nvertices; ++i) {
            performTransform(i);
        }
#endif
    }

private:
    const Array<TVertex *> *m_verticesRef;
};

template<typename TBone>
class ParallelUpdateLocalTransformProcessor VPVL2_DECL_FINAL {
public:
    ParallelUpdateLocalTransformProcessor(Array<TBone *> *bonesRef)
        : m_boneRefs(bonesRef)
    {
    }
    ~ParallelUpdateLocalTransformProcessor() {
        m_boneRefs = 0;
    }

    inline void performTransform(int index) const VPVL2_DECL_NOEXCEPT {
        TBone *bone = m_boneRefs->at(index);
        bone->updateLocalTransform();
    }
#ifdef VPVL2_LINK_INTEL_TBB
    void operator()(const tbb::blocked_range<int> &range) const {
        for (int i = range.begin(); i != range.end(); ++i) {
            performTransform(i);
        }
    }
#endif

    void execute() const {
        const int nbones = m_boneRefs->count();
#ifdef VPVL2_LINK_INTEL_TBB
        static tbb::affinity_partitioner partitioner;
        tbb::parallel_for(tbb::blocked_range<int>(0, nbones), *this, partitioner);
#else
#ifdef VPVL2_ENABLE_OPENMP
#pragma omp parallel for
#endif
        for (int i = 0; i < nbones; i++) {
            performTransform(i);
        }
#endif /* VPVL2_LINK_INTEL_TBB */
    }

private:
    mutable Array<TBone *> *m_boneRefs;
};

template<typename TRigidBody>
class ParallelUpdateRigidBodyProcessor VPVL2_DECL_FINAL {
public:
    ParallelUpdateRigidBodyProcessor(Array<TRigidBody *> *rigidBodyRefs)
        : m_rigidBodyRefs(rigidBodyRefs)
    {
    }
    ~ParallelUpdateRigidBodyProcessor() {
        m_rigidBodyRefs = 0;
    }

    inline void performTransform(int index) const VPVL2_DECL_NOEXCEPT {
        TRigidBody *body = m_rigidBodyRefs->at(index);
        body->syncLocalTransform();
    }
#ifdef VPVL2_LINK_INTEL_TBB
    void operator()(const tbb::blocked_range<int> &range) const {
        for (int i = range.begin(); i != range.end(); ++i) {
            performTransform(i);
        }
    }
#endif
    void execute() const {
        const int numRigidBodies = m_rigidBodyRefs->count();
#ifdef VPVL2_LINK_INTEL_TBB
        tbb::parallel_for(tbb::blocked_range<int>(0, numRigidBodies), *this);
#else /* VPVL2_LINK_INTEL_TBB */
#ifdef VPVL2_ENABLE_OPENMP
#pragma omp parallel for
#endif
        for (int i = 0; i < numRigidBodies; i++) {
            performTransform(i);
        }
#endif /* VPVL2_LINK_INTEL_TBB */
    }

private:
    mutable Array<TRigidBody *> *m_rigidBodyRefs;
};

template<typename TMaterial, typename TUnit>
class ParallelCalcAabbProcessor VPVL2_DECL_FINAL {
public:
    ParallelCalcAabbProcessor(const Array<TMaterial *> *materials, Array<Vector3> *value, const void *address)
        : m_materials(materials),
          m_bufferRef(static_cast<const TUnit *>(address)),
          m_aabb(value)
    {
    }
    ~ParallelCalcAabbProcessor() {
        m_bufferRef = 0;
    }

    static inline void performTransform(int i, const TUnit *bufferRef, Vector3 &min, Vector3 &max) {
        const TUnit &v = bufferRef[i];
        const Vector3 &position = v.position;
        min.setMin(position);
        max.setMax(position);
    }

#ifdef VPVL2_LINK_INTEL_TBB
    struct MaterialAabb {
        const TUnit *bufferRef;
        Vector3 min;
        Vector3 max;
        MaterialAabb()
            : bufferRef(0),
              min(kAabbMin),
              max(kAabbMax)
        {
        }
        MaterialAabb(const MaterialAabb &self, tbb::split /* split */)
            : bufferRef(self.bufferRef),
              min(self.min),
              max(self.max)
        {
        }
        void join(const MaterialAabb &self) VPVL2_DECL_NOEXCEPT {
            min.setMin(self.min);
            max.setMax(self.max);
        }
        void operator()(const tbb::blocked_range<int> &range) {
            Vector3 aabbMin(kAabbMin), aabbMax(kAabbMax);
            for (int i = range.begin(); i != range.end(); ++i) {
                performTransform(i, bufferRef, aabbMin, aabbMax);
            }
            min = aabbMin;
            max = aabbMax;
        }
    };
#endif

    void execute(bool enableParallel) {
        const int nmaterials = m_materials->count();
        Vector3 modelAabbMin(kAabbMin), modelAabbMax(kAabbMax);
#if defined(VPVL2_LINK_INTEL_TBB)
        if (enableParallel) {
            MaterialAabb aabb;
            for (int i = 0; i < nmaterials; i++) {
                const IMaterial *material = m_materials->at(i);
                const IMaterial::IndexRange &range = material->indexRange();
                aabb.bufferRef = m_bufferRef;
                tbb::parallel_reduce(tbb::blocked_range<int>(range.start, range.count), aabb);
                m_aabb->append(aabb.min);
                m_aabb->append(aabb.max);
                modelAabbMin.setMin(aabb.min);
                modelAabbMax.setMax(aabb.max);
            }
        }
        else {
#else
        {
            (void) enableParallel;
#endif /* VPVL2_LINK_INTEL_TBB */
            for (int i = 0; i < nmaterials; i++) {
                const IMaterial *material = m_materials->at(i);
                const IMaterial::IndexRange &range = material->indexRange();
                Vector3 aabbMin(kAabbMin), aabbMax(kAabbMax);
                for (int j = range.start, count = range.count; j < count; j++) {
                    performTransform(i, m_bufferRef, aabbMin, aabbMax);
                }
                m_aabb->append(aabbMin);
                m_aabb->append(aabbMax);
                modelAabbMin.setMin(aabbMin);
                modelAabbMax.setMax(aabbMax);
            }
        }
        m_aabb->append(modelAabbMin);
        m_aabb->append(modelAabbMax);
    }

private:
    const Array<TMaterial *> *m_materials;
    const TUnit *m_bufferRef;
    Array<Vector3> *m_aabb;
};

} /* namespace internal */
} /* namespace VPVL2_VERSION_NS */
} /* namespace vpvl2 */

#endif
