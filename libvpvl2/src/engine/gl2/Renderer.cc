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

#include <vpvl2/internal/gl2.h>

#include <btBulletDynamicsCommon.h>

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

namespace vpvl2
{
namespace gl2
{

#if 0
class Accelerator
{
public:
    static void initializeUserData(PMXModelUserData *userData) {
        userData->vertexBufferForCL = 0;
        userData->boneMatricesBuffer = 0;
        userData->originMatricesBuffer = 0;
        userData->outputMatricesBuffer = 0;
        userData->boneTransform = 0;
        userData->originTransform = 0;
        userData->bone1Indices = 0;
        userData->bone2Indices = 0;
        userData->weights = 0;
        userData->isBufferAllocated = false;
    }

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
    void uploadModel(PMXModelUserData *userData, const PMDModel *model) {
        if (!isAvailable())
            return;
        cl_int err;
        userData->vertexBufferForCL = clCreateFromGLBuffer(m_context,
                                                           CL_MEM_READ_WRITE,
                                                           userData->vertexBufferObjects[kModelVertices],
                                                           &err);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed creating OpenCL vertex buffer: %d", err);
            return;
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
            return;
        }
        userData->originMatricesBuffer = clCreateBuffer(m_context, CL_MEM_READ_ONLY, nBoneMatricesSize, 0, &err);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed creating originMatricesBuffer: %d", err);
            return;
        }
        userData->outputMatricesBuffer = clCreateBuffer(m_context, CL_MEM_READ_WRITE, nBoneMatricesSize, 0, &err);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed creating outputMatricesBuffer %d", err);
            return;
        }
        userData->weightsBuffer = clCreateBuffer(m_context, CL_MEM_READ_ONLY, nVerticesAlloc * sizeof(float), 0, &err);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed creating weightsBuffer: %d", err);
            return;
        }
        userData->bone1IndicesBuffer = clCreateBuffer(m_context, CL_MEM_READ_ONLY, nVerticesAlloc * sizeof(int), 0, &err);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed creating bone1IndicesBuffer: %d", err);
        }
        userData->bone2IndicesBuffer = clCreateBuffer(m_context, CL_MEM_READ_ONLY, nVerticesAlloc * sizeof(int), 0, &err);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed creating bone2IndicesBuffer: %d", err);
            return;
        }
        err = clGetKernelWorkGroupInfo(m_updateBoneMatricesKernel,
                                       m_device,
                                       CL_KERNEL_WORK_GROUP_SIZE,
                                       sizeof(userData->localWGSizeForUpdateBoneMatrices),
                                       &userData->localWGSizeForUpdateBoneMatrices,
                                       0);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed getting kernel work group information (CL_KERNEL_WORK_GROUP_SIZE): %d", err);
            return;
        }
        err = clGetKernelWorkGroupInfo(m_performSkinningKernel,
                                       m_device,
                                       CL_KERNEL_WORK_GROUP_SIZE,
                                       sizeof(userData->localWGSizeForPerformSkinning),
                                       &userData->localWGSizeForPerformSkinning,
                                       0);
        if (err != CL_SUCCESS) {
            m_delegate->log(Renderer::kLogWarning, "Failed getting kernel work group information (CL_KERNEL_WORK_GROUP_SIZE): %d", err);
            return;
        }
        userData->isBufferAllocated = true;
    }
    void deleteModel(PMXModelUserData *userData) {
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
    void updateModel(PMXModelUserData *userData, PMDModel *model) {
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
    static void initializeUserData(pmx::Model::UserData *userData) {}

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

    void uploadModel(pmx::Model::UserData * /* userData */, const pmx::Model * /* model */) {
    }
    void deleteModel(pmx::Model::UserData * /* userData */) {
    }
    void updateModel(pmx::Model::UserData * /* userData */, pmx::Model * /* model */) {
    }
};

#endif

#ifdef VPVL2_LINK_QT
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

#ifdef VPVL2_LINK_QT
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
    void setLightColor(const Vector3 &value) {
        glUniform3fv(m_lightColorUniformLocation, 1, value);
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
          m_uva0AttributeLocation(0),
          m_uva1AttributeLocation(0),
          m_uva2AttributeLocation(0),
          m_uva3AttributeLocation(0),
          m_uva4AttributeLocation(0),
          m_lightViewMatrixUniformLocation(0),
          m_normalMatrixUniformLocation(0),
          m_materialAmbientUniformLocation(0),
          m_materialDiffuseUniformLocation(0),
          m_mainTextureUniformLocation(0),
          m_hasMainTextureUniformLocation(0),
          m_sphereTextureUniformLocation(0),
          m_hasSphereTextureUniformLocation(0),
          m_isSPHTextureUniformLocation(0),
          m_isSPATextureUniformLocation(0),
          m_isSubTextureUniformLocation(0),
          m_toonTextureUniformLocation(0),
          m_hasToonTextureUniformLocation(0)
    {
    }
    ~ModelProgram() {
        m_texCoordAttributeLocation = 0;
        m_toonTexCoordAttributeLocation = 0;
        m_uva0AttributeLocation = 0;
        m_uva1AttributeLocation = 0;
        m_uva2AttributeLocation = 0;
        m_uva3AttributeLocation = 0;
        m_uva4AttributeLocation = 0;
        m_lightViewMatrixUniformLocation = 0;
        m_normalMatrixUniformLocation = 0;
        m_materialAmbientUniformLocation = 0;
        m_materialDiffuseUniformLocation = 0;
        m_mainTextureUniformLocation = 0;
        m_hasMainTextureUniformLocation = 0;
        m_sphereTextureUniformLocation = 0;
        m_hasSphereTextureUniformLocation = 0;
        m_isSPHTextureUniformLocation = 0;
        m_isSPATextureUniformLocation = 0;
        m_isSubTextureUniformLocation = 0;
        m_toonTextureUniformLocation = 0;
        m_hasToonTextureUniformLocation = 0;
    }

    virtual bool load(const char *vertexShaderSource, const char *fragmentShaderSource) {
        bool ret = ObjectProgram::load(vertexShaderSource, fragmentShaderSource);
        if (ret) {
            m_texCoordAttributeLocation = glGetAttribLocation(m_program, "inTexCoord");
            m_toonTexCoordAttributeLocation = glGetAttribLocation(m_program, "inToonTexCoord");
            m_uva0AttributeLocation = glGetAttribLocation(m_program, "inUVA0");
            m_uva1AttributeLocation = glGetAttribLocation(m_program, "inUVA1");
            m_uva2AttributeLocation = glGetAttribLocation(m_program, "inUVA2");
            m_uva3AttributeLocation = glGetAttribLocation(m_program, "inUVA3");
            m_uva4AttributeLocation = glGetAttribLocation(m_program, "inUVA4");
            m_lightViewMatrixUniformLocation = glGetUniformLocation(m_program, "lightViewMatrix");
            m_normalMatrixUniformLocation = glGetUniformLocation(m_program, "normalMatrix");
            m_materialAmbientUniformLocation = glGetUniformLocation(m_program, "materialAmbient");
            m_materialDiffuseUniformLocation = glGetUniformLocation(m_program, "materialDiffuse");
            m_mainTextureUniformLocation = glGetUniformLocation(m_program, "mainTexture");
            m_hasMainTextureUniformLocation = glGetUniformLocation(m_program, "hasMainTexture");
            m_sphereTextureUniformLocation = glGetUniformLocation(m_program, "sphereTexture");
            m_hasSphereTextureUniformLocation = glGetUniformLocation(m_program, "hasSphereTexture");
            m_isSPHTextureUniformLocation = glGetUniformLocation(m_program, "isSPHTexture");
            m_isSPATextureUniformLocation = glGetUniformLocation(m_program, "isSPATexture");
            m_isSubTextureUniformLocation = glGetUniformLocation(m_program, "isSubTexture");
            m_toonTextureUniformLocation = glGetUniformLocation(m_program, "toonTexture");
            m_hasToonTextureUniformLocation = glGetUniformLocation(m_program, "hasToonTexture");
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
    void setUVA0(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_uva0AttributeLocation);
        glVertexAttribPointer(m_uva0AttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setUVA1(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_uva1AttributeLocation);
        glVertexAttribPointer(m_uva1AttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setUVA2(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_uva2AttributeLocation);
        glVertexAttribPointer(m_uva2AttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setUVA3(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_uva3AttributeLocation);
        glVertexAttribPointer(m_uva3AttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setUVA4(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_uva4AttributeLocation);
        glVertexAttribPointer(m_uva4AttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setNormalMatrix(const float value[9]) {
        glUniformMatrix3fv(m_normalMatrixUniformLocation, 1, GL_FALSE, value);
    }
    void setMaterialAmbient(const Color &value) {
        glUniform3fv(m_materialAmbientUniformLocation, 1, value);
    }
    void setMaterialDiffuse(const Color &value) {
        glUniform4fv(m_materialDiffuseUniformLocation, 1, value);
    }
    void setLightPosition(const Vector3 &value) {
        float matrix[16];
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
    void setSphereTexture(GLuint value, pmx::Material::SphereTextureRenderMode mode) {
        if (value) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, value);
            glUniform1i(m_sphereTextureUniformLocation, 1);
            switch (mode) {
            case pmx::Material::kNone:
                glUniform1i(m_hasSphereTextureUniformLocation, 0);
                glUniform1i(m_isSPHTextureUniformLocation, 0);
                glUniform1i(m_isSPATextureUniformLocation, 0);
                glUniform1i(m_isSubTextureUniformLocation, 0);
                break;
            case pmx::Material::kModulate:
                glUniform1i(m_hasSphereTextureUniformLocation, 1);
                glUniform1i(m_isSPHTextureUniformLocation, 1);
                glUniform1i(m_isSPATextureUniformLocation, 0);
                glUniform1i(m_isSubTextureUniformLocation, 0);
                break;
            case pmx::Material::kAdditive:
                glUniform1i(m_hasSphereTextureUniformLocation, 1);
                glUniform1i(m_isSPHTextureUniformLocation, 0);
                glUniform1i(m_isSPATextureUniformLocation, 1);
                glUniform1i(m_isSubTextureUniformLocation, 0);
                break;
            case pmx::Material::kSubTexture:
                glUniform1i(m_hasSphereTextureUniformLocation, 1);
                glUniform1i(m_isSPHTextureUniformLocation, 0);
                glUniform1i(m_isSPATextureUniformLocation, 0);
                glUniform1i(m_isSubTextureUniformLocation, 1);
                break;
            }
        }
        else {
            glUniform1i(m_hasSphereTextureUniformLocation, 0);
        }
    }
    void setToonTexture(GLuint value) {
        if (value) {
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, value);
            glUniform1i(m_toonTextureUniformLocation, 2);
            glUniform1i(m_hasToonTextureUniformLocation, 1);
        }
        else {
            glUniform1i(m_hasToonTextureUniformLocation, 0);
        }
    }

private:
    GLuint m_texCoordAttributeLocation;
    GLuint m_uva0AttributeLocation;
    GLuint m_uva1AttributeLocation;
    GLuint m_uva2AttributeLocation;
    GLuint m_uva3AttributeLocation;
    GLuint m_uva4AttributeLocation;
    GLuint m_toonTexCoordAttributeLocation;
    GLuint m_lightViewMatrixUniformLocation;
    GLuint m_normalMatrixUniformLocation;
    GLuint m_materialAmbientUniformLocation;
    GLuint m_materialDiffuseUniformLocation;
    GLuint m_mainTextureUniformLocation;
    GLuint m_hasMainTextureUniformLocation;
    GLuint m_sphereTextureUniformLocation;
    GLuint m_hasSphereTextureUniformLocation;
    GLuint m_isSPHTextureUniformLocation;
    GLuint m_isSPATextureUniformLocation;
    GLuint m_isSubTextureUniformLocation;
    GLuint m_toonTextureUniformLocation;
    GLuint m_hasToonTextureUniformLocation;
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

Renderer::Renderer(IDelegate *delegate, int width, int height, int fps)
#ifdef VPVL2_LINK_QT
    : QGLFunctions(),
      #else
    :
      #endif /* VPVL2_LINK_QT */
      m_delegate(delegate),
      m_modelProgram(0),
      m_shadowProgram(0),
      m_accelerator(0),
      m_lightColor(0.6, 0.6, 0.6),
      m_lightPosition(0.5, 1, 0.5)
{
}

Renderer::~Renderer()
{
    const int nmodels = m_models.count();
    for (int i = 0; i < nmodels; i++) {
        pmx::Model *model = m_models[i];
        deleteModel(model);
    }
    delete m_accelerator;
    m_accelerator = 0;
    delete m_modelProgram;
    m_modelProgram = 0;
    delete m_shadowProgram;
    m_shadowProgram = 0;
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
    m_modelProgram = new ExtendedModelProgram(m_delegate);
    m_shadowProgram = new ShadowProgram(m_delegate);
#ifdef VPVL2_LINK_QT
    const QGLContext *context = QGLContext::currentContext();
    initializeGLFunctions(context);
    m_modelProgram->initializeContext(context);
    m_shadowProgram->initializeContext(context);
#endif
    std::string vertexShader;
    std::string fragmentShader;
    vertexShader = m_delegate->loadShader(kShadowVertexShader);
    fragmentShader = m_delegate->loadShader(kShadowFragmentShader);
    if (!m_shadowProgram->load(vertexShader.c_str(), fragmentShader.c_str()))
        return false;
    vertexShader = m_delegate->loadShader(kModelVertexShader);
    fragmentShader = m_delegate->loadShader(kModelFragmentShader);
    return m_modelProgram->load(vertexShader.c_str(), fragmentShader.c_str());
}

void Renderer::resize(int width, int height)
{
    //m_scene->setWidth(width);
    //m_scene->setHeight(height);
}

void Renderer::uploadModel(pmx::Model *model, const std::string &dir)
{
    uploadModel0(new PMXModelUserData(), model, dir);
}

void Renderer::uploadModel0(pmx::Model::UserData *userData, pmx::Model *model, const std::string &dir)
{
    PMXModelUserData *casted = static_cast<PMXModelUserData *>(userData);
    const Array<pmx::Material *> &materials = model->materials();
    const int nmaterials = materials.count();
    GLuint textureID = 0;
    PMXModelMaterialPrivate *materialPrivates = new PMXModelMaterialPrivate[nmaterials];
    for (int i = 0; i < nmaterials; i++) {
        const pmx::Material *material = materials[i];
        PMXModelMaterialPrivate &materialPrivate = materialPrivates[i];
        materialPrivate.mainTextureID = 0;
        materialPrivate.sphereTextureID = 0;
        materialPrivate.toonTextureID = 0;
        const IString *path = 0;
        path = material->mainTexture();
        if (path && m_delegate->uploadTexture(m_delegate->toUnicode(path), dir, textureID, false)) {
            m_delegate->log(kLogInfo, "Binding the texture as a main texture (ID=%d)", textureID);
            materialPrivate.mainTextureID = textureID;
        }
        path = material->sphereTexture();
        if (path && m_delegate->uploadTexture(m_delegate->toUnicode(path), dir, textureID, false)) {
            m_delegate->log(kLogInfo, "Binding the texture as a sphere texture (ID=%d)", textureID);
            materialPrivate.sphereTextureID = textureID;
        }
        if (material->isSharedToonTextureUsed()) {
            if (m_delegate->uploadToonTexture(material->toonTextureIndex(), textureID)) {
                m_delegate->log(kLogInfo, "Binding the texture as a shared toon texture (ID=%d)", textureID);
                materialPrivate.toonTextureID = textureID;
            }
        }
        else {
            path = material->toonTexture();
            if (m_delegate->uploadTexture(m_delegate->toUnicode(path), dir, textureID, true)) {
                m_delegate->log(kLogInfo, "Binding the texture as a static toon texture (ID=%d)", textureID);
                materialPrivate.toonTextureID = textureID;
            }
        }
    }
    glGenBuffers(kVertexBufferObjectMax, casted->vertexBufferObjects);
    /*
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, casted->vertexBufferObjects[kEdgeIndices]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->edgeIndicesCount() * pmx::Model::strideSize(PMDModel::kEdgeIndicesStride),
                 model->edgeIndicesPointer(), GL_STATIC_DRAW);
    m_delegate->log(kLogInfo,
                    "Binding edge indices to the vertex buffer object (ID=%d)",
                    casted->vertexBufferObjects[kEdgeIndices]);
                    */
    size_t size = pmx::Model::strideSize(pmx::Model::kIndexStride);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, casted->vertexBufferObjects[kModelIndices]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->indices().count() * size, model->indicesPtr(), GL_STATIC_DRAW);
    m_delegate->log(kLogInfo,
                    "Binding indices to the vertex buffer object (ID=%d)",
                    casted->vertexBufferObjects[kModelIndices]);
    size = pmx::Model::strideSize(pmx::Model::kVertexStride);
    glBindBuffer(GL_ARRAY_BUFFER, casted->vertexBufferObjects[kModelVertices]);
    glBufferData(GL_ARRAY_BUFFER, model->vertices().count() * size, model->vertexPtr(), GL_DYNAMIC_DRAW);
    m_delegate->log(kLogInfo,
                    "Binding model vertices to the vertex buffer object (ID=%d)",
                    casted->vertexBufferObjects[kModelVertices]);
    casted->materials = materialPrivates;
    //model->setSoftwareSkinningEnable(m_scene->isSoftwareSkinningEnabled());
    Accelerator::initializeUserData(casted);
    if (m_accelerator)
        m_accelerator->uploadModel(casted, model);
    model->setUserData(casted);
    model->update();
    model->setVisible(true);
    updateModel(model);
    m_delegate->log(kLogInfo, "Created the model: %s", m_delegate->toUnicode(model->name()).c_str());
    m_models.add(model);
}

void Renderer::deleteModel(pmx::Model *&model)
{
    if (model) {
        PMXModelUserData *userData = static_cast<PMXModelUserData *>(model->userData());
        const Array<pmx::Material *> &materials = model->materials();
        const int nmaterials = materials.count();
        for (int i = 0; i < nmaterials; i++) {
            PMXModelMaterialPrivate &materialPrivate = userData->materials[i];
            glDeleteTextures(1, &materialPrivate.mainTextureID);
            glDeleteTextures(1, &materialPrivate.sphereTextureID);
            glDeleteTextures(1, &materialPrivate.toonTextureID);
        }
        if (m_accelerator)
            m_accelerator->deleteModel(userData);
        glDeleteBuffers(kVertexBufferObjectMax, userData->vertexBufferObjects);
        delete[] userData->materials;
        delete userData;
        m_delegate->log(kLogInfo, "Destroyed the model: %s", m_delegate->toUnicode(model->name()).c_str());
        m_models.remove(model);
        delete model;
        model = 0;
    }
}

void Renderer::updateAllModel()
{
    const int nmodels = m_models.count();
    for (size_t i = 0; i < nmodels; i++) {
        pmx::Model *model = m_models[i];
        if (model->isVisible())
            updateModel(model);
    }
}

void Renderer::updateModel(pmx::Model *model)
{
    PMXModelUserData *userData = static_cast<PMXModelUserData *>(model->userData());
    size_t size = pmx::Model::strideSize(pmx::Model::kVertexStride);
    glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelVertices]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, model->vertices().count() * size, model->vertexPtr());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //if (m_accelerator && !m_scene->isSoftwareSkinningEnabled())
    if (m_accelerator)
        m_accelerator->updateModel(userData, model);
}

void Renderer::renderModel(const pmx::Model *model)
{
    const PMXModelUserData *userData = static_cast<PMXModelUserData *>(model->userData());

    m_modelProgram->bind();
    glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelVertices]);
    size_t offset = pmx::Model::strideOffset(pmx::Model::kVertexStride);
    size_t size   = pmx::Model::strideSize(pmx::Model::kVertexStride);
    m_modelProgram->setPosition(reinterpret_cast<const GLvoid *>(offset), size);
    offset = pmx::Model::strideOffset(pmx::Model::kNormalStride);
    size   = pmx::Model::strideSize(pmx::Model::kNormalStride);
    m_modelProgram->setNormal(reinterpret_cast<const GLvoid *>(offset), size);
    offset = pmx::Model::strideOffset(pmx::Model::kTexCoordStride);
    size   = pmx::Model::strideSize(pmx::Model::kTexCoordStride);
    m_modelProgram->setTexCoord(reinterpret_cast<const GLvoid *>(offset), size);
    offset = pmx::Model::strideOffset(pmx::Model::kUVA0Stride);
    size   = pmx::Model::strideSize(pmx::Model::kUVA0Stride);
    m_modelProgram->setUVA0(reinterpret_cast<const GLvoid *>(offset), size);
    offset = pmx::Model::strideOffset(pmx::Model::kUVA1Stride);
    size   = pmx::Model::strideSize(pmx::Model::kUVA1Stride);
    m_modelProgram->setUVA1(reinterpret_cast<const GLvoid *>(offset), size);
    offset = pmx::Model::strideOffset(pmx::Model::kUVA2Stride);
    size   = pmx::Model::strideSize(pmx::Model::kUVA2Stride);
    m_modelProgram->setUVA2(reinterpret_cast<const GLvoid *>(offset), size);
    offset = pmx::Model::strideOffset(pmx::Model::kUVA3Stride);
    size   = pmx::Model::strideSize(pmx::Model::kUVA3Stride);
    m_modelProgram->setUVA3(reinterpret_cast<const GLvoid *>(offset), size);
    offset = pmx::Model::strideOffset(pmx::Model::kUVA4Stride);
    size   = pmx::Model::strideSize(pmx::Model::kUVA4Stride);
    m_modelProgram->setUVA4(reinterpret_cast<const GLvoid *>(offset), size);

    float normalMatrix[9] = {
        m_modelViewMatrix[0], m_modelViewMatrix[1], m_modelViewMatrix[2],
        m_modelViewMatrix[4], m_modelViewMatrix[5], m_modelViewMatrix[6],
        m_modelViewMatrix[8], m_modelViewMatrix[9], m_modelViewMatrix[10],
    };
    m_modelProgram->setModelViewMatrix(m_modelViewMatrix);
    m_modelProgram->setProjectionMatrix(m_projectionMatrix);
    m_modelProgram->setNormalMatrix(normalMatrix);
    m_modelProgram->setLightColor(m_lightColor);
    m_modelProgram->setLightPosition(m_lightPosition);

    const Array<pmx::Material *> &materials = model->materials();
    const PMXModelMaterialPrivate *materialPrivates = userData->materials;
    const int nmaterials = materials.count();
    Color ambient, diffuse;
    GLenum type = GL_UNSIGNED_INT;

    offset = 0; size = pmx::Model::strideSize(pmx::Model::kIndexStride);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, userData->vertexBufferObjects[kModelIndices]);
    for (int i = 0; i < nmaterials; i++) {
        const pmx::Material *material = materials[i];
        const PMXModelMaterialPrivate &materialPrivate = materialPrivates[i];
        ambient = material->ambient();
        diffuse = material->diffuse();
        m_modelProgram->setMaterialAmbient(ambient);
        m_modelProgram->setMaterialDiffuse(diffuse);
        m_modelProgram->setMainTexture(materialPrivate.mainTextureID);
        m_modelProgram->setSphereTexture(materialPrivate.sphereTextureID, material->sphereTextureRenderMode());
        m_modelProgram->setToonTexture(materialPrivate.toonTextureID);
        material->isCullFaceDisabled() ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);
        const int nindices = material->indices();
        glDrawElements(GL_TRIANGLES, nindices, type, reinterpret_cast<const GLvoid *>(offset));
        offset += nindices * size;
    }
    m_modelProgram->unbind();
    glEnable(GL_CULL_FACE);
}

void Renderer::renderModelShadow(const pmx::Model *model)
{
    static const Vector3 plane(0.0f, 1.0f, 0.0f);
    const PMXModelUserData *userData = static_cast<PMXModelUserData *>(model->userData());
    const Scalar dot = plane.dot(m_lightPosition);
    float shadowMatrix[16];
    glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelVertices]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, userData->vertexBufferObjects[kModelIndices]);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int index = (i << 2) + j;
            shadowMatrix[index] = -plane[i] * m_lightPosition[j];
            if (i == j)
                shadowMatrix[index] += dot;
        }
    }
    m_shadowProgram->bind();
    m_shadowProgram->setModelViewMatrix(m_modelViewMatrix);
    m_shadowProgram->setProjectionMatrix(m_projectionMatrix);
    m_shadowProgram->setShadowMatrix(shadowMatrix);
    m_shadowProgram->setLightColor(m_lightColor);
    m_shadowProgram->setLightPosition(m_lightPosition);
    size_t offset = pmx::Model::strideOffset(pmx::Model::kVertexStride);
    size_t size = pmx::Model::strideSize(pmx::Model::kVertexStride);
    m_shadowProgram->setPosition(reinterpret_cast<const GLvoid *>(offset), size);
    glDrawElements(GL_TRIANGLES, model->indices().count(), GL_UNSIGNED_INT, 0);
    m_shadowProgram->unbind();
}

void Renderer::setModelViewMatrix(const float value[])
{
    memcpy(m_modelViewMatrix, value, sizeof(m_modelViewMatrix));
}

void Renderer::setProjectionMatrix(const float value[])
{
    memcpy(m_projectionMatrix, value, sizeof(m_projectionMatrix));
}

void Renderer::setLightColor(const Vector3 &value)
{
    m_lightColor = value;
}

void Renderer::setLightPosition(const Vector3 &value)
{
    m_lightPosition = value;
}

#if 0
void Renderer::renderModelEdge(const pmx::Model *model)
{
    if (model->edgeOffset() == 0.0f)
        return;
    const PMXModelUserData *userData = static_cast<PMXModelUserData *>(model->userData());
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
#endif

void Renderer::clear()
{
    //glViewport(0, 0, m_scene->width(), m_scene->height());
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::renderAllModels()
{
    int nmodels = m_models.count();
    for (int i = 0; i < nmodels; i++) {
        pmx::Model *model = m_models[i];
        if (model->isVisible()) {
            renderModel(model);
        }
    }
}

void Renderer::renderProjectiveShadow()
{
    const int nmodels = m_models.count();
    glCullFace(GL_FRONT);
    for (size_t i = 0; i < nmodels; i++) {
        pmx::Model *model = m_models[i];
        if (model->isVisible())
            renderModelShadow(model);
    }
    glCullFace(GL_BACK);
}

bool Renderer::isAcceleratorSupported()
{
#ifdef VPVL2_ENABLE_OPENCL
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
