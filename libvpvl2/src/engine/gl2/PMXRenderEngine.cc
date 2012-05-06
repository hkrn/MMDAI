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

#include "vpvl2/vpvl2.h"
#include "vpvl2/gl2/PMXRenderEngine.h"
#include "vpvl2/pmx/Material.h"
#include "vpvl2/pmx/Model.h"
#include "vpvl2/pmx/Vertex.h"

#include "EngineCommon.h"

namespace {

using namespace vpvl2;
using namespace vpvl2::gl2;

enum VertexBufferObjectType
{
    kModelVertices,
    kModelIndices,
    kVertexBufferObjectMax
};

struct MaterialTextures
{
    GLuint mainTextureID;
    GLuint sphereTextureID;
    GLuint toonTextureID;
};

class EdgeProgram : public BaseShaderProgram
{
public:
    EdgeProgram(IRenderDelegate *delegate)
        : BaseShaderProgram(delegate),
          m_normalAttributeLocation(0),
          m_edgeSizeAttributeLocation(0),
          m_colorUniformLocation(0),
          m_sizeUniformLocation(0)
    {
    }
    ~EdgeProgram() {
        m_normalAttributeLocation = 0;
        m_edgeSizeAttributeLocation = 0;
        m_colorUniformLocation = 0;
        m_sizeUniformLocation = 0;
    }

    bool load(const IString *vertexShaderSource, const IString *fragmentShaderSource, void *context) {
        bool ret = BaseShaderProgram::load(vertexShaderSource, fragmentShaderSource, context);
        if (ret) {
            m_normalAttributeLocation = glGetAttribLocation(m_program, "inNormal");
            m_edgeSizeAttributeLocation = glGetAttribLocation(m_program, "inEdgeSize");
            m_colorUniformLocation = glGetUniformLocation(m_program, "color");
            m_sizeUniformLocation = glGetUniformLocation(m_program, "size");
        }
        return ret;
    }
    void setNormal(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_normalAttributeLocation);
        glVertexAttribPointer(m_normalAttributeLocation, 3, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setEdgeSize(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_edgeSizeAttributeLocation);
        glVertexAttribPointer(m_edgeSizeAttributeLocation, 1, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setColor(const Color &value) {
        glUniform4fv(m_colorUniformLocation, 1, value);
    }
    void setSize(const Scalar &value) {
        glUniform1f(m_sizeUniformLocation, value);
    }

private:
    GLuint m_normalAttributeLocation;
    GLuint m_edgeSizeAttributeLocation;
    GLuint m_colorUniformLocation;
    GLuint m_sizeUniformLocation;
};

class ShadowProgram : public ObjectProgram
{
public:
    ShadowProgram(IRenderDelegate *delegate)
        : ObjectProgram(delegate),
          m_shadowMatrixUniformLocation(0)
    {
    }
    ~ShadowProgram() {
        m_shadowMatrixUniformLocation = 0;
    }

    bool load(const IString *vertexShaderSource, const IString *fragmentShaderSource, void *context) {
        bool ret = ObjectProgram::load(vertexShaderSource, fragmentShaderSource, context);
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
    ModelProgram(IRenderDelegate *delegate)
        : ObjectProgram(delegate),
          m_toonTexCoordAttributeLocation(0),
          m_uva1AttributeLocation(0),
          m_modelViewInverseMatrixUniformLocation(0),
          m_materialAmbientUniformLocation(0),
          m_materialDiffuseUniformLocation(0),
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
        m_toonTexCoordAttributeLocation = 0;
        m_uva1AttributeLocation = 0;
        m_modelViewInverseMatrixUniformLocation = 0;
        m_materialAmbientUniformLocation = 0;
        m_materialDiffuseUniformLocation = 0;
        m_sphereTextureUniformLocation = 0;
        m_hasSphereTextureUniformLocation = 0;
        m_isSPHTextureUniformLocation = 0;
        m_isSPATextureUniformLocation = 0;
        m_isSubTextureUniformLocation = 0;
        m_toonTextureUniformLocation = 0;
        m_hasToonTextureUniformLocation = 0;
    }

    bool load(const IString *vertexShaderSource, const IString *fragmentShaderSource, void *context) {
        bool ret = ObjectProgram::load(vertexShaderSource, fragmentShaderSource, context);
        if (ret) {
            m_toonTexCoordAttributeLocation = glGetAttribLocation(m_program, "inToonTexCoord");
            m_uva1AttributeLocation = glGetAttribLocation(m_program, "inUVA1");
            m_modelViewInverseMatrixUniformLocation = glGetUniformLocation(m_program, "modelViewInverseMatrix");
            m_materialAmbientUniformLocation = glGetUniformLocation(m_program, "materialAmbient");
            m_materialDiffuseUniformLocation = glGetUniformLocation(m_program, "materialDiffuse");
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
    void bind() {
        ObjectProgram::bind();
    }
    void setModelViewInverseMatrix(const GLfloat value[16]) {
        glUniformMatrix4fv(m_modelViewInverseMatrixUniformLocation, 1, GL_FALSE, value);
    }
    void setToonTexCoord(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_toonTexCoordAttributeLocation);
        glVertexAttribPointer(m_toonTexCoordAttributeLocation, 2, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setUVA1(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_uva1AttributeLocation);
        glVertexAttribPointer(m_uva1AttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setMaterialAmbient(const Color &value) {
        glUniform3fv(m_materialAmbientUniformLocation, 1, value);
    }
    void setMaterialDiffuse(const Color &value) {
        glUniform4fv(m_materialDiffuseUniformLocation, 1, value);
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
            case pmx::Material::kMultTexture:
                glUniform1i(m_hasSphereTextureUniformLocation, 1);
                glUniform1i(m_isSPHTextureUniformLocation, 1);
                glUniform1i(m_isSPATextureUniformLocation, 0);
                glUniform1i(m_isSubTextureUniformLocation, 0);
                break;
            case pmx::Material::kAddTexture:
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
    GLuint m_toonTexCoordAttributeLocation;
    GLuint m_uva1AttributeLocation;
    GLuint m_modelViewInverseMatrixUniformLocation;
    GLuint m_materialAmbientUniformLocation;
    GLuint m_materialDiffuseUniformLocation;
    GLuint m_sphereTextureUniformLocation;
    GLuint m_hasSphereTextureUniformLocation;
    GLuint m_isSPHTextureUniformLocation;
    GLuint m_isSPATextureUniformLocation;
    GLuint m_isSubTextureUniformLocation;
    GLuint m_toonTextureUniformLocation;
    GLuint m_hasToonTextureUniformLocation;
};

}

namespace vpvl2
{
namespace gl2
{

class PMXRenderEngine::PrivateContext
        #ifdef VPVL2_LINK_QT
        : protected QGLFunctions
        #endif
{
public:
    PrivateContext()
        : edgeProgram(0),
          modelProgram(0),
          shadowProgram(0),
          zplotProgram(0),
          materials(0)
    #ifdef VPVL2_ENABLE_OPENCL
        ,
          vertexBufferForCL(0),
          boneMatricesBuffer(0),
          originMatricesBuffer(0),
          outputMatricesBuffer(0),
          weightsBuffer(0),
          bone1IndicesBuffer(0),
          bone2IndicesBuffer(0),
          weights(0),
          boneTransform(0),
          originTransform(0),
          bone1Indices(0),
          bone2Indices(0),
          isBufferAllocated(false)
    #endif /* VPVL2_ENABLE_OPENCL */
        ,
          cullFaceState(true)
    {
    }
    virtual ~PrivateContext() {
        glDeleteBuffers(kVertexBufferObjectMax, vertexBufferObjects);
        delete edgeProgram;
        edgeProgram = 0;
        delete modelProgram;
        modelProgram = 0;
        delete shadowProgram;
        shadowProgram = 0;
        delete zplotProgram;
        zplotProgram = 0;
#ifdef VPVL2_ENABLE_OPENCL
        delete[] boneTransform;
        delete[] originTransform;
        delete[] bone1Indices;
        delete[] bone2Indices;
        delete[] weights;
        clReleaseMemObject(vertexBufferForCL);
        clReleaseMemObject(boneMatricesBuffer);
        clReleaseMemObject(originMatricesBuffer);
        clReleaseMemObject(outputMatricesBuffer);
        clReleaseMemObject(bone1IndicesBuffer);
        clReleaseMemObject(bone2IndicesBuffer);
        clReleaseMemObject(weightsBuffer);
        isBufferAllocated = false;
#endif /* VPVL2_ENABLE_OPENCL */
        cullFaceState = false;
    }

#ifdef VPVL2_LINK_QT
    void initializeContext(const QGLContext *context) {
        initializeGLFunctions(context);
    }
#endif /* VPVL2_LINK_QT */

    void releaseMaterials(pmx::Model *model) {
        if (materials) {
            const Array<pmx::Material *> &modelMaterials = model->materials();
            const int nmaterials = modelMaterials.count();
            for (int i = 0; i < nmaterials; i++) {
                MaterialTextures &materialPrivate = materials[i];
                glDeleteTextures(1, &materialPrivate.mainTextureID);
                glDeleteTextures(1, &materialPrivate.sphereTextureID);
                glDeleteTextures(1, &materialPrivate.toonTextureID);
            }
            delete[] materials;
            materials = 0;
        }
    }

    EdgeProgram *edgeProgram;
    ModelProgram *modelProgram;
    ShadowProgram *shadowProgram;
    ZPlotProgram *zplotProgram;
    GLuint vertexBufferObjects[kVertexBufferObjectMax];
    MaterialTextures *materials;
#ifdef VPVL2_ENABLE_OPENCL
    cl_mem vertexBufferForCL;
    cl_mem boneMatricesBuffer;
    cl_mem originMatricesBuffer;
    cl_mem outputMatricesBuffer;
    cl_mem weightsBuffer;
    cl_mem bone1IndicesBuffer;
    cl_mem bone2IndicesBuffer;
    size_t localWGSizeForUpdateBoneMatrices;
    size_t localWGSizeForPerformSkinning;
    float *weights;
    float *boneTransform;
    float *originTransform;
    int *bone1Indices;
    int *bone2Indices;
    bool isBufferAllocated;
#endif /* VPVL2_ENABLE_OPENCL */
    bool cullFaceState;
};

#if 0
class PMXRenderEngine::Accelerator : public BaseAccelerator
{
public:
    static void initializeUserData(PMXRenderEngine::PrivateContext *context) {
        context->vertexBufferForCL = 0;
        context->boneMatricesBuffer = 0;
        context->originMatricesBuffer = 0;
        context->outputMatricesBuffer = 0;
        context->boneTransform = 0;
        context->originTransform = 0;
        context->bone1Indices = 0;
        context->bone2Indices = 0;
        context->weights = 0;
        context->isBufferAllocated = false;
    }

    Accelerator(IRenderDelegate *delegate)
        : BaseAccelerator(delegate),
          m_updateBoneMatricesKernel(0),
          m_performSkinningKernel(0)
    {
    }
    ~Accelerator() {
        clReleaseKernel(m_performSkinningKernel);
        m_performSkinningKernel = 0;
        clReleaseKernel(m_updateBoneMatricesKernel);
        m_updateBoneMatricesKernel = 0;
    }

    bool createKernelPrograms() {
        if (!m_context)
            return false;
        cl_int err;
        const std::string &source = m_delegate->loadKernel(IRenderDelegate::kModelSkinningKernel, 0);
        const char *sourceText = source.c_str();
        const size_t sourceSize = source.size();
        clReleaseProgram(m_program);
        m_program = clCreateProgramWithSource(m_context, 1, &sourceText, &sourceSize, &err);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed creating an OpenCL program: %d", err);
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
    void uploadModel(PMXRenderEngine::PrivateContext *userData, const pmx::Model *model, void *context) {
        if (!isAvailable())
            return;
        cl_int err;
        userData->vertexBufferForCL = clCreateFromGLBuffer(m_context,
                                                           CL_MEM_READ_WRITE,
                                                           userData->vertexBufferObjects[kModelVertices],
                                                           &err);
        if (err != CL_SUCCESS) {
            log0(context, IRenderDelegate::kLogWarning, "Failed creating OpenCL vertex buffer: %d", err);
            return;
        }
        const int nBoneMatricesAllocs = model->bones().count() << 4;
        const int nBoneMatricesSize = nBoneMatricesAllocs * sizeof(float);
        userData->boneTransform = new float[nBoneMatricesAllocs];
        userData->originTransform = new float[nBoneMatricesAllocs];
        const Array<pmx::Vertex *> &vertices = model->vertices();
        const int nVerticesAlloc = vertices.count();
        userData->bone1Indices = new int[nVerticesAlloc];
        userData->bone2Indices = new int[nVerticesAlloc];
        userData->weights = new float[nVerticesAlloc];
        for (int i = 0; i < nVerticesAlloc; i++) {
            const pmx::Vertex *vertex = vertices[i];
            userData->bone1Indices[i] = vertex->bone(0)->index();
            userData->bone2Indices[i] = vertex->bone(1)->index();
            userData->weights[i] = vertex->weight(0);
        }
        userData->boneMatricesBuffer = clCreateBuffer(m_context, CL_MEM_READ_ONLY, nBoneMatricesSize, 0, &err);
        if (err != CL_SUCCESS) {
            log0(context, IRenderDelegate::kLogWarning, "Failed creating boneMatricesBuffer: %d", err);
            return;
        }
        userData->originMatricesBuffer = clCreateBuffer(m_context, CL_MEM_READ_ONLY, nBoneMatricesSize, 0, &err);
        if (err != CL_SUCCESS) {
            log0(context, IRenderDelegate::kLogWarning, "Failed creating originMatricesBuffer: %d", err);
            return;
        }
        userData->outputMatricesBuffer = clCreateBuffer(m_context, CL_MEM_READ_WRITE, nBoneMatricesSize, 0, &err);
        if (err != CL_SUCCESS) {
            log0(context, IRenderDelegate::kLogWarning, "Failed creating outputMatricesBuffer %d", err);
            return;
        }
        userData->weightsBuffer = clCreateBuffer(m_context, CL_MEM_READ_ONLY, nVerticesAlloc * sizeof(float), 0, &err);
        if (err != CL_SUCCESS) {
            log0(context, IRenderDelegate::kLogWarning, "Failed creating weightsBuffer: %d", err);
            return;
        }
        userData->bone1IndicesBuffer = clCreateBuffer(m_context, CL_MEM_READ_ONLY, nVerticesAlloc * sizeof(int), 0, &err);
        if (err != CL_SUCCESS) {
            log0(context, IRenderDelegate::kLogWarning, "Failed creating bone1IndicesBuffer: %d", err);
        }
        userData->bone2IndicesBuffer = clCreateBuffer(m_context, CL_MEM_READ_ONLY, nVerticesAlloc * sizeof(int), 0, &err);
        if (err != CL_SUCCESS) {
            log0(context, IRenderDelegate::kLogWarning, "Failed creating bone2IndicesBuffer: %d", err);
            return;
        }
        err = clGetKernelWorkGroupInfo(m_updateBoneMatricesKernel,
                                       m_device,
                                       CL_KERNEL_WORK_GROUP_SIZE,
                                       sizeof(userData->localWGSizeForUpdateBoneMatrices),
                                       &userData->localWGSizeForUpdateBoneMatrices,
                                       0);
        if (err != CL_SUCCESS) {
            log0(context, IRenderDelegate::kLogWarning, "Failed getting kernel work group information (CL_KERNEL_WORK_GROUP_SIZE): %d", err);
            return;
        }
        err = clGetKernelWorkGroupInfo(m_performSkinningKernel,
                                       m_device,
                                       CL_KERNEL_WORK_GROUP_SIZE,
                                       sizeof(userData->localWGSizeForPerformSkinning),
                                       &userData->localWGSizeForPerformSkinning,
                                       0);
        if (err != CL_SUCCESS) {
            log0(context, IRenderDelegate::kLogWarning, "Failed getting kernel work group information (CL_KERNEL_WORK_GROUP_SIZE): %d", err);
            return;
        }
        userData->isBufferAllocated = true;
    }
    void deleteModel(PMXRenderEngine::PrivateContext *context) {
        if (!isAvailable())
            return;
        delete[] context->boneTransform;
        delete[] context->originTransform;
        delete[] context->bone1Indices;
        delete[] context->bone2Indices;
        delete[] context->weights;
        clReleaseMemObject(context->vertexBufferForCL);
        clReleaseMemObject(context->boneMatricesBuffer);
        clReleaseMemObject(context->originMatricesBuffer);
        clReleaseMemObject(context->outputMatricesBuffer);
        clReleaseMemObject(context->bone1IndicesBuffer);
        clReleaseMemObject(context->bone2IndicesBuffer);
        clReleaseMemObject(context->weightsBuffer);
        context->isBufferAllocated = false;
    }
    void updateModel(PMXRenderEngine::PrivateContext *context, pmx::Model *model) {
        if (!isAvailable() || !context->isBufferAllocated)
            return;
        Array<IBone *> bones;
        model->getBones(bones);
        const int nbones = bones.count();
        Transform transform = Transform::getIdentity();
        for (int i = 0; i < nbones; i++) {
            IBone *bone = bones[i];
            int index = i << 4;
#define SOFTWARE_BONE_TRANSFORM
#ifndef SOFTWARE_BONE_TRANSFORM
            bone->localTransform().getOpenGLMatrix(&userData->boneTransform[index]);
            transform.setOrigin(bone->originPosition());
            transform.getOpenGLMatrix(&userData->originTransform[index]);
#else
            const Transform &transform = bone->localTransform();
            transform.getOpenGLMatrix(&context->boneTransform[index]);
#endif
        }
        size_t nsize = (nbones * sizeof(float)) << 4;
        cl_int err;
#ifndef SOFTWARE_BONE_TRANSFORM
        err = clEnqueueWriteBuffer(m_queue, userData->boneMatricesBuffer, CL_TRUE, 0, nsize, userData->boneTransform, 0, 0, 0);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed enqueue a command to write bone matrices buffer: %d", err);
            return;
        }
        err = clEnqueueWriteBuffer(m_queue, userData->originMatricesBuffer, CL_TRUE, 0, nsize, userData->originTransform, 0, 0, 0);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed enqueue a command to write origin matrices buffer: %d", err);
            return;
        }
        err = clSetKernelArg(m_updateBoneMatricesKernel, 0, sizeof(userData->boneMatricesBuffer), &userData->boneMatricesBuffer);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 1st argument of kernel (boneMatricesBuffer): %d", err);
            return;
        }
        err = clSetKernelArg(m_updateBoneMatricesKernel, 1, sizeof(userData->originMatricesBuffer), &userData->originMatricesBuffer);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 2nd argument of kernel (originMatricesBuffer): %d", err);
            return;
        }
        err = clSetKernelArg(m_updateBoneMatricesKernel, 2, sizeof(int), &nbones);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 3th argument of kernel (nbones): %d", err);
            return;
        }
        err = clSetKernelArg(m_updateBoneMatricesKernel, 3, sizeof(userData->outputMatricesBuffer), &userData->outputMatricesBuffer);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 4rd argument of kernel (outputMatricesBuffer): %d", err);
            return;
        }
        size_t local = userData->localWGSizeForUpdateBoneMatrices;
        size_t global = local * ((nbones + (local - 1)) / local);
        err = clEnqueueNDRangeKernel(m_queue, m_updateBoneMatricesKernel, 1, 0, &global, &local, 0, 0, 0);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed enqueue executing kernel");
            return;
        }
        clFinish(m_queue);
#else
        size_t local, global;
        err = clEnqueueWriteBuffer(m_queue, context->outputMatricesBuffer, CL_TRUE, 0, nsize, context->boneTransform, 0, 0, 0);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed enqueue a command to write output matrices buffer: %d", err);
            return;
        }
#endif
        /* force flushing OpenGL commands to acquire GL objects by OpenCL */
        glFinish();
        clEnqueueAcquireGLObjects(m_queue, 1, &context->vertexBufferForCL, 0, 0, 0);
        int nvertices = model->vertices().count();
        err = clEnqueueWriteBuffer(m_queue, context->bone1IndicesBuffer, CL_TRUE, 0, nvertices * sizeof(int), context->bone1Indices, 0, 0, 0);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed enqueue a command to write bone1 indices buffer: %d", err);
            return;
        }
        err = clEnqueueWriteBuffer(m_queue, context->bone2IndicesBuffer, CL_TRUE, 0, nvertices * sizeof(int), context->bone2Indices, 0, 0, 0);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed enqueue a command to write bone2 indices buffer: %d", err);
            return;
        }
        err = clEnqueueWriteBuffer(m_queue, context->weightsBuffer, CL_TRUE, 0, nvertices * sizeof(float), context->weights, 0, 0, 0);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed enqueue a command to write weights buffer: %d", err);
            return;
        }
        err = clSetKernelArg(m_performSkinningKernel, 0, sizeof(context->outputMatricesBuffer), &context->outputMatricesBuffer);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 1st argument of kernel (skinningMatrices): %d", err);
            return;
        }
        err = clSetKernelArg(m_performSkinningKernel, 1, sizeof(context->weightsBuffer), &context->weightsBuffer);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 2nd argument of kernel (weights): %d", err);
            return;
        }
        err = clSetKernelArg(m_performSkinningKernel, 2, sizeof(context->bone1IndicesBuffer), &context->bone1IndicesBuffer);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 3rd argument of kernel (bone1Indices): %d", err);
            return;
        }
        err = clSetKernelArg(m_performSkinningKernel, 3, sizeof(context->bone2IndicesBuffer), &context->bone2IndicesBuffer);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 4th argument of kernel (bone2Indices): %d", err);
            return;
        }
        const Vector3 &lightDirection = kZeroV3; // FIXME: get light direction
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
        size_t strideSize = model->strideSize(pmx::Model::kVertexStride) >> 4;
        err = clSetKernelArg(m_performSkinningKernel, 6, sizeof(strideSize), &strideSize);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 7th argument of kernel (strideSize): %d", err);
            return;
        }
        size_t offsetPosition = model->strideOffset(pmx::Model::kVertexStride) >> 4;
        err = clSetKernelArg(m_performSkinningKernel, 7, sizeof(offsetPosition), &offsetPosition);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 8th argument of kernel (offsetPosition): %d", err);
            return;
        }
        size_t offsetNormal = model->strideOffset(pmx::Model::kNormalStride) >> 4;
        err = clSetKernelArg(m_performSkinningKernel, 8, sizeof(offsetNormal), &offsetNormal);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 9th argument of kernel (offsetNormal): %d", err);
            return;
        }
        size_t offsetToonTexture = model->strideOffset(pmx::Model::kToonTextureStride) >> 4;
        err = clSetKernelArg(m_performSkinningKernel, 9, sizeof(offsetToonTexture), &offsetToonTexture);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 10th argument of kernel (offsetTexCoord): %d", err);
            return;
        }
        size_t offsetEdge = model->strideOffset(pmx::Model::kEdgeVerticesStride) >> 4;
        err = clSetKernelArg(m_performSkinningKernel, 10, sizeof(offsetEdge), &offsetEdge);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 11th argument of kernel (offsetEdge): %d", err);
            return;
        }
        err = clSetKernelArg(m_performSkinningKernel, 11, sizeof(context->vertexBufferForCL), &context->vertexBufferForCL);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 12th argument of kernel (vertices): %d", err);
            return;
        }
        local = context->localWGSizeForUpdateBoneMatrices;
        global = local * ((nvertices + (local - 1)) / local);
        err = clEnqueueNDRangeKernel(m_queue, m_performSkinningKernel, 1, 0, &global, &local, 0, 0, 0);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed enqueue executing kernel: %d", err);
            return;
        }
        clEnqueueReleaseGLObjects(m_queue, 1, &context->vertexBufferForCL, 0, 0, 0);
        clFinish(m_queue);
    }

private:
    cl_kernel m_updateBoneMatricesKernel;
    cl_kernel m_performSkinningKernel;
};
#else
class PMXRenderEngine::Accelerator : public BaseAccelerator {
public:
    static void initializeUserData(PMXRenderEngine::PrivateContext * /* context */) {}

    Accelerator(IRenderDelegate *delegate) : BaseAccelerator(delegate) {}
    ~Accelerator() {}

    void uploadModel(PMXRenderEngine::PrivateContext * /* context */, const pmx::Model * /* model */, void * /* context */) {
    }
    void deleteModel(PMXRenderEngine::PrivateContext * /* context */) {
    }
    void updateModel(PMXRenderEngine::PrivateContext * /* context */, pmx::Model * /* model */) {
    }
};
#endif

PMXRenderEngine::PMXRenderEngine(IRenderDelegate *delegate, const Scene *scene, pmx::Model *model)
#ifdef VPVL2_LINK_QT
    : QGLFunctions(),
      #else
    :
      #endif /* VPVL2_LINK_QT */
      m_delegate(delegate),
      m_scene(scene),
      m_model(model),
      m_context(0),
      m_accelerator(0)
{
    m_context = new PrivateContext();
}

PMXRenderEngine::~PMXRenderEngine()
{
    if (m_context) {
        m_context->releaseMaterials(m_model);
        delete m_context;
        m_context = 0;
    }
    if (m_accelerator)
        m_accelerator->deleteModel(m_context);
    delete m_accelerator;
    m_accelerator = 0;
    m_delegate = 0;
    m_scene = 0;
    m_model = 0;
    m_accelerator = 0;
}

IModel *PMXRenderEngine::model() const
{
    return m_model;
}

bool PMXRenderEngine::upload(const IString *dir)
{
    bool ret = true;
    void *context = 0;
    m_delegate->allocateContext(m_model, context);
    m_context->edgeProgram = new EdgeProgram(m_delegate);
    m_context->modelProgram = new ModelProgram(m_delegate);
    m_context->shadowProgram = new ShadowProgram(m_delegate);
    m_context->zplotProgram = new ZPlotProgram(m_delegate);
#ifdef VPVL2_LINK_QT
    const QGLContext *glContext = QGLContext::currentContext();
    initializeGLFunctions(glContext);
    m_context->initializeContext(glContext);
    m_context->edgeProgram->initializeContext(glContext);
    m_context->modelProgram->initializeContext(glContext);
    m_context->shadowProgram->initializeContext(glContext);
    m_context->zplotProgram->initializeContext(glContext);
#endif /* VPVL2_LINK_QT */
    IString *vertexShaderSource = 0;
    IString *fragmentShaderSource = 0;
    vertexShaderSource = m_delegate->loadShaderSource(IRenderDelegate::kEdgeVertexShader, m_model, context);
    fragmentShaderSource = m_delegate->loadShaderSource(IRenderDelegate::kEdgeFragmentShader, m_model, context);
    ret = m_context->edgeProgram->load(vertexShaderSource, fragmentShaderSource, context);
    delete vertexShaderSource;
    delete fragmentShaderSource;
    if (!ret)
        return ret;
    vertexShaderSource = m_delegate->loadShaderSource(IRenderDelegate::kModelVertexShader, m_model, context);
    fragmentShaderSource = m_delegate->loadShaderSource(IRenderDelegate::kModelFragmentShader, m_model, context);
    ret = m_context->modelProgram->load(vertexShaderSource, fragmentShaderSource, context);
    delete vertexShaderSource;
    delete fragmentShaderSource;
    if (!ret)
        return ret;
    vertexShaderSource = m_delegate->loadShaderSource(IRenderDelegate::kShadowVertexShader, m_model, context);
    fragmentShaderSource = m_delegate->loadShaderSource(IRenderDelegate::kShadowFragmentShader, m_model, context);
    ret = m_context->shadowProgram->load(vertexShaderSource, fragmentShaderSource, context);
    delete vertexShaderSource;
    delete fragmentShaderSource;
    if (!ret)
        return ret;
    vertexShaderSource = m_delegate->loadShaderSource(IRenderDelegate::kZPlotVertexShader, m_model, context);
    fragmentShaderSource = m_delegate->loadShaderSource(IRenderDelegate::kZPlotFragmentShader, m_model, context);
    ret = m_context->zplotProgram->load(vertexShaderSource, fragmentShaderSource, context);
    delete vertexShaderSource;
    delete fragmentShaderSource;
    if (!ret)
        return ret;
    const Array<pmx::Material *> &materials = m_model->materials();
    const int nmaterials = materials.count();
    GLuint textureID = 0;
    MaterialTextures *materialPrivates = m_context->materials = new MaterialTextures[nmaterials];
    for (int i = 0; i < nmaterials; i++) {
        const pmx::Material *material = materials[i];
        MaterialTextures &materialPrivate = materialPrivates[i];
        materialPrivate.mainTextureID = 0;
        materialPrivate.sphereTextureID = 0;
        materialPrivate.toonTextureID = 0;
        const IString *path = 0;
        path = material->mainTexture();
        if (path && m_delegate->uploadTexture(context, path, dir, &textureID, false)) {
            log0(context, IRenderDelegate::kLogInfo, "Binding the texture as a main texture (ID=%d)", textureID);
            materialPrivate.mainTextureID = textureID;
        }
        path = material->sphereTexture();
        if (path && m_delegate->uploadTexture(context, path, dir, &textureID, false)) {
            log0(context, IRenderDelegate::kLogInfo, "Binding the texture as a sphere texture (ID=%d)", textureID);
            materialPrivate.sphereTextureID = textureID;
        }
        if (material->isSharedToonTextureUsed()) {
            if (m_delegate->uploadToonTexture(context, material->toonTextureIndex(), &textureID)) {
                log0(context, IRenderDelegate::kLogInfo, "Binding the texture as a shared toon texture (ID=%d)", textureID);
                materialPrivate.toonTextureID = textureID;
            }
        }
        else {
            path = material->toonTexture();
            if (path && m_delegate->uploadTexture(context, path, dir, &textureID, true)) {
                log0(context, IRenderDelegate::kLogInfo, "Binding the texture as a static toon texture (ID=%d)", textureID);
                materialPrivate.toonTextureID = textureID;
            }
        }
    }
    glGenBuffers(kVertexBufferObjectMax, m_context->vertexBufferObjects);
    size_t size = pmx::Model::strideSize(pmx::Model::kIndexStride);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelIndices]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_model->indices().count() * size, m_model->indicesPtr(), GL_STATIC_DRAW);
    log0(context, IRenderDelegate::kLogInfo,
         "Binding indices to the vertex buffer object (ID=%d)",
         m_context->vertexBufferObjects[kModelIndices]);
    size = pmx::Model::strideSize(pmx::Model::kVertexStride);
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelVertices]);
    glBufferData(GL_ARRAY_BUFFER, m_model->vertices().count() * size, m_model->vertexPtr(), GL_DYNAMIC_DRAW);
    log0(context, IRenderDelegate::kLogInfo,
         "Binding model vertices to the vertex buffer object (ID=%d)",
         m_context->vertexBufferObjects[kModelVertices]);
    //model->setSoftwareSkinningEnable(m_scene->isSoftwareSkinningEnabled());
    Accelerator::initializeUserData(m_context);
    if (m_accelerator)
        m_accelerator->uploadModel(m_context, m_model, context);
    m_model->performUpdate();
    m_model->setVisible(true);
    update();
    log0(context, IRenderDelegate::kLogInfo, "Created the model: %s", m_model->name()->toByteArray());
    m_delegate->releaseContext(m_model, context);
    return ret;
}

void PMXRenderEngine::update()
{
    if (!m_context)
        return;
    size_t size = pmx::Model::strideSize(pmx::Model::kVertexStride);
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelVertices]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_model->vertices().count() * size, m_model->vertexPtr());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //if (m_accelerator && !m_scene->isSoftwareSkinningEnabled())
    if (m_accelerator)
        m_accelerator->updateModel(m_context, m_model);
}

void PMXRenderEngine::renderModel()
{
    if (!m_model->isVisible() || !m_context)
        return;
    ModelProgram *modelProgram = m_context->modelProgram;
    modelProgram->bind();
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelVertices]);
    size_t offset = pmx::Model::strideOffset(pmx::Model::kVertexStride);
    size_t size   = pmx::Model::strideSize(pmx::Model::kVertexStride);
    modelProgram->setPosition(reinterpret_cast<const GLvoid *>(offset), size);
    offset = pmx::Model::strideOffset(pmx::Model::kNormalStride);
    size   = pmx::Model::strideSize(pmx::Model::kNormalStride);
    modelProgram->setNormal(reinterpret_cast<const GLvoid *>(offset), size);
    offset = pmx::Model::strideOffset(pmx::Model::kTexCoordStride);
    size   = pmx::Model::strideSize(pmx::Model::kTexCoordStride);
    modelProgram->setTexCoord(reinterpret_cast<const GLvoid *>(offset), size);
    offset = pmx::Model::strideOffset(pmx::Model::kUVA1Stride);
    size   = pmx::Model::strideSize(pmx::Model::kUVA1Stride);
    modelProgram->setUVA1(reinterpret_cast<const GLvoid *>(offset), size);
    const Scene::IMatrices *matrices = m_scene->matrices();
    float matrix4x4[16];
    matrices->getModelViewProjection(matrix4x4);
    modelProgram->setModelViewProjectionMatrix(matrix4x4);
    matrices->getNormal(matrix4x4);
    modelProgram->setNormalMatrix(matrix4x4);
    m_scene->camera()->modelViewTransform().getOpenGLMatrix(matrix4x4);
    modelProgram->setModelViewInverseMatrix(matrix4x4);
    matrices->getLightViewProjection(matrix4x4);
    modelProgram->setLightViewProjectionMatrix(matrix4x4);
    const Scene::ILight *light = m_scene->light();
    void *texture = light->shadowMappingTexture();
    GLuint textureID = texture ? *static_cast<GLuint *>(texture) : 0;
    modelProgram->setLightColor(light->color());
    modelProgram->setLightDirection(light->direction());
    const Array<pmx::Material *> &materials = m_model->materials();
    const MaterialTextures *materialPrivates = m_context->materials;
    const int nmaterials = materials.count();
    offset = 0; size = pmx::Model::strideSize(pmx::Model::kIndexStride);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelIndices]);
    for (int i = 0; i < nmaterials; i++) {
        const pmx::Material *material = materials[i];
        const MaterialTextures &materialPrivate = materialPrivates[i];
        modelProgram->setMaterialAmbient(material->ambient());
        modelProgram->setMaterialDiffuse(material->diffuse());
        modelProgram->setMainTexture(materialPrivate.mainTextureID);
        modelProgram->setSphereTexture(materialPrivate.sphereTextureID, material->sphereTextureRenderMode());
        modelProgram->setToonTexture(materialPrivate.toonTextureID);
        if (texture && material->isSelfShadowDrawn())
            modelProgram->setDepthTexture(textureID);
        else
            modelProgram->setDepthTexture(0);
        if (material->isCullFaceDisabled() && m_context->cullFaceState) {
            glDisable(GL_CULL_FACE);
            m_context->cullFaceState = false;
        }
        else if (!m_context->cullFaceState) {
            glEnable(GL_CULL_FACE);
            m_context->cullFaceState = true;
        }
        const int nindices = material->indices();
        glDrawElements(GL_TRIANGLES, nindices, GL_UNSIGNED_INT, reinterpret_cast<const GLvoid *>(offset));
        offset += nindices * size;
    }
    modelProgram->unbind();
    if (!m_context->cullFaceState) {
        glEnable(GL_CULL_FACE);
        m_context->cullFaceState = true;
    }
}

void PMXRenderEngine::renderShadow()
{
    static const Vector3 plane(0.0f, 1.0f, 0.0f);
    const Scene::ILight *light = m_scene->light();
    const Vector3 &lightDirection = light->direction();
    const Scalar dot = plane.dot(lightDirection);
    float shadowMatrix[16];
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelVertices]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelIndices]);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int index = (i << 2) + j;
            shadowMatrix[index] = -plane[i] * lightDirection[j];
            if (i == j)
                shadowMatrix[index] += dot;
        }
    }
    ShadowProgram *shadowProgram = m_context->shadowProgram;
    shadowProgram->bind();
    const Scene::IMatrices *matrices = m_scene->matrices();
    float matrix4x4[16];
    matrices->getModelViewProjection(matrix4x4);
    shadowProgram->setModelViewProjectionMatrix(matrix4x4);
    shadowProgram->setShadowMatrix(shadowMatrix);
    shadowProgram->setLightColor(light->color());
    shadowProgram->setLightDirection(light->direction());
    size_t offset = pmx::Model::strideOffset(pmx::Model::kVertexStride);
    size_t size = pmx::Model::strideSize(pmx::Model::kVertexStride);
    shadowProgram->setPosition(reinterpret_cast<const GLvoid *>(offset), size);
    glCullFace(GL_FRONT);
    const Array<pmx::Material *> &materials = m_model->materials();
    const int nmaterials = materials.count();
    offset = 0; size = pmx::Model::strideSize(pmx::Model::kIndexStride);
    for (int i = 0; i < nmaterials; i++) {
        const pmx::Material *material = materials[i];
        const int nindices = material->indices();
        if (material->hasShadow())
            glDrawElements(GL_TRIANGLES, nindices, GL_UNSIGNED_INT, reinterpret_cast<const GLvoid *>(offset));
        offset += nindices * size;
    }
    glCullFace(GL_BACK);
    shadowProgram->unbind();
}

void PMXRenderEngine::renderEdge()
{
    EdgeProgram *edgeProgram = m_context->edgeProgram;
    edgeProgram->bind();
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelVertices]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelIndices]);
    size_t offset = pmx::Model::strideOffset(pmx::Model::kVertexStride);
    size_t size   = pmx::Model::strideSize(pmx::Model::kVertexStride);
    edgeProgram->setPosition(reinterpret_cast<const GLvoid *>(offset), size);
    offset = pmx::Model::strideOffset(pmx::Model::kNormalStride);
    size   = pmx::Model::strideSize(pmx::Model::kNormalStride);
    edgeProgram->setNormal(reinterpret_cast<const GLvoid *>(offset), size);
    offset = pmx::Model::strideOffset(pmx::Model::kEdgeSizeStride);
    size   = pmx::Model::strideSize(pmx::Model::kEdgeSizeStride);
    edgeProgram->setEdgeSize(reinterpret_cast<const GLvoid *>(offset), size);
    float matrix4x4[16];
    m_scene->matrices()->getModelViewProjection(matrix4x4);
    edgeProgram->setModelViewProjectionMatrix(matrix4x4);
    glCullFace(GL_FRONT);
    const Array<pmx::Material *> &materials = m_model->materials();
    const int nmaterials = materials.count();
    offset = 0; size = pmx::Model::strideSize(pmx::Model::kIndexStride);
    for (int i = 0; i < nmaterials; i++) {
        const pmx::Material *material = materials[i];
        const int nindices = material->indices();
        edgeProgram->setColor(material->edgeColor());
        edgeProgram->setSize(material->edgeSize());
        if (material->isEdgeDrawn())
            glDrawElements(GL_TRIANGLES, nindices, GL_UNSIGNED_INT, reinterpret_cast<const GLvoid *>(offset));
        offset += nindices * size;
    }
    glCullFace(GL_BACK);
    edgeProgram->unbind();
}

void PMXRenderEngine::renderZPlot()
{
    ZPlotProgram *zplotProgram = m_context->zplotProgram;
    zplotProgram->bind();
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelVertices]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelIndices]);
    size_t offset = pmx::Model::strideOffset(pmx::Model::kVertexStride);
    size_t size   = pmx::Model::strideSize(pmx::Model::kVertexStride);
    zplotProgram->setPosition(reinterpret_cast<const GLvoid *>(offset), size);
    float matrix4x4[16];
    m_scene->matrices()->getLightViewProjection(matrix4x4);
    zplotProgram->setModelViewProjectionMatrix(matrix4x4);
    glCullFace(GL_FRONT);
    const Array<pmx::Material *> &materials = m_model->materials();
    const int nmaterials = materials.count();
    offset = 0; size = pmx::Model::strideSize(pmx::Model::kIndexStride);
    for (int i = 0; i < nmaterials; i++) {
        const pmx::Material *material = materials[i];
        const int nindices = material->indices();
        if (material->isShadowMapDrawn())
            glDrawElements(GL_TRIANGLES, nindices, GL_UNSIGNED_INT, reinterpret_cast<const GLvoid *>(offset));
        offset += nindices * size;
    }
    glCullFace(GL_BACK);
    zplotProgram->unbind();
}

bool PMXRenderEngine::isAcceleratorAvailable() const
{
    return Scene::isAcceleratorSupported() && m_accelerator ? m_accelerator->isAvailable() : false;
}

bool PMXRenderEngine::initializeAccelerator()
{
    if (m_accelerator)
        return true;
    if (Scene::isAcceleratorSupported()) {
        m_accelerator = new PMXRenderEngine::Accelerator(m_delegate);
        return m_accelerator->initializeContext() && m_accelerator->createKernelPrograms();
    }
    return false;
}

void PMXRenderEngine::log0(void *context, IRenderDelegate::LogLevel level, const char *format...)
{
    va_list ap;
    va_start(ap, format);
    m_delegate->log(context, level, format, ap);
    va_end(ap);
}

} /* namespace gl2 */
} /* namespace vpvl2 */
