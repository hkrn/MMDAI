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
public:
    PMXAccelerator(Context *context)
        : m_context(context),
          m_program(0),
          m_updateBoneMatricesKernel(0),
          m_performSkinningKernel(0),
          m_vertexBuffer(0),
          m_boneMatricesBuffer(0),
          m_originMatricesBuffer(0),
          m_outputMatricesBuffer(0),
          m_weightsBuffer(0),
          m_bone1IndicesBuffer(0),
          m_bone2IndicesBuffer(0),
          m_weights(0),
          m_boneTransform(0),
          m_originTransform(0),
          m_bone1Indices(0),
          m_bone2Indices(0),
          m_isBufferAllocated(false)
    {
    }
    ~PMXAccelerator() {
        clReleaseProgram(m_program);
        m_program = 0;
        clReleaseKernel(m_performSkinningKernel);
        m_performSkinningKernel = 0;
        clReleaseKernel(m_updateBoneMatricesKernel);
        m_updateBoneMatricesKernel = 0;
        m_meshMatrices.releaseArrayAll();
        delete[] m_boneTransform;
        delete[] m_originTransform;
        delete[] m_bone1Indices;
        delete[] m_bone2Indices;
        delete[] m_weights;
        clReleaseMemObject(m_vertexBuffer);
        clReleaseMemObject(m_boneMatricesBuffer);
        clReleaseMemObject(m_originMatricesBuffer);
        clReleaseMemObject(m_outputMatricesBuffer);
        clReleaseMemObject(m_bone1IndicesBuffer);
        clReleaseMemObject(m_bone2IndicesBuffer);
        clReleaseMemObject(m_weightsBuffer);
        m_isBufferAllocated = false;
    }

    bool isAvailable() const {
        return m_context->isAvailable() && m_program;
    }
    bool createKernelProgram() {
        cl_int err;
        const IString *source = m_context->renderDelegate()->loadKernelSource(IRenderDelegate::kModelSkinningKernel, 0);
        const char *sourceText = reinterpret_cast<const char *>(source->toByteArray());
        const size_t sourceSize = source->length();
        clReleaseProgram(m_program);
        cl_context context = m_context->computeContext();
        m_program = clCreateProgramWithSource(context, 1, &sourceText, &sourceSize, &err);
        delete source;
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed creating an OpenCL program: %d", err);
            return false;
        }
        cl_device_id device = m_context->hostDevice();
        const char *flags = "-cl-mad-enable -DMAC -DGUID_ARG";
        err = clBuildProgram(m_program, 1, &device, flags, 0, 0);
        if (err != CL_SUCCESS) {
            size_t buildLogSize;
            clGetProgramBuildInfo(m_program, device, CL_PROGRAM_BUILD_LOG, 0, 0, &buildLogSize);
            cl_char *buildLog = new cl_char[buildLogSize + 1];
            clGetProgramBuildInfo(m_program, device, CL_PROGRAM_BUILD_LOG, buildLogSize, buildLog, 0);
            buildLog[buildLogSize] = 0;
            log0(0, IRenderDelegate::kLogWarning, "Failed building a program: %s", buildLog);
            delete[] buildLog;
            return false;
        }
        clReleaseKernel(m_updateBoneMatricesKernel);
        m_updateBoneMatricesKernel = clCreateKernel(m_program, "updateBoneMatrices", &err);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed creating a kernel: %d", err);
            return false;
        }
        clReleaseKernel(m_performSkinningKernel);
        m_performSkinningKernel = clCreateKernel(m_program, "performSkinning", &err);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed creating a kernel: %d", err);
            return false;
        }
        return true;
    }
    void uploadModel(pmx::Model * /* model */, GLuint buffer, void *context) {
        cl_int err;
        cl_context computeContext = m_context->computeContext();
        m_vertexBuffer = clCreateFromGLBuffer(computeContext,
                                              CL_MEM_READ_WRITE,
                                              buffer,
                                              &err);
        if (err != CL_SUCCESS) {
            log0(context, IRenderDelegate::kLogWarning, "Failed creating OpenCL vertex buffer: %d", err);
            return;
        }
        m_isBufferAllocated = true;
    }
    void updateModel(pmx::Model * /* model */) {
        if (!m_isBufferAllocated)
            return;
    }

private:
    void log0(void *context, IRenderDelegate::LogLevel level, const char *format...) {
        va_list ap;
        va_start(ap, format);
        m_context->renderDelegate()->log(context, level, format, ap);
        va_end(ap);
    }

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
