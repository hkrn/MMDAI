/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2012  hkrn                                    */
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

#include <vpvl/internal/gl2.h>

#include <btBulletDynamicsCommon.h>

#ifdef VPVL_LINK_ASSIMP
#include <aiScene.h>
#include <map>
#endif

namespace vpvl
{
namespace gl2
{

const GLsizei kShadowMappingTextureWidth = 1024;
const GLsizei kShadowMappingTextureHeight = 1024;

static void CreateLookAt(const Vector3 &eye, float matrix[16])
{
#ifndef VPVL_BUILD_IOS
    glMatrixMode(GL_MODELVIEW);
    //gluLookAt(eye.x(), eye.y(), eye.z(), 0, 0, 0, 0, 1, 0);
    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
    /*
    static const Vector3 zero(0.0f, 0.0f, 0.0f), up(0.0f, 1.0f, 0.0f);
    const Vector3 &z = (zero - eye).normalized();
    const Vector3 &x = z.cross(up).normalized();
    const Vector3 &y = x.cross(z);
    float m[] = {
        x.x(), x.y(), x.z(), 0.0f,
        y.x(), y.y(), y.z(), 0.0f,
        -z.z(), -z.z(), -z.z(), 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    memcpy(matrix, m, sizeof(float) * 16);
    */
#endif
}

#ifdef VPVL_ENABLE_OPENCL
class Accelerator
{
public:
    Accelerator(Renderer::IDelegate *delegate)
        : m_delegate(delegate),
          m_context(0),
          m_queue(0),
          m_device(0),
          m_updateBoneMatricesKernel(0),
          m_performSkinningKernel(0),
          m_program(0)
    {
    }
    ~Accelerator() {
        clReleaseProgram(m_program);
        m_program = 0;
        clReleaseKernel(m_performSkinningKernel);
        m_performSkinningKernel = 0;
        clReleaseKernel(m_updateBoneMatricesKernel);
        m_updateBoneMatricesKernel = 0;
        clReleaseCommandQueue(m_queue);
        m_queue = 0;
        clReleaseContext(m_context);
        m_context = 0;
    }

    bool isAvailable() const {
        return m_context && m_queue && m_program;
    }
    bool initializeContext() {
        if (m_context && m_queue)
            return true;
        cl_int err = 0;
        cl_uint nplatforms;
        err = clGetPlatformIDs(0, 0, &nplatforms);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed getting number of OpenCL platforms: %d", err);
            return false;
        }
        cl_platform_id *platforms = new cl_platform_id[nplatforms];
        err = clGetPlatformIDs(nplatforms, platforms, 0);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed getting OpenCL platforms: %d", err);
            delete[] platforms;
            return false;
        }
        for (int i = 0; i < nplatforms; i++) {
            cl_char buffer[BUFSIZ];
            clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, sizeof(buffer), buffer, 0);
            m_delegate->log(Renderer::kLogInfo, "CL_PLATFORM_VENDOR: %s", buffer);
            clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, sizeof(buffer), buffer, 0);
            m_delegate->log(Renderer::kLogInfo, "CL_PLATFORM_NAME: %s", buffer);
            clGetPlatformInfo(platforms[i], CL_PLATFORM_VERSION, sizeof(buffer), buffer, 0);
            m_delegate->log(Renderer::kLogInfo, "CL_PLATFORM_VERSION: %s", buffer);
        }
        cl_platform_id firstPlatform = platforms[0];
        err = clGetDeviceIDs(firstPlatform, CL_DEVICE_TYPE_ALL, 1, &m_device, 0);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed getting a OpenCL device: %d", err);
            delete[] platforms;
            return false;
        }
        {
            cl_char buffer[BUFSIZ];
            cl_uint frequency, addressBits;
            cl_device_type type;
            clGetDeviceInfo(m_device, CL_DRIVER_VERSION, sizeof(buffer), buffer, 0);
            m_delegate->log(Renderer::kLogInfo, "CL_DRIVER_VERSION: %s", buffer);
            clGetDeviceInfo(m_device, CL_DEVICE_NAME, sizeof(buffer), buffer, 0);
            m_delegate->log(Renderer::kLogInfo, "CL_DEVICE_NAME: %s", buffer);
            clGetDeviceInfo(m_device, CL_DEVICE_VENDOR, sizeof(buffer), buffer, 0);
            m_delegate->log(Renderer::kLogInfo, "CL_DEVICE_VENDOR: %s", buffer);
            clGetDeviceInfo(m_device, CL_DEVICE_TYPE, sizeof(type), &type, 0);
            m_delegate->log(Renderer::kLogInfo, "CL_DEVICE_TYPE: %d", type);
            clGetDeviceInfo(m_device, CL_DEVICE_ADDRESS_BITS, sizeof(addressBits), &addressBits, 0);
            m_delegate->log(Renderer::kLogInfo, "CL_DEVICE_ADDRESS_BITS: %d", addressBits);
            clGetDeviceInfo(m_device, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(frequency), &frequency, 0);
            m_delegate->log(Renderer::kLogInfo, "CL_DEVICE_MAX_CLOCK_FREQUENCY: %d", frequency);
            clGetDeviceInfo(m_device, CL_DEVICE_EXTENSIONS, sizeof(buffer), buffer, 0);
            m_delegate->log(Renderer::kLogInfo, "CL_DEVICE_EXTENSIONS: %s", buffer);
        }
        cl_context_properties props[] = {
            CL_CONTEXT_PLATFORM,
            reinterpret_cast<cl_context_properties>(firstPlatform),
    #ifdef __APPLE__
            CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE,
            reinterpret_cast<cl_context_properties>(CGLGetShareGroup(CGLGetCurrentContext())),
    #endif
            0
        };
        clReleaseContext(m_context);
        m_context = clCreateContext(props, 1, &m_device, 0, 0, &err);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed initialize a OpenCL context: %d", err);
            delete[] platforms;
            return false;
        }
        clReleaseCommandQueue(m_queue);
        m_queue = clCreateCommandQueue(m_context, m_device, 0, &err);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed initialize a OpenCL command queue: %d", err);
            delete[] platforms;
            return false;
        }
        delete[] platforms;
        return true;
    }
    bool createKernelPrograms() {
        if (!m_context)
            return false;
        cl_int err;
        const std::string &source = m_delegate->loadKernel(Renderer::kModelSkinningKernel);
        const char *sourceText = source.c_str();
        const size_t sourceSize = source.size();
        clReleaseProgram(m_program);
        m_program = clCreateProgramWithSource(m_context, 1, &sourceText, &sourceSize, &err);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed creating an OpenCL program: %d", err);
            return false;
        }
        const char *flags = "-cl-mad-enable -DMAC -DGUID_ARG";
        err = clBuildProgram(m_program, 1, &m_device, flags, 0, 0);
        if (err != CL_SUCCESS) {
            size_t buildLogSize;
            clGetProgramBuildInfo(m_program, m_device, CL_PROGRAM_BUILD_LOG, 0, 0, &buildLogSize);
            cl_char *buildLog = new cl_char[buildLogSize + 1];
            clGetProgramBuildInfo(m_program, m_device, CL_PROGRAM_BUILD_LOG, buildLogSize, buildLog, 0);
            buildLog[buildLogSize] = 0;
            m_delegate->log(Renderer::kLogWarning, "Failed building a program: %s", buildLog);
            delete[] buildLog;
            return false;
        }
        clReleaseKernel(m_updateBoneMatricesKernel);
        m_updateBoneMatricesKernel = clCreateKernel(m_program, "updateBoneMatrices", &err);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed creating a kernel: %d", err);
            return false;
        }
        clReleaseKernel(m_performSkinningKernel);
        m_performSkinningKernel = clCreateKernel(m_program, "performSkinning", &err);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed creating a kernel: %d", err);
            return false;
        }
        return true;
    }
    void uploadModel(PMDModelUserData *userData, const PMDModel *model) {
        if (!isAvailable())
            return;
        cl_int err;
        userData->vertexBufferForCL = 0;
        userData->boneMatricesBuffer = 0;
        userData->originMatricesBuffer = 0;
        userData->outputMatricesBuffer = 0;
        userData->boneTransform = 0;
        userData->originTransform = 0;
        userData->bone1Indices = 0;
        userData->bone2Indices = 0;
        userData->weights = 0;
        userData->isBufferAllocated = true;
        userData->vertexBufferForCL = clCreateFromGLBuffer(m_context,
                                                           CL_MEM_READ_WRITE,
                                                           userData->vertexBufferObjects[kModelVertices],
                                                           &err);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed creating OpenCL vertex buffer: %d", err);
            userData->isBufferAllocated = false;
        }
        const int nBoneMatricesAllocs = model->bones().count() << 4;
        const int nBoneMatricesSize = nBoneMatricesAllocs * sizeof(float);
        userData->boneTransform = new float[nBoneMatricesAllocs];
        userData->originTransform = new float[nBoneMatricesAllocs];
        const VertexList &vertices = model->vertices();
        const int nVerticesAlloc = vertices.count();
        userData->bone1Indices = new int[nVerticesAlloc];
        userData->bone2Indices = new int[nVerticesAlloc];
        userData->weights = new float[nVerticesAlloc];
        for (int i = 0; i < nVerticesAlloc; i++) {
            const Vertex *vertex = vertices[i];
            userData->bone1Indices[i] = vertex->bone1();
            userData->bone2Indices[i] = vertex->bone2();
            userData->weights[i] = vertex->weight();
        }
        userData->boneMatricesBuffer = clCreateBuffer(m_context, CL_MEM_READ_ONLY, nBoneMatricesSize, 0, &err);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed creating boneMatricesBuffer: %d", err);
            userData->isBufferAllocated = false;
        }
        userData->originMatricesBuffer = clCreateBuffer(m_context, CL_MEM_READ_ONLY, nBoneMatricesSize, 0, &err);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed creating originMatricesBuffer: %d", err);
            userData->isBufferAllocated = false;
        }
        userData->outputMatricesBuffer = clCreateBuffer(m_context, CL_MEM_READ_WRITE, nBoneMatricesSize, 0, &err);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed creating outputMatricesBuffer %d", err);
            userData->isBufferAllocated = false;
        }
        userData->weightsBuffer = clCreateBuffer(m_context, CL_MEM_READ_ONLY, nVerticesAlloc * sizeof(float), 0, &err);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed creating weightsBuffer: %d", err);
            userData->isBufferAllocated = false;
        }
        userData->bone1IndicesBuffer = clCreateBuffer(m_context, CL_MEM_READ_ONLY, nVerticesAlloc * sizeof(int), 0, &err);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed creating bone1IndicesBuffer: %d", err);
            userData->isBufferAllocated = false;
        }
        userData->bone2IndicesBuffer = clCreateBuffer(m_context, CL_MEM_READ_ONLY, nVerticesAlloc * sizeof(int), 0, &err);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed creating bone2IndicesBuffer: %d", err);
            userData->isBufferAllocated = false;
        }
        err = clGetKernelWorkGroupInfo(m_updateBoneMatricesKernel,
                                       m_device,
                                       CL_KERNEL_WORK_GROUP_SIZE,
                                       sizeof(userData->localWGSizeForUpdateBoneMatrices),
                                       &userData->localWGSizeForUpdateBoneMatrices,
                                       0);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed getting kernel work group information (CL_KERNEL_WORK_GROUP_SIZE): %d", err);
            userData->isBufferAllocated = false;
        }
        err = clGetKernelWorkGroupInfo(m_performSkinningKernel,
                                       m_device,
                                       CL_KERNEL_WORK_GROUP_SIZE,
                                       sizeof(userData->localWGSizeForPerformSkinning),
                                       &userData->localWGSizeForPerformSkinning,
                                       0);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed getting kernel work group information (CL_KERNEL_WORK_GROUP_SIZE): %d", err);
            userData->isBufferAllocated = false;
        }
    }
    void deleteModel(PMDModelUserData *userData) {
        if (!isAvailable())
            return;
        delete[] userData->boneTransform;
        delete[] userData->originTransform;
        delete[] userData->bone1Indices;
        delete[] userData->bone2Indices;
        delete[] userData->weights;
        clReleaseMemObject(userData->vertexBufferForCL);
        clReleaseMemObject(userData->boneMatricesBuffer);
        clReleaseMemObject(userData->originMatricesBuffer);
        clReleaseMemObject(userData->outputMatricesBuffer);
        clReleaseMemObject(userData->bone1IndicesBuffer);
        clReleaseMemObject(userData->bone2IndicesBuffer);
        clReleaseMemObject(userData->weightsBuffer);
        userData->isBufferAllocated = false;
    }
    void updateModel(PMDModelUserData *userData, PMDModel *model) {
        if (!isAvailable() || !userData->isBufferAllocated)
            return;
        const BoneList &bones = model->bones();
        const int nbones = bones.count();
        Transform transform = Transform::getIdentity();
        for (int i = 0; i < nbones; i++) {
            Bone *bone = bones[i];
            int index = i << 4;
#define SOFTWARE_BONE_TRANSFORM
#ifndef SOFTWARE_BONE_TRANSFORM
            bone->localTransform().getOpenGLMatrix(&userData->boneTransform[index]);
            transform.setOrigin(bone->originPosition());
            transform.getOpenGLMatrix(&userData->originTransform[index]);
#else
            bone->getSkinTransform(transform);
            transform.getOpenGLMatrix(&userData->boneTransform[index]);
#endif
        }
        size_t nsize = (nbones * sizeof(float)) << 4;
        cl_int err;
#ifndef SOFTWARE_BONE_TRANSFORM
        err = clEnqueueWriteBuffer(m_queue, userData->boneMatricesBuffer, CL_TRUE, 0, nsize, userData->boneTransform, 0, 0, 0);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed enqueue a command to write bone matrices buffer: %d", err);
            return;
        }
        err = clEnqueueWriteBuffer(m_queue, userData->originMatricesBuffer, CL_TRUE, 0, nsize, userData->originTransform, 0, 0, 0);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed enqueue a command to write origin matrices buffer: %d", err);
            return;
        }
        err = clSetKernelArg(m_updateBoneMatricesKernel, 0, sizeof(userData->boneMatricesBuffer), &userData->boneMatricesBuffer);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed setting 1st argument of kernel (boneMatricesBuffer): %d", err);
            return;
        }
        err = clSetKernelArg(m_updateBoneMatricesKernel, 1, sizeof(userData->originMatricesBuffer), &userData->originMatricesBuffer);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed setting 2nd argument of kernel (originMatricesBuffer): %d", err);
            return;
        }
        err = clSetKernelArg(m_updateBoneMatricesKernel, 2, sizeof(int), &nbones);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed setting 3th argument of kernel (nbones): %d", err);
            return;
        }
        err = clSetKernelArg(m_updateBoneMatricesKernel, 3, sizeof(userData->outputMatricesBuffer), &userData->outputMatricesBuffer);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed setting 4rd argument of kernel (outputMatricesBuffer): %d", err);
            return;
        }
        size_t local = userData->localWGSizeForUpdateBoneMatrices;
        size_t global = local * ((nbones + (local - 1)) / local);
        err = clEnqueueNDRangeKernel(m_queue, m_updateBoneMatricesKernel, 1, 0, &global, &local, 0, 0, 0);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed enqueue executing kernel");
            return;
        }
        clFinish(m_queue);
#else
        size_t local, global;
        err = clEnqueueWriteBuffer(m_queue, userData->outputMatricesBuffer, CL_TRUE, 0, nsize, userData->boneTransform, 0, 0, 0);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed enqueue a command to write output matrices buffer: %d", err);
            return;
        }
#endif
        /* force flushing OpenGL commands to acquire GL objects by OpenCL */
        glFinish();
        clEnqueueAcquireGLObjects(m_queue, 1, &userData->vertexBufferForCL, 0, 0, 0);
        int nvertices = model->vertices().count();
        err = clEnqueueWriteBuffer(m_queue, userData->bone1IndicesBuffer, CL_TRUE, 0, nvertices * sizeof(int), userData->bone1Indices, 0, 0, 0);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed enqueue a command to write bone1 indices buffer: %d", err);
            return;
        }
        err = clEnqueueWriteBuffer(m_queue, userData->bone2IndicesBuffer, CL_TRUE, 0, nvertices * sizeof(int), userData->bone2Indices, 0, 0, 0);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed enqueue a command to write bone2 indices buffer: %d", err);
            return;
        }
        err = clEnqueueWriteBuffer(m_queue, userData->weightsBuffer, CL_TRUE, 0, nvertices * sizeof(float), userData->weights, 0, 0, 0);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed enqueue a command to write weights buffer: %d", err);
            return;
        }
        err = clSetKernelArg(m_performSkinningKernel, 0, sizeof(userData->outputMatricesBuffer), &userData->outputMatricesBuffer);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed setting 1st argument of kernel (skinningMatrices): %d", err);
            return;
        }
        err = clSetKernelArg(m_performSkinningKernel, 1, sizeof(userData->weightsBuffer), &userData->weightsBuffer);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed setting 2nd argument of kernel (weights): %d", err);
            return;
        }
        err = clSetKernelArg(m_performSkinningKernel, 2, sizeof(userData->bone1IndicesBuffer), &userData->bone1IndicesBuffer);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed setting 3rd argument of kernel (bone1Indices): %d", err);
            return;
        }
        err = clSetKernelArg(m_performSkinningKernel, 3, sizeof(userData->bone2IndicesBuffer), &userData->bone2IndicesBuffer);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed setting 4th argument of kernel (bone2Indices): %d", err);
            return;
        }
        const Vector3 &lightPosition = model->lightPosition();
        err = clSetKernelArg(m_performSkinningKernel, 4, sizeof(lightPosition), &lightPosition);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed setting 5th argument of kernel (lightPosition): %d", err);
            return;
        }
        err = clSetKernelArg(m_performSkinningKernel, 5, sizeof(nvertices), &nvertices);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed setting 6th argument of kernel (nvertices): %d", err);
            return;
        }
        size_t strideSize = model->strideSize(PMDModel::kVerticesStride) >> 4;
        err = clSetKernelArg(m_performSkinningKernel, 6, sizeof(strideSize), &strideSize);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed setting 7th argument of kernel (strideSize): %d", err);
            return;
        }
        size_t offsetPosition = model->strideOffset(PMDModel::kVerticesStride) >> 4;
        err = clSetKernelArg(m_performSkinningKernel, 7, sizeof(offsetPosition), &offsetPosition);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed setting 8th argument of kernel (offsetPosition): %d", err);
            return;
        }
        size_t offsetNormal = model->strideOffset(PMDModel::kNormalsStride) >> 4;
        err = clSetKernelArg(m_performSkinningKernel, 8, sizeof(offsetNormal), &offsetNormal);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed setting 9th argument of kernel (offsetNormal): %d", err);
            return;
        }
        size_t offsetToonTexture = model->strideOffset(PMDModel::kToonTextureStride) >> 4;
        err = clSetKernelArg(m_performSkinningKernel, 9, sizeof(offsetToonTexture), &offsetToonTexture);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed setting 10th argument of kernel (offsetTexCoord): %d", err);
            return;
        }
        size_t offsetEdge = model->strideOffset(PMDModel::kEdgeVerticesStride) >> 4;
        err = clSetKernelArg(m_performSkinningKernel, 10, sizeof(offsetEdge), &offsetEdge);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed setting 11th argument of kernel (offsetEdge): %d", err);
            return;
        }
        err = clSetKernelArg(m_performSkinningKernel, 11, sizeof(userData->vertexBufferForCL), &userData->vertexBufferForCL);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed setting 12th argument of kernel (vertices): %d", err);
            return;
        }
        local = userData->localWGSizeForUpdateBoneMatrices;
        global = local * ((nvertices + (local - 1)) / local);
        err = clEnqueueNDRangeKernel(m_queue, m_performSkinningKernel, 1, 0, &global, &local, 0, 0, 0);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed enqueue executing kernel: %d", err);
            return;
        }
        clEnqueueReleaseGLObjects(m_queue, 1, &userData->vertexBufferForCL, 0, 0, 0);
        clFinish(m_queue);
    }

private:
    Renderer::IDelegate *m_delegate;
    cl_context m_context;
    cl_command_queue m_queue;
    cl_device_id m_device;
    cl_kernel m_updateBoneMatricesKernel;
    cl_kernel m_performSkinningKernel;
    cl_program m_program;
};
#else
class Accelerator {
public:
    Accelerator(Renderer::IDelegate * /* delegate */) {}
    ~Accelerator() {}

    bool isAvailable() const {
        return false;
    }
    bool initializeContext() {
        return true;
    }
    bool createKernelPrograms() {
        return true;
    }

    void uploadModel(PMDModelUserData * /* userData */, const PMDModel * /* model */) {
    }
    void deleteModel(PMDModelUserData * /* userData */) {
    }
    void updateModel(PMDModelUserData * /* userData */, PMDModel * /* model */) {
    }
};

#endif

#ifdef VPVL_LINK_QT
class ShaderProgram : protected QGLFunctions
        #else
class ShaderProgram
        #endif
{
public:
    ShaderProgram(Renderer::IDelegate *delegate)
        : m_program(0),
          m_delegate(delegate),
          m_modelViewUniformLocation(0),
          m_projectionUniformLocation(0),
          m_positionAttributeLocation(0),
          m_normalAttributeLocation(0),
          m_message(0)
    {
    }
    virtual ~ShaderProgram() {
        if (m_program) {
            glDeleteProgram(m_program);
            m_program = 0;
        }
        delete[] m_message;
        m_message = 0;
        m_modelViewUniformLocation = 0;
        m_projectionUniformLocation = 0;
        m_positionAttributeLocation = 0;
        m_normalAttributeLocation = 0;
    }

#ifdef VPVL_LINK_QT
    virtual void initializeContext(const QGLContext *context) {
        initializeGLFunctions(context);
    }
#endif

    virtual bool load(const char *vertexShaderSource, const char *fragmentShaderSource) {
        GLuint vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
        GLuint fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
        if (vertexShader && fragmentShader) {
            GLuint program = glCreateProgram();
            glAttachShader(program, vertexShader);
            glAttachShader(program, fragmentShader);
            glLinkProgram(program);
            glValidateProgram(program);
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            GLint linked;
            glGetProgramiv(program, GL_LINK_STATUS, &linked);
            if (!linked) {
                GLint len;
                glGetShaderiv(program, GL_INFO_LOG_LENGTH, &len);
                if (len > 0) {
                    delete[] m_message;
                    m_message = new char[len];
                    glGetProgramInfoLog(program, len, NULL, m_message);
                    m_delegate->log(Renderer::kLogWarning, "%s", m_message);
                }
                glDeleteProgram(program);
                return false;
            }
            m_modelViewUniformLocation = glGetUniformLocation(program, "modelViewMatrix");
            m_projectionUniformLocation = glGetUniformLocation(program, "projectionMatrix");
            m_positionAttributeLocation = glGetAttribLocation(program, "inPosition");
            m_normalAttributeLocation = glGetAttribLocation(program, "inNormal");
            m_program = program;
            m_delegate->log(Renderer::kLogInfo, "Created a shader program (ID=%d)", m_program);
            return true;
        }
        else {
            return false;
        }
    }
    virtual void bind() {
        glUseProgram(m_program);
    }
    virtual void unbind() {
        glUseProgram(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    void setModelViewMatrix(const float value[16]) {
        glUniformMatrix4fv(m_modelViewUniformLocation, 1, GL_FALSE, value);
    }
    void setProjectionMatrix(const float value[16]) {
        glUniformMatrix4fv(m_projectionUniformLocation, 1, GL_FALSE, value);
    }
    void setPosition(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_positionAttributeLocation);
        glVertexAttribPointer(m_positionAttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setNormal(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_normalAttributeLocation);
        glVertexAttribPointer(m_normalAttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
    }

protected:
    GLuint m_program;

private:
    GLuint compileShader(const char *source, GLenum type) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, NULL);
        glCompileShader(shader);
        GLint compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint len;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
            if (len > 0) {
                delete[] m_message;
                m_message = new char[len];
                glGetShaderInfoLog(shader, len, NULL, m_message);
                m_delegate->log(Renderer::kLogWarning, "%s", m_message);
            }
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

    Renderer::IDelegate *m_delegate;
    GLuint m_modelViewUniformLocation;
    GLuint m_projectionUniformLocation;
    GLuint m_positionAttributeLocation;
    GLuint m_normalAttributeLocation;
    char *m_message;
};

class EdgeProgram : public ShaderProgram
{
public:
    EdgeProgram(Renderer::IDelegate *delegate)
        : ShaderProgram(delegate),
          m_boneAttributesAttributeLocation(0),
          m_edgeAttributeLocation(0),
          m_boneMatricesUniformLocation(0),
          m_colorUniformLocation(0)
    {
    }
    ~EdgeProgram() {
        m_boneAttributesAttributeLocation = 0;
        m_edgeAttributeLocation = 0;
        m_boneMatricesUniformLocation = 0;
        m_colorUniformLocation = 0;
    }

    virtual bool load(const char *vertexShaderSource, const char *fragmentShaderSource) {
        bool ret = ShaderProgram::load(vertexShaderSource, fragmentShaderSource);
        if (ret) {
            m_boneAttributesAttributeLocation = glGetAttribLocation(m_program, "inBoneAttributes");
            m_edgeAttributeLocation = glGetAttribLocation(m_program, "inEdgeOffset");
            m_boneMatricesUniformLocation = glGetUniformLocation(m_program, "boneMatrices");
            m_colorUniformLocation = glGetUniformLocation(m_program, "color");
        }
        return ret;
    }
    void setBoneAttributes(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_boneAttributesAttributeLocation);
        glVertexAttribPointer(m_boneAttributesAttributeLocation, 3, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setEdge(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_edgeAttributeLocation);
        glVertexAttribPointer(m_edgeAttributeLocation, 1, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setBoneMatrices(const GLfloat *ptr, GLsizei size) {
        glUniformMatrix4fv(m_boneMatricesUniformLocation, size, GL_FALSE, ptr);
    }
    void setColor(const Vector3 &value) {
        glUniform4fv(m_colorUniformLocation, 1, value);
    }

private:
    GLuint m_boneAttributesAttributeLocation;
    GLuint m_edgeAttributeLocation;
    GLuint m_boneMatricesUniformLocation;
    GLuint m_colorUniformLocation;
};

class ZPlotProgram : public ShaderProgram
{
public:
    ZPlotProgram(Renderer::IDelegate *delegate)
        : ShaderProgram(delegate)
    {
    }
    ~ZPlotProgram() {
    }
};

class ObjectProgram : public ShaderProgram
{
public:
    ObjectProgram(Renderer::IDelegate *delegate)
        : ShaderProgram(delegate),
          m_lightColorUniformLocation(0),
          m_lightPositionUniformLocation(0)
    {
    }
    ~ObjectProgram() {
        m_lightColorUniformLocation = 0;
        m_lightPositionUniformLocation = 0;
    }

    virtual bool load(const char *vertexShaderSource, const char *fragmentShaderSource) {
        bool ret = ShaderProgram::load(vertexShaderSource, fragmentShaderSource);
        if (ret) {
            m_lightColorUniformLocation = glGetUniformLocation(m_program, "lightColor");
            m_lightPositionUniformLocation = glGetUniformLocation(m_program, "lightPosition");
        }
        return ret;
    }
    void setLightColor(const Color &value) {
        glUniform4fv(m_lightColorUniformLocation, 1, value);
    }
    void setLightPosition(const Vector3 &value) {
        glUniform3fv(m_lightPositionUniformLocation, 1, value);
    }

private:
    GLuint m_lightColorUniformLocation;
    GLuint m_lightPositionUniformLocation;
};

class ShadowProgram : public ObjectProgram
{
public:
    ShadowProgram(Renderer::IDelegate *delegate)
        : ObjectProgram(delegate),
          m_shadowMatrixUniformLocation(0)
    {
    }
    ~ShadowProgram() {
        m_shadowMatrixUniformLocation = 0;
    }

    virtual bool load(const char *vertexShaderSource, const char *fragmentShaderSource) {
        bool ret = ObjectProgram::load(vertexShaderSource, fragmentShaderSource);
        if (ret) {
            m_shadowMatrixUniformLocation = glGetUniformLocation(m_program, "shadowMatrix");
        }
        return ret;
    }
    void setShadowMatrix(const float value[16]) {
        glUniformMatrix4fv(m_shadowMatrixUniformLocation, 1, GL_FALSE, value);
    }

private:
    GLuint m_shadowMatrixUniformLocation;
};

class ModelProgram : public ObjectProgram
{
public:
    ModelProgram(Renderer::IDelegate *delegate)
        : ObjectProgram(delegate),
          m_texCoordAttributeLocation(0),
          m_toonTexCoordAttributeLocation(0),
          m_boneAttributesAttributeLocation(0),
          m_boneMatricesUniformLocation(0),
          m_lightViewMatrixUniformLocation(0),
          m_normalMatrixUniformLocation(0),
          m_materialAmbientUniformLocation(0),
          m_materialDiffuseUniformLocation(0),
          m_hasMainTextureUniformLocation(0),
          m_hasSubTextureUniformLocation(0),
          m_isMainSphereMapUniformLocation(0),
          m_isSubSphereMapUniformLocation(0),
          m_isMainAdditiveUniformLocation(0),
          m_isSubAdditiveUniformLocation(0),
          m_mainTextureUniformLocation(0),
          m_subTextureUniformLocation(0),
          m_toonTextureUniformLocation(0)
    {
    }
    ~ModelProgram() {
        m_texCoordAttributeLocation = 0;
        m_toonTexCoordAttributeLocation = 0;
        m_boneAttributesAttributeLocation = 0;
        m_boneMatricesUniformLocation = 0;
        m_lightViewMatrixUniformLocation = 0;
        m_normalMatrixUniformLocation = 0;
        m_materialAmbientUniformLocation = 0;
        m_materialDiffuseUniformLocation = 0;
        m_hasMainTextureUniformLocation = 0;
        m_hasSubTextureUniformLocation = 0;
        m_isMainSphereMapUniformLocation = 0;
        m_isSubSphereMapUniformLocation = 0;
        m_isMainAdditiveUniformLocation = 0;
        m_isSubAdditiveUniformLocation = 0;
        m_mainTextureUniformLocation = 0;
        m_subTextureUniformLocation = 0;
        m_toonTextureUniformLocation = 0;
    }

    virtual bool load(const char *vertexShaderSource, const char *fragmentShaderSource) {
        bool ret = ObjectProgram::load(vertexShaderSource, fragmentShaderSource);
        if (ret) {
            m_texCoordAttributeLocation = glGetAttribLocation(m_program, "inTexCoord");
            m_toonTexCoordAttributeLocation = glGetAttribLocation(m_program, "inToonTexCoord");
            m_boneAttributesAttributeLocation = glGetAttribLocation(m_program, "inBoneAttributes");
            m_boneMatricesUniformLocation = glGetUniformLocation(m_program, "boneMatrices");
            m_lightViewMatrixUniformLocation = glGetUniformLocation(m_program, "lightViewMatrix");
            m_normalMatrixUniformLocation = glGetUniformLocation(m_program, "normalMatrix");
            m_materialAmbientUniformLocation = glGetUniformLocation(m_program, "materialAmbient");
            m_materialDiffuseUniformLocation = glGetUniformLocation(m_program, "materialDiffuse");
            m_hasMainTextureUniformLocation = glGetUniformLocation(m_program, "hasMainTexture");
            m_hasSubTextureUniformLocation = glGetUniformLocation(m_program, "hasSubTexture");
            m_isMainSphereMapUniformLocation = glGetUniformLocation(m_program, "isMainSphereMap");
            m_isSubSphereMapUniformLocation = glGetUniformLocation(m_program, "isSubSphereMap");
            m_isMainAdditiveUniformLocation = glGetUniformLocation(m_program, "isMainAdditive");
            m_isSubAdditiveUniformLocation = glGetUniformLocation(m_program, "isSubAdditive");
            m_mainTextureUniformLocation = glGetUniformLocation(m_program, "mainTexture");
            m_subTextureUniformLocation = glGetUniformLocation(m_program, "subTexture");
            m_toonTextureUniformLocation = glGetUniformLocation(m_program, "toonTexture");
        }
        return ret;
    }
    virtual void bind() {
        ObjectProgram::bind();
    }
    void setTexCoord(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_texCoordAttributeLocation);
        glVertexAttribPointer(m_texCoordAttributeLocation, 2, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setToonTexCoord(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_toonTexCoordAttributeLocation);
        glVertexAttribPointer(m_toonTexCoordAttributeLocation, 2, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setBoneAttributes(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_boneAttributesAttributeLocation);
        glVertexAttribPointer(m_boneAttributesAttributeLocation, 3, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setBoneMatrices(const GLfloat *ptr, GLsizei size) {
        glUniformMatrix4fv(m_boneMatricesUniformLocation, size, GL_FALSE, ptr);
    }
    void setNormalMatrix(const float value[9]) {
        glUniformMatrix3fv(m_normalMatrixUniformLocation, 1, GL_FALSE, value);
    }
    void setMaterialAmbient(const Color &value) {
        glUniform4fv(m_materialAmbientUniformLocation, 1, value);
    }
    void setMaterialDiffuse(const Color &value) {
        glUniform4fv(m_materialDiffuseUniformLocation, 1, value);
    }
    void setIsMainSphereMap(bool value) {
        glUniform1i(m_isMainSphereMapUniformLocation, value ? 1 : 0);
    }
    void setIsSubSphereMap(bool value) {
        glUniform1i(m_isSubSphereMapUniformLocation, value ? 1 : 0);
    }
    void setIsMainAdditive(bool value) {
        glUniform1i(m_isMainAdditiveUniformLocation, value ? 1 : 0);
    }
    void setIsSubAdditive(bool value) {
        glUniform1i(m_isSubAdditiveUniformLocation, value ? 1 : 0);
    }
    void setLightPosition(const Vector3 &value) {
        float matrix[16];
        CreateLookAt(value, matrix);
        glUniformMatrix4fv(m_lightViewMatrixUniformLocation, 1, GL_FALSE, matrix);
        ObjectProgram::setLightPosition(value);
    }
    void setMainTexture(GLuint value) {
        if (value) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, value);
            glUniform1i(m_mainTextureUniformLocation, 0);
            glUniform1i(m_hasMainTextureUniformLocation, 1);
        }
        else {
            glUniform1i(m_hasMainTextureUniformLocation, 0);
        }
    }
    void setSubTexture(GLuint value) {
        if (value) {
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, value);
            glUniform1i(m_subTextureUniformLocation, 2);
            glUniform1i(m_hasSubTextureUniformLocation, 1);
        }
        else {
            glUniform1i(m_hasSubTextureUniformLocation, 0);
        }
    }
    void setToonTexture(GLuint value) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, value);
        glUniform1i(m_toonTextureUniformLocation, 1);
    }

private:
    GLuint m_texCoordAttributeLocation;
    GLuint m_toonTexCoordAttributeLocation;
    GLuint m_boneAttributesAttributeLocation;
    GLuint m_boneMatricesUniformLocation;
    GLuint m_lightViewMatrixUniformLocation;
    GLuint m_normalMatrixUniformLocation;
    GLuint m_materialAmbientUniformLocation;
    GLuint m_materialDiffuseUniformLocation;
    GLuint m_hasMainTextureUniformLocation;
    GLuint m_hasSubTextureUniformLocation;
    GLuint m_isMainSphereMapUniformLocation;
    GLuint m_isSubSphereMapUniformLocation;
    GLuint m_isMainAdditiveUniformLocation;
    GLuint m_isSubAdditiveUniformLocation;
    GLuint m_mainTextureUniformLocation;
    GLuint m_subTextureUniformLocation;
    GLuint m_toonTextureUniformLocation;
};

class ExtendedModelProgram : public ModelProgram
{
public:
    ExtendedModelProgram(Renderer::IDelegate *delegate)
        : ModelProgram(delegate),
          m_biasMatrixUniformLocation(0),
          m_shadowTextureUniformLocation(0)
    {
    }
    ~ExtendedModelProgram() {
        m_biasMatrixUniformLocation = 0;
        m_shadowTextureUniformLocation = 0;
    }

    virtual bool load(const char *vertexShaderSource, const char *fragmentShaderSource) {
        bool ret = ModelProgram::load(vertexShaderSource, fragmentShaderSource);
        if (ret) {
            m_biasMatrixUniformLocation = glGetUniformLocation(m_program, "biasMatrix");
            m_shadowTextureUniformLocation = glGetUniformLocation(m_program, "shadowTexture");
        }
        return ret;
    }
    virtual void bind() {
        static const float matrix[] = {
            0.5f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.5f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.5f, 0.0f,
            0.5f, 0.5f, 0.5f, 1.0f
        };
        ModelProgram::bind();
        glUniformMatrix4fv(m_biasMatrixUniformLocation, 1, GL_FALSE, matrix);
    }

    void setShadowTexture(GLuint value) {
        if (value) {
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, value);
            glUniform1i(m_shadowTextureUniformLocation, 3);
        }
    }

private:
    GLuint m_biasMatrixUniformLocation;
    GLuint m_shadowTextureUniformLocation;
};

class AssetProgram : public ObjectProgram
{
public:
    AssetProgram(Renderer::IDelegate *delegate)
        : ObjectProgram(delegate),
          m_texCoordAttributeLocation(0),
          m_colorAttributeLocation(0),
          m_normalMatrixUniformLocation(0),
          m_transformMatrixUniformLocation(0),
          m_materialAmbientUniformLocation(0),
          m_materialDiffuseUniformLocation(0),
          m_materialSpecularUniformLocation(0),
          m_materialShininessUniformLocation(0),
          m_hasTextureUniformLocation(0),
          m_hasColorVertexUniformLocation(0),
          m_textureUniformLocation(0),
          m_opacityUniformLocation(0)
    {
    }
    ~AssetProgram() {
        m_texCoordAttributeLocation = 0;
        m_colorAttributeLocation = 0;
        m_normalMatrixUniformLocation = 0;
        m_transformMatrixUniformLocation = 0;
        m_materialAmbientUniformLocation = 0;
        m_materialDiffuseUniformLocation = 0;
        m_materialEmissionUniformLocation = 0;
        m_materialSpecularUniformLocation = 0;
        m_materialShininessUniformLocation = 0;
        m_hasTextureUniformLocation = 0;
        m_hasColorVertexUniformLocation = 0;
        m_textureUniformLocation = 0;
        m_opacityUniformLocation = 0;
    }

    virtual bool load(const char *vertexShaderSource, const char *fragmentShaderSource) {
        bool ret = ObjectProgram::load(vertexShaderSource, fragmentShaderSource);
        if (ret) {
            m_texCoordAttributeLocation = glGetAttribLocation(m_program, "inTexCoord");
            m_colorAttributeLocation = glGetAttribLocation(m_program, "inColor");
            m_normalMatrixUniformLocation = glGetUniformLocation(m_program, "normalMatrix");
            m_transformMatrixUniformLocation = glGetUniformLocation(m_program, "transformMatrix");
            m_materialAmbientUniformLocation = glGetUniformLocation(m_program, "materialAmbient");
            m_materialDiffuseUniformLocation = glGetUniformLocation(m_program, "materialDiffuse");
            m_materialEmissionUniformLocation = glGetUniformLocation(m_program, "materialEmission");
            m_materialSpecularUniformLocation = glGetUniformLocation(m_program, "materialSpecular");
            m_materialShininessUniformLocation = glGetUniformLocation(m_program, "materialShininess");
            m_hasTextureUniformLocation = glGetUniformLocation(m_program, "hasTexture");
            m_hasColorVertexUniformLocation = glGetUniformLocation(m_program, "hasColorVertex");
            m_textureUniformLocation = glGetUniformLocation(m_program, "mainTexture");
            m_opacityUniformLocation = glGetUniformLocation(m_program, "opacity");
        }
        return ret;
    }

    void setTexCoord(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_texCoordAttributeLocation);
        glVertexAttribPointer(m_texCoordAttributeLocation, 2, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setColor(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_colorAttributeLocation);
        glVertexAttribPointer(m_colorAttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setHasColor(bool value) {
        glUniform1i(m_hasColorVertexUniformLocation, value ? 1 : 0);
    }
    void setNormalMatrix(const float value[16]) {
        glUniformMatrix3fv(m_normalMatrixUniformLocation, 1, GL_FALSE, value);
    }
    void setTransformMatrix(const float value[9]) {
        glUniformMatrix4fv(m_transformMatrixUniformLocation, 1, GL_FALSE, value);
    }
    void setMaterialAmbient(const Color &value) {
        glUniform4fv(m_materialAmbientUniformLocation, 1, value);
    }
    void setMaterialDiffuse(const Color &value) {
        glUniform4fv(m_materialDiffuseUniformLocation, 1, value);
    }
    void setMaterialEmission(const Color &value) {
        glUniform4fv(m_materialEmissionUniformLocation, 1, value);
    }
    void setMaterialSpecular(const Color &value) {
        glUniform4fv(m_materialSpecularUniformLocation, 1, value);
    }
    void setMaterialShininess(float value) {
        glUniform1f(m_materialShininessUniformLocation, value);
    }
    void setOpacity(float value) {
        glUniform1f(m_opacityUniformLocation, value);
    }
    void setTexture(GLuint value) {
        if (value) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, value);
            glUniform1i(m_textureUniformLocation, 0);
            glUniform1i(m_hasTextureUniformLocation, 1);
        }
        else {
            glUniform1i(m_hasTextureUniformLocation, 0);
        }
    }

private:
    GLuint m_texCoordAttributeLocation;
    GLuint m_colorAttributeLocation;
    GLuint m_normalMatrixUniformLocation;
    GLuint m_transformMatrixUniformLocation;
    GLuint m_materialAmbientUniformLocation;
    GLuint m_materialDiffuseUniformLocation;
    GLuint m_materialEmissionUniformLocation;
    GLuint m_materialSpecularUniformLocation;
    GLuint m_materialShininessUniformLocation;
    GLuint m_hasTextureUniformLocation;
    GLuint m_hasColorVertexUniformLocation;
    GLuint m_textureUniformLocation;
    GLuint m_opacityUniformLocation;
};

}
}

namespace {

const std::string CanonicalizePath(const std::string &path)
{
    const std::string from("\\"), to("/");
    std::string ret(path);
    std::string::size_type pos(path.find(from));
    while (pos != std::string::npos) {
        ret.replace(pos, from.length(), to);
        pos = ret.find(from, pos + to.length());
    }
    return ret;
}

}

namespace vpvl
{

namespace gl2
{

Renderer::Renderer(IDelegate *delegate, int width, int height, int fps)
#ifdef VPVL_LINK_QT
    : QGLFunctions(),
      #else
    :
      #endif /* VPVL_LINK_QT */
      m_delegate(delegate),
      m_edgeProgram(0),
      m_modelProgram(0),
      m_shadowProgram(0),
      m_scene(0),
      m_accelerator(0),
      m_selected(0),
      m_frameBufferID(0),
      m_depthTextureID(0)
{
    m_scene = new Scene(width, height, fps);
}

Renderer::~Renderer()
{
    Array<PMDModel *> models;
    models.copy(m_scene->models());
    const int nmodels = models.count();
    for (int i = 0; i < nmodels; i++) {
        PMDModel *model = models[i];
        deleteModel(model);
    }
    Array<Asset *> assets;
    assets.copy(m_assets);
    const int nassets = assets.count();
    for (int i = 0; i < nassets; i++) {
        Asset *asset = assets[i];
        deleteAsset(asset);
    }
    glDeleteFramebuffers(1, &m_frameBufferID);
    m_frameBufferID = 0;
    glDeleteTextures(1, &m_depthTextureID);
    m_depthTextureID = 0;
    delete m_accelerator;
    m_accelerator = 0;
    delete m_edgeProgram;
    m_edgeProgram = 0;
    delete m_modelProgram;
    m_modelProgram = 0;
    delete m_shadowProgram;
    m_shadowProgram = 0;
    delete m_scene;
    m_scene = 0;
}

void Renderer::initializeSurface()
{
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

bool Renderer::createShaderPrograms()
{
    m_edgeProgram = new EdgeProgram(m_delegate);
    m_modelProgram = new ExtendedModelProgram(m_delegate);
    m_shadowProgram = new ShadowProgram(m_delegate);
    m_zplotProgram = new ZPlotProgram(m_delegate);
#ifdef VPVL_LINK_QT
    const QGLContext *context = QGLContext::currentContext();
    initializeGLFunctions(context);
    m_edgeProgram->initializeContext(context);
    m_modelProgram->initializeContext(context);
    m_shadowProgram->initializeContext(context);
    m_zplotProgram->initializeContext(context);
#endif
    std::string vertexShader;
    std::string fragmentShader;
    vertexShader = m_delegate->loadShader(kEdgeVertexShader);
    fragmentShader = m_delegate->loadShader(kEdgeFragmentShader);
    if (!m_edgeProgram->load(vertexShader.c_str(), fragmentShader.c_str()))
        return false;
    vertexShader = m_delegate->loadShader(kModelVertexShader);
    fragmentShader = m_delegate->loadShader(kModelFragmentShader);
    if (!m_modelProgram->load(vertexShader.c_str(), fragmentShader.c_str()))
        return false;
    vertexShader = m_delegate->loadShader(kShadowVertexShader);
    fragmentShader = m_delegate->loadShader(kShadowFragmentShader);
    if (!m_shadowProgram->load(vertexShader.c_str(), fragmentShader.c_str()))
        return false;
    vertexShader = m_delegate->loadShader(kZPlotVertexShader);
    fragmentShader = m_delegate->loadShader(kZPlotFragmentShader);
    return m_zplotProgram->load(vertexShader.c_str(), fragmentShader.c_str());
}

bool Renderer::createShadowFrameBuffers()
{
    bool ret = true;
#ifndef  VPVL_BUILD_IOS
    glGenTextures(1, &m_depthTextureID);
    glBindTexture(GL_TEXTURE_2D, m_depthTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, kShadowMappingTextureWidth, kShadowMappingTextureHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
    glBindTexture(GL_TEXTURE_2D, 0);
    glGenFramebuffers(1, &m_frameBufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferID);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTextureID, 0);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status == GL_FRAMEBUFFER_COMPLETE) {
        m_delegate->log(kLogInfo,
                        "Created a framebuffer (textureID = %d, frameBufferID = %d)",
                        m_depthTextureID,
                        m_frameBufferID);
    }
    else {
        m_delegate->log(kLogWarning, "Failed creating a framebuffer: %d", status);
        ret = false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
    return ret;
}

void Renderer::resize(int width, int height)
{
    m_scene->setWidth(width);
    m_scene->setHeight(height);
}

void Renderer::uploadModel(PMDModel *model, const std::string &dir)
{
    uploadModel0(new PMDModelUserData(), model, dir);
}

void Renderer::uploadModel0(PMDModel::UserData *userData, PMDModel *model, const std::string &dir)
{
    PMDModelUserData *casted = static_cast<PMDModelUserData *>(userData);
    const MaterialList &materials = model->materials();
    const int nmaterials = materials.count();
    GLuint textureID = 0;
    PMDModelMaterialPrivate *materialPrivates = new PMDModelMaterialPrivate[nmaterials];
    bool hasSingleSphere = false, hasMultipleSphere = false;
    for (int i = 0; i < nmaterials; i++) {
        const Material *material = materials[i];
        const std::string &primary = m_delegate->toUnicode(material->mainTextureName());
        const std::string &second = m_delegate->toUnicode(material->subTextureName());
        PMDModelMaterialPrivate &materialPrivate = materialPrivates[i];
        materialPrivate.mainTextureID = 0;
        materialPrivate.subTextureID = 0;
        if (!primary.empty()) {
            if (m_delegate->uploadTexture(dir + "/" + primary, textureID, false)) {
                materialPrivate.mainTextureID = textureID;
                m_delegate->log(kLogInfo, "Binding the texture as a primary texture (ID=%d)", textureID);
            }
        }
        if (!second.empty()) {
            if (m_delegate->uploadTexture(dir + "/" + second, textureID, false)) {
                materialPrivate.subTextureID = textureID;
                m_delegate->log(kLogInfo, "Binding the texture as a secondary texture (ID=%d)", textureID);
            }
        }
        hasSingleSphere |= material->isMainSphereModulate() && !material->isSubSphereAdd();
        hasMultipleSphere |= material->isSubSphereAdd();
    }
    casted->hasSingleSphereMap = hasSingleSphere;
    casted->hasMultipleSphereMap = hasMultipleSphere;
    m_delegate->log(kLogInfo,
                    "Sphere map information: hasSingleSphere=%s, hasMultipleSphere=%s",
                    hasSingleSphere ? "true" : "false",
                    hasMultipleSphere ? "true" : "false");
    glGenBuffers(kVertexBufferObjectMax, casted->vertexBufferObjects);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, casted->vertexBufferObjects[kEdgeIndices]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->edgeIndicesCount() * model->strideSize(PMDModel::kEdgeIndicesStride),
                 model->edgeIndicesPointer(), GL_STATIC_DRAW);
    m_delegate->log(kLogInfo,
                    "Binding edge indices to the vertex buffer object (ID=%d)",
                    casted->vertexBufferObjects[kEdgeIndices]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, casted->vertexBufferObjects[kShadowIndices]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->indices().count() * model->strideSize(PMDModel::kIndicesStride),
                 model->indicesPointer(), GL_STATIC_DRAW);
    m_delegate->log(kLogInfo,
                    "Binding indices to the vertex buffer object (ID=%d)",
                    casted->vertexBufferObjects[kShadowIndices]);
    glBindBuffer(GL_ARRAY_BUFFER, casted->vertexBufferObjects[kModelVertices]);
    glBufferData(GL_ARRAY_BUFFER, model->vertices().count() * model->strideSize(PMDModel::kVerticesStride),
                 model->verticesPointer(), GL_DYNAMIC_DRAW);
    m_delegate->log(kLogInfo,
                    "Binding model vertices to the vertex buffer object (ID=%d)",
                    casted->vertexBufferObjects[kModelVertices]);
    if (m_delegate->uploadToonTexture("toon0.bmp", dir, textureID)) {
        casted->toonTextureID[0] = textureID;
        m_delegate->log(kLogInfo, "Binding the texture as a toon texture (ID=%d)", textureID);
    }
    for (int i = 0; i < PMDModel::kCustomTextureMax; i++) {
        const uint8_t *name = model->toonTexture(i);
        if (m_delegate->uploadToonTexture(reinterpret_cast<const char *>(name), dir, textureID)) {
            casted->toonTextureID[i + 1] = textureID;
            m_delegate->log(kLogInfo, "Binding the texture as a toon texture (ID=%d)", textureID);
        }
    }
    casted->materials = materialPrivates;
    model->setLightPosition(m_scene->lightPosition());
    model->setSoftwareSkinningEnable(m_scene->isSoftwareSkinningEnabled());
    if (m_accelerator)
        m_accelerator->uploadModel(casted, model);
    model->setUserData(casted);
    model->updateImmediate();
    updateModel(model);
    m_delegate->log(kLogInfo, "Created the model: %s", m_delegate->toUnicode(model->name()).c_str());
    m_scene->addModel(model);
}

void Renderer::deleteModel(PMDModel *&model)
{
    if (model) {
        PMDModelUserData *userData = static_cast<PMDModelUserData *>(model->userData());
        const MaterialList &materials = model->materials();
        const int nmaterials = materials.count();
        for (int i = 0; i < nmaterials; i++) {
            PMDModelMaterialPrivate &materialPrivate = userData->materials[i];
            glDeleteTextures(1, &materialPrivate.mainTextureID);
            glDeleteTextures(1, &materialPrivate.subTextureID);
        }
        for (int i = 0; i < PMDModel::kCustomTextureMax; i++) {
            glDeleteTextures(1, &userData->toonTextureID[i]);
        }
        if (m_accelerator)
            m_accelerator->deleteModel(userData);
        glDeleteBuffers(kVertexBufferObjectMax, userData->vertexBufferObjects);
        delete[] userData->materials;
        delete userData;
        m_delegate->log(kLogInfo, "Destroyed the model: %s", m_delegate->toUnicode(model->name()).c_str());
        m_scene->removeModel(model);
        delete model;
        model = 0;
    }
}

void Renderer::updateAllModel()
{
    size_t size = 0;
    PMDModel **models = m_scene->getRenderingOrder(size);
    for (size_t i = 0; i < size; i++) {
        PMDModel *model = models[i];
        if (model->isVisible())
            updateModel(model);
    }
}

void Renderer::updateModel(PMDModel *model)
{
    PMDModelUserData *userData = static_cast<PMDModelUserData *>(model->userData());
    int nvertices = model->vertices().count();
    size_t strideSize = model->strideSize(PMDModel::kVerticesStride);
    glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelVertices]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, nvertices * strideSize, model->verticesPointer());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    if (m_accelerator && !m_scene->isSoftwareSkinningEnabled())
        m_accelerator->updateModel(userData, model);
}

void Renderer::renderModel(const PMDModel *model)
{
    const PMDModelUserData *userData = static_cast<PMDModelUserData *>(model->userData());

    m_modelProgram->bind();
    glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelVertices]);
    m_modelProgram->setPosition(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kVerticesStride)),
                                model->strideSize(PMDModel::kVerticesStride));
    m_modelProgram->setNormal(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kNormalsStride)),
                              model->strideSize(PMDModel::kNormalsStride));
    m_modelProgram->setTexCoord(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kTextureCoordsStride)),
                                model->strideSize(PMDModel::kTextureCoordsStride));

    if (!model->isSoftwareSkinningEnabled()) {
        m_modelProgram->setBoneAttributes(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kBoneAttributesStride)),
                                          model->strideSize(PMDModel::kBoneAttributesStride));
        // XXX: boneMatricesPointer is removed, we must implement updateBoneMatrices.
        //m_modelProgram->setBoneMatrices(model->boneMatricesPointer(), model->bones().count());
    }

    float matrix4x4[16], matrix3x3[9];
    m_scene->getModelViewMatrix(matrix4x4);
    m_modelProgram->setModelViewMatrix(matrix4x4);
    m_scene->getProjectionMatrix(matrix4x4);
    m_modelProgram->setProjectionMatrix(matrix4x4);
    m_scene->getNormalMatrix(matrix3x3);
    m_modelProgram->setNormalMatrix(matrix3x3);
    m_modelProgram->setLightColor(m_scene->lightColor());
    m_modelProgram->setLightPosition(m_scene->lightPosition());
    if (m_depthTextureID) {
        ExtendedModelProgram *modelProgram = static_cast<ExtendedModelProgram *>(m_modelProgram);
        modelProgram->setShadowTexture(m_depthTextureID);
    }
    if (model->isToonEnabled() && (model->isSoftwareSkinningEnabled() || (m_accelerator && m_accelerator->isAvailable()))) {
        m_modelProgram->setToonTexCoord(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kToonTextureStride)),
                                        model->strideSize(PMDModel::kToonTextureStride));
    }

    const MaterialList &materials = model->materials();
    const PMDModelMaterialPrivate *materialPrivates = userData->materials;
    const int nmaterials = materials.count();
    const bool hasSingleSphereMap = userData->hasSingleSphereMap;
    const bool hasMultipleSphereMap = userData->hasMultipleSphereMap;
    Color ambient, diffuse;
    size_t offset = 0;

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, userData->vertexBufferObjects[kShadowIndices]);
    for (int i = 0; i < nmaterials; i++) {
        const Material *material = materials[i];
        const PMDModelMaterialPrivate &materialPrivate = materialPrivates[i];
        const float opacity = material->opacity();
        const bool isMainSphereAdd = material->isMainSphereAdd();
        ambient = material->ambient();
        ambient.setW(ambient.w() * opacity);
        diffuse = material->diffuse();
        diffuse.setW(diffuse.w() * opacity);
        m_modelProgram->setMaterialAmbient(ambient);
        m_modelProgram->setMaterialDiffuse(diffuse);
        m_modelProgram->setMainTexture(materialPrivate.mainTextureID);
        m_modelProgram->setToonTexture(userData->toonTextureID[material->toonID()]);
        m_modelProgram->setIsMainSphereMap(isMainSphereAdd || material->isMainSphereModulate());
        m_modelProgram->setIsMainAdditive(isMainSphereAdd);
        if (hasMultipleSphereMap) {
            const bool isSubSphereAdd = material->isSubSphereAdd();
            m_modelProgram->setIsSubSphereMap(isSubSphereAdd || material->isSubSphereModulate());
            m_modelProgram->setIsSubAdditive(isSubSphereAdd);
            m_modelProgram->setSubTexture(materialPrivate.subTextureID);
        }
        opacity < 1.0f ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);
        const int nindices = material->countIndices();
        glDrawElements(GL_TRIANGLES, nindices, GL_UNSIGNED_SHORT, reinterpret_cast<const GLvoid *>(offset));
        offset += (nindices << 1);
    }

    m_modelProgram->unbind();
    glEnable(GL_CULL_FACE);
}

void Renderer::renderModelShadow(const PMDModel *model)
{
    static const Vector3 plane(0.0f, 1.0f, 0.0f);
    const PMDModelUserData *userData = static_cast<PMDModelUserData *>(model->userData());
    const Vector3 &light = m_scene->lightPosition();
    const Scalar dot = plane.dot(light);
    float modelViewMatrix[16], projectionMatrix[16], shadowMatrix[16];
    m_scene->getModelViewMatrix(modelViewMatrix);
    m_scene->getProjectionMatrix(projectionMatrix);
    glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelVertices]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, userData->vertexBufferObjects[kShadowIndices]);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int index = (i << 2) + j;
            shadowMatrix[index] = -plane[i] * light[j];
            if (i == j)
                shadowMatrix[index] += dot;
        }
    }
    m_shadowProgram->bind();
    m_shadowProgram->setModelViewMatrix(modelViewMatrix);
    m_shadowProgram->setProjectionMatrix(projectionMatrix);
    m_shadowProgram->setShadowMatrix(shadowMatrix);
    m_shadowProgram->setLightColor(m_scene->lightColor());
    m_shadowProgram->setLightPosition(m_scene->lightPosition());
    m_shadowProgram->setPosition(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kVerticesStride)),
                                 model->strideSize(PMDModel::kVerticesStride));
    glDrawElements(GL_TRIANGLES, model->indices().count(), GL_UNSIGNED_SHORT, 0);
    m_shadowProgram->unbind();
}

void Renderer::renderModelZPlot(const PMDModel *model)
{
#ifndef VPVL_BUILD_IOS
    const PMDModelUserData *userData = static_cast<PMDModelUserData *>(model->userData());
    float modelViewMatrix[16], projectionMatrix[16];
    m_scene->getProjectionMatrix(projectionMatrix);
    m_scene->getModelViewMatrix(modelViewMatrix);
    glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelVertices]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, userData->vertexBufferObjects[kShadowIndices]);
    Vector3 center;
    Scalar radius, angle = 15.0f;
    model->getBoundingSphere(center, radius);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    Scalar eye = radius / btSin(radian(angle * 0.5f));
    //gluPerspective(angle, 1.0f, 1.0f, eye + radius + 50.0f);
    Vector3 eyev = m_scene->lightPosition() * eye + center;
    //gluLookAt(eyev.x(), eyev.y(), eyev.z(), center.x(), center.y(), center.z(), 0.0, 1.0, 0.0);
    glGetFloatv(GL_MODELVIEW, modelViewMatrix);
    m_zplotProgram->bind();
    m_zplotProgram->setModelViewMatrix(modelViewMatrix);
    m_zplotProgram->setProjectionMatrix(projectionMatrix);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(4.0f, 4.0f);
    m_zplotProgram->setPosition(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kVerticesStride)),
                                model->strideSize(PMDModel::kVerticesStride));
    glDrawElements(GL_TRIANGLES, model->indices().count(), GL_UNSIGNED_SHORT, 0);
    glDisable(GL_POLYGON_OFFSET_FILL);
    m_zplotProgram->unbind();
#endif
}

void Renderer::renderModelEdge(const PMDModel *model)
{
    if (model->edgeOffset() == 0.0f)
        return;
    const PMDModelUserData *userData = static_cast<PMDModelUserData *>(model->userData());
    float modelViewMatrix[16], projectionMatrix[16];
    m_scene->getModelViewMatrix(modelViewMatrix);
    m_scene->getProjectionMatrix(projectionMatrix);
    m_edgeProgram->bind();
    glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelVertices]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, userData->vertexBufferObjects[kEdgeIndices]);
    m_edgeProgram->setColor(model->edgeColor());
    m_edgeProgram->setModelViewMatrix(modelViewMatrix);
    m_edgeProgram->setProjectionMatrix(projectionMatrix);
    if (!model->isSoftwareSkinningEnabled() && !(m_accelerator && m_accelerator->isAvailable())) {
        m_edgeProgram->setPosition(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kVerticesStride)),
                                   model->strideSize(PMDModel::kVerticesStride));
        m_edgeProgram->setNormal(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kNormalsStride)),
                                 model->strideSize(PMDModel::kNormalsStride));
        m_edgeProgram->setBoneAttributes(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kBoneAttributesStride)),
                                         model->strideSize(PMDModel::kBoneAttributesStride));
        m_edgeProgram->setEdge(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kEdgeVerticesStride)),
                               model->strideSize(PMDModel::kEdgeVerticesStride));
        // XXX: boneMatricesPointer is removed, we must implement updateBoneMatrices alternative.
        //m_edgeProgram->setBoneMatrices(model->boneMatricesPointer(), model->bones().count());
    }
    else {
        m_edgeProgram->setPosition(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kEdgeVerticesStride)),
                                   model->strideSize(PMDModel::kEdgeVerticesStride));
    }
    glCullFace(GL_FRONT);
    glDrawElements(GL_TRIANGLES, model->edgeIndicesCount(), GL_UNSIGNED_SHORT, 0);
    glCullFace(GL_BACK);
    m_edgeProgram->unbind();
}

void Renderer::renderAsset(const Asset *asset)
{
#ifdef VPVL_LINK_ASSIMP
    const aiScene *a = asset->getScene();
    renderAssetRecurse(a, a->mRootNode, asset);
#endif
}

void Renderer::uploadAsset(Asset *asset, const std::string &dir)
{
#ifdef VPVL_LINK_ASSIMP
    uploadAsset0(new AssetUserData(), asset, dir);
#else
    (void) asset;
    (void) dir;
#endif
}

void Renderer::uploadAsset0(Asset::UserData *userData, Asset *asset, const std::string &dir)
{
#ifdef VPVL_LINK_ASSIMP
    const aiScene *scene = asset->getScene();
    const unsigned int nmaterials = scene->mNumMaterials;
    AssetUserData *casted = static_cast<AssetUserData *>(userData);
    aiString texturePath;
    std::string path, canonicalized, filename;
    asset->setUserData(casted);
    for (unsigned int i = 0; i < nmaterials; i++) {
        aiMaterial *material = scene->mMaterials[i];
        aiReturn found = AI_SUCCESS;
        GLuint textureID;
        int textureIndex = 0;
        while (found == AI_SUCCESS) {
            found = material->GetTexture(aiTextureType_DIFFUSE, textureIndex, &texturePath);
            path = texturePath.data;
            if (casted->textures[path] == 0) {
                canonicalized = m_delegate->toUnicode(reinterpret_cast<const uint8_t *>(CanonicalizePath(path).c_str()));
                filename = dir + "/" + canonicalized;
                if (m_delegate->uploadTexture(filename, textureID, false)) {
                    casted->textures[path] = textureID;
                    m_delegate->log(kLogInfo, "Loaded a texture: %s (ID=%d)", canonicalized.c_str(), textureID);
                }
            }
            textureIndex++;
        }
    }
    uploadAssetRecurse(scene, scene->mRootNode, casted);
    m_assets.add(asset);
#else
    (void) userData;
    (void) asset;
    (void) dir;
#endif
}

void Renderer::deleteAsset(Asset *&asset)
{
#ifdef VPVL_LINK_ASSIMP
    if (asset) {
        const aiScene *scene = asset->getScene();
        const unsigned int nmaterials = scene->mNumMaterials;
        AssetUserData *userData = static_cast<AssetUserData *>(asset->userData());
        aiString texturePath;
        for (unsigned int i = 0; i < nmaterials; i++) {
            aiMaterial *material = scene->mMaterials[i];
            aiReturn found = AI_SUCCESS;
            GLuint textureID;
            int textureIndex = 0;
            while (found == AI_SUCCESS) {
                found = material->GetTexture(aiTextureType_DIFFUSE, textureIndex, &texturePath);
                textureID = userData->textures[texturePath.data];
                glDeleteTextures(1, &textureID);
                userData->textures.erase(texturePath.data);
                textureIndex++;
            }
        }
        deleteAssetRecurse(scene, scene->mRootNode, userData);
        delete userData;
        delete asset;
        m_assets.remove(asset);
        asset = 0;
    }
#else
    (void) asset;
#endif
}


void Renderer::clear()
{
    glViewport(0, 0, m_scene->width(), m_scene->height());
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::renderAllAssets()
{
    const int nassets = m_assets.count();
    for (int i = 0; i < nassets; i++)
        renderAsset(m_assets[i]);
}

void Renderer::renderAllModels()
{
    size_t size = 0;
    PMDModel **models = m_scene->getRenderingOrder(size);
    for (size_t i = 0; i < size; i++) {
        PMDModel *model = models[i];
        if (model->isVisible()) {
            renderModel(model);
            renderModelEdge(model);
        }
    }
}

void Renderer::renderProjectiveShadow()
{
    size_t size = 0;
    PMDModel **models = m_scene->getRenderingOrder(size);
    glCullFace(GL_FRONT);
    for (size_t i = 0; i < size; i++) {
        PMDModel *model = models[i];
        if (model->isVisible())
            renderModelShadow(model);
    }
    glCullFace(GL_BACK);
}

void Renderer::renderZPlot()
{
    if (m_depthTextureID && m_frameBufferID) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferID);
        glViewport(0, 0, kShadowMappingTextureWidth, kShadowMappingTextureHeight);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glClear(GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_FRONT);
        size_t size = 0;
        PMDModel **models = m_scene->getRenderingOrder(size);
        for (size_t i = 0; i < size; i++) {
            PMDModel *model = models[i];
            if (model->isVisible())
                renderModelZPlot(model);
        }
        glCullFace(GL_BACK);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void Renderer::releaseProject(Project *project)
{
#ifdef VPVL_ENABLE_PROJECT
    const std::vector<std::string> &assetUUIDs = project->assetUUIDs();
    for (std::vector<std::string>::const_iterator it = assetUUIDs.begin(); it != assetUUIDs.end(); it++) {
        Asset *asset = project->asset(*it);
        project->removeAsset(asset);
        deleteAsset(asset);
    }
    const std::vector<std::string> &modelUUIDs = project->modelUUIDs();
    for (std::vector<std::string>::const_iterator it = modelUUIDs.begin(); it != modelUUIDs.end(); it++) {
        PMDModel *model = project->model(*it);
        project->removeModel(model);
        model->deleteAllMotions();
        deleteModel(model);
    }
#else
    (void) project;
#endif
}

#ifdef VPVL_LINK_ASSIMP
void Renderer::uploadAssetRecurse(const aiScene *scene, const aiNode *node, Asset::UserData *userData)
{
    const unsigned int nmeshes = node->mNumMeshes;
    AssetVertex assetVertex;
    AssetUserData *casted = static_cast<AssetUserData *>(userData);
    AssetProgram *program = new AssetProgram(m_delegate);
#ifdef VPVL_LINK_QT
    program->initializeContext(QGLContext::currentContext());
#endif
    program->load(m_delegate->loadShader(Renderer::kAssetVertexShader).c_str(),
                  m_delegate->loadShader(Renderer::kAssetFragmentShader).c_str());
    casted->programs[node] = program;
    for (unsigned int i = 0; i < nmeshes; i++) {
        const struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        const aiVector3D *vertices = mesh->mVertices;
        const aiVector3D *normals = mesh->mNormals;
        const bool hasNormals = mesh->HasNormals();
        const bool hasColors = mesh->HasVertexColors(0);
        const bool hasTexCoords = mesh->HasTextureCoords(0);
        const aiColor4D *colors = hasColors ? mesh->mColors[0] : 0;
        const aiVector3D *texcoords = hasTexCoords ? mesh->mTextureCoords[0] : 0;
        AssetVertices &assetVertices = casted->vertices[mesh];
        AssetIndices &indices = casted->indices[mesh];
        const unsigned int nfaces = mesh->mNumFaces;
        int index = 0;
        for (unsigned int j = 0; j < nfaces; j++) {
            const struct aiFace &face = mesh->mFaces[j];
            const unsigned int nindices = face.mNumIndices;
            for (unsigned int k = 0; k < nindices; k++) {
                int vertexIndex = face.mIndices[k];
                if (hasColors) {
                    const aiColor4D &c = colors[vertexIndex];
                    assetVertex.color.setValue(c.r, c.g, c.b, c.a);
                }
                else {
                    assetVertex.color.setZero();
                    assetVertex.color.setW(1.0f);
                }
                if (hasTexCoords) {
                    const aiVector3D &p = texcoords[vertexIndex];
                    assetVertex.texcoord.setValue(p.x, p.y, 0.0f);
                }
                else {
                    assetVertex.texcoord.setZero();
                }
                if (hasNormals) {
                    const aiVector3D &n = normals[vertexIndex];
                    assetVertex.normal.setValue(n.x, n.y, n.z);
                }
                else {
                    assetVertex.normal.setZero();
                }
                const aiVector3D &v = vertices[vertexIndex];
                assetVertex.position.setValue(v.x, v.y, v.z, 1.0f);
                assetVertices.push_back(assetVertex);
                indices.push_back(index);
                index++;
            }
        }
        AssetVBO &vbo = casted->vbo[mesh];
        size_t vsize = assetVertices.size() * sizeof(assetVertices[0]);
        glGenBuffers(1, &vbo.vertices);
        glBindBuffer(GL_ARRAY_BUFFER, vbo.vertices);
        glBufferData(GL_ARRAY_BUFFER, vsize, assetVertices[0].position, GL_STATIC_DRAW);
        glGenBuffers(1, &vbo.indices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.indices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), &indices[0], GL_STATIC_DRAW);
    }
    const unsigned int nChildNodes = node->mNumChildren;
    for (unsigned int i = 0; i < nChildNodes; i++)
        uploadAssetRecurse(scene, node->mChildren[i], casted);
}

void Renderer::deleteAssetRecurse(const aiScene *scene, const aiNode *node, Asset::UserData *userData)
{
    const unsigned int nmeshes = node->mNumMeshes;
    AssetUserData *casted = static_cast<AssetUserData *>(userData);
    for (unsigned int i = 0; i < nmeshes; i++) {
        const struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        const AssetVBO &vbo = casted->vbo[mesh];
        glDeleteBuffers(1, &vbo.vertices);
        glDeleteBuffers(1, &vbo.indices);
    }
    delete casted->programs[node];
    const unsigned int nChildNodes = node->mNumChildren;
    for (unsigned int i = 0; i < nChildNodes; i++)
        deleteAssetRecurse(scene, node->mChildren[i], casted);
}

void Renderer::setAssetMaterial(const aiMaterial *material, const Asset *asset, AssetProgram *program)
{
    int textureIndex = 0;
    aiString texturePath;
    if (material->GetTexture(aiTextureType_DIFFUSE, textureIndex, &texturePath) == aiReturn_SUCCESS) {
        AssetUserData *userData = static_cast<AssetUserData *>(asset->userData());
        GLuint textureID = userData->textures[texturePath.data];
        program->setTexture(textureID);
    }
    else {
        program->setTexture(0);
    }
    aiColor4D ambient, diffuse, emission, specular;
    Color color(0.0f, 0.0f, 0.0f, 0.0f);
    if (aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &ambient) == aiReturn_SUCCESS) {
        color.setValue(ambient.r, ambient.g, ambient.b, ambient.a);
    }
    else {
        color.setValue(0.2f, 0.2f, 0.2f, 1.0f);
    }
    program->setMaterialAmbient(color);
    if (aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse) == aiReturn_SUCCESS) {
        color.setValue(diffuse.r, diffuse.g, diffuse.b, diffuse.a);
    }
    else {
        color.setValue(0.8f, 0.8f, 0.8f, 1.0f);
    }
    program->setMaterialDiffuse(color);
    if (aiGetMaterialColor(material, AI_MATKEY_COLOR_EMISSIVE, &emission) == aiReturn_SUCCESS) {
        color.setValue(emission.r, emission.g, emission.b, emission.a);
    }
    else {
        color.setValue(0.0f, 0.0f, 0.0f, 0.0f);
    }
    program->setMaterialEmission(color);
    if (aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &specular) == aiReturn_SUCCESS) {
        color.setValue(specular.r, specular.g, specular.b, specular.a);
    }
    else {
        color.setValue(0.0f, 0.0f, 0.0f, 1.0f);
    }
    program->setMaterialSpecular(color);
    float shininess, strength;
    int ret1 = aiGetMaterialFloat(material, AI_MATKEY_SHININESS, &shininess);
    int ret2 = aiGetMaterialFloat(material, AI_MATKEY_SHININESS_STRENGTH, &strength);
    if (ret1 == aiReturn_SUCCESS && ret2 == aiReturn_SUCCESS) {
        program->setMaterialShininess(shininess * strength);
    }
    else if (ret1 == aiReturn_SUCCESS) {
        program->setMaterialShininess(shininess);
    }
    else {
        program->setMaterialShininess(15.0f);
    }
    float opacity;
    if (aiGetMaterialFloat(material, AI_MATKEY_OPACITY, &opacity) == aiReturn_SUCCESS) {
        program->setOpacity(opacity * asset->opacity());
    }
    else {
        program->setOpacity(asset->opacity());
    }
    int wireframe, twoside;
    if (aiGetMaterialInteger(material, AI_MATKEY_ENABLE_WIREFRAME, &wireframe) == aiReturn_SUCCESS && wireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if (aiGetMaterialInteger(material, AI_MATKEY_TWOSIDED, &twoside) == aiReturn_SUCCESS && twoside)
        glEnable(GL_CULL_FACE);
    else
        glDisable(GL_CULL_FACE);
}

void Renderer::renderAssetRecurse(const aiScene *scene, const aiNode *node, const Asset *asset)
{
    const btScalar &scaleFactor = asset->scaleFactor();
    const Bone *bone = asset->parentBone();
    float matrix4x4[16], matrix3x3[9];
    aiVector3D aiS, aiP;
    aiQuaternion aiQ;
    node->mTransformation.Decompose(aiS, aiQ, aiP);
    Transform transform(btMatrix3x3(Quaternion(aiQ.x, aiQ.y, aiQ.z, aiQ.w) * asset->rotation())
                        .scaled(Vector3(aiS.x * scaleFactor, aiS.y * scaleFactor, aiS.z * scaleFactor)),
                        Vector3(aiP.x,aiP.y, aiP.z) + asset->position());
    if (bone) {
        const Transform &boneTransform = bone->localTransform();
        transform.setBasis(boneTransform.getBasis() * transform.getBasis());
        transform.setOrigin(boneTransform.getOrigin() + transform.getOrigin());
    }
    AssetUserData *userData = static_cast<AssetUserData *>(asset->userData());
    AssetVertex v;
    const GLvoid *vertexPtr = 0;
    const GLvoid *normalPtr = reinterpret_cast<const GLvoid *>(reinterpret_cast<const uint8_t *>(&v.normal) - reinterpret_cast<const uint8_t *>(&v.position));
    const GLvoid *texcoordPtr = reinterpret_cast<const GLvoid *>(reinterpret_cast<const uint8_t *>(&v.texcoord) - reinterpret_cast<const uint8_t *>(&v.position));
    const GLvoid *colorPtr = reinterpret_cast<const GLvoid *>(reinterpret_cast<const uint8_t *>(&v.color) - reinterpret_cast<const uint8_t *>(&v.position));
    const unsigned int nmeshes = node->mNumMeshes;
    const size_t stride = sizeof(AssetVertex);
    AssetProgram *program = userData->programs[node];
    program->bind();
    m_scene->getModelViewMatrix(matrix4x4);
    program->setModelViewMatrix(matrix4x4);
    m_scene->getProjectionMatrix(matrix4x4);
    program->setProjectionMatrix(matrix4x4);
    m_scene->getNormalMatrix(matrix3x3);
    program->setNormalMatrix(matrix3x3);
    transform.getOpenGLMatrix(matrix4x4);
    program->setTransformMatrix(matrix4x4);
    program->setLightColor(m_scene->lightColor());
    program->setLightPosition(m_scene->lightPosition());
    for (unsigned int i = 0; i < nmeshes; i++) {
        const struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        const AssetVBO &vbo = userData->vbo[mesh];
        const AssetIndices &indices = userData->indices[mesh];
        setAssetMaterial(scene->mMaterials[mesh->mMaterialIndex], asset, program);
        glBindBuffer(GL_ARRAY_BUFFER, vbo.vertices);
        program->setPosition(vertexPtr, stride);
        program->setNormal(normalPtr, stride);
        program->setTexCoord(texcoordPtr, stride);
        program->setColor(colorPtr, stride);
        program->setHasColor(mesh->HasVertexColors(0));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.indices);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    }
    program->unbind();
    const unsigned int nChildNodes = node->mNumChildren;
    for (unsigned int i = 0; i < nChildNodes; i++)
        renderAssetRecurse(scene, node->mChildren[i], asset);
}
#endif

bool Renderer::isAcceleratorSupported()
{
#ifdef VPVL_ENABLE_OPENCL
    return true;
#else
    return false;
#endif
}

bool Renderer::isAcceleratorAvailable() const
{
    return isAcceleratorSupported() && m_accelerator ? m_accelerator->isAvailable() : false;
}

bool Renderer::initializeAccelerator()
{
    if (m_accelerator)
        return true;
    if (isAcceleratorSupported()) {
        m_accelerator = new Accelerator(m_delegate);
        return m_accelerator->initializeContext() && m_accelerator->createKernelPrograms();
    }
    return false;
}

}
}
