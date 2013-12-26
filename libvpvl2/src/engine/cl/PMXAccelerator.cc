/**

 Copyright (c) 2010-2013  hkrn

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

#include "vpvl2/vpvl2.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#include "CL/cl.hpp"
#pragma clang diagnostic pop

#include "vpvl2/IApplicationContext.h"
#include "vpvl2/cl/PMXAccelerator.h"
#include "vpvl2/pmx/Bone.h"
#include "vpvl2/pmx/Material.h"
#include "vpvl2/pmx/Vertex.h"
#include "vpvl2/internal/util.h"

#include <vector>

namespace vpvl2
{
namespace cl
{

static const char kProgramCompileFlags[] = "-cl-fast-relaxed-math";
static const int kMaxBonesPerVertex = 4;

struct PMXAccelerator::PrivateContext {
    static void dumpPlatform(const ::cl::Platform &platform) {
        std::string value;
        platform.getInfo(CL_PLATFORM_NAME, &value);
        VPVL2_LOG(INFO, "CL_PLATFORM_NAME: " << value);
        platform.getInfo(CL_PLATFORM_VERSION, &value);
        VPVL2_LOG(INFO, "CL_PLATFORM_VERSION: " << value);
        platform.getInfo(CL_PLATFORM_PROFILE, &value);
        VPVL2_LOG(INFO, "CL_PLATFORM_PROFILE: " << value);
        platform.getInfo(CL_PLATFORM_VENDOR, &value);
        VPVL2_LOG(INFO, "CL_PLATFORM_VENDOR: " << value);
        platform.getInfo(CL_PLATFORM_EXTENSIONS, &value);
        VPVL2_LOG(INFO, "CL_PLATFORM_EXTENSIONS: " << value);
    }
    static void dumpDevice(const ::cl::Device &device) {
        std::string value;
        device.getInfo(CL_DEVICE_NAME, &value);
        VPVL2_LOG(INFO, "CL_DEVICE_NAME: " << value);
        device.getInfo(CL_DRIVER_VERSION, &value);
        VPVL2_LOG(INFO, "CL_DRIVER_VERSION: " << value);
        device.getInfo(CL_DEVICE_VERSION, &value);
        VPVL2_LOG(INFO, "CL_DEVICE_VERSION: " << value);
        device.getInfo(CL_DEVICE_PROFILE, &value);
        VPVL2_LOG(INFO, "CL_DEVICE_PROFILE: " << value);
        device.getInfo(CL_DEVICE_VENDOR, &value);
        VPVL2_LOG(INFO, "CL_DEVICE_VENDOR: " << value);
        device.getInfo(CL_DEVICE_EXTENSIONS, &value);
        VPVL2_LOG(INFO, "CL_DEVICE_EXTENSIONS: " << value);
    }

    PrivateContext(const Scene *sceneRef, IApplicationContext *applicationContextRef, IModel *modelRef, Scene::AccelerationType accelerationType)
        : sceneRef(sceneRef),
          modelRef(modelRef),
          context(0),
          commandQueue(0),
          program(0),
          performSkinningKernel(0),
          materialEdgeSizeBuffer(0),
          boneWeightsBuffer(0),
          boneIndicesBuffer(0),
          boneMatricesBuffer(0),
          aabbMinBuffer(0),
          aabbMaxBuffer(0),
          localWGSizeForPerformSkinning(0),
          isBufferAllocated(false)
    {
        cl_device_type deviceType;
        switch (accelerationType) {
        case Scene::kOpenCLAccelerationType1:
            deviceType = CL_DEVICE_TYPE_ALL;
            break;
        case Scene::kOpenCLAccelerationType2:
            deviceType = CL_DEVICE_TYPE_CPU;
            break;
        default:
            deviceType = CL_DEVICE_TYPE_DEFAULT;
            break;
        }
        std::vector< ::cl::Platform > platforms;
        ::cl::Platform::get(&platforms);
        if (platforms.size() > 0) {
            const ::cl::Platform &platform = platforms[0];
            cl_context_properties properties[] = {
                CL_CONTEXT_PLATFORM, (cl_context_properties)(platform)(),
    #if !defined(VPVL2_OS_OSX)
                CL_GL_CONTEXT_KHR, reinterpret_cast<cl_context_properties>(Scene::opaqueCurrentPlatformOpenGLContext()),
    #endif
    #if defined(VPVL2_LINK_EGL)
                CL_EGL_DISPLAY_KHR, reinterpret_cast<cl_context_properties>(Scene::opaqueCurrentPlatformOpenGLDevice()),
    #elif defined(VPVL2_OS_WINDOWS)
                CL_WGL_HDC_KHR, reinterpret_cast<cl_context_properties>(Scene::opaqueCurrentPlatformOpenGLDevice()),
    #elif defined(VPVL2_OS_OSX)
                CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, reinterpret_cast<cl_context_properties>(Scene::opaqueCurrentPlatformOpenGLDevice()),
    #elif defined(VPVL2_OS_LINUX)
                CL_GLX_DISPLAY_KHR, reinterpret_cast<cl_context_properties>(Scene::opaqueCurrentPlatformOpenGLDevice()),
    #endif
                0
            };
            dumpPlatform(platform);
            context = new ::cl::Context(deviceType, properties);
            devices = context->getInfo<CL_CONTEXT_DEVICES>();
            const IString *source = applicationContextRef->loadKernelSource(IApplicationContext::kModelSkinningKernel, 0);
            if (source && devices.size() > 0) {
                const ::cl::Device &device = devices[0];
                dumpDevice(device);
                const char *sourceText = reinterpret_cast<const char *>(source->toByteArray());
                const vsize sourceSize = source->length(IString::kUTF8);
                ::cl::Program::Sources sourceData(1, std::make_pair(sourceText, sourceSize));
                internal::deleteObject(program);
                program = new ::cl::Program(*context, sourceData);
                program->build(devices);
                internal::deleteObject(performSkinningKernel);
                performSkinningKernel = new ::cl::Kernel(*program, "performSkinning2");
                performSkinningKernel->getWorkGroupInfo(device, CL_KERNEL_WORK_GROUP_SIZE, &localWGSizeForPerformSkinning);
                commandQueue = new ::cl::CommandQueue(*context, device);
            }
            internal::deleteObject(source);
        }
    }
    ~PrivateContext() {
        localWGSizeForPerformSkinning = 0;
        isBufferAllocated = false;
        internal::deleteObject(materialEdgeSizeBuffer);
        internal::deleteObject(boneWeightsBuffer);
        internal::deleteObject(boneIndicesBuffer);
        internal::deleteObject(boneMatricesBuffer);
        internal::deleteObject(aabbMinBuffer);
        internal::deleteObject(aabbMaxBuffer);
        internal::deleteObject(performSkinningKernel);
        internal::deleteObject(program);
        internal::deleteObject(commandQueue);
        internal::deleteObject(context);
        modelRef = 0;
    }

    const Scene *sceneRef;
    IModel *modelRef;
    std::vector< ::cl::Device > devices;
    ::cl::Context *context;
    ::cl::CommandQueue *commandQueue;
    ::cl::Program *program;
    ::cl::Kernel *performSkinningKernel;
    ::cl::Buffer *materialEdgeSizeBuffer;
    ::cl::Buffer *boneWeightsBuffer;
    ::cl::Buffer *boneIndicesBuffer;
    ::cl::Buffer *boneMatricesBuffer;
    ::cl::Buffer *aabbMinBuffer;
    ::cl::Buffer *aabbMaxBuffer;
    Array<float32> boneTransform;
    vsize localWGSizeForPerformSkinning;
    bool isBufferAllocated;
};

PMXAccelerator::PMXAccelerator(const Scene *sceneRef, IApplicationContext *applicationContextRef, IModel *modelRef, Scene::AccelerationType accelerationType)
    : m_context(new PrivateContext(sceneRef, applicationContextRef, modelRef, accelerationType))
{
}

PMXAccelerator::~PMXAccelerator()
{
    internal::deleteObject(m_context);
}

bool PMXAccelerator::isAvailable() const
{
    return m_context->context && m_context->program;
}

void PMXAccelerator::upload(VertexBufferBridgeArray &buffers, const IModel::IndexBuffer *indexBufferRef)
{
    const int nbuffers = buffers.count();
    for (int i = 0; i < nbuffers; i++) {
        VertexBufferBridge &buffer = buffers[i];
        buffer.mem = new ::cl::BufferGL(*m_context->context, CL_MEM_READ_WRITE, buffer.name);
    }
    Array<IBone *> bones;
    Array<IVertex *> vertices;
    const IModel *modelRef = m_context->modelRef;
    modelRef->getBoneRefs(bones);
    modelRef->getVertexRefs(vertices);
    const int numBoneMatricesAllocs = bones.count() << 4;
    const int numBoneMatricesSize = numBoneMatricesAllocs * sizeof(float32);
    const int nvertices = vertices.count();
    const int numVerticesAlloc = nvertices * kMaxBonesPerVertex;
    Array<int> boneIndices;
    Array<float32> boneWeights, materialEdgeSize;
    boneIndices.resize(numVerticesAlloc);
    boneWeights.resize(numVerticesAlloc);
    materialEdgeSize.resize(nvertices);
    for (int i = 0; i < nvertices; i++) {
        const IVertex *vertex = vertices[i];
        for (int j = 0; j < kMaxBonesPerVertex; j++) {
            const IBone *bone = vertex->boneRef(j);
            const int index = i * kMaxBonesPerVertex + j;
            boneIndices[index] = bone ? bone->index() : -1;
            boneWeights[index] = vertex->weight(j);
        }
    }
    Array<IMaterial *> materials;
    modelRef->getMaterialRefs(materials);
    const int nmaterials = materials.count();
    vsize offset = 0;
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = materials[i];
        const int nindices = material->indexRange().count, offsetTo = offset + nindices;
        const IVertex::EdgeSizePrecision &edgeSize = material->edgeSize();
        for (int j = offset; j < offsetTo; j++) {
            const int index = indexBufferRef->indexAt(j);
            materialEdgeSize[index] = float32(edgeSize);
        }
        offset = offsetTo;
    }
    m_context->boneTransform.resize(numBoneMatricesAllocs);
    internal::deleteObject(m_context->materialEdgeSizeBuffer);
    m_context->materialEdgeSizeBuffer = new ::cl::Buffer(*m_context->context, CL_MEM_READ_ONLY, nvertices * sizeof(float32));
    internal::deleteObject(m_context->boneMatricesBuffer);
    m_context->boneMatricesBuffer = new ::cl::Buffer(*m_context->context, CL_MEM_READ_ONLY, numBoneMatricesSize);
    internal::deleteObject(m_context->boneIndicesBuffer);
    m_context->boneIndicesBuffer = new ::cl::Buffer(*m_context->context, CL_MEM_READ_ONLY, numVerticesAlloc * sizeof(int32));
    internal::deleteObject(m_context->boneWeightsBuffer);
    m_context->boneWeightsBuffer = new ::cl::Buffer(*m_context->context, CL_MEM_READ_ONLY, numVerticesAlloc * sizeof(float32));
    internal::deleteObject(m_context->boneMatricesBuffer);
    m_context->boneMatricesBuffer = new ::cl::Buffer(*m_context->context, CL_MEM_READ_ONLY, numBoneMatricesSize);
    internal::deleteObject(m_context->aabbMinBuffer);
    m_context->aabbMinBuffer = new ::cl::Buffer(*m_context->context, CL_MEM_READ_WRITE, sizeof(Vector3));
    internal::deleteObject(m_context->aabbMaxBuffer);
    m_context->aabbMaxBuffer = new ::cl::Buffer(*m_context->context, CL_MEM_READ_WRITE, sizeof(Vector3));
    ::cl::CommandQueue *queue = m_context->commandQueue;
    queue->enqueueWriteBuffer(*m_context->materialEdgeSizeBuffer, CL_TRUE, 0, nvertices * sizeof(float32), &materialEdgeSize[0]);
    queue->enqueueWriteBuffer(*m_context->boneIndicesBuffer, CL_TRUE, 0, numVerticesAlloc * sizeof(int32), &boneIndices[0]);
    queue->enqueueWriteBuffer(*m_context->boneWeightsBuffer, CL_TRUE, 0, numVerticesAlloc * sizeof(float32), &boneWeights[0]);
    m_context->isBufferAllocated = true;
}

void PMXAccelerator::update(const IModel::DynamicVertexBuffer *dynamicBufferRef, const VertexBufferBridge &buffer, Vector3 &aabbMin, Vector3 &aabbMax)
{
    if (!m_context->isBufferAllocated) {
        return;
    }
    Array<IBone *> bones;
    Array<IVertex *> vertices;
    const IModel *modelRef = m_context->modelRef;
    modelRef->getBoneRefs(bones);
    modelRef->getVertexRefs(vertices);
    int nvertices = vertices.count();
    const int nbones = bones.count();
    for (int i = 0; i < nbones; i++) {
        IBone *bone = bones[i];
        int index = i << 4;
        bone->localTransform().getOpenGLMatrix(&m_context->boneTransform[index]);
    }
    vsize nsize = (nbones * sizeof(float32)) << 4;
    ::cl::BufferGL *vertexBuffer = static_cast< ::cl::BufferGL *>(buffer.mem);
    std::vector< ::cl::Memory > objects;
    objects.push_back(*vertexBuffer);
    ::cl::CommandQueue *queue = m_context->commandQueue;
    queue->enqueueAcquireGLObjects(&objects);
    queue->enqueueWriteBuffer(*m_context->boneMatricesBuffer, CL_TRUE, 0, nsize, &m_context->boneTransform[0]);
    queue->enqueueWriteBuffer(*m_context->aabbMinBuffer, CL_TRUE, 0, sizeof(aabbMin), &aabbMin);
    queue->enqueueWriteBuffer(*m_context->aabbMaxBuffer, CL_TRUE, 0, sizeof(aabbMax), &aabbMax);
    int argumentIndex = 0;
    ::cl::Kernel *kernel = m_context->performSkinningKernel;
    kernel->setArg(argumentIndex++, sizeof(m_context->boneMatricesBuffer), m_context->boneMatricesBuffer);
    kernel->setArg(argumentIndex++, sizeof(m_context->boneWeightsBuffer), m_context->boneWeightsBuffer);
    kernel->setArg(argumentIndex++, sizeof(m_context->boneIndicesBuffer), m_context->boneIndicesBuffer);
    kernel->setArg(argumentIndex++, sizeof(m_context->materialEdgeSizeBuffer), m_context->materialEdgeSizeBuffer);
    const Scene *sceneRef = m_context->sceneRef;
    Vector3 lightDirection = sceneRef->lightRef()->direction();
    kernel->setArg(argumentIndex++, sizeof(lightDirection), &lightDirection);
    const ICamera *camera = sceneRef->cameraRef();
    float32 edgeScaleFactor = float32(modelRef->edgeScaleFactor(camera->position()) * modelRef->edgeWidth());
    kernel->setArg(argumentIndex++, sizeof(edgeScaleFactor), &edgeScaleFactor);
    kernel->setArg(argumentIndex++, sizeof(nvertices), &nvertices);
    vsize strideSize = dynamicBufferRef->strideSize() >> 4;
    kernel->setArg(argumentIndex++, sizeof(strideSize), &strideSize);
    vsize offsetPosition = dynamicBufferRef->strideOffset(IModel::DynamicVertexBuffer::kVertexStride) >> 4;
    kernel->setArg(argumentIndex++, sizeof(offsetPosition), &offsetPosition);
    vsize offsetNormal = dynamicBufferRef->strideOffset(IModel::DynamicVertexBuffer::kNormalStride) >> 4;
    kernel->setArg(argumentIndex++, sizeof(offsetNormal), &offsetNormal);
    //vsize offsetMorphDelta = dynamicBufferRef->strideOffset(IModel::DynamicVertexBuffer::kMorphDeltaStride) >> 4;
    //kernel->setArg(argumentIndex++, sizeof(offsetMorphDelta), &offsetMorphDelta);
    vsize offsetEdgeVertex = dynamicBufferRef->strideOffset(IModel::DynamicVertexBuffer::kEdgeVertexStride) >> 4;
    kernel->setArg(argumentIndex++, sizeof(offsetEdgeVertex), &offsetEdgeVertex);
    kernel->setArg(argumentIndex++, sizeof(m_context->aabbMinBuffer), m_context->aabbMinBuffer);
    kernel->setArg(argumentIndex++, sizeof(m_context->aabbMaxBuffer), m_context->aabbMaxBuffer);
    kernel->setArg(argumentIndex++, sizeof(vertexBuffer), vertexBuffer);
    const vsize local = m_context->localWGSizeForPerformSkinning;
    const ::cl::NDRange offsetRange, localRange(local);
    const ::cl::NDRange globalRange(local * ((nvertices + (local - 1)) / local));
    queue->enqueueNDRangeKernel(*m_context->performSkinningKernel, offsetRange, globalRange, localRange);
    queue->enqueueReleaseGLObjects(&objects);
    queue->enqueueReadBuffer(*m_context->aabbMinBuffer, CL_TRUE, 0, sizeof(aabbMin), &aabbMin);
    queue->enqueueReadBuffer(*m_context->aabbMaxBuffer, CL_TRUE, 0, sizeof(aabbMax), &aabbMax);
    queue->finish();
}

void PMXAccelerator::release(VertexBufferBridgeArray &buffers) const
{
    const int nbuffers = buffers.count();
    for (int i = 0; i < nbuffers; i++) {
        VertexBufferBridge &buffer = buffers[i];
        ::cl::BufferGL *mem = static_cast< ::cl::BufferGL *>(buffer.mem);
        internal::deleteObject(mem);
    }
    buffers.clear();
}

} /* namespace cl */
} /* namespace vpvl2 */
