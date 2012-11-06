/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
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

#ifndef VPVL2_CL_PMXACCELERATOR_H_
#define VPVL2_CL_PMXACCELERATOR_H_

#include "vpvl2/cl/Context.h"
#include "vpvl2/pmx/Model.h"

namespace vpvl2
{

class Scene;

namespace cl
{

class PMXAccelerator
{
public:
    struct Buffer {
        Buffer(GLuint n) : name(n), mem(0) {}
        GLuint name;
        cl_mem mem;
    };
    typedef Array<Buffer> Buffers;
    PMXAccelerator(Context *contextRef, IModel *modelRef);
    ~PMXAccelerator();

    bool isAvailable() const;
    bool createKernelProgram();
    void upload(Buffers &buffers, const IModel::IIndexBuffer *indexBufferRef, void *context);
    void update(const IModel::IDynamicVertexBuffer *dynamicBufferRef,
                const Scene *sceneRef,
                const Buffer &buffer,
                Vector3 &aabbMax,
                Vector3 &);
    void release(Buffers &buffers) const;

private:
    void log0(void *context, IRenderContext::LogLevel level, const char *format...);

    Context *m_contextRef;
    IModel *m_modelRef;
    Array<IBone *> m_bones;
    Array<IMaterial *> m_materials;
    Array<IVertex *> m_vertices;
    cl_program m_program;
    cl_kernel m_performSkinningKernel;
    cl_mem m_materialEdgeSizeBuffer;
    cl_mem m_boneWeightsBuffer;
    cl_mem m_boneIndicesBuffer;
    cl_mem m_boneMatricesBuffer;
    cl_mem m_aabbMinBuffer;
    cl_mem m_aabbMaxBuffer;
    cl_char *m_buildLogPtr;
    size_t m_localWGSizeForPerformSkinning;
    float *m_boneTransform;
    bool m_isBufferAllocated;
};

} /* namespace cl */
} /* namespace vpvl2 */

#endif
