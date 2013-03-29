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

#include "vpvl2/vpvl2.h"

#include "vpvl2/cl/PMXAccelerator.h"
#include "vpvl2/pmx/Bone.h"
#include "vpvl2/pmx/Material.h"
#include "vpvl2/pmx/Vertex.h"

namespace vpvl2
{
namespace cl
{

static const char kProgramCompileFlags[] = "-cl-fast-relaxed-math";
static const int kMaxBonesPerVertex = 4;

PMXAccelerator::PMXAccelerator(Context *contextRef, IModel *modelRef)
    : m_contextRef(contextRef),
      m_modelRef(modelRef),
      m_program(0),
      m_performSkinningKernel(0),
      m_materialEdgeSizeBuffer(0),
      m_boneWeightsBuffer(0),
      m_boneIndicesBuffer(0),
      m_boneMatricesBuffer(0),
      m_aabbMinBuffer(0),
      m_aabbMaxBuffer(0),
      m_localWGSizeForPerformSkinning(0),
      m_boneTransform(0),
      m_isBufferAllocated(false)
{
}

PMXAccelerator::~PMXAccelerator()
{
    clReleaseProgram(m_program);
    m_program = 0;
    clReleaseKernel(m_performSkinningKernel);
    m_performSkinningKernel = 0;
    clReleaseMemObject(m_materialEdgeSizeBuffer);
    m_materialEdgeSizeBuffer = 0;
    clReleaseMemObject(m_boneIndicesBuffer);
    m_boneIndicesBuffer = 0;
    clReleaseMemObject(m_boneWeightsBuffer);
    m_boneWeightsBuffer = 0;
    clReleaseMemObject(m_aabbMinBuffer);
    m_aabbMinBuffer = 0;
    clReleaseMemObject(m_aabbMaxBuffer);
    m_aabbMaxBuffer = 0;
    m_localWGSizeForPerformSkinning = 0;
    delete[] m_boneTransform;
    m_boneTransform = 0;
    m_isBufferAllocated = false;
    m_contextRef = 0;
    m_modelRef = 0;
}

bool PMXAccelerator::isAvailable() const
{
    return m_contextRef->isAvailable() && m_program;
}

bool PMXAccelerator::createKernelProgram()
{
    cl_int err;
    const IString *source = m_contextRef->renderContext()->loadKernelSource(IRenderContext::kModelSkinningKernel, 0);
    const char *sourceText = reinterpret_cast<const char *>(source->toByteArray());
    const size_t sourceSize = source->length(IString::kUTF8);
    clReleaseProgram(m_program);
    cl_context context = m_contextRef->computeContext();
    m_program = clCreateProgramWithSource(context, 1, &sourceText, &sourceSize, &err);
    delete source;
    if (err != CL_SUCCESS) {
        VPVL2_LOG(LOG(ERROR) << "Failed creating an OpenCL program: " << err);
        return false;
    }
    cl_device_id device = m_contextRef->hostDevice();
    err = clBuildProgram(m_program, 1, &device, kProgramCompileFlags, 0, 0);
    if (err != CL_SUCCESS) {
        size_t buildLogSize;
        clGetProgramBuildInfo(m_program, device, CL_PROGRAM_BUILD_LOG, 0, 0, &buildLogSize);
        Array<cl_char> buildLog;
        buildLog.resize(buildLogSize + 1);
        clGetProgramBuildInfo(m_program, device, CL_PROGRAM_BUILD_LOG, buildLogSize, &buildLog[0], 0);
        buildLog[buildLogSize] = 0;
        VPVL2_LOG(LOG(ERROR) << "Failed building a program: " << reinterpret_cast<const char *>(&buildLog[0]));
        return false;
    }
    clReleaseKernel(m_performSkinningKernel);
    m_performSkinningKernel = clCreateKernel(m_program, "performSkinning2", &err);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(LOG(ERROR) << "Failed creating a kernel (performSkinning2): " << err);
        return false;
    }
    return true;
}

void PMXAccelerator::upload(Buffers &buffers, const IModel::IIndexBuffer *indexBufferRef)
{
    cl_int err;
    cl_context computeContext = m_contextRef->computeContext();
    const int nbuffers = buffers.count();
    for (int i = 0; i < nbuffers; i++) {
        Buffer &buffer = buffers[i];
        buffer.mem = clCreateFromGLBuffer(computeContext,
                                          CL_MEM_READ_WRITE,
                                          buffer.name,
                                          &err);
        if (err != CL_SUCCESS) {
            VPVL2_LOG(LOG(ERROR) << "Failed creating OpenCL vertex buffer: " << err);
            return;
        }
    }
    Array<IBone *> bones;
    Array<IVertex *> vertices;
    m_modelRef->getBoneRefs(bones);
    m_modelRef->getVertexRefs(vertices);
    const int nBoneMatricesAllocs = bones.count() << 4;
    const int nBoneMatricesSize = nBoneMatricesAllocs * sizeof(float);
    const int nvertices = vertices.count();
    const int nVerticesAlloc = nvertices * kMaxBonesPerVertex;
    Array<int> boneIndices;
    Array<float> boneWeights, materialEdgeSize;
    boneIndices.resize(nVerticesAlloc);
    boneWeights.resize(nVerticesAlloc);
    materialEdgeSize.resize(nvertices);
    for (int i = 0; i < nvertices; i++) {
        const IVertex *vertex = vertices[i];
        for (int j = 0; j < kMaxBonesPerVertex; j++) {
            const IBone *bone = vertex->bone(j);
            const int index = i * kMaxBonesPerVertex + j;
            boneIndices[index] = bone ? bone->index() : -1;
            boneWeights[index] = vertex->weight(j);
        }
    }
    Array<IMaterial *> materials;
    m_modelRef->getMaterialRefs(materials);
    const int nmaterials = materials.count();
    size_t offset = 0;
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = materials[i];
        const int nindices = material->indexRange().count, offsetTo = offset + nindices;
        const float edgeSize = material->edgeSize();
        for (int j = offset; j < offsetTo; j++) {
            const int index = indexBufferRef->indexAt(j);
            materialEdgeSize[index] = edgeSize;
        }
        offset = offsetTo;
    }
    delete[] m_boneTransform;
    m_boneTransform = new float[nBoneMatricesAllocs];
    clReleaseMemObject(m_materialEdgeSizeBuffer);
    m_materialEdgeSizeBuffer = clCreateBuffer(computeContext, CL_MEM_READ_ONLY, nvertices * sizeof(float), 0, &err);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(LOG(ERROR) << "Failed creating materialEdgeSizeBuffer: " << err);
        return;
    }
    clReleaseMemObject(m_boneMatricesBuffer);
    m_boneMatricesBuffer = clCreateBuffer(computeContext, CL_MEM_READ_ONLY, nBoneMatricesSize, 0, &err);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(LOG(ERROR) << "Failed creating boneMatricesBuffer: " << err);
        return;
    }
    clReleaseMemObject(m_boneIndicesBuffer);
    m_boneIndicesBuffer = clCreateBuffer(computeContext, CL_MEM_READ_ONLY, nVerticesAlloc * sizeof(int), 0, &err);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(LOG(ERROR) << "Failed creating boneIndicesBuffer: " << err);
        return;
    }
    clReleaseMemObject(m_boneWeightsBuffer);
    m_boneWeightsBuffer = clCreateBuffer(computeContext, CL_MEM_READ_ONLY, nVerticesAlloc * sizeof(float), 0, &err);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(LOG(ERROR) << "Failed creating boneWeightsBuffer: " << err);
        return;
    }
    clReleaseMemObject(m_boneMatricesBuffer);
    m_boneMatricesBuffer = clCreateBuffer(computeContext, CL_MEM_READ_ONLY, nBoneMatricesSize, 0, &err);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(LOG(ERROR) << "Failed creating boneMatricesBuffer: " << err);
        return;
    }
    clReleaseMemObject(m_aabbMinBuffer);
    m_aabbMinBuffer = clCreateBuffer(computeContext, CL_MEM_READ_WRITE, sizeof(Vector3), 0, &err);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(LOG(ERROR) << "Failed creating aabbMinBuffer: " << err);
        return;
    }
    clReleaseMemObject(m_aabbMaxBuffer);
    m_aabbMaxBuffer = clCreateBuffer(computeContext, CL_MEM_READ_WRITE, sizeof(Vector3), 0, &err);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(LOG(ERROR) << "Failed creating aabbMaxBuffer: " << err);
        return;
    }
    cl_device_id device = m_contextRef->hostDevice();
    err = clGetKernelWorkGroupInfo(m_performSkinningKernel,
                                   device,
                                   CL_KERNEL_WORK_GROUP_SIZE,
                                   sizeof(m_localWGSizeForPerformSkinning),
                                   &m_localWGSizeForPerformSkinning,
                                   0);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(LOG(ERROR) << "Failed getting kernel work group information (CL_KERNEL_WORK_GROUP_SIZE): " << err);
        return;
    }
    cl_command_queue queue = m_contextRef->commandQueue();
    err = clEnqueueWriteBuffer(queue, m_materialEdgeSizeBuffer, CL_TRUE, 0, nvertices * sizeof(float), &materialEdgeSize[0], 0, 0, 0);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(LOG(ERROR) << "Failed enqueue a command to write materialEdgeSizeBuffer: " << err);
        return;
    }
    err = clEnqueueWriteBuffer(queue, m_boneIndicesBuffer, CL_TRUE, 0, nVerticesAlloc * sizeof(int), &boneIndices[0], 0, 0, 0);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(LOG(ERROR) << "Failed enqueue a command to write boneIndicesBuffer: " << err);
        return;
    }
    err = clEnqueueWriteBuffer(queue, m_boneWeightsBuffer, CL_TRUE, 0, nVerticesAlloc * sizeof(float), &boneWeights[0], 0, 0, 0);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(LOG(ERROR) << "Failed enqueue a command to write boneWeightsBuffer: " << err);
        return;
    }
    m_isBufferAllocated = true;
}

void PMXAccelerator::update(const IModel::IDynamicVertexBuffer *dynamicBufferRef,
                            const Scene *sceneRef,
                            const Buffer &buffer,
                            Vector3 &aabbMin,
                            Vector3 &aabbMax)
{
    if (!m_isBufferAllocated) {
        return;
    }
    Array<IBone *> bones;
    Array<IVertex *> vertices;
    m_modelRef->getBoneRefs(bones);
    m_modelRef->getVertexRefs(vertices);
    const int nvertices = vertices.count();
    const int nbones = bones.count();
    for (int i = 0; i < nbones; i++) {
        IBone *bone = bones[i];
        int index = i << 4;
        bone->localTransform().getOpenGLMatrix(&m_boneTransform[index]);
    }
    size_t nsize = (nbones * sizeof(float)) << 4;
    cl_command_queue queue = m_contextRef->commandQueue();
    cl_mem vertexBuffer = buffer.mem;
    /* force flushing OpenGL commands to acquire GL objects by OpenCL */
    glFinish();
    clEnqueueAcquireGLObjects(queue, 1, &vertexBuffer, 0, 0, 0);
    cl_int err = clEnqueueWriteBuffer(queue, m_boneMatricesBuffer, CL_TRUE, 0, nsize, m_boneTransform, 0, 0, 0);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(LOG(ERROR) << "Failed enqueue a command to write boneMatricesBuffer: " << err);
        return;
    }
    err = clEnqueueWriteBuffer(queue, m_aabbMinBuffer, CL_TRUE, 0, sizeof(aabbMin), &aabbMin, 0, 0, 0);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(LOG(ERROR) << "Failed enqueue a command to write aabbMinBuffer: " << err);
        return;
    }
    err = clEnqueueWriteBuffer(queue, m_aabbMaxBuffer, CL_TRUE, 0, sizeof(aabbMax), &aabbMax, 0, 0, 0);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(LOG(ERROR) << "Failed enqueue a command to write aabbMaxBuffer: " << err);
        return;
    }
    int argumentIndex = 0;
    err = clSetKernelArg(m_performSkinningKernel, argumentIndex++, sizeof(m_boneMatricesBuffer), &m_boneMatricesBuffer);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(LOG(ERROR) << "Failed setting 1st argument of kernel (localMatrices): " << err);
        return;
    }
    err = clSetKernelArg(m_performSkinningKernel, argumentIndex++, sizeof(m_boneWeightsBuffer), &m_boneWeightsBuffer);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(LOG(ERROR) << "Failed setting 2nd argument of kernel (boneWeights): " << err);
        return;
    }
    err = clSetKernelArg(m_performSkinningKernel, argumentIndex++, sizeof(m_boneIndicesBuffer), &m_boneIndicesBuffer);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(LOG(ERROR) << "Failed setting 3rd argument of kernel (boneIndices): " << err);
        return;
    }
    err = clSetKernelArg(m_performSkinningKernel, argumentIndex++, sizeof(m_materialEdgeSizeBuffer), &m_materialEdgeSizeBuffer);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(LOG(ERROR) << "Failed setting " << argumentIndex << "th argument of kernel (materialEdgeSize): " << err);
        return;
    }
    const Vector3 &lightDirection = sceneRef->light()->direction();
    err = clSetKernelArg(m_performSkinningKernel, argumentIndex++, sizeof(lightDirection), &lightDirection);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(LOG(ERROR) << "Failed setting " << argumentIndex << "th argument of kernel (lightDirection): " << err);
        return;
    }
    const ICamera *camera = sceneRef->camera();
    const Scalar &edgeScaleFactor = m_modelRef->edgeScaleFactor(camera->position()) * m_modelRef->edgeWidth();
    err = clSetKernelArg(m_performSkinningKernel, argumentIndex++, sizeof(edgeScaleFactor), &edgeScaleFactor);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(LOG(ERROR) << "Failed setting " << argumentIndex << "th argument of kernel (edgeScaleFactor): " << err);
        return;
    }
    err = clSetKernelArg(m_performSkinningKernel, argumentIndex++, sizeof(nvertices), &nvertices);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(LOG(ERROR) << "Failed setting " << argumentIndex << "th argument of kernel (nvertices): " << err);
        return;
    }
    size_t strideSize = dynamicBufferRef->strideSize() >> 4;
    err = clSetKernelArg(m_performSkinningKernel, argumentIndex++, sizeof(strideSize), &strideSize);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(LOG(ERROR) << "Failed setting " << argumentIndex << "th argument of kernel (strideSize): " << err);
        return;
    }
    size_t offsetPosition = dynamicBufferRef->strideOffset(IModel::IDynamicVertexBuffer::kVertexStride) >> 4;
    err = clSetKernelArg(m_performSkinningKernel, argumentIndex++, sizeof(offsetPosition), &offsetPosition);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(LOG(ERROR) << "Failed setting " << argumentIndex << "th argument of kernel (offsetPosition): " << err);
        return;
    }
    size_t offsetNormal = dynamicBufferRef->strideOffset(IModel::IDynamicVertexBuffer::kNormalStride) >> 4;
    err = clSetKernelArg(m_performSkinningKernel, argumentIndex++, sizeof(offsetNormal), &offsetNormal);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(LOG(ERROR) << "Failed setting " << argumentIndex << "th argument of kernel (offsetNormal): " << err);
        return;
    }
    size_t offsetMorphDelta = dynamicBufferRef->strideOffset(IModel::IDynamicVertexBuffer::kMorphDeltaStride) >> 4;
    err = clSetKernelArg(m_performSkinningKernel, argumentIndex++, sizeof(offsetMorphDelta), &offsetMorphDelta);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(LOG(ERROR) << "Failed setting " << argumentIndex << "th argument of kernel (offsetMorphDelta): " << err);
        return;
    }
    size_t offsetEdgeVertex = dynamicBufferRef->strideOffset(IModel::IDynamicVertexBuffer::kEdgeVertexStride) >> 4;
    err = clSetKernelArg(m_performSkinningKernel, argumentIndex++, sizeof(offsetEdgeVertex), &offsetEdgeVertex);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(LOG(ERROR) << "Failed setting " << argumentIndex << "th argument of kernel (offsetEdgeVertex): " << err);
        return;
    }
    err = clSetKernelArg(m_performSkinningKernel, argumentIndex++, sizeof(m_aabbMinBuffer), &m_aabbMinBuffer);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(LOG(ERROR) << "Failed setting " << argumentIndex << "th argument of kernel (aabbMin): " << err);
        return;
    }
    err = clSetKernelArg(m_performSkinningKernel, argumentIndex++, sizeof(m_aabbMaxBuffer), &m_aabbMaxBuffer);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(LOG(ERROR) << "Failed setting " << argumentIndex << "th argument of kernel (aabbMax): " << err);
        return;
    }
    err = clSetKernelArg(m_performSkinningKernel, argumentIndex++, sizeof(vertexBuffer), &vertexBuffer);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(LOG(ERROR) << "Failed setting " << argumentIndex << "th argument of kernel (vertices): " << err);
        return;
    }
    size_t local = m_localWGSizeForPerformSkinning;
    size_t global = local * ((nvertices + (local - 1)) / local);
    err = clEnqueueNDRangeKernel(queue, m_performSkinningKernel, 1, 0, &global, &local, 0, 0, 0);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(LOG(ERROR) << "Failed enqueue executing kernel: " << err);
        return;
    }
    clEnqueueReleaseGLObjects(queue, 1, &vertexBuffer, 0, 0, 0);
    clEnqueueReadBuffer(queue, m_aabbMinBuffer, CL_TRUE, 0, sizeof(aabbMin), &aabbMin, 0, 0, 0);
    clEnqueueReadBuffer(queue, m_aabbMaxBuffer, CL_TRUE, 0, sizeof(aabbMax), &aabbMax, 0, 0, 0);
    clFinish(queue);
}

void PMXAccelerator::release(Buffers &buffers) const
{
    const int nbuffers = buffers.count();
    for (int i = 0; i < nbuffers; i++) {
        Buffer &buffer = buffers[i];
        clReleaseMemObject(buffer.mem);
    }
    buffers.clear();
}

} /* namespace cl */
} /* namespace vpvl2 */
