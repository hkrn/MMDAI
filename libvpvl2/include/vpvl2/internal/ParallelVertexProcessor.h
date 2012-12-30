/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
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

#include <vpvl2/Common.h>
#include <vpvl2/IMaterial.h>

#ifdef VPVL2_LINK_INTEL_TBB
#include <tbb/tbb.h>
#endif

namespace vpvl2
{

namespace internal
{

#ifdef VPVL2_ENABLE_OPENMP

static inline bool GreaterOMP(const Vector3 &left, const Vector3 &right)
{
    return left.x() < right.x() ||  left.y() < right.y() || left.z() < right.z();
}

static inline bool LessOMP(const Vector3 &left, const Vector3 &right)
{
    return left.x() > right.x() ||  left.y() > right.y() || left.z() > right.z();
}

template<typename TModel, typename TVertex, typename TUnit>
static void UpdateModelVerticesOMP(const TModel *modelRef,
                                   const Array<TVertex *> &verticesRef,
                                   const Vector3 &cameraPosition,
                                   TUnit *bufferPtr)
{
    const int edgeScaleFactor = modelRef->edgeScaleFactor(cameraPosition);
    const int nvertices = verticesRef.count();
    Vector3 position, aabbMin(SIMD_INFINITY, SIMD_INFINITY, SIMD_INFINITY),
            aabbMax(-SIMD_INFINITY, -SIMD_INFINITY, -SIMD_INFINITY);
#pragma omp parallel for
    for (int i = 0; i < nvertices; ++i) {
        const TVertex *vertex = verticesRef[i];
        const IMaterial *material = vertex->material();
        const float materialEdgeSize = (material ? material->edgeSize() : 0) * edgeScaleFactor;
        TUnit &v = bufferPtr[i];
        v.update(vertex, materialEdgeSize, i, position);
#pragma omp flush(aabbMin)
        if (LessOMP(aabbMin, position)) {
#pragma omp critical
            {
                aabbMin.setMin(position);
            }
        }
#pragma omp flush(aabbMax)
        if (GreaterOMP(aabbMax, position)) {
#pragma omp critical
            {
                aabbMax.setMax(position);
            }
        }
    }
}

template<typename TVertex, typename TUnit>
static void InitializeModelVerticesOMP(const Array<TVertex *> &verticesRef,
                                       TUnit *bufferPtr)
{
    const int nvertices = verticesRef.count();
#pragma omp parallel for
    for (int i = 0; i < nvertices; ++i) {
        const TVertex *vertex = verticesRef[i];
        TUnit &v = bufferPtr[i];
        v.update(vertex, i);
    }
}

#endif /* VPVL2_ENABLE_OPENMP */

template<typename TModel, typename TVertex, typename TUnit>
class ParallelSkinningVertexProcessor {
public:
    ParallelSkinningVertexProcessor(const TModel *modelRef,
                                    const Array<TVertex *> *verticesRef,
                                    const Vector3 &cameraPosition,
                                    void *address)
        : m_verticesRef(verticesRef),
          m_edgeScaleFactor(modelRef->edgeScaleFactor(cameraPosition)),
          m_aabbMin(SIMD_INFINITY, SIMD_INFINITY, SIMD_INFINITY),
          m_aabbMax(-SIMD_INFINITY, -SIMD_INFINITY, -SIMD_INFINITY),
          m_bufferPtr(static_cast<TUnit *>(address))
    {
    }
    ~ParallelSkinningVertexProcessor() {
        m_verticesRef = 0;
        m_bufferPtr = 0;
    }

    const Vector3 &aabbMin() const { return m_aabbMin; }
    const Vector3 &aabbMax() const { return m_aabbMax; }

#ifdef VPVL2_LINK_INTEL_TBB
    ParallelSkinningVertexProcessor(const ParallelSkinningVertexProcessor &self, tbb::split split)
        : m_verticesRef(self.m_verticesRef),
          m_edgeScaleFactor(self.m_edgeScaleFactor),
          m_aabbMin(SIMD_INFINITY, SIMD_INFINITY, SIMD_INFINITY),
          m_aabbMax(-SIMD_INFINITY, -SIMD_INFINITY, -SIMD_INFINITY),
          m_bufferPtr(self.m_bufferPtr)
    {
    }
    void join(const ParallelSkinningVertexProcessor &self) {
        m_aabbMin.setMin(self.m_aabbMin);
        m_aabbMax.setMax(self.m_aabbMax);
    }
    void operator()(const tbb::blocked_range<int> &range) const {
        Vector3 aabbMin(m_aabbMin), aabbMax(m_aabbMax), position;
        for (int i = range.begin(); i != range.end(); ++i) {
            const TVertex *vertex = m_verticesRef->at(i);
            const IMaterial *material = vertex->material();
            const float materialEdgeSize = (material ? material->edgeSize() : 0) * m_edgeScaleFactor;
            TUnit &v = m_bufferPtr[i];
            v.update(vertex, materialEdgeSize, i, position);
            aabbMin.setMin(position);
            aabbMax.setMax(position);
        }
        m_aabbMin = aabbMin;
        m_aabbMax = aabbMax;
    }
#endif /* VPVL2_LINK_INTEL_TBB */

private:
    const Array<TVertex *> *m_verticesRef;
    const Scalar m_edgeScaleFactor;
    mutable Vector3 m_aabbMin;
    mutable Vector3 m_aabbMax;
    TUnit *m_bufferPtr;
};

template<typename TModel, typename TVertex, typename TUnit>
class ParallelInitializeVertexProcessor {
public:
    ParallelInitializeVertexProcessor(const Array<TVertex *> *verticesRef,
                                      void *address)
        : m_verticesRef(verticesRef),
          m_bufferPtr(static_cast<TUnit *>(address))
    {
    }
    ~ParallelInitializeVertexProcessor() {
        m_verticesRef = 0;
        m_bufferPtr = 0;
    }

#ifdef VPVL2_LINK_INTEL_TBB
    ParallelInitializeVertexProcessor(const ParallelInitializeVertexProcessor &self)
        : m_verticesRef(self.m_verticesRef),
          m_bufferPtr(self.m_bufferPtr)
    {
    }
    void operator()(const tbb::blocked_range<int> &range) const {
        Vector3 position;
        for (int i = range.begin(); i != range.end(); ++i) {
            const TVertex *vertex = m_verticesRef->at(i);
            TUnit &v = m_bufferPtr[i];
            v.update(vertex, i);
        }
    }
#endif /* VPVL2_LINK_INTEL_TBB */

private:
    const Array<TVertex *> *m_verticesRef;
    TUnit *m_bufferPtr;
};

} /* namespace internal */
} /* namespace vpvl2 */
