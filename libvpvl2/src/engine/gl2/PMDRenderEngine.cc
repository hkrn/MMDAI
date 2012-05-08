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
#include "vpvl2/gl2/PMDRenderEngine.h"

#include "vpvl2/IRenderDelegate.h"
#include "vpvl2/pmd/Model.h"

#include "EngineCommon.h"

namespace {

using namespace vpvl2;
using namespace vpvl2::gl2;

class EdgeProgram : public BaseShaderProgram
{
public:
    EdgeProgram(IRenderDelegate *delegate)
        : BaseShaderProgram(delegate),
          m_normalAttributeLocation(0),
          m_boneAttributesAttributeLocation(0),
          m_edgeAttributeLocation(0),
          m_boneMatricesUniformLocation(0),
          m_colorUniformLocation(0)
    {
    }
    ~EdgeProgram() {
        m_normalAttributeLocation = 0;
        m_boneAttributesAttributeLocation = 0;
        m_edgeAttributeLocation = 0;
        m_boneMatricesUniformLocation = 0;
        m_colorUniformLocation = 0;
    }

    bool load(const IString *vertexShaderSource, const IString *fragmentShaderSource, void *context) {
        bool ret = BaseShaderProgram::load(vertexShaderSource, fragmentShaderSource, context);
        if (ret) {
            m_normalAttributeLocation = glGetAttribLocation(m_program, "inNormal");
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
    void setNormal(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_normalAttributeLocation);
        glVertexAttribPointer(m_normalAttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
    }

private:
    GLuint m_normalAttributeLocation;
    GLuint m_boneAttributesAttributeLocation;
    GLuint m_edgeAttributeLocation;
    GLuint m_boneMatricesUniformLocation;
    GLuint m_colorUniformLocation;
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
          m_modelViewInverseMatrixUniformLocation(0),
          m_materialAmbientUniformLocation(0),
          m_materialDiffuseUniformLocation(0),
          m_hasSubTextureUniformLocation(0),
          m_isMainSphereMapUniformLocation(0),
          m_isSubSphereMapUniformLocation(0),
          m_isMainAdditiveUniformLocation(0),
          m_isSubAdditiveUniformLocation(0),
          m_subTextureUniformLocation(0),
          m_toonTextureUniformLocation(0),
          m_useToonUniformLocation(0)
    {
    }
    ~ModelProgram() {
        m_toonTexCoordAttributeLocation = 0;
        m_modelViewInverseMatrixUniformLocation = 0;
        m_materialAmbientUniformLocation = 0;
        m_materialDiffuseUniformLocation = 0;
        m_hasSubTextureUniformLocation = 0;
        m_isMainSphereMapUniformLocation = 0;
        m_isSubSphereMapUniformLocation = 0;
        m_isMainAdditiveUniformLocation = 0;
        m_isSubAdditiveUniformLocation = 0;
        m_subTextureUniformLocation = 0;
        m_toonTextureUniformLocation = 0;
        m_useToonUniformLocation = 0;
    }

    bool load(const IString *vertexShaderSource, const IString *fragmentShaderSource, void *context) {
        bool ret = ObjectProgram::load(vertexShaderSource, fragmentShaderSource, context);
        if (ret) {
            m_toonTexCoordAttributeLocation = glGetAttribLocation(m_program, "inToonTexCoord");
            m_modelViewInverseMatrixUniformLocation = glGetUniformLocation(m_program, "modelViewInverseMatrix");
            m_materialAmbientUniformLocation = glGetUniformLocation(m_program, "materialAmbient");
            m_materialDiffuseUniformLocation = glGetUniformLocation(m_program, "materialDiffuse");
            m_hasSubTextureUniformLocation = glGetUniformLocation(m_program, "hasSubTexture");
            m_isMainSphereMapUniformLocation = glGetUniformLocation(m_program, "isMainSphereMap");
            m_isSubSphereMapUniformLocation = glGetUniformLocation(m_program, "isSubSphereMap");
            m_isMainAdditiveUniformLocation = glGetUniformLocation(m_program, "isMainAdditive");
            m_isSubAdditiveUniformLocation = glGetUniformLocation(m_program, "isSubAdditive");
            m_subTextureUniformLocation = glGetUniformLocation(m_program, "subTexture");
            m_toonTextureUniformLocation = glGetUniformLocation(m_program, "toonTexture");
            m_useToonUniformLocation = glGetUniformLocation(m_program, "useToon");
        }
        return ret;
    }
    void setToonTexCoord(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_toonTexCoordAttributeLocation);
        glVertexAttribPointer(m_toonTexCoordAttributeLocation, 2, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setModelViewInverseMatrix(const GLfloat value[16]) {
        glUniformMatrix4fv(m_modelViewInverseMatrixUniformLocation, 1, GL_FALSE, value);
    }
    void setMaterialAmbient(const Color &value) {
        glUniform3fv(m_materialAmbientUniformLocation, 1, value);
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
    void setToonEnable(bool value) {
        glUniform1i(m_useToonUniformLocation, value ? 1 : 0);
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
    GLuint m_toonTexCoordAttributeLocation;
    GLuint m_modelViewInverseMatrixUniformLocation;
    GLuint m_materialAmbientUniformLocation;
    GLuint m_materialDiffuseUniformLocation;
    GLuint m_hasSubTextureUniformLocation;
    GLuint m_isMainSphereMapUniformLocation;
    GLuint m_isSubSphereMapUniformLocation;
    GLuint m_isMainAdditiveUniformLocation;
    GLuint m_isSubAdditiveUniformLocation;
    GLuint m_subTextureUniformLocation;
    GLuint m_toonTextureUniformLocation;
    GLuint m_useToonUniformLocation;
};

enum VertexBufferObjectType
{
    kModelVertices,
    kEdgeIndices,
    kModelIndices,
    kVertexBufferObjectMax
};

struct PMDModelMaterialPrivate
{
    GLuint mainTextureID;
    GLuint subTextureID;
};

}

namespace vpvl2
{
namespace gl2
{

using namespace vpvl;

static const float kIdentityMatrix3x3[] = {
    1, 0, 0,
    0, 1, 0,
    0, 0, 1
};
static const float kIdentityMatrix4x4[] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
};

class PMDRenderEngine::PrivateContext
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
          hasSingleSphereMap(false),
          hasMultipleSphereMap(false),
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
        for (int i = 0; i < PMDModel::kCustomTextureMax; i++) {
            glDeleteTextures(1, &toonTextureID[i]);
        }
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

    void releaseMaterials(vpvl::PMDModel *model) {
        if (materials) {
            const MaterialList &modelMaterials = model->materials();
            const int nmaterials = modelMaterials.count();
            for (int i = 0; i < nmaterials; i++) {
                PMDModelMaterialPrivate &materialPrivate = materials[i];
                glDeleteTextures(1, &materialPrivate.mainTextureID);
                glDeleteTextures(1, &materialPrivate.subTextureID);
            }
            delete[] materials;
            materials = 0;
        }
    }

    EdgeProgram *edgeProgram;
    ModelProgram *modelProgram;
    ShadowProgram *shadowProgram;
    ZPlotProgram *zplotProgram;
    GLuint toonTextureID[PMDModel::kCustomTextureMax];
    GLuint vertexBufferObjects[kVertexBufferObjectMax];
    bool hasSingleSphereMap;
    bool hasMultipleSphereMap;
    PMDModelMaterialPrivate *materials;
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
#endif
    bool cullFaceState;
};

#ifdef VPVL2_ENABLE_OPENCL
class PMDRenderEngine::Accelerator : public BaseAccelerator
{
public:
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
        const IString *source = m_delegate->loadKernelSource(IRenderDelegate::kModelSkinningKernel, 0);
        const char *sourceText = reinterpret_cast<const char *>(source->toByteArray());
        const size_t sourceSize = source->length();
        clReleaseProgram(m_program);
        m_program = clCreateProgramWithSource(m_context, 1, &sourceText, &sourceSize, &err);
        delete source;
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
    void uploadModel(PMDRenderEngine::PrivateContext *userData, const PMDModel *model, void *context) {
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
    void updateModel(PMDRenderEngine::PrivateContext *userData, PMDModel *model) {
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
        err = clEnqueueWriteBuffer(m_queue, userData->outputMatricesBuffer, CL_TRUE, 0, nsize, userData->boneTransform, 0, 0, 0);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed enqueue a command to write output matrices buffer: %d", err);
            return;
        }
#endif
        /* force flushing OpenGL commands to acquire GL objects by OpenCL */
        glFinish();
        clEnqueueAcquireGLObjects(m_queue, 1, &userData->vertexBufferForCL, 0, 0, 0);
        int nvertices = model->vertices().count();
        err = clEnqueueWriteBuffer(m_queue, userData->bone1IndicesBuffer, CL_TRUE, 0, nvertices * sizeof(int), userData->bone1Indices, 0, 0, 0);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed enqueue a command to write bone1 indices buffer: %d", err);
            return;
        }
        err = clEnqueueWriteBuffer(m_queue, userData->bone2IndicesBuffer, CL_TRUE, 0, nvertices * sizeof(int), userData->bone2Indices, 0, 0, 0);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed enqueue a command to write bone2 indices buffer: %d", err);
            return;
        }
        err = clEnqueueWriteBuffer(m_queue, userData->weightsBuffer, CL_TRUE, 0, nvertices * sizeof(float), userData->weights, 0, 0, 0);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed enqueue a command to write weights buffer: %d", err);
            return;
        }
        err = clSetKernelArg(m_performSkinningKernel, 0, sizeof(userData->outputMatricesBuffer), &userData->outputMatricesBuffer);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 1st argument of kernel (skinningMatrices): %d", err);
            return;
        }
        err = clSetKernelArg(m_performSkinningKernel, 1, sizeof(userData->weightsBuffer), &userData->weightsBuffer);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 2nd argument of kernel (weights): %d", err);
            return;
        }
        err = clSetKernelArg(m_performSkinningKernel, 2, sizeof(userData->bone1IndicesBuffer), &userData->bone1IndicesBuffer);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 3rd argument of kernel (bone1Indices): %d", err);
            return;
        }
        err = clSetKernelArg(m_performSkinningKernel, 3, sizeof(userData->bone2IndicesBuffer), &userData->bone2IndicesBuffer);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 4th argument of kernel (bone2Indices): %d", err);
            return;
        }
        const Vector3 &lightDirection = -model->lightPosition();
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
        size_t strideSize = model->strideSize(PMDModel::kVerticesStride) >> 4;
        err = clSetKernelArg(m_performSkinningKernel, 6, sizeof(strideSize), &strideSize);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 7th argument of kernel (strideSize): %d", err);
            return;
        }
        size_t offsetPosition = model->strideOffset(PMDModel::kVerticesStride) >> 4;
        err = clSetKernelArg(m_performSkinningKernel, 7, sizeof(offsetPosition), &offsetPosition);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 8th argument of kernel (offsetPosition): %d", err);
            return;
        }
        size_t offsetNormal = model->strideOffset(PMDModel::kNormalsStride) >> 4;
        err = clSetKernelArg(m_performSkinningKernel, 8, sizeof(offsetNormal), &offsetNormal);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 9th argument of kernel (offsetNormal): %d", err);
            return;
        }
        size_t offsetToonTexture = model->strideOffset(PMDModel::kToonTextureStride) >> 4;
        err = clSetKernelArg(m_performSkinningKernel, 9, sizeof(offsetToonTexture), &offsetToonTexture);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 10th argument of kernel (offsetTexCoord): %d", err);
            return;
        }
        size_t offsetEdge = model->strideOffset(PMDModel::kEdgeVerticesStride) >> 4;
        err = clSetKernelArg(m_performSkinningKernel, 10, sizeof(offsetEdge), &offsetEdge);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 11th argument of kernel (offsetEdge): %d", err);
            return;
        }
        err = clSetKernelArg(m_performSkinningKernel, 11, sizeof(userData->vertexBufferForCL), &userData->vertexBufferForCL);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed setting 12th argument of kernel (vertices): %d", err);
            return;
        }
        local = userData->localWGSizeForUpdateBoneMatrices;
        global = local * ((nvertices + (local - 1)) / local);
        err = clEnqueueNDRangeKernel(m_queue, m_performSkinningKernel, 1, 0, &global, &local, 0, 0, 0);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed enqueue executing kernel: %d", err);
            return;
        }
        clEnqueueReleaseGLObjects(m_queue, 1, &userData->vertexBufferForCL, 0, 0, 0);
        clFinish(m_queue);
    }
    void log0(void *context, IRenderDelegate::LogLevel level, const char *format...) {
        va_list ap;
        va_start(ap, format);
        m_delegate->log(context, level, format, ap);
        va_end(ap);
    }

private:
    cl_kernel m_updateBoneMatricesKernel;
    cl_kernel m_performSkinningKernel;
};
#else
class PMDRenderEngine::Accelerator : public BaseAccelerator {
public:
    static void initializeUserData(PMDRenderEngine::PrivateContext * /* userData */) {}

    Accelerator(IRenderDelegate *delegate) : BaseAccelerator(delegate) {}
    ~Accelerator() {}

    void uploadModel(PrivateContext * /* userData */, const PMDModel * /* model */, void * /* context */) {
    }
    void deleteModel(PrivateContext * /* userData */) {
    }
    void updateModel(PrivateContext * /* userData */, PMDModel * /* model */) {
    }
};
#endif

PMDRenderEngine::PMDRenderEngine(IRenderDelegate *delegate, const Scene *scene, pmd::Model *model)
#ifdef VPVL_LINK_QT
    : QGLFunctions(),
      #else
    :
      #endif /* VPVL_LINK_QT */
      m_delegate(delegate),
      m_scene(scene),
      m_model(model),
      m_context(0),
      m_accelerator(0)
{
    m_context = new PrivateContext();
}

PMDRenderEngine::~PMDRenderEngine()
{
    if (m_context) {
        m_context->releaseMaterials(m_model->ptr());
        delete m_context;
        m_context = 0;
    }
    delete m_accelerator;
    m_accelerator = 0;
    m_model = 0;
    m_delegate = 0;
    m_scene = 0;
}

IModel *PMDRenderEngine::model() const
{
    return m_model;
}

bool PMDRenderEngine::upload(const IString *dir)
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
    PMDModel *model = m_model->ptr();
    const MaterialList &materials = model->materials();
    const int nmaterials = materials.count();
    GLuint textureID = 0;
    PMDModelMaterialPrivate *materialPrivates = m_context->materials = new PMDModelMaterialPrivate[nmaterials];
    bool hasSingleSphere = false, hasMultipleSphere = false;
    for (int i = 0; i < nmaterials; i++) {
        const Material *material = materials[i];
        IString *primary = m_delegate->toUnicode(material->mainTextureName());
        IString *second = m_delegate->toUnicode(material->subTextureName());
        PMDModelMaterialPrivate &materialPrivate = materialPrivates[i];
        materialPrivate.mainTextureID = 0;
        materialPrivate.subTextureID = 0;
        if (m_delegate->uploadTexture(context, primary, dir, &textureID, false)) {
            materialPrivate.mainTextureID = textureID;
            log0(context, IRenderDelegate::kLogInfo, "Binding the texture as a primary texture (ID=%d)", textureID);
        }
        if (m_delegate->uploadTexture(context, second, dir, &textureID, false)) {
            materialPrivate.subTextureID = textureID;
            log0(context, IRenderDelegate::kLogInfo, "Binding the texture as a secondary texture (ID=%d)", textureID);
        }
        delete primary;
        delete second;
        hasSingleSphere |= material->isMainSphereModulate() && !material->isSubSphereAdd();
        hasMultipleSphere |= material->isSubSphereAdd();
    }
    m_context->hasSingleSphereMap = hasSingleSphere;
    m_context->hasMultipleSphereMap = hasMultipleSphere;
    log0(context, IRenderDelegate::kLogInfo,
         "Sphere map information: hasSingleSphere=%s, hasMultipleSphere=%s",
         hasSingleSphere ? "true" : "false",
         hasMultipleSphere ? "true" : "false");
    glGenBuffers(kVertexBufferObjectMax, m_context->vertexBufferObjects);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_context->vertexBufferObjects[kEdgeIndices]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->edgeIndicesCount() * model->strideSize(PMDModel::kEdgeIndicesStride),
                 model->edgeIndicesPointer(), GL_STATIC_DRAW);
    log0(context, IRenderDelegate::kLogInfo,
         "Binding edge indices to the vertex buffer object (ID=%d)",
         m_context->vertexBufferObjects[kEdgeIndices]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelIndices]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->indices().count() * model->strideSize(PMDModel::kIndicesStride),
                 model->indicesPointer(), GL_STATIC_DRAW);
    log0(context, IRenderDelegate::kLogInfo,
         "Binding indices to the vertex buffer object (ID=%d)",
         m_context->vertexBufferObjects[kModelIndices]);
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelVertices]);
    glBufferData(GL_ARRAY_BUFFER, model->vertices().count() * model->strideSize(PMDModel::kVerticesStride),
                 model->verticesPointer(), GL_DYNAMIC_DRAW);
    log0(context, IRenderDelegate::kLogInfo,
         "Binding model vertices to the vertex buffer object (ID=%d)",
         m_context->vertexBufferObjects[kModelVertices]);
    if (m_delegate->uploadToonTexture(context, "toon0.bmp", dir, &textureID)) {
        m_context->toonTextureID[0] = textureID;
        log0(context, IRenderDelegate::kLogInfo, "Binding the texture as a toon texture (ID=%d)", textureID);
    }
    for (int i = 0; i < PMDModel::kCustomTextureMax; i++) {
        const uint8_t *name = model->toonTexture(i);
        if (m_delegate->uploadToonTexture(context, reinterpret_cast<const char *>(name), dir, &textureID)) {
            m_context->toonTextureID[i + 1] = textureID;
            log0(context, IRenderDelegate::kLogInfo, "Binding the texture as a toon texture (ID=%d)", textureID);
        }
    }
    //model->setSoftwareSkinningEnable(m_scene->isSoftwareSkinningEnabled());
    if (m_accelerator)
        m_accelerator->uploadModel(m_context, model, context);
    model->updateImmediate();
    update();
    IString *modelName = m_delegate->toUnicode(model->name());
    log0(context, IRenderDelegate::kLogInfo, "Created the model: %s", modelName->toByteArray());
    delete modelName;
    m_delegate->releaseContext(m_model, context);
    return ret;
}

void PMDRenderEngine::update()
{
    if (!m_model->isVisible())
        return;
    PMDModel *model = m_model->ptr();
    model->setLightPosition(-m_scene->light()->direction());
    int nvertices = model->vertices().count();
    size_t strideSize = model->strideSize(PMDModel::kVerticesStride);
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelVertices]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, nvertices * strideSize, model->verticesPointer());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    if (m_accelerator)
        m_accelerator->updateModel(m_context, model);
}

void PMDRenderEngine::renderModel()
{
    if (!m_model->isVisible())
        return;
    PMDModel *model = m_model->ptr();
    ModelProgram *modelProgram = m_context->modelProgram;
    modelProgram->bind();
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelVertices]);
    modelProgram->setPosition(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kVerticesStride)),
                              model->strideSize(PMDModel::kVerticesStride));
    modelProgram->setNormal(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kNormalsStride)),
                            model->strideSize(PMDModel::kNormalsStride));
    modelProgram->setTexCoord(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kTextureCoordsStride)),
                              model->strideSize(PMDModel::kTextureCoordsStride));
    const Scene::IMatrices *matrices = m_scene->matrices();
    float matrix4x4[16];
    matrices->getModelViewProjection(matrix4x4);
    modelProgram->setModelViewProjectionMatrix(matrix4x4);
    matrices->getNormal(matrix4x4);
    modelProgram->setNormalMatrix(matrix4x4);
    m_scene->camera()->modelViewTransform().inverse().getOpenGLMatrix(matrix4x4);
    modelProgram->setModelViewInverseMatrix(matrix4x4);
    matrices->getLightViewProjection(matrix4x4);
    modelProgram->setLightViewProjectionMatrix(matrix4x4);
    const Scene::ILight *light = m_scene->light();
    void *texture = light->shadowMappingTexture();
    GLuint textureID = texture ? *static_cast<GLuint *>(texture) : 0;
    modelProgram->setLightColor(light->color());
    modelProgram->setLightDirection(light->direction());
    modelProgram->setToonEnable(light->isToonEnabled());
    if (model->isToonEnabled() && (model->isSoftwareSkinningEnabled() || (m_accelerator && m_accelerator->isAvailable()))) {
        modelProgram->setToonTexCoord(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kToonTextureStride)),
                                      model->strideSize(PMDModel::kToonTextureStride));
    }

    const MaterialList &materials = model->materials();
    const PMDModelMaterialPrivate *materialPrivates = m_context->materials;
    const int nmaterials = materials.count();
    //const bool hasSingleSphereMap = m_context->hasSingleSphereMap;
    const bool hasMultipleSphereMap = m_context->hasMultipleSphereMap;
    Color ambient, diffuse;
    size_t offset = 0, size = model->strideSize(vpvl::PMDModel::kIndicesStride);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelIndices]);
    for (int i = 0; i < nmaterials; i++) {
        const Material *material = materials[i];
        const PMDModelMaterialPrivate &materialPrivate = materialPrivates[i];
        const float opacity = material->opacity();
        const bool isMainSphereAdd = material->isMainSphereAdd();
        ambient = material->ambient();
        ambient.setW(ambient.w() * opacity);
        diffuse = material->diffuse();
        diffuse.setW(diffuse.w() * opacity);
        modelProgram->setMaterialAmbient(ambient);
        modelProgram->setMaterialDiffuse(diffuse);
        modelProgram->setMainTexture(materialPrivate.mainTextureID);
        modelProgram->setToonTexture(m_context->toonTextureID[material->toonID()]);
        modelProgram->setIsMainSphereMap(isMainSphereAdd || material->isMainSphereModulate());
        modelProgram->setIsMainAdditive(isMainSphereAdd);
        if (hasMultipleSphereMap) {
            const bool isSubSphereAdd = material->isSubSphereAdd();
            modelProgram->setIsSubSphereMap(isSubSphereAdd || material->isSubSphereModulate());
            modelProgram->setIsSubAdditive(isSubSphereAdd);
            modelProgram->setSubTexture(materialPrivate.subTextureID);
        }
        if (texture && !btFuzzyZero(opacity - 0.98)) {
            modelProgram->setDepthTexture(textureID);
        }
        else {
            modelProgram->setDepthTexture(0);
        }
        if (!btFuzzyZero(opacity - 1.0f) && m_context->cullFaceState) {
            glDisable(GL_CULL_FACE);
            m_context->cullFaceState = false;
        }
        else if (!m_context->cullFaceState) {
            glEnable(GL_CULL_FACE);
            m_context->cullFaceState = true;
        }
        const int nindices = material->countIndices();
        glDrawElements(GL_TRIANGLES, nindices, GL_UNSIGNED_SHORT, reinterpret_cast<const GLvoid *>(offset));
        offset += nindices * size;
    }
    modelProgram->unbind();
    if (!m_context->cullFaceState) {
        glEnable(GL_CULL_FACE);
        m_context->cullFaceState = true;
    }
}

void PMDRenderEngine::renderShadow()
{
    if (!m_model->isVisible())
        return;
    static const Vector3 plane(0.0f, 1.0f, 0.0f);
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelVertices]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelIndices]);
    const Scene::ILight *light = m_scene->light();
    const Vector3 &direction = light->direction();
    const Scalar dot = plane.dot(-direction);
    float shadowMatrix[16];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int index = (i << 2) + j;
            shadowMatrix[index] = plane[i] * direction[j];
            if (i == j)
                shadowMatrix[index] += dot;
        }
    }
    ShadowProgram *shadowProgram = m_context->shadowProgram;
    PMDModel *model = m_model->ptr();
    shadowProgram->bind();
    const Scene::IMatrices *matrices = m_scene->matrices();
    float matrix4x4[16];
    matrices->getModelViewProjection(matrix4x4);
    shadowProgram->setModelViewProjectionMatrix(matrix4x4);
    shadowProgram->setShadowMatrix(shadowMatrix);
    shadowProgram->setLightColor(light->color());
    shadowProgram->setLightDirection(direction);
    shadowProgram->setPosition(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kVerticesStride)),
                               model->strideSize(PMDModel::kVerticesStride));
    glCullFace(GL_FRONT);
    glDrawElements(GL_TRIANGLES, model->indices().count(), GL_UNSIGNED_SHORT, 0);
    glCullFace(GL_BACK);
    shadowProgram->unbind();
}

void PMDRenderEngine::renderZPlot()
{
    if (!m_model->isVisible())
        return;
    ZPlotProgram *zplotProgram = m_context->zplotProgram;
    PMDModel *model = m_model->ptr();
    float matrix4x4[16];
    zplotProgram->bind();
    m_scene->matrices()->getLightViewProjection(matrix4x4);
    zplotProgram->setModelViewProjectionMatrix(matrix4x4);
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelVertices]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelIndices]);
    zplotProgram->setPosition(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kVerticesStride)),
                              model->strideSize(PMDModel::kVerticesStride));
    glCullFace(GL_FRONT);
    const vpvl::MaterialList &materials = model->materials();
    const int nmaterials = materials.count();
    size_t offset = 0, size = model->strideSize(vpvl::PMDModel::kIndicesStride);
    for (int i = 0; i < nmaterials; i++) {
        const vpvl::Material *material = materials[i];
        const int nindices = material->countIndices();
        if (!btFuzzyZero(material->opacity() - 0.98))
            glDrawElements(GL_TRIANGLES, nindices, GL_UNSIGNED_SHORT, reinterpret_cast<const GLvoid *>(offset));
        offset += nindices * size;
    }
    glCullFace(GL_BACK);
    zplotProgram->unbind();
}

void PMDRenderEngine::renderEdge()
{
    if (!m_model->isVisible())
        return;
    EdgeProgram *edgeProgram = m_context->edgeProgram;
    PMDModel *model = m_model->ptr();
    edgeProgram->bind();
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelVertices]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_context->vertexBufferObjects[kEdgeIndices]);
    edgeProgram->setColor(model->edgeColor());
    float matrix4x4[16];
    m_scene->matrices()->getModelViewProjection(matrix4x4);
    edgeProgram->setModelViewProjectionMatrix(matrix4x4);
    if (!model->isSoftwareSkinningEnabled() && !(m_accelerator && m_accelerator->isAvailable())) {
        edgeProgram->setPosition(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kVerticesStride)),
                                 model->strideSize(PMDModel::kVerticesStride));
        edgeProgram->setNormal(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kNormalsStride)),
                               model->strideSize(PMDModel::kNormalsStride));
        edgeProgram->setBoneAttributes(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kBoneAttributesStride)),
                                       model->strideSize(PMDModel::kBoneAttributesStride));
        edgeProgram->setEdge(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kEdgeVerticesStride)),
                             model->strideSize(PMDModel::kEdgeVerticesStride));
        // XXX: boneMatricesPointer is removed, we must implement updateBoneMatrices alternative.
        //m_edgeProgram->setBoneMatrices(model->boneMatricesPointer(), model->bones().count());
    }
    else {
        edgeProgram->setPosition(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kEdgeVerticesStride)),
                                 model->strideSize(PMDModel::kEdgeVerticesStride));
    }
    glCullFace(GL_FRONT);
    glDrawElements(GL_TRIANGLES, model->edgeIndicesCount(), GL_UNSIGNED_SHORT, 0);
    glCullFace(GL_BACK);
    edgeProgram->unbind();
}

bool PMDRenderEngine::isAcceleratorAvailable() const
{
    return Scene::isAcceleratorSupported() && m_accelerator ? m_accelerator->isAvailable() : false;
}

bool PMDRenderEngine::initializeAccelerator()
{
    if (m_accelerator)
        return true;
    if (Scene::isAcceleratorSupported()) {
        m_accelerator = new Accelerator(m_delegate);
        return m_accelerator->initializeContext() && m_accelerator->createKernelPrograms();
    }
    return false;
}

void PMDRenderEngine::log0(void *context, IRenderDelegate::LogLevel level, const char *format...)
{
    va_list ap;
    va_start(ap, format);
    m_delegate->log(context, level, format, ap);
    va_end(ap);
}

} /* namespace gl2 */
} /* namespace vpvl2 */
