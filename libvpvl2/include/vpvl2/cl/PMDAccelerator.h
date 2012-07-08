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

#ifndef VPVL2_CL_PMDACCELERATOR_H_
#define VPVL2_CL_PMDACCELERATOR_H_

#define SOFTWARE_BONE_TRANSFORM

#include "vpvl2/Scene.h"
#include "vpvl2/cl/Context.h"
#include "vpvl2/pmd/Model.h"

namespace vpvl2
{
namespace cl
{

class PMDAccelerator
{
public:
    PMDAccelerator(Context *context);
    ~PMDAccelerator();

    bool isAvailable() const;
    bool createKernelProgram();
    void uploadModel(pmd::Model *model, GLuint buffer, void *context);
    void updateModel(pmd::Model *model, const Scene *scene);

private:
    void log0(void *context, IRenderDelegate::LogLevel level, const char *format...);

    Context *m_context;
    cl_program m_program;
    cl_kernel m_performSkinningKernel;
    cl_mem m_verticesBuffer;
    cl_mem m_edgeOffsetBuffer;
    cl_mem m_boneMatricesBuffer;
    cl_mem m_boneWeightsBuffer;
    cl_mem m_boneIndicesBuffer;
    size_t m_localWGSizeForPerformSkinning;
    float *m_boneTransform;
    bool m_isBufferAllocated;
};

} /* namespace cl */
} /* namespace vpvl2 */

#endif
