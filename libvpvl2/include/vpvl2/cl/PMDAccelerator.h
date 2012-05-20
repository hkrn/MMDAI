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

#include "vpvl2/cl/Context.h"
#include "vpvl2/pmd/Model.h"
#include "vpvl/PMDModel.h"

namespace vpvl2
{
namespace cl
{

using namespace vpvl;

class PMDAccelerator
{
public:
    PMDAccelerator(Context *context)
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
    ~PMDAccelerator() {
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
    void uploadModel(pmd::Model *model, GLuint buffer, void *context) {
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
        const PMDModel *m = model->ptr();
        model->getMeshTransforms(m_meshTransforms, m_meshIndices, m_meshWeights, m_meshMatrices);

        const int nBoneMatricesAllocs = m->bones().count() << 4;
        const int nBoneMatricesSize = nBoneMatricesAllocs * sizeof(float);
        m_boneTransform = new float[nBoneMatricesAllocs];
        m_originTransform = new float[nBoneMatricesAllocs];
        const VertexList &vertices = m->vertices();
        const int nVerticesAlloc = vertices.count();
        m_bone1Indices = new int[nVerticesAlloc];
        m_bone2Indices = new int[nVerticesAlloc];
        m_weights = new float[nVerticesAlloc];
        for (int i = 0; i < nVerticesAlloc; i++) {
            const Vertex *vertex = vertices[i];
            m_bone1Indices[i] = vertex->bone1();
            m_bone2Indices[i] = vertex->bone2();
            m_weights[i] = vertex->weight();
        }
        m_boneMatricesBuffer = clCreateBuffer(computeContext, CL_MEM_READ_ONLY, nBoneMatricesSize, 0, &err);
        if (err != CL_SUCCESS) {
            log0(context, IRenderDelegate::kLogWarning, "Failed creating boneMatricesBuffer: %d", err);
            return;
        }
        m_originMatricesBuffer = clCreateBuffer(computeContext, CL_MEM_READ_ONLY, nBoneMatricesSize, 0, &err);
        if (err != CL_SUCCESS) {
            log0(context, IRenderDelegate::kLogWarning, "Failed creating originMatricesBuffer: %d", err);
            return;
        }
        m_outputMatricesBuffer = clCreateBuffer(computeContext, CL_MEM_READ_WRITE, nBoneMatricesSize, 0, &err);
        if (err != CL_SUCCESS) {
            log0(context, IRenderDelegate::kLogWarning, "Failed creating outputMatricesBuffer %d", err);
            return;
        }
        m_weightsBuffer = clCreateBuffer(computeContext, CL_MEM_READ_ONLY, nVerticesAlloc * sizeof(float), 0, &err);
        if (err != CL_SUCCESS) {
            log0(context, IRenderDelegate::kLogWarning, "Failed creating weightsBuffer: %d", err);
            return;
        }
        m_bone1IndicesBuffer = clCreateBuffer(computeContext, CL_MEM_READ_ONLY, nVerticesAlloc * sizeof(int), 0, &err);
        if (err != CL_SUCCESS) {
            log0(context, IRenderDelegate::kLogWarning, "Failed creating bone1IndicesBuffer: %d", err);
        }
        m_bone2IndicesBuffer = clCreateBuffer(computeContext, CL_MEM_READ_ONLY, nVerticesAlloc * sizeof(int), 0, &err);
        if (err != CL_SUCCESS) {
            log0(context, IRenderDelegate::kLogWarning, "Failed creating bone2IndicesBuffer: %d", err);
            return;
        }
        cl_device_id device = m_context->hostDevice();
        err = clGetKernelWorkGroupInfo(m_updateBoneMatricesKernel,
                                       device,
                                       CL_KERNEL_WORK_GROUP_SIZE,
                                       sizeof(m_localWGSizeForUpdateBoneMatrices),
                                       &m_localWGSizeForUpdateBoneMatrices,
                                       0);
        if (err != CL_SUCCESS) {
            log0(context, IRenderDelegate::kLogWarning, "Failed getting kernel work group information (CL_KERNEL_WORK_GROUP_SIZE): %d", err);
            return;
        }
        err = clGetKernelWorkGroupInfo(m_performSkinningKernel,
                                       device,
                                       CL_KERNEL_WORK_GROUP_SIZE,
                                       sizeof(m_localWGSizeForPerformSkinning),
                                       &m_localWGSizeForPerformSkinning,
                                       0);
        if (err != CL_SUCCESS) {
            log0(context, IRenderDelegate::kLogWarning, "Failed getting kernel work group information (CL_KERNEL_WORK_GROUP_SIZE): %d", err);
            return;
        }
        m_isBufferAllocated = true;
    }
    void updateModel(pmd::Model *model) {
        if (!m_isBufferAllocated)
            return;
        const PMDModel *m = model->ptr();
        const BoneList &bones = m->bones();
        const int nbones = bones.count();
        Transform transform = Transform::getIdentity();
        for (int i = 0; i < nbones; i++) {
            Bone *bone = bones[i];
            int index = i << 4;
#define SOFTWARE_BONE_TRANSFORM
#ifndef SOFTWARE_BONE_TRANSFORM
            bone->localTransform().getOpenGLMatrix(&m_boneTransform[index]);
            transform.setOrigin(bone->originPosition());
            transform.getOpenGLMatrix(&m_originTransform[index]);
#else
            bone->getSkinTransform(transform);
            transform.getOpenGLMatrix(&m_boneTransform[index]);
#endif
        }
        size_t nsize = (nbones * sizeof(float)) << 4;
        cl_int err;
#ifndef SOFTWARE_BONE_TRANSFORM
        err = clEnqueueWriteBuffer(queue, m_boneMatricesBuffer, CL_TRUE, 0, nsize, m_boneTransform, 0, 0, 0);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed enqueue a command to write bone matrices buffer: %d", err);
            return;
        }
        err = clEnqueueWriteBuffer(queue, m_originMatricesBuffer, CL_TRUE, 0, nsize, m_originTransform, 0, 0, 0);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed enqueue a command to write origin matrices buffer: %d", err);
            return;
        }
        err = clSetKernelArg(m_updateBoneMatricesKernel, 0, sizeof(m_boneMatricesBuffer), &m_boneMatricesBuffer);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 1st argument of kernel (boneMatricesBuffer): %d", err);
            return;
        }
        err = clSetKernelArg(m_updateBoneMatricesKernel, 1, sizeof(m_originMatricesBuffer), &m_originMatricesBuffer);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 2nd argument of kernel (originMatricesBuffer): %d", err);
            return;
        }
        err = clSetKernelArg(m_updateBoneMatricesKernel, 2, sizeof(int), &nbones);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 3th argument of kernel (nbones): %d", err);
            return;
        }
        err = clSetKernelArg(m_updateBoneMatricesKernel, 3, sizeof(m_outputMatricesBuffer), &m_outputMatricesBuffer);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 4rd argument of kernel (outputMatricesBuffer): %d", err);
            return;
        }
        size_t local = m_localWGSizeForUpdateBoneMatrices;
        size_t global = local * ((nbones + (local - 1)) / local);
        err = clEnqueueNDRangeKernel(queue, m_updateBoneMatricesKernel, 1, 0, &global, &local, 0, 0, 0);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed enqueue executing kernel");
            return;
        }
        clFinish(queue);
#else
        size_t local, global;
        cl_command_queue queue = m_context->commandQueue();
        err = clEnqueueWriteBuffer(queue, m_outputMatricesBuffer, CL_TRUE, 0, nsize, m_boneTransform, 0, 0, 0);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed enqueue a command to write output matrices buffer: %d", err);
            return;
        }
#endif
        /* force flushing OpenGL commands to acquire GL objects by OpenCL */
        glFinish();
        clEnqueueAcquireGLObjects(queue, 1, &m_vertexBuffer, 0, 0, 0);
        int nvertices = m->vertices().count();
        err = clEnqueueWriteBuffer(queue, m_bone1IndicesBuffer, CL_TRUE, 0, nvertices * sizeof(int), m_bone1Indices, 0, 0, 0);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed enqueue a command to write bone1 indices buffer: %d", err);
            return;
        }
        err = clEnqueueWriteBuffer(queue, m_bone2IndicesBuffer, CL_TRUE, 0, nvertices * sizeof(int), m_bone2Indices, 0, 0, 0);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed enqueue a command to write bone2 indices buffer: %d", err);
            return;
        }
        err = clEnqueueWriteBuffer(queue, m_weightsBuffer, CL_TRUE, 0, nvertices * sizeof(float), m_weights, 0, 0, 0);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed enqueue a command to write weights buffer: %d", err);
            return;
        }
        err = clSetKernelArg(m_performSkinningKernel, 0, sizeof(m_outputMatricesBuffer), &m_outputMatricesBuffer);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 1st argument of kernel (skinningMatrices): %d", err);
            return;
        }
        err = clSetKernelArg(m_performSkinningKernel, 1, sizeof(m_weightsBuffer), &m_weightsBuffer);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 2nd argument of kernel (weights): %d", err);
            return;
        }
        err = clSetKernelArg(m_performSkinningKernel, 2, sizeof(m_bone1IndicesBuffer), &m_bone1IndicesBuffer);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 3rd argument of kernel (bone1Indices): %d", err);
            return;
        }
        err = clSetKernelArg(m_performSkinningKernel, 3, sizeof(m_bone2IndicesBuffer), &m_bone2IndicesBuffer);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 4th argument of kernel (bone2Indices): %d", err);
            return;
        }
        const Vector3 &lightDirection = -m->lightPosition();
        err = clSetKernelArg(m_performSkinningKernel, 4, sizeof(lightDirection), &lightDirection);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 5th argument of kernel (lightDirection): %d", err);
            return;
        }
        err = clSetKernelArg(m_performSkinningKernel, 5, sizeof(nvertices), &nvertices);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 6th argument of kernel (nvertices): %d", err);
            return;
        }
        size_t strideSize = m->strideSize(PMDModel::kVerticesStride) >> 4;
        err = clSetKernelArg(m_performSkinningKernel, 6, sizeof(strideSize), &strideSize);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 7th argument of kernel (strideSize): %d", err);
            return;
        }
        size_t offsetPosition = m->strideOffset(PMDModel::kVerticesStride) >> 4;
        err = clSetKernelArg(m_performSkinningKernel, 7, sizeof(offsetPosition), &offsetPosition);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 8th argument of kernel (offsetPosition): %d", err);
            return;
        }
        size_t offsetNormal = m->strideOffset(PMDModel::kNormalsStride) >> 4;
        err = clSetKernelArg(m_performSkinningKernel, 8, sizeof(offsetNormal), &offsetNormal);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 9th argument of kernel (offsetNormal): %d", err);
            return;
        }
        size_t offsetToonTexture = m->strideOffset(PMDModel::kToonTextureStride) >> 4;
        err = clSetKernelArg(m_performSkinningKernel, 9, sizeof(offsetToonTexture), &offsetToonTexture);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 10th argument of kernel (offsetTexCoord): %d", err);
            return;
        }
        size_t offsetEdge = m->strideOffset(PMDModel::kEdgeVerticesStride) >> 4;
        err = clSetKernelArg(m_performSkinningKernel, 10, sizeof(offsetEdge), &offsetEdge);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 11th argument of kernel (offsetEdge): %d", err);
            return;
        }
        err = clSetKernelArg(m_performSkinningKernel, 11, sizeof(m_vertexBuffer), &m_vertexBuffer);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 12th argument of kernel (vertices): %d", err);
            return;
        }
        local = m_localWGSizeForUpdateBoneMatrices;
        global = local * ((nvertices + (local - 1)) / local);
        err = clEnqueueNDRangeKernel(queue, m_performSkinningKernel, 1, 0, &global, &local, 0, 0, 0);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed enqueue executing kernel: %d", err);
            return;
        }
        clEnqueueReleaseGLObjects(queue, 1, &m_vertexBuffer, 0, 0, 0);
        clFinish(queue);
    }

private:
    void log0(void *context, IRenderDelegate::LogLevel level, const char *format...) {
        va_list ap;
        va_start(ap, format);
        m_context->renderDelegate()->log(context, level, format, ap);
        va_end(ap);
    }

    Context *m_context;
    pmd::Model::MeshTranforms m_meshTransforms;
    pmd::Model::MeshIndices m_meshIndices;
    pmd::Model::MeshWeights m_meshWeights;
    pmd::Model::MeshMatrices m_meshMatrices;
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
