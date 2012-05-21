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

#include "vpvl2/vpvl2.h"
#include "vpvl2/cl/PMXAccelerator.h"

namespace vpvl2
{
namespace cl
{

PMXAccelerator::PMXAccelerator(Context *context)
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

PMXAccelerator::~PMXAccelerator()
{
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

bool PMXAccelerator::isAvailable() const
{
    return m_context->isAvailable() && m_program;
}

bool PMXAccelerator::createKernelProgram()
{
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

void PMXAccelerator::uploadModel(pmx::Model * /* model */, GLuint buffer, void *context)
{
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

void PMXAccelerator::updateModel(pmx::Model * /* model */)
{
    if (!m_isBufferAllocated)
        return;
}

void PMXAccelerator::log0(void *context, IRenderDelegate::LogLevel level, const char *format...)
{
    va_list ap;
    va_start(ap, format);
    m_context->renderDelegate()->log(context, level, format, ap);
    va_end(ap);
}

} /* namespace cl */
} /* namespace vpvl2 */
