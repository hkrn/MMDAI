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
#include "vpvl2/pmx/Bone.h"
#include "vpvl2/pmx/Material.h"
#include "vpvl2/pmx/Vertex.h"

namespace vpvl2
{
namespace cl
{

static const char kProgramCompileFlags[] = "-cl-fast-relaxed-math -DMAC -DGUID_ARG";
static const int kMaxBonesPerVertex = 4;

PMXAccelerator::PMXAccelerator(Context *contextRef, IModel *modelRef)
    : m_contextRef(contextRef),
      m_modelRef(modelRef),
      m_program(0),
      m_performSkinningKernel(0),
      m_verticesBuffer(0),
      m_materialEdgeSizeBuffer(0),
      m_boneWeightsBuffer(0),
      m_boneIndicesBuffer(0),
      m_boneMatricesBuffer(0),
      m_localWGSizeForPerformSkinning(0),
      m_boneTransform(0),
      m_isBufferAllocated(false)
{
    modelRef->getBoneRefs(m_bones);
    modelRef->getMaterialRefs(m_materials);
    modelRef->getVertexRefs(m_vertices);
}

PMXAccelerator::~PMXAccelerator()
{
    clReleaseProgram(m_program);
    m_program = 0;
    clReleaseKernel(m_performSkinningKernel);
    m_performSkinningKernel = 0;
    clReleaseMemObject(m_verticesBuffer);
    m_verticesBuffer = 0;;
    clReleaseMemObject(m_materialEdgeSizeBuffer);
    m_materialEdgeSizeBuffer = 0;
    clReleaseMemObject(m_boneIndicesBuffer);
    m_boneIndicesBuffer = 0;
    clReleaseMemObject(m_boneWeightsBuffer);
    m_boneWeightsBuffer = 0;
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
    const IString *source = m_contextRef->renderDelegate()->loadKernelSource(IRenderDelegate::kModelSkinningKernel, 0);
    const char *sourceText = reinterpret_cast<const char *>(source->toByteArray());
    const size_t sourceSize = source->length(IString::kUTF8);
    clReleaseProgram(m_program);
    cl_context context = m_contextRef->computeContext();
    m_program = clCreateProgramWithSource(context, 1, &sourceText, &sourceSize, &err);
    delete source;
    if (err != CL_SUCCESS) {
        log0(0, IRenderDelegate::kLogWarning, "Failed creating an OpenCL program: %d", err);
        return false;
    }
    cl_device_id device = m_contextRef->hostDevice();
    err = clBuildProgram(m_program, 1, &device, kProgramCompileFlags, 0, 0);
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
    clReleaseKernel(m_performSkinningKernel);
    m_performSkinningKernel = clCreateKernel(m_program, "performSkinning2", &err);
    if (err != CL_SUCCESS) {
        log0(0, IRenderDelegate::kLogWarning, "Failed creating a kernel (performSkinning2): %d", err);
        return false;
    }
    return true;
}

void PMXAccelerator::upload(GLuint buffer, const IModel::IIndexBuffer *indexBuffer, void *context)
{
    cl_int err;
    cl_context computeContext = m_contextRef->computeContext();
    clReleaseMemObject(m_verticesBuffer);
    m_verticesBuffer = clCreateFromGLBuffer(computeContext,
                                            CL_MEM_READ_WRITE,
                                            buffer,
                                            &err);
    if (err != CL_SUCCESS) {
        log0(context, IRenderDelegate::kLogWarning, "Failed creating OpenCL vertex buffer: %d", err);
        return;
    }
    const int nBoneMatricesAllocs = m_bones.count() << 4;
    const int nBoneMatricesSize = nBoneMatricesAllocs * sizeof(float);
    const int nvertices = m_vertices.count();
    const int nVerticesAlloc = nvertices * kMaxBonesPerVertex;
    Array<int> boneIndices;
    Array<float> boneWeights, materialEdgeSize;
    boneIndices.resize(nVerticesAlloc);
    boneWeights.resize(nVerticesAlloc);
    materialEdgeSize.resize(nvertices);
    for (int i = 0; i < nvertices; i++) {
        const IVertex *vertex = m_vertices[i];
        for (int j = 0; j < kMaxBonesPerVertex; j++) {
            const IBone *bone = vertex->bone(j);
            const int index = i * kMaxBonesPerVertex + j;
            boneIndices[index] = bone ? bone->index() : -1;
            boneWeights[index] = vertex->weight(j);
        }
    }
    const int nmaterials = m_materials.count();
    size_t offset = 0;
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = m_materials[i];
        const int nindices = material->indices(), offsetTo = offset + nindices;
        const float edgeSize = material->edgeSize();
        for (int j = offset; j < offsetTo; j++) {
            const int index = indexBuffer->indexAt(j);
            materialEdgeSize[index] = edgeSize;
        }
        offset += nindices;
    }
    delete[] m_boneTransform;
    m_boneTransform = new float[nBoneMatricesAllocs];
    clReleaseMemObject(m_materialEdgeSizeBuffer);
    m_materialEdgeSizeBuffer = clCreateBuffer(computeContext, CL_MEM_READ_ONLY, nvertices * sizeof(float), 0, &err);
    if (err != CL_SUCCESS) {
        log0(context, IRenderDelegate::kLogWarning, "Failed creating materialEdgeSizeBuffer: %d", err);
        return;
    }
    clReleaseMemObject(m_boneMatricesBuffer);
    m_boneMatricesBuffer = clCreateBuffer(computeContext, CL_MEM_READ_ONLY, nBoneMatricesSize, 0, &err);
    if (err != CL_SUCCESS) {
        log0(context, IRenderDelegate::kLogWarning, "Failed creating boneMatricesBuffer: %d", err);
        return;
    }
    clReleaseMemObject(m_boneIndicesBuffer);
    m_boneIndicesBuffer = clCreateBuffer(computeContext, CL_MEM_READ_ONLY, nVerticesAlloc * sizeof(int), 0, &err);
    if (err != CL_SUCCESS) {
        log0(context, IRenderDelegate::kLogWarning, "Failed creating boneIndicesBuffer: %d", err);
        return;
    }
    clReleaseMemObject(m_boneWeightsBuffer);
    m_boneWeightsBuffer = clCreateBuffer(computeContext, CL_MEM_READ_ONLY, nVerticesAlloc * sizeof(float), 0, &err);
    if (err != CL_SUCCESS) {
        log0(context, IRenderDelegate::kLogWarning, "Failed creating boneWeightsBuffer: %d", err);
        return;
    }
    clReleaseMemObject(m_boneMatricesBuffer);
    m_boneMatricesBuffer = clCreateBuffer(computeContext, CL_MEM_READ_ONLY, nBoneMatricesSize, 0, &err);
    if (err != CL_SUCCESS) {
        log0(context, IRenderDelegate::kLogWarning, "Failed creating boneMatricesBuffer: %d", err);
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
        log0(context, IRenderDelegate::kLogWarning, "Failed getting kernel work group information (CL_KERNEL_WORK_GROUP_SIZE): %d", err);
        return;
    }
    cl_command_queue queue = m_contextRef->commandQueue();
    err = clEnqueueWriteBuffer(queue, m_materialEdgeSizeBuffer, CL_TRUE, 0, nvertices * sizeof(float), &materialEdgeSize[0], 0, 0, 0);
    if (err != CL_SUCCESS) {
        log0(0, IRenderDelegate::kLogWarning, "Failed enqueue a command to write materialEdgeSizeBuffer: %d", err);
        return;
    }
    err = clEnqueueWriteBuffer(queue, m_boneIndicesBuffer, CL_TRUE, 0, nVerticesAlloc * sizeof(int), &boneIndices[0], 0, 0, 0);
    if (err != CL_SUCCESS) {
        log0(0, IRenderDelegate::kLogWarning, "Failed enqueue a command to write boneIndicesBuffer: %d", err);
        return;
    }
    err = clEnqueueWriteBuffer(queue, m_boneWeightsBuffer, CL_TRUE, 0, nVerticesAlloc * sizeof(float), &boneWeights[0], 0, 0, 0);
    if (err != CL_SUCCESS) {
        log0(0, IRenderDelegate::kLogWarning, "Failed enqueue a command to write boneWeightsBuffer: %d", err);
        return;
    }
    m_isBufferAllocated = true;
}

void PMXAccelerator::update(const IModel::IDynamicVertexBuffer *dynamicBuffer,
                            const Scene *scene,
                            Vector3 &aabbMin,
                            Vector3 &aabbMax)
{
    if (!m_isBufferAllocated)
        return;
    const int nvertices = m_vertices.count();
    const int nbones = m_bones.count();
    for (int i = 0; i < nbones; i++) {
        IBone *bone = m_bones[i];
        int index = i << 4;
        bone->localTransform().getOpenGLMatrix(&m_boneTransform[index]);
    }
    size_t nsize = (nbones * sizeof(float)) << 4;
    cl_int err;
    size_t local, global;
    cl_command_queue queue = m_contextRef->commandQueue();
    /* force flushing OpenGL commands to acquire GL objects by OpenCL */
    glFinish();
    clEnqueueAcquireGLObjects(queue, 1, &m_verticesBuffer, 0, 0, 0);
    err = clEnqueueWriteBuffer(queue, m_boneMatricesBuffer, CL_TRUE, 0, nsize, m_boneTransform, 0, 0, 0);
    if (err != CL_SUCCESS) {
        log0(0, IRenderDelegate::kLogWarning, "Failed enqueue a command to write boneMatricesBuffer: %d", err);
        return;
    }
    int argumentIndex = 0;
    err = clSetKernelArg(m_performSkinningKernel, argumentIndex++, sizeof(m_boneMatricesBuffer), &m_boneMatricesBuffer);
    if (err != CL_SUCCESS) {
        log0(0, IRenderDelegate::kLogWarning, "Failed setting 1st argument of kernel (localMatrices): %d", err);
        return;
    }
    err = clSetKernelArg(m_performSkinningKernel, argumentIndex++, sizeof(m_boneWeightsBuffer), &m_boneWeightsBuffer);
    if (err != CL_SUCCESS) {
        log0(0, IRenderDelegate::kLogWarning, "Failed setting 2nd argument of kernel (boneWeights): %d", err);
        return;
    }
    err = clSetKernelArg(m_performSkinningKernel, argumentIndex++, sizeof(m_boneIndicesBuffer), &m_boneIndicesBuffer);
    if (err != CL_SUCCESS) {
        log0(0, IRenderDelegate::kLogWarning, "Failed setting 3rd argument of kernel (boneIndices): %d", err);
        return;
    }
    err = clSetKernelArg(m_performSkinningKernel, argumentIndex++, sizeof(m_materialEdgeSizeBuffer), &m_materialEdgeSizeBuffer);
    if (err != CL_SUCCESS) {
        log0(0, IRenderDelegate::kLogWarning, "Failed setting %dth argument of kernel (materialEdgeSize): %d", argumentIndex, err);
        return;
    }
    const Vector3 &lightDirection = scene->light()->direction();
    err = clSetKernelArg(m_performSkinningKernel, argumentIndex++, sizeof(lightDirection), &lightDirection);
    if (err != CL_SUCCESS) {
        log0(0, IRenderDelegate::kLogWarning, "Failed setting %dth argument of kernel (lightDirection): %d", argumentIndex, err);
        return;
    }
    const ICamera *camera = scene->camera();
    const Vector3 &cameraPosition = camera->position() + Vector3(0, 0, camera->distance());
    const Scalar &edgeScaleFactor = m_modelRef->edgeScaleFactor(cameraPosition) * m_modelRef->edgeWidth();
    err = clSetKernelArg(m_performSkinningKernel, argumentIndex++, sizeof(edgeScaleFactor), &edgeScaleFactor);
    if (err != CL_SUCCESS) {
        log0(0, IRenderDelegate::kLogWarning, "Failed setting %dth argument of kernel (edgeScaleFactor): %d", argumentIndex, err);
        return;
    }
    err = clSetKernelArg(m_performSkinningKernel, argumentIndex++, sizeof(nvertices), &nvertices);
    if (err != CL_SUCCESS) {
        log0(0, IRenderDelegate::kLogWarning, "Failed setting %dth argument of kernel (nvertices): %d", argumentIndex, err);
        return;
    }
    size_t strideSize = dynamicBuffer->strideSize() >> 4;
    err = clSetKernelArg(m_performSkinningKernel, argumentIndex++, sizeof(strideSize), &strideSize);
    if (err != CL_SUCCESS) {
        log0(0, IRenderDelegate::kLogWarning, "Failed setting %th argument of kernel (strideSize): %d", argumentIndex, err);
        return;
    }
    size_t offsetPosition = dynamicBuffer->strideOffset(IModel::IDynamicVertexBuffer::kVertexStride) >> 4;
    err = clSetKernelArg(m_performSkinningKernel, argumentIndex++, sizeof(offsetPosition), &offsetPosition);
    if (err != CL_SUCCESS) {
        log0(0, IRenderDelegate::kLogWarning, "Failed setting %th argument of kernel (offsetPosition): %d", argumentIndex, err);
        return;
    }
    size_t offsetNormal = dynamicBuffer->strideOffset(IModel::IDynamicVertexBuffer::kNormalStride) >> 4;
    err = clSetKernelArg(m_performSkinningKernel, argumentIndex++, sizeof(offsetNormal), &offsetNormal);
    if (err != CL_SUCCESS) {
        log0(0, IRenderDelegate::kLogWarning, "Failed setting %th argument of kernel (offsetNormal): %d", argumentIndex, err);
        return;
    }
    size_t offsetMorphDelta = dynamicBuffer->strideOffset(IModel::IDynamicVertexBuffer::kMorphDeltaStride) >> 4;
    err = clSetKernelArg(m_performSkinningKernel, argumentIndex++, sizeof(offsetMorphDelta), &offsetMorphDelta);
    if (err != CL_SUCCESS) {
        log0(0, IRenderDelegate::kLogWarning, "Failed setting %dth argument of kernel (offsetMorphDelta): %d", argumentIndex, err);
        return;
    }
    size_t offsetEdgeVertex = dynamicBuffer->strideOffset(IModel::IDynamicVertexBuffer::kEdgeVertexStride) >> 4;
    err = clSetKernelArg(m_performSkinningKernel, argumentIndex++, sizeof(offsetEdgeVertex), &offsetEdgeVertex);
    if (err != CL_SUCCESS) {
        log0(0, IRenderDelegate::kLogWarning, "Failed setting %dth argument of kernel (offsetEdgeVertex): %d", argumentIndex, err);
        return;
    }
    err = clSetKernelArg(m_performSkinningKernel, argumentIndex++, sizeof(m_verticesBuffer), &m_verticesBuffer);
    if (err != CL_SUCCESS) {
        log0(0, IRenderDelegate::kLogWarning, "Failed setting %th argument of kernel (vertices): %d", argumentIndex, err);
        return;
    }
    local = m_localWGSizeForPerformSkinning;
    global = local * ((nvertices + (local - 1)) / local);
    err = clEnqueueNDRangeKernel(queue, m_performSkinningKernel, 1, 0, &global, &local, 0, 0, 0);
    if (err != CL_SUCCESS) {
        log0(0, IRenderDelegate::kLogWarning, "Failed enqueue executing kernel: %d", err);
        return;
    }
    clEnqueueReleaseGLObjects(queue, 1, &m_verticesBuffer, 0, 0, 0);
    clFinish(queue);
    /* hack */
    size_t offset = dynamicBuffer->strideOffset(IModel::IDynamicVertexBuffer::kVertexStride);
    size_t size = dynamicBuffer->strideSize();
    for (int i = 0; i < nvertices; i++) {
        const uint8_t *ptr = static_cast<const uint8_t *>(dynamicBuffer->bytes()) + offset + size * i;
        const Vector3 &v = *reinterpret_cast<const Vector3 *>(ptr);
        aabbMin.setMin(v);
        aabbMax.setMax(v);
    }
}

void PMXAccelerator::log0(void *context, IRenderDelegate::LogLevel level, const char *format...)
{
    va_list ap;
    va_start(ap, format);
    m_contextRef->renderDelegate()->log(context, level, format, ap);
    va_end(ap);
}

} /* namespace cl */
} /* namespace vpvl2 */
