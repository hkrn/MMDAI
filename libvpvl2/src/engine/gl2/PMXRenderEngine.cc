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

#include "EngineCommon.h"
#include "vpvl2/internal/util.h" /* internal::snprintf */
#include "vpvl2/gl2/PMXRenderEngine.h"
#ifdef VPVL2_ENABLE_OPENCL
#include "vpvl2/cl/Context.h"
#include "vpvl2/cl/PMXAccelerator.h"
#endif

namespace {

using namespace vpvl2;
using namespace vpvl2::gl2;

enum VertexBufferObjectType
{
    kModelDynamicVertexBufferEven,
    kModelDynamicVertexBufferOdd,
    kModelStaticVertexBuffer,
    kModelIndexBuffer,
    kMaxVertexBufferObjectType
};

enum VertexArrayObjectType
{
    kVertexArrayObjectEven,
    kVertexArrayObjectOdd,
    kEdgeVertexArrayObjectEven,
    kEdgeVertexArrayObjectOdd,
    kMaxVertexArrayObjectType
};

struct MaterialTextures
{
    GLuint mainTextureID;
    GLuint sphereTextureID;
    GLuint toonTextureID;
};

class ExtendedZPlotProgram : public ZPlotProgram
{
public:
    ExtendedZPlotProgram(IRenderContext *renderContextRef)
        : ZPlotProgram(renderContextRef),
          m_boneMatricesUniformLocation(0)
    {
    }
    ~ExtendedZPlotProgram() {
        m_boneMatricesUniformLocation = 0;
    }

    void setBoneMatrices(const Scalar *value, size_t size) {
        glUniformMatrix4fv(m_boneMatricesUniformLocation, size, GL_FALSE, value);
    }

protected:
    virtual void bindAttributeLocations() {
        ZPlotProgram::bindAttributeLocations();
        glBindAttribLocation(m_program, IModel::IBuffer::kBoneIndexStride, "inBoneIndices");
        glBindAttribLocation(m_program, IModel::IBuffer::kBoneWeightStride, "inBoneWeights");
    }
    virtual void getUniformLocations() {
        ZPlotProgram::getUniformLocations();
        m_boneMatricesUniformLocation = glGetUniformLocation(m_program, "boneMatrices");
    }

private:
    GLuint m_boneMatricesUniformLocation;
};

class EdgeProgram : public BaseShaderProgram
{
public:
    EdgeProgram(IRenderContext *renderContextRef)
        : BaseShaderProgram(renderContextRef),
          m_colorUniformLocation(0),
          m_edgeSizeUniformLocation(0),
          m_opacityUniformLocation(0),
          m_boneMatricesUniformLocation(0)
    {
    }
    ~EdgeProgram() {
        m_colorUniformLocation = 0;
        m_edgeSizeUniformLocation = 0;
        m_opacityUniformLocation = 0;
        m_boneMatricesUniformLocation = 0;
    }

    void setColor(const Color &value) {
        glUniform4fv(m_colorUniformLocation, 1, value);
    }
    void setSize(const Scalar &value) {
        glUniform1f(m_edgeSizeUniformLocation, value);
    }
    void setOpacity(const Scalar &value) {
        glUniform1f(m_opacityUniformLocation, value);
    }
    void setBoneMatrices(const Scalar *value, size_t size) {
        glUniformMatrix4fv(m_boneMatricesUniformLocation, size, GL_FALSE, value);
    }

protected:
    virtual void bindAttributeLocations() {
        BaseShaderProgram::bindAttributeLocations();
        glBindAttribLocation(m_program, IModel::IBuffer::kEdgeSizeStride, "inEdgeSize");
        glBindAttribLocation(m_program, IModel::IBuffer::kBoneIndexStride, "inBoneIndices");
        glBindAttribLocation(m_program, IModel::IBuffer::kBoneWeightStride, "inBoneWeights");
    }
    virtual void getUniformLocations() {
        BaseShaderProgram::getUniformLocations();
        m_colorUniformLocation = glGetUniformLocation(m_program, "color");
        m_edgeSizeUniformLocation = glGetUniformLocation(m_program, "edgeSize");
        m_opacityUniformLocation = glGetUniformLocation(m_program, "opacity");
        m_boneMatricesUniformLocation = glGetUniformLocation(m_program, "boneMatrices");
    }

private:
    GLuint m_colorUniformLocation;
    GLuint m_edgeSizeUniformLocation;
    GLuint m_opacityUniformLocation;
    GLuint m_boneMatricesUniformLocation;
};

class ShadowProgram : public ObjectProgram
{
public:
    ShadowProgram(IRenderContext *renderContextRef)
        : ObjectProgram(renderContextRef),
          m_shadowMatrixUniformLocation(0),
          m_boneMatricesUniformLocation(0)
    {
    }
    ~ShadowProgram() {
        m_shadowMatrixUniformLocation = 0;
        m_boneMatricesUniformLocation = 0;
    }

    void setShadowMatrix(const float value[16]) {
        glUniformMatrix4fv(m_shadowMatrixUniformLocation, 1, GL_FALSE, value);
    }
    void setBoneMatrices(const Scalar *value, size_t size) {
        glUniformMatrix4fv(m_boneMatricesUniformLocation, size, GL_FALSE, value);
    }

protected:
    virtual void bindAttributeLocations() {
        glBindAttribLocation(m_program, IModel::IBuffer::kBoneIndexStride, "inBoneIndices");
        glBindAttribLocation(m_program, IModel::IBuffer::kBoneWeightStride, "inBoneWeights");
    }
    virtual void getUniformLocations() {
        ObjectProgram::getUniformLocations();
        m_shadowMatrixUniformLocation = glGetUniformLocation(m_program, "shadowMatrix");
        m_boneMatricesUniformLocation = glGetUniformLocation(m_program, "boneMatrices");
    }

private:
    GLuint m_shadowMatrixUniformLocation;
    GLuint m_boneMatricesUniformLocation;
};

class ModelProgram : public ObjectProgram
{
public:
    ModelProgram(IRenderContext *renderContextRef)
        : ObjectProgram(renderContextRef),
          m_cameraPositionUniformLocation(0),
          m_materialColorUniformLocation(0),
          m_materialSpecularUniformLocation(0),
          m_materialShininessUniformLocation(0),
          m_mainTextureBlendUniformLocation(0),
          m_sphereTextureBlendUniformLocation(0),
          m_toonTextureBlendUniformLocation(0),
          m_sphereTextureUniformLocation(0),
          m_hasSphereTextureUniformLocation(0),
          m_isSPHTextureUniformLocation(0),
          m_isSPATextureUniformLocation(0),
          m_isSubTextureUniformLocation(0),
          m_toonTextureUniformLocation(0),
          m_hasToonTextureUniformLocation(0),
          m_useToonUniformLocation(0),
          m_boneMatricesUniformLocation(0)
    {
    }
    ~ModelProgram() {
        m_cameraPositionUniformLocation = 0;
        m_materialColorUniformLocation = 0;
        m_materialSpecularUniformLocation = 0;
        m_materialShininessUniformLocation = 0;
        m_mainTextureBlendUniformLocation = 0;
        m_sphereTextureBlendUniformLocation = 0;
        m_toonTextureBlendUniformLocation = 0;
        m_sphereTextureUniformLocation = 0;
        m_hasSphereTextureUniformLocation = 0;
        m_isSPHTextureUniformLocation = 0;
        m_isSPATextureUniformLocation = 0;
        m_isSubTextureUniformLocation = 0;
        m_toonTextureUniformLocation = 0;
        m_hasToonTextureUniformLocation = 0;
        m_useToonUniformLocation = 0;
        m_boneMatricesUniformLocation = 0;
    }

    void setCameraPosition(const Vector3 &value) {
        glUniform3fv(m_cameraPositionUniformLocation, 1, value);
    }
    void setMaterialColor(const Color &value) {
        glUniform4fv(m_materialColorUniformLocation, 1, value);
    }
    void setMaterialSpecular(const Color &value) {
        glUniform3fv(m_materialSpecularUniformLocation, 1, value);
    }
    void setMaterialShininess(const Scalar &value) {
        glUniform1f(m_materialShininessUniformLocation, value);
    }
    void setMainTextureBlend(const Color &value) {
        glUniform4fv(m_mainTextureBlendUniformLocation, 1, value);
    }
    void setSphereTextureBlend(const Color &value) {
        glUniform4fv(m_sphereTextureBlendUniformLocation, 1, value);
    }
    void setToonTextureBlend(const Color &value) {
        glUniform4fv(m_toonTextureBlendUniformLocation, 1, value);
    }
    void setToonEnable(bool value) {
        glUniform1i(m_useToonUniformLocation, value ? 1 : 0);
    }
    void setSphereTexture(GLuint value, IMaterial::SphereTextureRenderMode mode) {
        if (value) {
            switch (mode) {
            case IMaterial::kNone:
            default:
                glUniform1i(m_hasSphereTextureUniformLocation, 0);
                glUniform1i(m_isSPHTextureUniformLocation, 0);
                glUniform1i(m_isSPATextureUniformLocation, 0);
                glUniform1i(m_isSubTextureUniformLocation, 0);
                break;
            case IMaterial::kMultTexture:
                enableSphereTexture(value);
                glUniform1i(m_isSPHTextureUniformLocation, 1);
                glUniform1i(m_isSPATextureUniformLocation, 0);
                glUniform1i(m_isSubTextureUniformLocation, 0);
                break;
            case IMaterial::kAddTexture:
                enableSphereTexture(value);
                glUniform1i(m_isSPHTextureUniformLocation, 0);
                glUniform1i(m_isSPATextureUniformLocation, 1);
                glUniform1i(m_isSubTextureUniformLocation, 0);
                break;
            case IMaterial::kSubTexture:
                enableSphereTexture(value);
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
    void setBoneMatrices(const Scalar *value, size_t size) {
        glUniformMatrix4fv(m_boneMatricesUniformLocation, size, GL_FALSE, value);
    }

protected:
    virtual void bindAttributeLocations() {
        ObjectProgram::bindAttributeLocations();
        glBindAttribLocation(m_program, IModel::IBuffer::kUVA0Stride, "inUVA0");
        glBindAttribLocation(m_program, IModel::IBuffer::kUVA1Stride, "inUVA1");
        glBindAttribLocation(m_program, IModel::IBuffer::kBoneIndexStride, "inBoneIndices");
        glBindAttribLocation(m_program, IModel::IBuffer::kBoneWeightStride, "inBoneWeights");
    }
    virtual void getUniformLocations() {
        ObjectProgram::getUniformLocations();
        m_cameraPositionUniformLocation = glGetUniformLocation(m_program, "cameraPosition");
        m_materialColorUniformLocation = glGetUniformLocation(m_program, "materialColor");
        m_materialSpecularUniformLocation = glGetUniformLocation(m_program, "materialSpecular");
        m_materialShininessUniformLocation = glGetUniformLocation(m_program, "materialShininess");
        m_mainTextureBlendUniformLocation = glGetUniformLocation(m_program, "mainTextureBlend");
        m_sphereTextureBlendUniformLocation = glGetUniformLocation(m_program, "sphereTextureBlend");
        m_toonTextureBlendUniformLocation = glGetUniformLocation(m_program, "toonTextureBlend");
        m_sphereTextureUniformLocation = glGetUniformLocation(m_program, "sphereTexture");
        m_hasSphereTextureUniformLocation = glGetUniformLocation(m_program, "hasSphereTexture");
        m_isSPHTextureUniformLocation = glGetUniformLocation(m_program, "isSPHTexture");
        m_isSPATextureUniformLocation = glGetUniformLocation(m_program, "isSPATexture");
        m_isSubTextureUniformLocation = glGetUniformLocation(m_program, "isSubTexture");
        m_toonTextureUniformLocation = glGetUniformLocation(m_program, "toonTexture");
        m_hasToonTextureUniformLocation = glGetUniformLocation(m_program, "hasToonTexture");
        m_useToonUniformLocation = glGetUniformLocation(m_program, "useToon");
        m_boneMatricesUniformLocation = glGetUniformLocation(m_program, "boneMatrices");
    }

private:
    void enableSphereTexture(GLuint value) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, value);
        glUniform1i(m_sphereTextureUniformLocation, 1);
        glUniform1i(m_hasSphereTextureUniformLocation, 1);
    }

    GLuint m_cameraPositionUniformLocation;
    GLuint m_materialColorUniformLocation;
    GLuint m_materialSpecularUniformLocation;
    GLuint m_materialShininessUniformLocation;
    GLuint m_mainTextureBlendUniformLocation;
    GLuint m_sphereTextureBlendUniformLocation;
    GLuint m_toonTextureBlendUniformLocation;
    GLuint m_sphereTextureUniformLocation;
    GLuint m_hasSphereTextureUniformLocation;
    GLuint m_isSPHTextureUniformLocation;
    GLuint m_isSPATextureUniformLocation;
    GLuint m_isSubTextureUniformLocation;
    GLuint m_toonTextureUniformLocation;
    GLuint m_hasToonTextureUniformLocation;
    GLuint m_useToonUniformLocation;
    GLuint m_boneMatricesUniformLocation;
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
    PrivateContext(const IModel *model, bool isVertexShaderSkinning)
        : modelRef(model),
          indexBuffer(0),
          staticBuffer(0),
          dynamicBuffer(0),
          matrixBuffer(0),
          edgeProgram(0),
          modelProgram(0),
          shadowProgram(0),
          zplotProgram(0),
          materials(0),
          cullFaceState(true),
          isVertexShaderSkinning(isVertexShaderSkinning),
          updateEven(true)
    {
        model->getIndexBuffer(indexBuffer);
        model->getStaticVertexBuffer(staticBuffer);
        model->getDynamicVertexBuffer(dynamicBuffer, indexBuffer);
        if (isVertexShaderSkinning)
            model->getMatrixBuffer(matrixBuffer, dynamicBuffer, indexBuffer);
        switch (indexBuffer->type()) {
        case IModel::IIndexBuffer::kIndex32:
            indexType = GL_UNSIGNED_INT;
            break;
        case IModel::IIndexBuffer::kIndex16:
            indexType = GL_UNSIGNED_SHORT;
            break;
        case IModel::IIndexBuffer::kIndex8:
            indexType = GL_UNSIGNED_BYTE;
            break;
        case IModel::IIndexBuffer::kMaxIndexType:
        default:
            indexType = GL_UNSIGNED_INT;
            break;
        }
#ifdef VPVL2_LINK_QT
        initializeGLFunctions();
#endif
    }
    virtual ~PrivateContext() {
        if (materials) {
            Array<IMaterial *> modelMaterials;
            modelRef->getMaterialRefs(modelMaterials);
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
        glDeleteBuffers(kMaxVertexBufferObjectType, vertexBufferObjects);
        delete indexBuffer;
        indexBuffer = 0;
        delete dynamicBuffer;
        dynamicBuffer = 0;
        delete staticBuffer;
        staticBuffer = 0;
        delete edgeProgram;
        edgeProgram = 0;
        delete modelProgram;
        modelProgram = 0;
        delete shadowProgram;
        shadowProgram = 0;
        delete zplotProgram;
        zplotProgram = 0;
        cullFaceState = false;
        isVertexShaderSkinning = false;
    }

    void getVertexBundleType(VertexArrayObjectType &vao, VertexBufferObjectType &vbo) {
        if (updateEven) {
            vao = kVertexArrayObjectOdd;
            vbo = kModelDynamicVertexBufferOdd;
        }
        else {
            vao = kVertexArrayObjectEven;
            vbo = kModelDynamicVertexBufferEven;
        }
    }
    void getEdgeBundleType(VertexArrayObjectType &vao, VertexBufferObjectType &vbo) {
        if (updateEven) {
            vao = kEdgeVertexArrayObjectOdd;
            vbo = kModelDynamicVertexBufferOdd;
        }
        else {
            vao = kEdgeVertexArrayObjectEven;
            vbo = kModelDynamicVertexBufferEven;
        }
    }

    const IModel *modelRef;
    IModel::IIndexBuffer *indexBuffer;
    IModel::IStaticVertexBuffer *staticBuffer;
    IModel::IDynamicVertexBuffer *dynamicBuffer;
    IModel::IMatrixBuffer *matrixBuffer;
    EdgeProgram *edgeProgram;
    ModelProgram *modelProgram;
    ShadowProgram *shadowProgram;
    ExtendedZPlotProgram *zplotProgram;
    GLuint vertexBufferObjects[kMaxVertexBufferObjectType];
    GLuint vertexArrayObjects[kMaxVertexArrayObjectType];
    GLenum indexType;
    MaterialTextures *materials;
#ifdef VPVL2_ENABLE_OPENCL
    cl::PMXAccelerator::Buffers buffers;
#endif
    bool cullFaceState;
    bool isVertexShaderSkinning;
    bool updateEven;
};

PMXRenderEngine::PMXRenderEngine(IRenderContext *renderContext,
                                 Scene *scene,
                                 cl::PMXAccelerator *accelerator,
                                 IModel *modelRef)
    :
      #ifdef VPVL2_LINK_QT
      QGLFunctions(),
      #endif /* VPVL2_LINK_QT */
      m_accelerator(accelerator),
      m_renderContextRef(renderContext),
      m_sceneRef(scene),
      m_modelRef(modelRef),
      m_context(0),
      m_bundle(renderContext),
      m_aabbMin(SIMD_INFINITY, SIMD_INFINITY, SIMD_INFINITY),
      m_aabbMax(-SIMD_INFINITY, -SIMD_INFINITY, -SIMD_INFINITY)
{
    bool vss = m_sceneRef->accelerationType() == Scene::kVertexShaderAccelerationType1;
    m_context = new PrivateContext(modelRef, vss);
#ifdef VPVL2_ENABLE_OPENCL
    if (vss || (m_accelerator && m_accelerator->isAvailable()))
        m_context->dynamicBuffer->setSkinningEnable(false);
#endif
#ifdef VPVL2_LINK_QT
    initializeGLFunctions();
#endif
    m_bundle.initialize();
}

PMXRenderEngine::~PMXRenderEngine()
{
#ifdef VPVL2_ENABLE_OPENCL
    if (m_context) {
        m_accelerator->release(m_context->buffers);
    }
    delete m_accelerator;
#endif
    if (m_context) {
        m_bundle.releaseVertexArrayObjects(m_context->vertexArrayObjects, kMaxVertexArrayObjectType);
        delete m_context;
        m_context = 0;
    }
    m_renderContextRef = 0;
    m_sceneRef = 0;
    m_modelRef = 0;
    m_accelerator = 0;
    m_aabbMin.setZero();
    m_aabbMax.setZero();
}

IModel *PMXRenderEngine::model() const
{
    return m_modelRef;
}

bool PMXRenderEngine::upload(const IString *dir)
{
    bool ret = true, vss = false;
    void *userData = 0;
    if (!m_context) {
        vss = m_sceneRef->accelerationType() == Scene::kVertexShaderAccelerationType1;
        m_context = new PrivateContext(m_modelRef, vss);
        m_context->dynamicBuffer->setSkinningEnable(false);
    }
    vss = m_context->isVertexShaderSkinning;
    m_renderContextRef->allocateUserData(m_modelRef, userData);
    m_renderContextRef->startProfileSession(IRenderContext::kProfileUploadModelProcess, m_modelRef);
    EdgeProgram *edgeProgram = m_context->edgeProgram = new EdgeProgram(m_renderContextRef);
    ModelProgram *modelProgram = m_context->modelProgram = new ModelProgram(m_renderContextRef);
    ShadowProgram *shadowProgram = m_context->shadowProgram = new ShadowProgram(m_renderContextRef);
    ExtendedZPlotProgram *zplotProgram = m_context->zplotProgram = new ExtendedZPlotProgram(m_renderContextRef);
    if (!createProgram(edgeProgram, dir,
                       IRenderContext::kEdgeVertexShader,
                       IRenderContext::kEdgeWithSkinningVertexShader,
                       IRenderContext::kEdgeFragmentShader,
                       userData)) {
        return releaseUserData0(userData);
    }
    if (!createProgram(modelProgram, dir,
                       IRenderContext::kModelVertexShader,
                       IRenderContext::kModelWithSkinningVertexShader,
                       IRenderContext::kModelFragmentShader,
                       userData)) {
        return releaseUserData0(userData);
    }
    if (!createProgram(shadowProgram, dir,
                       IRenderContext::kShadowVertexShader,
                       IRenderContext::kShadowWithSkinningVertexShader,
                       IRenderContext::kShadowFragmentShader,
                       userData)) {
        return releaseUserData0(userData);
    }
    if (!createProgram(zplotProgram, dir,
                       IRenderContext::kZPlotVertexShader,
                       IRenderContext::kZPlotWithSkinningVertexShader,
                       IRenderContext::kZPlotFragmentShader,
                       userData)) {
        return releaseUserData0(userData);
    }
    if (!uploadMaterials(dir, userData)) {
        return releaseUserData0(userData);
    }

    glGenBuffers(kMaxVertexBufferObjectType, m_context->vertexBufferObjects);
    GLuint dvbo0 = m_context->vertexBufferObjects[kModelDynamicVertexBufferEven];
    glBindBuffer(GL_ARRAY_BUFFER, dvbo0);
    glBufferData(GL_ARRAY_BUFFER, m_context->dynamicBuffer->size(), 0, GL_DYNAMIC_DRAW);
    log0(userData, IRenderContext::kLogInfo,
         "Binding model dynamic vertex buffer to the vertex buffer object (ID=%d)", dvbo0);

    GLuint dvbo1 = m_context->vertexBufferObjects[kModelDynamicVertexBufferOdd];
    glBindBuffer(GL_ARRAY_BUFFER, dvbo1);
    glBufferData(GL_ARRAY_BUFFER, m_context->dynamicBuffer->size(), 0, GL_DYNAMIC_DRAW);
    log0(userData, IRenderContext::kLogInfo,
         "Binding model dynamic vertex buffer to the vertex buffer object (ID=%d)", dvbo1);

    const IModel::IStaticVertexBuffer *staticBuffer = m_context->staticBuffer;
    GLuint svbo = m_context->vertexBufferObjects[kModelStaticVertexBuffer];
    glBindBuffer(GL_ARRAY_BUFFER, svbo);
    glBufferData(GL_ARRAY_BUFFER, staticBuffer->size(), 0, GL_STATIC_DRAW);
    void *address = m_bundle.mapBuffer(GL_ARRAY_BUFFER, 0, staticBuffer->size());
    staticBuffer->update(address);
    m_bundle.unmapBuffer(GL_ARRAY_BUFFER, address);
    log0(userData, IRenderContext::kLogInfo,
         "Binding model static vertex buffer to the vertex buffer object (ID=%d)", svbo);

    const IModel::IIndexBuffer *indexBuffer = m_context->indexBuffer;
    GLuint ibo = m_context->vertexBufferObjects[kModelIndexBuffer];
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBuffer->size(), indexBuffer->bytes(), GL_STATIC_DRAW);
    log0(userData, IRenderContext::kLogInfo,
         "Binding indices to the vertex buffer object (ID=%d)",
         m_context->vertexBufferObjects[kModelIndexBuffer]);

    m_bundle.allocateVertexArrayObjects(m_context->vertexArrayObjects, kMaxVertexArrayObjectType);
    GLuint vao = m_context->vertexArrayObjects[kVertexArrayObjectEven];
    if (m_bundle.bindVertexArrayObject(vao)) {
        log0(userData, IRenderContext::kLogInfo, "Binding an vertex array object for even frame (ID=%d)", vao);
    }
    createVertexBundle(dvbo0, svbo, ibo, vss);
    vao = m_context->vertexArrayObjects[kVertexArrayObjectOdd];
    if (m_bundle.bindVertexArrayObject(vao)) {
        log0(userData, IRenderContext::kLogInfo, "Binding an vertex array object for odd frame (ID=%d)", vao);
        createVertexBundle(dvbo1, svbo, ibo, vss);
    }
    vao = m_context->vertexArrayObjects[kEdgeVertexArrayObjectEven];
    if (m_bundle.bindVertexArrayObject(vao)) {
        log0(userData, IRenderContext::kLogInfo, "Binding an edge vertex array object for even frame (ID=%d)", vao);
    }
    createEdgeBundle(dvbo0, svbo, ibo, vss);
    vao = m_context->vertexArrayObjects[kEdgeVertexArrayObjectOdd];
    if (m_bundle.bindVertexArrayObject(vao)) {
        log0(userData, IRenderContext::kLogInfo, "Binding an edge vertex array object for odd frame (ID=%d)", vao);
        createEdgeBundle(dvbo1, svbo, ibo, vss);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

#ifdef VPVL2_ENABLE_OPENCL
    if (m_accelerator && m_accelerator->isAvailable()) {
        const GLuint *vbo = m_context->vertexBufferObjects;
        cl::PMXAccelerator::Buffers &buffers = m_context->buffers;
        m_accelerator->release(buffers);
        buffers.add(cl::PMXAccelerator::Buffer(vbo[kModelDynamicVertexBufferEven]));
        buffers.add(cl::PMXAccelerator::Buffer(vbo[kModelDynamicVertexBufferOdd]));
        m_accelerator->upload(buffers, m_context->indexBuffer, userData);
    }
#endif
    m_modelRef->setVisible(true);
    update(); // for updating even frame
    update(); // for updating odd frame
    log0(userData, IRenderContext::kLogInfo, "Created the model: %s",
         m_modelRef->name() ? m_modelRef->name()->toByteArray() : 0);
    m_renderContextRef->stopProfileSession(IRenderContext::kProfileUploadModelProcess, m_modelRef);
    m_renderContextRef->releaseUserData(m_modelRef, userData);
    return ret;
}

void PMXRenderEngine::update()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_context)
        return;
    m_renderContextRef->startProfileSession(IRenderContext::kProfileUpdateModelProcess, m_modelRef);
    VertexBufferObjectType vbo = m_context->updateEven
            ? kModelDynamicVertexBufferEven : kModelDynamicVertexBufferOdd;
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[vbo]);
    IModel::IDynamicVertexBuffer *dynamicBuffer = m_context->dynamicBuffer;
    void *address = m_bundle.mapBuffer(GL_ARRAY_BUFFER, 0, dynamicBuffer->size());
    m_modelRef->performUpdate();
    if (m_context->isVertexShaderSkinning) {
        m_context->matrixBuffer->update(address);
    }
    else {
        const ICamera *camera = m_sceneRef->camera();
        dynamicBuffer->update(address, camera->position(), m_aabbMin, m_aabbMax);
    }
    m_bundle.unmapBuffer(GL_ARRAY_BUFFER, address);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
#ifdef VPVL2_ENABLE_OPENCL
    if (m_accelerator && m_accelerator->isAvailable()) {
        const cl::PMXAccelerator::Buffer &buffer = m_context->buffers[m_context->updateEven ? 0 : 1];
        m_accelerator->update(dynamicBuffer, m_sceneRef, buffer, m_aabbMin, m_aabbMax);
    }
#endif
    m_modelRef->setAabb(m_aabbMin, m_aabbMax);
    m_context->updateEven = m_context->updateEven ? false :true;
    m_renderContextRef->stopProfileSession(IRenderContext::kProfileUpdateModelProcess, m_modelRef);
}

void PMXRenderEngine::renderModel()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_context)
        return;
    m_renderContextRef->startProfileSession(IRenderContext::kProfileRenderModelProcess, m_modelRef);
    ModelProgram *modelProgram = m_context->modelProgram;
    modelProgram->bind();
    float matrix4x4[16];
    m_renderContextRef->getMatrix(matrix4x4, m_modelRef,
                                  IRenderContext::kWorldMatrix
                                  | IRenderContext::kViewMatrix
                                  | IRenderContext::kProjectionMatrix
                                  | IRenderContext::kCameraMatrix);
    modelProgram->setModelViewProjectionMatrix(matrix4x4);
    m_renderContextRef->getMatrix(matrix4x4, m_modelRef,
                                  IRenderContext::kWorldMatrix
                                  | IRenderContext::kViewMatrix
                                  | IRenderContext::kCameraMatrix);
    modelProgram->setNormalMatrix(matrix4x4);
    m_renderContextRef->getMatrix(matrix4x4, m_modelRef,
                                  IRenderContext::kWorldMatrix
                                  | IRenderContext::kViewMatrix
                                  | IRenderContext::kProjectionMatrix
                                  | IRenderContext::kLightMatrix);
    modelProgram->setLightViewProjectionMatrix(matrix4x4);
    const ILight *light = m_sceneRef->light();
    void *texture = light->depthTexture();
    GLuint textureID = texture ? *static_cast<GLuint *>(texture) : 0;
    modelProgram->setLightColor(light->color());
    modelProgram->setLightDirection(light->direction());
    modelProgram->setToonEnable(light->isToonEnabled());
    modelProgram->setSoftShadowEnable(light->isSoftShadowEnabled());
    modelProgram->setDepthTextureSize(light->depthTextureSize());
    modelProgram->setCameraPosition(m_sceneRef->camera()->lookAt());
    const Scalar &opacity = m_modelRef->opacity();
    modelProgram->setOpacity(opacity);
    Array<IMaterial *> materials;
    m_modelRef->getMaterialRefs(materials);
    const MaterialTextures *materialPrivates = m_context->materials;
    const int nmaterials = materials.count();
    const bool hasModelTransparent = !btFuzzyZero(opacity - 1.0f),
            isVertexShaderSkinning = m_context->isVertexShaderSkinning;
    const Vector3 &lc = light->color();
    bool &cullFaceState = m_context->cullFaceState;
    Color diffuse, specular;
    size_t offset = 0, size = m_context->indexBuffer->strideSize();
    bindVertexBundle();
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = materials[i];
        const MaterialTextures &materialPrivate = materialPrivates[i];
        const Color &ma = material->ambient(), &md = material->diffuse(), &ms = material->specular();
        diffuse.setValue(ma.x() + md.x() * lc.x(), ma.y() + md.y() * lc.y(), ma.z() + md.z() * lc.z(), md.w());
        specular.setValue(ms.x() * lc.x(), ms.y() * lc.y(), ms.z() * lc.z(), 1.0);
        modelProgram->setMaterialColor(diffuse);
        modelProgram->setMaterialSpecular(specular);
        modelProgram->setMaterialShininess(material->shininess());
        modelProgram->setMainTextureBlend(material->mainTextureBlend());
        modelProgram->setSphereTextureBlend(material->sphereTextureBlend());
        modelProgram->setToonTextureBlend(material->toonTextureBlend());
        modelProgram->setMainTexture(materialPrivate.mainTextureID);
        modelProgram->setSphereTexture(materialPrivate.sphereTextureID, material->sphereTextureRenderMode());
        modelProgram->setToonTexture(materialPrivate.toonTextureID);
        if (texture && material->isSelfShadowDrawn())
            modelProgram->setDepthTexture(textureID);
        else
            modelProgram->setDepthTexture(0);
        if (isVertexShaderSkinning) {
            IModel::IMatrixBuffer *matrixBuffer = m_context->matrixBuffer;
            modelProgram->setBoneMatrices(matrixBuffer->bytes(i), matrixBuffer->size(i));
        }
        if (!hasModelTransparent && cullFaceState && material->isCullFaceDisabled()) {
            glDisable(GL_CULL_FACE);
            cullFaceState = false;
        }
        else if (!cullFaceState && !material->isCullFaceDisabled()) {
            glEnable(GL_CULL_FACE);
            cullFaceState = true;
        }
        const int nindices = material->sizeofIndices();
        m_renderContextRef->startProfileSession(IRenderContext::kProfileRenderModelMaterialDrawCall, material);
        glDrawElements(GL_TRIANGLES, nindices, m_context->indexType, reinterpret_cast<const GLvoid *>(offset));
        m_renderContextRef->stopProfileSession(IRenderContext::kProfileRenderModelMaterialDrawCall, material);
        offset += nindices * size;
    }
    unbindVertexBundle();
    modelProgram->unbind();
    if (!cullFaceState) {
        glEnable(GL_CULL_FACE);
        cullFaceState = true;
    }
    m_renderContextRef->stopProfileSession(IRenderContext::kProfileRenderModelProcess, m_modelRef);
}

void PMXRenderEngine::renderShadow()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_context)
        return;
    m_renderContextRef->startProfileSession(IRenderContext::kProfileRenderShadowProcess, m_modelRef);
    ShadowProgram *shadowProgram = m_context->shadowProgram;
    shadowProgram->bind();
    float matrix4x4[16];
    m_renderContextRef->getMatrix(matrix4x4, m_modelRef,
                                  IRenderContext::kWorldMatrix
                                  | IRenderContext::kViewMatrix
                                  | IRenderContext::kProjectionMatrix
                                  | IRenderContext::kShadowMatrix);
    shadowProgram->setModelViewProjectionMatrix(matrix4x4);
    const ILight *light = m_sceneRef->light();
    shadowProgram->setLightColor(light->color());
    shadowProgram->setLightDirection(light->direction());
    Array<IMaterial *> materials;
    m_modelRef->getMaterialRefs(materials);
    const int nmaterials = materials.count();
    const bool isVertexShaderSkinning = m_context->isVertexShaderSkinning;
    size_t offset = 0, size = m_context->indexBuffer->strideSize();
    bindVertexBundle();
    glDisable(GL_CULL_FACE);
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = materials[i];
        const int nindices = material->sizeofIndices();
        if (material->hasShadow()) {
            if (isVertexShaderSkinning) {
                IModel::IMatrixBuffer *matrixBuffer = m_context->matrixBuffer;
                shadowProgram->setBoneMatrices(matrixBuffer->bytes(i), matrixBuffer->size(i));
            }
            m_renderContextRef->startProfileSession(IRenderContext::kProfileRenderShadowMaterialDrawCall, material);
            glDrawElements(GL_TRIANGLES, nindices, m_context->indexType, reinterpret_cast<const GLvoid *>(offset));
            m_renderContextRef->stopProfileSession(IRenderContext::kProfileRenderShadowMaterialDrawCall, material);
        }
        offset += nindices * size;
    }
    unbindVertexBundle();
    glEnable(GL_CULL_FACE);
    shadowProgram->unbind();
    m_renderContextRef->stopProfileSession(IRenderContext::kProfileRenderShadowProcess, m_modelRef);
}

void PMXRenderEngine::renderEdge()
{
    if (!m_modelRef || !m_modelRef->isVisible() || btFuzzyZero(m_modelRef->edgeWidth()) || !m_context)
        return;
    m_renderContextRef->startProfileSession(IRenderContext::kProfileRenderEdgeProcess, m_modelRef);
    EdgeProgram *edgeProgram = m_context->edgeProgram;
    edgeProgram->bind();
    float matrix4x4[16];
    const Scalar &opacity = m_modelRef->opacity();
    m_renderContextRef->getMatrix(matrix4x4, m_modelRef,
                                  IRenderContext::kWorldMatrix
                                  | IRenderContext::kViewMatrix
                                  | IRenderContext::kProjectionMatrix
                                  | IRenderContext::kCameraMatrix);
    edgeProgram->setModelViewProjectionMatrix(matrix4x4);
    edgeProgram->setOpacity(opacity);
    Array<IMaterial *> materials;
    m_modelRef->getMaterialRefs(materials);
    const int nmaterials = materials.count();
    const bool isVertexShaderSkinning = m_context->isVertexShaderSkinning;
    Scalar edgeScaleFactor = 0;
    if (isVertexShaderSkinning) {
        const ICamera *camera = m_sceneRef->camera();
        edgeScaleFactor = m_modelRef->edgeScaleFactor(camera->position());
    }
    size_t offset = 0, size = m_context->indexBuffer->strideSize();
    bool isOpaque = btFuzzyZero(opacity - 1);
    if (isOpaque)
        glDisable(GL_BLEND);
    glCullFace(GL_FRONT);
    bindEdgeBundle();
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = materials[i];
        const int nindices = material->sizeofIndices();
        edgeProgram->setColor(material->edgeColor());
        if (material->isEdgeDrawn()) {
            if (isVertexShaderSkinning) {
                IModel::IMatrixBuffer *matrixBuffer = m_context->matrixBuffer;
                edgeProgram->setBoneMatrices(matrixBuffer->bytes(i), matrixBuffer->size(i));
                edgeProgram->setSize(material->edgeSize() * edgeScaleFactor);
            }
            m_renderContextRef->startProfileSession(IRenderContext::kProfileRenderEdgeMateiralDrawCall, material);
            glDrawElements(GL_TRIANGLES, nindices, m_context->indexType, reinterpret_cast<const GLvoid *>(offset));
            m_renderContextRef->stopProfileSession(IRenderContext::kProfileRenderEdgeMateiralDrawCall, material);
        }
        offset += nindices * size;
    }
    unbindVertexBundle();
    glCullFace(GL_BACK);
    if (isOpaque)
        glEnable(GL_BLEND);
    edgeProgram->unbind();
    m_renderContextRef->stopProfileSession(IRenderContext::kProfileRenderEdgeProcess, m_modelRef);
}

void PMXRenderEngine::renderZPlot()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_context)
        return;
    m_renderContextRef->startProfileSession(IRenderContext::kProfileRenderZPlotProcess, m_modelRef);
    ExtendedZPlotProgram *zplotProgram = m_context->zplotProgram;
    zplotProgram->bind();
    float matrix4x4[16];
    m_renderContextRef->getMatrix(matrix4x4, m_modelRef,
                                  IRenderContext::kWorldMatrix
                                  | IRenderContext::kViewMatrix
                                  | IRenderContext::kProjectionMatrix
                                  | IRenderContext::kLightMatrix);
    zplotProgram->setModelViewProjectionMatrix(matrix4x4);
    Array<IMaterial *> materials;
    m_modelRef->getMaterialRefs(materials);
    const int nmaterials = materials.count();
    const bool isVertexShaderSkinning = m_context->isVertexShaderSkinning;
    size_t offset = 0, size = m_context->indexBuffer->strideSize();
    bindVertexBundle();
    glDisable(GL_CULL_FACE);
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = materials[i];
        const int nindices = material->sizeofIndices();
        if (material->isShadowMapDrawn()) {
            if (isVertexShaderSkinning) {
                IModel::IMatrixBuffer *matrixBuffer = m_context->matrixBuffer;
                zplotProgram->setBoneMatrices(matrixBuffer->bytes(i), matrixBuffer->size(i));
            }
            m_renderContextRef->startProfileSession(IRenderContext::kProfileRenderZPlotMaterialDrawCall, material);
            glDrawElements(GL_TRIANGLES, nindices, m_context->indexType, reinterpret_cast<const GLvoid *>(offset));
            m_renderContextRef->stopProfileSession(IRenderContext::kProfileRenderZPlotMaterialDrawCall, material);
        }
        offset += nindices * size;
    }
    unbindVertexBundle();
    glEnable(GL_CULL_FACE);
    zplotProgram->unbind();
    m_renderContextRef->stopProfileSession(IRenderContext::kProfileRenderZPlotProcess, m_modelRef);
}

bool PMXRenderEngine::hasPreProcess() const
{
    return false;
}

bool PMXRenderEngine::hasPostProcess() const
{
    return false;
}

void PMXRenderEngine::preparePostProcess()
{
    /* do nothing */
}

void PMXRenderEngine::performPreProcess()
{
    /* do nothing */
}

void PMXRenderEngine::performPostProcess(IEffect * /* nextPostEffect */)
{
    /* do nothing */
}

IEffect *PMXRenderEngine::effect(IEffect::ScriptOrderType /* type */) const
{
    return 0;
}

void PMXRenderEngine::setEffect(IEffect::ScriptOrderType /* type */, IEffect * /* effect */, const IString * /* dir */)
{
    /* do nothing */
}

void PMXRenderEngine::log0(void *userData, IRenderContext::LogLevel level, const char *format...)
{
    va_list ap;
    va_start(ap, format);
    m_renderContextRef->log(userData, level, format, ap);
    va_end(ap);
}

bool PMXRenderEngine::createProgram(BaseShaderProgram *program,
                                    const IString *dir,
                                    IRenderContext::ShaderType vertexShaderType,
                                    IRenderContext::ShaderType vertexSkinningShaderType,
                                    IRenderContext::ShaderType fragmentShaderType,
                                    void *userData)
{
    IString *vertexShaderSource = 0;
    IString *fragmentShaderSource = 0;
    if (m_context->isVertexShaderSkinning)
        vertexShaderSource = m_renderContextRef->loadShaderSource(vertexSkinningShaderType, m_modelRef, dir, userData);
    else
        vertexShaderSource = m_renderContextRef->loadShaderSource(vertexShaderType, m_modelRef, dir, userData);
    fragmentShaderSource = m_renderContextRef->loadShaderSource(fragmentShaderType, m_modelRef, dir, userData);
    program->addShaderSource(vertexShaderSource, GL_VERTEX_SHADER, userData);
    program->addShaderSource(fragmentShaderSource, GL_FRAGMENT_SHADER, userData);
    bool ok = program->linkProgram(userData);
    delete vertexShaderSource;
    delete fragmentShaderSource;
    return ok;
}

bool PMXRenderEngine::uploadMaterials(const IString *dir, void *userData)
{
    Array<IMaterial *> materials;
    m_modelRef->getMaterialRefs(materials);
    const int nmaterials = materials.count();
    IRenderContext::Texture texture;
    MaterialTextures *materialPrivates = m_context->materials = new MaterialTextures[nmaterials];
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = materials[i];
        MaterialTextures &materialPrivate = materialPrivates[i];
        GLuint textureID = 0;
        texture.object = &textureID;
        materialPrivate.mainTextureID = 0;
        materialPrivate.sphereTextureID = 0;
        materialPrivate.toonTextureID = 0;
        const IString *path = 0;
        path = material->mainTexture();
        if (path) {
            if (m_renderContextRef->uploadTexture(path, dir, IRenderContext::kTexture2D, texture, userData)) {
                materialPrivate.mainTextureID = textureID = *static_cast<GLuint *>(texture.object);
                if (textureID > 0)
                    log0(userData, IRenderContext::kLogInfo, "Binding the texture as a main texture (ID=%d)", textureID);
            }
            else {
                return false;
            }
        }
        path = material->sphereTexture();
        if (path) {
            if (m_renderContextRef->uploadTexture(path, dir, IRenderContext::kTexture2D, texture, userData)) {
                materialPrivate.sphereTextureID = textureID = *static_cast<GLuint *>(texture.object);
                if (textureID > 0)
                    log0(userData, IRenderContext::kLogInfo, "Binding the texture as a sphere texture (ID=%d)", textureID);
            }
            else {
                return false;
            }
        }
        if (material->isSharedToonTextureUsed()) {
            char buf[16];
            internal::snprintf(buf, sizeof(buf), "toon%02d.bmp", material->toonTextureIndex() + 1);
            IString *s = m_renderContextRef->toUnicode(reinterpret_cast<const uint8_t *>(buf));
            bool ret = m_renderContextRef->uploadTexture(s, 0, IRenderContext::kToonTexture, texture, userData);
            delete s;
            if (ret) {
                materialPrivate.toonTextureID = textureID = *static_cast<const GLuint *>(texture.object);
                if (textureID > 0)
                    log0(userData, IRenderContext::kLogInfo, "Binding the texture as a toon texture (ID=%d)", textureID);
            }
            else {
                return false;
            }
        }
        else {
            path = material->toonTexture();
            if (path) {
                if (m_renderContextRef->uploadTexture(path, dir, IRenderContext::kToonTexture, texture, userData)) {
                    materialPrivate.toonTextureID = textureID = *static_cast<const GLuint *>(texture.object);
                    log0(userData, IRenderContext::kLogInfo, "Binding the texture as a toon texture (ID=%d)", textureID);
                }
                else {
                    return false;
                }
            }
        }
    }
    return true;
}

bool PMXRenderEngine::releaseUserData0(void *userData)
{
    if (m_context) {
        m_bundle.releaseVertexArrayObjects(m_context->vertexArrayObjects, kMaxVertexArrayObjectType);
        delete m_context;
        m_context = 0;
    }
    m_renderContextRef->releaseUserData(m_modelRef, userData);
    return false;
}

void PMXRenderEngine::createVertexBundle(GLuint dvbo, GLuint svbo, GLuint ibo, bool vss)
{
    glBindBuffer(GL_ARRAY_BUFFER, dvbo);
    bindDynamicVertexAttributePointers();
    glBindBuffer(GL_ARRAY_BUFFER, svbo);
    bindStaticVertexAttributePointers();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glEnableVertexAttribArray(IModel::IBuffer::kVertexStride);
    glEnableVertexAttribArray(IModel::IBuffer::kNormalStride);
    glEnableVertexAttribArray(IModel::IBuffer::kTextureCoordStride);
    glEnableVertexAttribArray(IModel::IBuffer::kUVA0Stride);
    glEnableVertexAttribArray(IModel::IBuffer::kUVA1Stride);
    if (vss) {
        glEnableVertexAttribArray(IModel::IBuffer::kBoneIndexStride);
        glEnableVertexAttribArray(IModel::IBuffer::kBoneWeightStride);
    }
    m_bundle.unbindVertexArrayObject();
}

void PMXRenderEngine::createEdgeBundle(GLuint dvbo, GLuint svbo, GLuint ibo, bool vss)
{
    glBindBuffer(GL_ARRAY_BUFFER, dvbo);
    bindEdgeVertexAttributePointers();
    glBindBuffer(GL_ARRAY_BUFFER, svbo);
    bindStaticVertexAttributePointers();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glEnableVertexAttribArray(IModel::IBuffer::kVertexStride);
    if (vss) {
        glEnableVertexAttribArray(IModel::IBuffer::kNormalStride);
        glEnableVertexAttribArray(IModel::IBuffer::kEdgeSizeStride);
        glEnableVertexAttribArray(IModel::IBuffer::kBoneIndexStride);
        glEnableVertexAttribArray(IModel::IBuffer::kBoneWeightStride);
    }
   m_bundle.unbindVertexArrayObject();
}

void PMXRenderEngine::bindVertexBundle()
{
    VertexArrayObjectType vao;
    VertexBufferObjectType vbo;
    m_context->getVertexBundleType(vao, vbo);
    if (!m_bundle.bindVertexArrayObject(m_context->vertexArrayObjects[vao])) {
        glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[vbo]);
        bindDynamicVertexAttributePointers();
        glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelStaticVertexBuffer]);
        bindStaticVertexAttributePointers();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelIndexBuffer]);
    }
}

void PMXRenderEngine::bindEdgeBundle()
{
    VertexArrayObjectType vao;
    VertexBufferObjectType vbo;
    m_context->getEdgeBundleType(vao, vbo);
    if (!m_bundle.bindVertexArrayObject(m_context->vertexArrayObjects[vao])) {
        glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[vbo]);
        bindEdgeVertexAttributePointers();
        glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelStaticVertexBuffer]);
        bindStaticVertexAttributePointers();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelIndexBuffer]);
    }
}

void PMXRenderEngine::unbindVertexBundle()
{
    if (!m_bundle.unbindVertexArrayObject()) {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}

void PMXRenderEngine::bindDynamicVertexAttributePointers()
{
    const IModel::IDynamicVertexBuffer *dynamicBuffer = m_context->dynamicBuffer;
    size_t offset, size;
    offset = dynamicBuffer->strideOffset(IModel::IDynamicVertexBuffer::kVertexStride);
    size   = dynamicBuffer->strideSize();
    glVertexAttribPointer(IModel::IBuffer::kVertexStride, 3, GL_FLOAT, GL_FALSE,
                          size, reinterpret_cast<const GLvoid *>(offset));
    offset = dynamicBuffer->strideOffset(IModel::IDynamicVertexBuffer::kNormalStride);
    glVertexAttribPointer(IModel::IBuffer::kNormalStride, 3, GL_FLOAT, GL_FALSE,
                          size, reinterpret_cast<const GLvoid *>(offset));
    offset = dynamicBuffer->strideOffset(IModel::IDynamicVertexBuffer::kUVA0Stride);
    glVertexAttribPointer(IModel::IBuffer::kUVA0Stride, 4, GL_FLOAT, GL_FALSE,
                          size, reinterpret_cast<const GLvoid *>(offset));
    offset = dynamicBuffer->strideOffset(IModel::IDynamicVertexBuffer::kUVA1Stride);
    glVertexAttribPointer(IModel::IBuffer::kUVA1Stride, 4, GL_FLOAT, GL_FALSE,
                          size, reinterpret_cast<const GLvoid *>(offset));
}

void PMXRenderEngine::bindEdgeVertexAttributePointers()
{
    const IModel::IDynamicVertexBuffer *dynamicBuffer = m_context->dynamicBuffer;
    size_t offset, size;
    offset = dynamicBuffer->strideOffset(IModel::IDynamicVertexBuffer::kEdgeVertexStride);
    size   = dynamicBuffer->strideSize();
    glVertexAttribPointer(IModel::IBuffer::kVertexStride, 3, GL_FLOAT, GL_FALSE,
                          size, reinterpret_cast<const GLvoid *>(offset));
    if (m_context->isVertexShaderSkinning) {
        offset = dynamicBuffer->strideOffset(IModel::IDynamicVertexBuffer::kNormalStride);
        glVertexAttribPointer(IModel::IBuffer::kNormalStride, 3, GL_FLOAT, GL_FALSE,
                              size, reinterpret_cast<const GLvoid *>(offset));
        glVertexAttribPointer(IModel::IBuffer::kEdgeSizeStride, 4, GL_FLOAT, GL_FALSE,
                              size, reinterpret_cast<const GLvoid *>(offset));
    }
}

void PMXRenderEngine::bindStaticVertexAttributePointers()
{
    const IModel::IStaticVertexBuffer *staticBuffer = m_context->staticBuffer;
    size_t offset, size;
    offset = staticBuffer->strideOffset(IModel::IStaticVertexBuffer::kTextureCoordStride);
    size   = staticBuffer->strideSize();
    glVertexAttribPointer(IModel::IBuffer::kTextureCoordStride, 2, GL_FLOAT, GL_FALSE,
                          size, reinterpret_cast<const GLvoid *>(offset));
    if (m_context->isVertexShaderSkinning) {
        offset = staticBuffer->strideOffset(IModel::IStaticVertexBuffer::kBoneIndexStride);
        glVertexAttribPointer(IModel::IBuffer::kBoneIndexStride, 4, GL_FLOAT, GL_FALSE,
                              size, reinterpret_cast<const GLvoid *>(offset));
        offset = staticBuffer->strideOffset(IModel::IStaticVertexBuffer::kBoneWeightStride);
        glVertexAttribPointer(IModel::IBuffer::kBoneWeightStride, 4, GL_FLOAT, GL_FALSE,
                              size, reinterpret_cast<const GLvoid *>(offset));
    }
}

} /* namespace gl2 */
} /* namespace vpvl2 */
