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
namespace cl
{

class PMXAccelerator
{
public:
    PMXAccelerator(Context *context);
    ~PMXAccelerator();

    bool isAvailable() const;
    bool createKernelProgram();
    void uploadModel(pmx::Model *model, GLuint buffer, void *context);
    void updateModel(pmx::Model *model);

private:
    void log0(void *context, IRenderDelegate::LogLevel level, const char *format...);

    Context *m_context;
    pmx::Model::MeshTranforms m_meshTransforms;
    pmx::Model::MeshIndices m_meshIndices;
    pmx::Model::MeshWeights m_meshWeights;
    pmx::Model::MeshMatrices m_meshMatrices;
    cl_program m_program;
    cl_kernel m_updateBoneMatricesKernel;
    cl_kernel m_performSkinningKernel;
    cl_mem m_vertexBuffer;
    cl_mem m_boneMatricesBuffer;
    cl_mem m_originMatricesBuffer;
    cl_mem m_outputMatricesBuffer;
    cl_mem m_weightsBuffer;
    cl_mem m_bone1IndicesBuffer;
    cl_mem m_bone2IndicesBuffer;
    size_t m_localWGSizeForUpdateBoneMatrices;
    size_t m_localWGSizeForPerformSkinning;
    float *m_weights;
    float *m_boneTransform;
    float *m_originTransform;
    int *m_bone1Indices;
    int *m_bone2Indices;
    bool m_isBufferAllocated;
};

} /* namespace cl */
} /* namespace vpvl2 */

#endif
