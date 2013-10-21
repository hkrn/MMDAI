/**

 Copyright (c) 2009-2011  Nagoya Institute of Technology
                          Department of Computer Science
               2010-2013  hkrn

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

#include "EngineCommon.h"
#include "vpvl2/extensions/gl/VertexBundle.h"
#include "vpvl2/extensions/gl/VertexBundleLayout.h"
#include "vpvl2/internal/util.h" /* internal::snprintf */
#include "vpvl2/gl2/PMXRenderEngine.h"
#include "vpvl2/cl/PMXAccelerator.h"

using namespace vpvl2;
using namespace vpvl2::gl2;
using namespace vpvl2::extensions::gl;

namespace {

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

struct MaterialTextureRefs
{
    MaterialTextureRefs()
        : mainTextureRef(0),
          sphereTextureRef(0),
          toonTextureRef(0)
    {
    }
    ~MaterialTextureRefs() {
        mainTextureRef = 0;
        sphereTextureRef = 0;
        toonTextureRef = 0;
    }
    ITexture *mainTextureRef;
    ITexture *sphereTextureRef;
    ITexture *toonTextureRef;
};

class ExtendedZPlotProgram : public ZPlotProgram
{
public:
    ExtendedZPlotProgram(const IApplicationContext::FunctionResolver *resolver)
        : ZPlotProgram(resolver),
          m_boneMatricesUniformLocation(-1)
    {
    }
    ~ExtendedZPlotProgram() {
        m_boneMatricesUniformLocation = -1;
    }

    void setBoneMatrices(const Scalar *value, vsize size) {
        uniformMatrix4fv(m_boneMatricesUniformLocation, size, kGL_FALSE, value);
    }

protected:
    virtual void bindAttributeLocations() {
        ZPlotProgram::bindAttributeLocations();
        bindAttribLocation(m_program, IModel::Buffer::kBoneIndexStride, "inBoneIndices");
        bindAttribLocation(m_program, IModel::Buffer::kBoneWeightStride, "inBoneWeights");
    }
    virtual void getUniformLocations() {
        ZPlotProgram::getUniformLocations();
        m_boneMatricesUniformLocation = getUniformLocation(m_program, "boneMatrices");
    }

private:
    GLint m_boneMatricesUniformLocation;
};

class EdgeProgram : public BaseShaderProgram
{
public:
    EdgeProgram(const IApplicationContext::FunctionResolver *resolver)
        : BaseShaderProgram(resolver),
          m_colorUniformLocation(-1),
          m_edgeSizeUniformLocation(-1),
          m_opacityUniformLocation(-1),
          m_boneMatricesUniformLocation(-1)
    {
    }
    ~EdgeProgram() {
        m_colorUniformLocation = -1;
        m_edgeSizeUniformLocation = -1;
        m_opacityUniformLocation = -1;
        m_boneMatricesUniformLocation = -1;
    }

    void setColor(const Color &value) {
        uniform4fv(m_colorUniformLocation, 1, value);
    }
    void setSize(const Scalar &value) {
        uniform1f(m_edgeSizeUniformLocation, value);
    }
    void setOpacity(const Scalar &value) {
        uniform1f(m_opacityUniformLocation, value);
    }
    void setBoneMatrices(const Scalar *value, vsize size) {
        uniformMatrix4fv(m_boneMatricesUniformLocation, size, kGL_FALSE, value);
    }

protected:
    virtual void bindAttributeLocations() {
        BaseShaderProgram::bindAttributeLocations();
        bindAttribLocation(m_program, IModel::Buffer::kNormalStride, "inNormal");
        bindAttribLocation(m_program, IModel::Buffer::kBoneIndexStride, "inBoneIndices");
        bindAttribLocation(m_program, IModel::Buffer::kBoneWeightStride, "inBoneWeights");
    }
    virtual void getUniformLocations() {
        BaseShaderProgram::getUniformLocations();
        m_colorUniformLocation = getUniformLocation(m_program, "color");
        m_edgeSizeUniformLocation = getUniformLocation(m_program, "edgeSize");
        m_opacityUniformLocation = getUniformLocation(m_program, "opacity");
        m_boneMatricesUniformLocation = getUniformLocation(m_program, "boneMatrices");
    }

private:
    GLint m_colorUniformLocation;
    GLint m_edgeSizeUniformLocation;
    GLint m_opacityUniformLocation;
    GLint m_boneMatricesUniformLocation;
};

class ShadowProgram : public ObjectProgram
{
public:
    ShadowProgram(const IApplicationContext::FunctionResolver *resolver)
        : ObjectProgram(resolver),
          m_shadowMatrixUniformLocation(-1),
          m_boneMatricesUniformLocation(-1)
    {
    }
    ~ShadowProgram() {
        m_shadowMatrixUniformLocation = -1;
        m_boneMatricesUniformLocation = -1;
    }

    void setShadowMatrix(const float value[16]) {
        uniformMatrix4fv(m_shadowMatrixUniformLocation, 1, kGL_FALSE, value);
    }
    void setBoneMatrices(const Scalar *value, vsize size) {
        uniformMatrix4fv(m_boneMatricesUniformLocation, size, kGL_FALSE, value);
    }

protected:
    virtual void bindAttributeLocations() {
        bindAttribLocation(m_program, IModel::Buffer::kBoneIndexStride, "inBoneIndices");
        bindAttribLocation(m_program, IModel::Buffer::kBoneWeightStride, "inBoneWeights");
    }
    virtual void getUniformLocations() {
        ObjectProgram::getUniformLocations();
        m_shadowMatrixUniformLocation = getUniformLocation(m_program, "shadowMatrix");
        m_boneMatricesUniformLocation = getUniformLocation(m_program, "boneMatrices");
    }

private:
    GLint m_shadowMatrixUniformLocation;
    GLint m_boneMatricesUniformLocation;
};

class ModelProgram : public ObjectProgram
{
public:
    ModelProgram(const IApplicationContext::FunctionResolver *resolver)
        : ObjectProgram(resolver),
          m_cameraPositionUniformLocation(-1),
          m_materialColorUniformLocation(-1),
          m_materialSpecularUniformLocation(-1),
          m_materialShininessUniformLocation(-1),
          m_mainTextureBlendUniformLocation(-1),
          m_sphereTextureBlendUniformLocation(-1),
          m_toonTextureBlendUniformLocation(-1),
          m_sphereTextureUniformLocation(-1),
          m_hasSphereTextureUniformLocation(-1),
          m_isSPHTextureUniformLocation(-1),
          m_isSPATextureUniformLocation(-1),
          m_isSubTextureUniformLocation(-1),
          m_toonTextureUniformLocation(-1),
          m_hasToonTextureUniformLocation(-1),
          m_useToonUniformLocation(-1),
          m_boneMatricesUniformLocation(-1)
    {
    }
    ~ModelProgram() {
        m_cameraPositionUniformLocation = -1;
        m_materialColorUniformLocation = -1;
        m_materialSpecularUniformLocation = -1;
        m_materialShininessUniformLocation = -1;
        m_mainTextureBlendUniformLocation = -1;
        m_sphereTextureBlendUniformLocation = -1;
        m_toonTextureBlendUniformLocation = -1;
        m_sphereTextureUniformLocation = -1;
        m_hasSphereTextureUniformLocation = -1;
        m_isSPHTextureUniformLocation = -1;
        m_isSPATextureUniformLocation = -1;
        m_isSubTextureUniformLocation = -1;
        m_toonTextureUniformLocation = -1;
        m_hasToonTextureUniformLocation = -1;
        m_useToonUniformLocation = -1;
        m_boneMatricesUniformLocation = -1;
    }

    void setCameraPosition(const Vector3 &value) {
        uniform3fv(m_cameraPositionUniformLocation, 1, value);
    }
    void setMaterialColor(const Color &value) {
        uniform4fv(m_materialColorUniformLocation, 1, value);
    }
    void setMaterialSpecular(const Color &value) {
        uniform3fv(m_materialSpecularUniformLocation, 1, value);
    }
    void setMaterialShininess(const Scalar &value) {
        uniform1f(m_materialShininessUniformLocation, value);
    }
    void setMainTextureBlend(const Color &value) {
        uniform4fv(m_mainTextureBlendUniformLocation, 1, value);
    }
    void setSphereTextureBlend(const Color &value) {
        uniform4fv(m_sphereTextureBlendUniformLocation, 1, value);
    }
    void setToonTextureBlend(const Color &value) {
        uniform4fv(m_toonTextureBlendUniformLocation, 1, value);
    }
    void setToonEnable(bool value) {
        uniform1i(m_useToonUniformLocation, value ? 1 : 0);
    }
    void setSphereTexture(const ITexture *value, IMaterial::SphereTextureRenderMode mode) {
        if (value) {
            switch (mode) {
            case IMaterial::kNone:
            default:
                uniform1i(m_hasSphereTextureUniformLocation, 0);
                uniform1i(m_isSPHTextureUniformLocation, 0);
                uniform1i(m_isSPATextureUniformLocation, 0);
                uniform1i(m_isSubTextureUniformLocation, 0);
                break;
            case IMaterial::kMultTexture:
                enableSphereTexture(value);
                uniform1i(m_isSPHTextureUniformLocation, 1);
                uniform1i(m_isSPATextureUniformLocation, 0);
                uniform1i(m_isSubTextureUniformLocation, 0);
                break;
            case IMaterial::kAddTexture:
                enableSphereTexture(value);
                uniform1i(m_isSPHTextureUniformLocation, 0);
                uniform1i(m_isSPATextureUniformLocation, 1);
                uniform1i(m_isSubTextureUniformLocation, 0);
                break;
            case IMaterial::kSubTexture:
                enableSphereTexture(value);
                uniform1i(m_isSPHTextureUniformLocation, 0);
                uniform1i(m_isSPATextureUniformLocation, 0);
                uniform1i(m_isSubTextureUniformLocation, 1);
                break;
            }
        }
        else {
            uniform1i(m_hasSphereTextureUniformLocation, 0);
        }
    }
    void setToonTexture(const ITexture *value) {
        if (value) {
            activeTexture(Texture2D::kGL_TEXTURE0 + 2);
            bindTexture(Texture2D::kGL_TEXTURE_2D, static_cast<GLuint>(value->data()));
            uniform1i(m_toonTextureUniformLocation, 2);
            uniform1i(m_hasToonTextureUniformLocation, 1);
        }
        else {
            uniform1i(m_hasToonTextureUniformLocation, 0);
        }
    }
    void setBoneMatrices(const Scalar *value, vsize size) {
        uniformMatrix4fv(m_boneMatricesUniformLocation, size, kGL_FALSE, value);
    }

protected:
    virtual void bindAttributeLocations() {
        ObjectProgram::bindAttributeLocations();
        bindAttribLocation(m_program, IModel::Buffer::kUVA1Stride, "inUVA1");
        bindAttribLocation(m_program, IModel::Buffer::kBoneIndexStride, "inBoneIndices");
        bindAttribLocation(m_program, IModel::Buffer::kBoneWeightStride, "inBoneWeights");
    }
    virtual void getUniformLocations() {
        ObjectProgram::getUniformLocations();
        m_cameraPositionUniformLocation = getUniformLocation(m_program, "cameraPosition");
        m_materialColorUniformLocation = getUniformLocation(m_program, "materialColor");
        m_materialSpecularUniformLocation = getUniformLocation(m_program, "materialSpecular");
        m_materialShininessUniformLocation = getUniformLocation(m_program, "materialShininess");
        m_mainTextureBlendUniformLocation = getUniformLocation(m_program, "mainTextureBlend");
        m_sphereTextureBlendUniformLocation = getUniformLocation(m_program, "sphereTextureBlend");
        m_toonTextureBlendUniformLocation = getUniformLocation(m_program, "toonTextureBlend");
        m_sphereTextureUniformLocation = getUniformLocation(m_program, "sphereTexture");
        m_hasSphereTextureUniformLocation = getUniformLocation(m_program, "hasSphereTexture");
        m_isSPHTextureUniformLocation = getUniformLocation(m_program, "isSPHTexture");
        m_isSPATextureUniformLocation = getUniformLocation(m_program, "isSPATexture");
        m_isSubTextureUniformLocation = getUniformLocation(m_program, "isSubTexture");
        m_toonTextureUniformLocation = getUniformLocation(m_program, "toonTexture");
        m_hasToonTextureUniformLocation = getUniformLocation(m_program, "hasToonTexture");
        m_useToonUniformLocation = getUniformLocation(m_program, "useToon");
        m_boneMatricesUniformLocation = getUniformLocation(m_program, "boneMatrices");
    }

private:
    void enableSphereTexture(const ITexture *value) {
        activeTexture(Texture2D::kGL_TEXTURE0 + 1);
        bindTexture(Texture2D::kGL_TEXTURE_2D, static_cast<GLuint>(value->data()));
        uniform1i(m_sphereTextureUniformLocation, 1);
        uniform1i(m_hasSphereTextureUniformLocation, 1);
    }

    GLint m_cameraPositionUniformLocation;
    GLint m_materialColorUniformLocation;
    GLint m_materialSpecularUniformLocation;
    GLint m_materialShininessUniformLocation;
    GLint m_mainTextureBlendUniformLocation;
    GLint m_sphereTextureBlendUniformLocation;
    GLint m_toonTextureBlendUniformLocation;
    GLint m_sphereTextureUniformLocation;
    GLint m_hasSphereTextureUniformLocation;
    GLint m_isSPHTextureUniformLocation;
    GLint m_isSPATextureUniformLocation;
    GLint m_isSubTextureUniformLocation;
    GLint m_toonTextureUniformLocation;
    GLint m_hasToonTextureUniformLocation;
    GLint m_useToonUniformLocation;
    GLint m_boneMatricesUniformLocation;
};

}

namespace vpvl2
{
namespace gl2
{

class PMXRenderEngine::PrivateContext
{
public:
    PrivateContext(const IModel *model, const IApplicationContext::FunctionResolver *resolver, bool isVertexShaderSkinning)
        : modelRef(model),
          indexBuffer(0),
          staticBuffer(0),
          dynamicBuffer(0),
          matrixBuffer(0),
          edgeProgram(0),
          modelProgram(0),
          shadowProgram(0),
          zplotProgram(0),
          buffer(resolver),
          aabbMin(SIMD_INFINITY, SIMD_INFINITY, SIMD_INFINITY),
          aabbMax(-SIMD_INFINITY, -SIMD_INFINITY, -SIMD_INFINITY),
          cullFaceState(true),
          isVertexShaderSkinning(isVertexShaderSkinning),
          updateEven(true)
    {
        model->getIndexBuffer(indexBuffer);
        model->getStaticVertexBuffer(staticBuffer);
        model->getDynamicVertexBuffer(dynamicBuffer, indexBuffer);
        if (isVertexShaderSkinning) {
            model->getMatrixBuffer(matrixBuffer, dynamicBuffer, indexBuffer);
        }
        switch (indexBuffer->type()) {
        case IModel::IndexBuffer::kIndex32:
            indexType = kGL_UNSIGNED_INT;
            break;
        case IModel::IndexBuffer::kIndex16:
            indexType = kGL_UNSIGNED_SHORT;
            break;
        case IModel::IndexBuffer::kIndex8:
            indexType = kGL_UNSIGNED_BYTE;
            break;
        case IModel::IndexBuffer::kMaxIndexType:
        default:
            indexType = kGL_UNSIGNED_INT;
            break;
        }
        for (int i = 0; i < kMaxVertexArrayObjectType; i++) {
            bundles[i] = new VertexBundleLayout(resolver);
        }
    }
    virtual ~PrivateContext() {
        for (int i = 0; i < kMaxVertexArrayObjectType; i++) {
            internal::deleteObject(bundles[i]);
        }
        allocatedTextures.releaseAll();
        internal::deleteObject(indexBuffer);
        internal::deleteObject(dynamicBuffer);
        internal::deleteObject(staticBuffer);
        internal::deleteObject(edgeProgram);
        internal::deleteObject(modelProgram);
        internal::deleteObject(shadowProgram);
        internal::deleteObject(zplotProgram);
        aabbMin.setZero();
        aabbMax.setZero();
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
    IModel::IndexBuffer *indexBuffer;
    IModel::StaticVertexBuffer *staticBuffer;
    IModel::DynamicVertexBuffer *dynamicBuffer;
    IModel::MatrixBuffer *matrixBuffer;
    EdgeProgram *edgeProgram;
    ModelProgram *modelProgram;
    ShadowProgram *shadowProgram;
    ExtendedZPlotProgram *zplotProgram;
    VertexBundle buffer;
    VertexBundleLayout *bundles[kMaxVertexArrayObjectType];
    GLenum indexType;
    PointerHash<HashPtr, ITexture> allocatedTextures;
    Array<MaterialTextureRefs> materialTextureRefs;
    Vector3 aabbMin;
    Vector3 aabbMax;
#ifdef VPVL2_ENABLE_OPENCL
    cl::PMXAccelerator::VertexBufferBridgeArray buffers;
#endif
    bool cullFaceState;
    bool isVertexShaderSkinning;
    bool updateEven;
};

PMXRenderEngine::PMXRenderEngine(IApplicationContext *applicationContextRef,
                                 Scene *scene,
                                 cl::PMXAccelerator *accelerator,
                                 IModel *modelRef)
    : cullFace(reinterpret_cast<PFNGLCULLFACEPROC>(applicationContextRef->sharedFunctionResolverInstance()->resolveSymbol("glCullFace"))),
      enable(reinterpret_cast<PFNGLENABLEPROC>(applicationContextRef->sharedFunctionResolverInstance()->resolveSymbol("glEnable"))),
      disable(reinterpret_cast<PFNGLDISABLEPROC>(applicationContextRef->sharedFunctionResolverInstance()->resolveSymbol("glDisable"))),
      drawElements(reinterpret_cast<PFNGLDRAWELEMENTSPROC>(applicationContextRef->sharedFunctionResolverInstance()->resolveSymbol("glDrawElements"))),
      genQueries(reinterpret_cast<PFNGLGENQUERIESPROC>(applicationContextRef->sharedFunctionResolverInstance()->resolveSymbol("glGenQueries"))),
      beginQuery(reinterpret_cast<PFNGLBEGINQUERYPROC>(applicationContextRef->sharedFunctionResolverInstance()->resolveSymbol("glBeginQuery"))),
      endQuery(reinterpret_cast<PFNGLENDQUERYPROC>(applicationContextRef->sharedFunctionResolverInstance()->resolveSymbol("glEndQuery"))),
      getQueryObjectiv(reinterpret_cast<PFNGLGETQUERYOBJECTIVPROC>(applicationContextRef->sharedFunctionResolverInstance()->resolveSymbol("glGetQueryObjectiv"))),
      deleteQueries(reinterpret_cast<PFNGLDELETEQUERIESPROC>(applicationContextRef->sharedFunctionResolverInstance()->resolveSymbol("glDeleteQueries"))),
      enableVertexAttribArray(reinterpret_cast<PFNGLENABLEVERTEXATTRIBARRAYPROC>(applicationContextRef->sharedFunctionResolverInstance()->resolveSymbol("glEnableVertexAttribArray"))),
      vertexAttribPointer(reinterpret_cast<PFNGLVERTEXATTRIBPOINTERPROC>(applicationContextRef->sharedFunctionResolverInstance()->resolveSymbol("glVertexAttribPointer"))),
      m_accelerator(accelerator),
      m_applicationContextRef(applicationContextRef),
      m_sceneRef(scene),
      m_modelRef(modelRef),
      m_context(new PrivateContext(modelRef, applicationContextRef->sharedFunctionResolverInstance(), m_sceneRef->accelerationType() == Scene::kVertexShaderAccelerationType1))
{
    bool vss = m_sceneRef->accelerationType() == Scene::kVertexShaderAccelerationType1;
#ifdef VPVL2_ENABLE_OPENCL
    if (vss || (m_accelerator && m_accelerator->isAvailable())) {
#else
    if (vss) {
#endif
        m_context->dynamicBuffer->setSkinningEnable(false);
    }
}

PMXRenderEngine::~PMXRenderEngine()
{
#ifdef VPVL2_ENABLE_OPENCL
    if (m_context) {
        m_accelerator->release(m_context->buffers);
    }
    internal::deleteObject(m_accelerator);
#endif
    internal::deleteObject(m_context);
    m_applicationContextRef = 0;
    m_sceneRef = 0;
    m_modelRef = 0;
    m_accelerator = 0;
}

IModel *PMXRenderEngine::parentModelRef() const
{
    return m_modelRef && m_modelRef->parentSceneRef() ? m_modelRef : 0;
}

bool PMXRenderEngine::upload(void *userData)
{
    bool ret = true, vss = false;
    const IApplicationContext::FunctionResolver *resolver = m_applicationContextRef->sharedFunctionResolverInstance();
    if (!m_context) {
        vss = m_sceneRef->accelerationType() == Scene::kVertexShaderAccelerationType1;
        m_context = new PrivateContext(m_modelRef, resolver, vss);
        m_context->dynamicBuffer->setSkinningEnable(false);
    }
    vss = m_context->isVertexShaderSkinning;
    EdgeProgram *edgeProgram = m_context->edgeProgram = new EdgeProgram(resolver);
    ModelProgram *modelProgram = m_context->modelProgram = new ModelProgram(resolver);
    ShadowProgram *shadowProgram = m_context->shadowProgram = new ShadowProgram(resolver);
    ExtendedZPlotProgram *zplotProgram = m_context->zplotProgram = new ExtendedZPlotProgram(resolver);
    if (!createProgram(edgeProgram,
                       IApplicationContext::kEdgeVertexShader,
                       IApplicationContext::kEdgeWithSkinningVertexShader,
                       IApplicationContext::kEdgeFragmentShader,
                       userData)) {
        return false;
    }
    if (!createProgram(modelProgram,
                       IApplicationContext::kModelVertexShader,
                       IApplicationContext::kModelWithSkinningVertexShader,
                       IApplicationContext::kModelFragmentShader,
                       userData)) {
        return false;
    }
    if (!createProgram(shadowProgram,
                       IApplicationContext::kShadowVertexShader,
                       IApplicationContext::kShadowWithSkinningVertexShader,
                       IApplicationContext::kShadowFragmentShader,
                       userData)) {
        return false;
    }
    if (!createProgram(zplotProgram,
                       IApplicationContext::kZPlotVertexShader,
                       IApplicationContext::kZPlotWithSkinningVertexShader,
                       IApplicationContext::kZPlotFragmentShader,
                       userData)) {
        return false;
    }
    if (!uploadMaterials(userData)) {
        return false;
    }
    VertexBundle &buffer = m_context->buffer;
    buffer.create(VertexBundle::kVertexBuffer, kModelDynamicVertexBufferEven, VertexBundle::kGL_DYNAMIC_DRAW, 0, m_context->dynamicBuffer->size());
    buffer.create(VertexBundle::kVertexBuffer, kModelDynamicVertexBufferOdd, VertexBundle::kGL_DYNAMIC_DRAW, 0, m_context->dynamicBuffer->size());
    VPVL2_VLOG(2, "Binding model dynamic vertex buffer to the vertex buffer object: size=" << m_context->dynamicBuffer->size());
    const IModel::StaticVertexBuffer *staticBuffer = m_context->staticBuffer;
    buffer.create(VertexBundle::kVertexBuffer, kModelStaticVertexBuffer, VertexBundle::kGL_STATIC_DRAW, 0, staticBuffer->size());
    buffer.bind(VertexBundle::kVertexBuffer, kModelStaticVertexBuffer);
    void *address = buffer.map(VertexBundle::kVertexBuffer, 0, staticBuffer->size());
    staticBuffer->update(address);
    VPVL2_VLOG(2, "Binding model static vertex buffer to the vertex buffer object: ptr=" << address << " size=" << staticBuffer->size());
    buffer.unmap(VertexBundle::kVertexBuffer, address);
    buffer.unbind(VertexBundle::kVertexBuffer);
    const IModel::IndexBuffer *indexBuffer = m_context->indexBuffer;
    buffer.create(VertexBundle::kIndexBuffer, kModelIndexBuffer, VertexBundle::kGL_STATIC_DRAW, indexBuffer->bytes(), indexBuffer->size());
    VPVL2_VLOG(2, "Binding indices to the vertex buffer object: ptr=" << indexBuffer->bytes() << " size=" << indexBuffer->size());
    VertexBundleLayout *bundleME = m_context->bundles[kVertexArrayObjectEven];
    if (bundleME->create() && bundleME->bind()) {
        VPVL2_VLOG(2, "Binding an vertex array object for even frame: " << bundleME->name());
        createVertexBundle(kModelDynamicVertexBufferEven);
    }
    bundleME->unbind();
    VertexBundleLayout *bundleMO = m_context->bundles[kVertexArrayObjectOdd];
    if (bundleMO->create() && bundleMO->bind()) {
        VPVL2_VLOG(2, "Binding an vertex array object for odd frame: " << bundleMO->name());
        createVertexBundle(kModelDynamicVertexBufferOdd);
    }
    bundleMO->unbind();
    VertexBundleLayout *bundleEE = m_context->bundles[kEdgeVertexArrayObjectEven];
    if (bundleEE->create() && bundleEE->bind()) {
        VPVL2_VLOG(2, "Binding an edge vertex array object for even frame: " << bundleEE->name());
        createEdgeBundle(kModelDynamicVertexBufferEven);
    }
    bundleEE->unbind();
    VertexBundleLayout *bundleEO = m_context->bundles[kEdgeVertexArrayObjectOdd];
    if (bundleEO->create() && bundleEO->bind()) {
        VPVL2_VLOG(2, "Binding an edge vertex array object for odd frame: " << bundleEO->name());
        createEdgeBundle(kModelDynamicVertexBufferOdd);
    }
    bundleEO->unbind();
    buffer.unbind(VertexBundle::kVertexBuffer);
    buffer.unbind(VertexBundle::kIndexBuffer);
#ifdef VPVL2_ENABLE_OPENCL
    if (m_accelerator && m_accelerator->isAvailable()) {
        const VertexBundle &buffer = m_context->buffer;
        cl::PMXAccelerator::VertexBufferBridgeArray &buffers = m_context->buffers;
        m_accelerator->release(buffers);
        buffers.append(cl::PMXAccelerator::VertexBufferBridge(buffer.findName(kModelDynamicVertexBufferEven)));
        buffers.append(cl::PMXAccelerator::VertexBufferBridge(buffer.findName(kModelDynamicVertexBufferOdd)));
        m_accelerator->upload(buffers, m_context->indexBuffer);
    }
#endif
    m_modelRef->setVisible(true);
    update(); // for updating even frame
    update(); // for updating odd frame
    VPVL2_VLOG(2, "Created the model: jp=" << internal::cstr(m_modelRef->name(IEncoding::kJapanese), "(null)") << " en=" << internal::cstr(m_modelRef->name(IEncoding::kEnglish), "(null)"));
    return ret;
}

void PMXRenderEngine::update()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_context)
        return;
    VertexBufferObjectType vbo = m_context->updateEven
            ? kModelDynamicVertexBufferEven : kModelDynamicVertexBufferOdd;
    IModel::DynamicVertexBuffer *dynamicBuffer = m_context->dynamicBuffer;
    m_context->buffer.bind(VertexBundle::kVertexBuffer, vbo);
    if (void *address = m_context->buffer.map(VertexBundle::kVertexBuffer, 0, dynamicBuffer->size())) {
        const ICamera *camera = m_sceneRef->cameraRef();
        dynamicBuffer->update(address, camera->position(), m_context->aabbMin, m_context->aabbMax);
        if (m_context->isVertexShaderSkinning) {
            m_context->matrixBuffer->update(address);
        }
        m_context->buffer.unmap(VertexBundle::kVertexBuffer, address);
    }
    m_context->buffer.unbind(VertexBundle::kVertexBuffer);
#ifdef VPVL2_ENABLE_OPENCL
    if (m_accelerator && m_accelerator->isAvailable()) {
        const cl::PMXAccelerator::VertexBufferBridge &buffer = m_context->buffers[m_context->updateEven ? 0 : 1];
        m_accelerator->update(dynamicBuffer, buffer, m_context->aabbMin, m_context->aabbMax);
    }
#endif
    m_modelRef->setAabb(m_context->aabbMin, m_context->aabbMax);
    m_context->updateEven = m_context->updateEven ? false :true;
}

void PMXRenderEngine::setUpdateOptions(int options)
{
    if (m_context) {
        IModel::DynamicVertexBuffer *dynamicBuffer = m_context->dynamicBuffer;
        dynamicBuffer->setParallelUpdateEnable(internal::hasFlagBits(options, kParallelUpdate));
    }
}

void PMXRenderEngine::renderModel()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_context)
        return;
    ModelProgram *modelProgram = m_context->modelProgram;
    modelProgram->bind();
    float matrix4x4[16];
    m_applicationContextRef->getMatrix(matrix4x4, m_modelRef,
                                       IApplicationContext::kWorldMatrix
                                       | IApplicationContext::kViewMatrix
                                       | IApplicationContext::kProjectionMatrix
                                       | IApplicationContext::kCameraMatrix);
    modelProgram->setModelViewProjectionMatrix(matrix4x4);
    m_applicationContextRef->getMatrix(matrix4x4, m_modelRef,
                                       IApplicationContext::kWorldMatrix
                                       | IApplicationContext::kViewMatrix
                                       | IApplicationContext::kCameraMatrix);
    modelProgram->setNormalMatrix(matrix4x4);
    m_applicationContextRef->getMatrix(matrix4x4, m_modelRef,
                                       IApplicationContext::kWorldMatrix
                                       | IApplicationContext::kViewMatrix
                                       | IApplicationContext::kProjectionMatrix
                                       | IApplicationContext::kLightMatrix);
    modelProgram->setLightViewProjectionMatrix(matrix4x4);
    const ILight *light = m_sceneRef->lightRef();
    GLuint textureID = 0;
    if (const IShadowMap *shadowMapRef = m_sceneRef->shadowMapRef()) {
        const void *texture = shadowMapRef->textureRef();
        textureID = texture ? *static_cast<const GLuint *>(texture) : 0;
        modelProgram->setDepthTextureSize(shadowMapRef->size());
    }
    modelProgram->setLightColor(light->color());
    modelProgram->setLightDirection(light->direction());
    modelProgram->setToonEnable(light->isToonEnabled());
    modelProgram->setCameraPosition(m_sceneRef->cameraRef()->lookAt());
    const Scalar &opacity = m_modelRef->opacity();
    modelProgram->setOpacity(opacity);
    Array<IMaterial *> materials;
    m_modelRef->getMaterialRefs(materials);
    const int nmaterials = materials.count();
    const bool hasModelTransparent = !btFuzzyZero(opacity - 1.0f),
            isVertexShaderSkinning = m_context->isVertexShaderSkinning;
    const Vector3 &lc = light->color();
    bool &cullFaceState = m_context->cullFaceState;
    Color diffuse, specular;
    vsize offset = 0, size = m_context->indexBuffer->strideSize();
    bindVertexBundle();
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = materials[i];
        const MaterialTextureRefs &materialPrivate = m_context->materialTextureRefs[i];
        const Color &ma = material->ambient(), &md = material->diffuse(), &ms = material->specular();
        diffuse.setValue(ma.x() + md.x() * lc.x(), ma.y() + md.y() * lc.y(), ma.z() + md.z() * lc.z(), md.w());
        specular.setValue(ms.x() * lc.x(), ms.y() * lc.y(), ms.z() * lc.z(), 1.0);
        modelProgram->setMaterialColor(diffuse);
        modelProgram->setMaterialSpecular(specular);
        modelProgram->setMaterialShininess(material->shininess());
        modelProgram->setMainTextureBlend(material->mainTextureBlend());
        modelProgram->setSphereTextureBlend(material->sphereTextureBlend());
        modelProgram->setToonTextureBlend(material->toonTextureBlend());
        modelProgram->setMainTexture(materialPrivate.mainTextureRef);
        modelProgram->setSphereTexture(materialPrivate.sphereTextureRef, material->sphereTextureRenderMode());
        modelProgram->setToonTexture(materialPrivate.toonTextureRef);
        if (textureID && material->isSelfShadowEnabled())
            modelProgram->setDepthTexture(textureID);
        else
            modelProgram->setDepthTexture(0);
        if (isVertexShaderSkinning) {
            const IModel::MatrixBuffer *matrixBuffer = m_context->matrixBuffer;
            modelProgram->setBoneMatrices(matrixBuffer->bytes(i), matrixBuffer->size(i));
        }
        if (!hasModelTransparent && cullFaceState && material->isCullingDisabled()) {
            disable(kGL_CULL_FACE);
            cullFaceState = false;
        }
        else if (!cullFaceState && !material->isCullingDisabled()) {
            enable(kGL_CULL_FACE);
            cullFaceState = true;
        }
        const int nindices = material->indexRange().count;
        drawElements(kGL_TRIANGLES, nindices, m_context->indexType, reinterpret_cast<const GLvoid *>(offset));
        offset += nindices * size;
    }
    unbindVertexBundle();
    modelProgram->unbind();
    if (!cullFaceState) {
        enable(kGL_CULL_FACE);
        cullFaceState = true;
    }
}

void PMXRenderEngine::renderShadow()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_context)
        return;
    ShadowProgram *shadowProgram = m_context->shadowProgram;
    shadowProgram->bind();
    float matrix4x4[16];
    m_applicationContextRef->getMatrix(matrix4x4, m_modelRef,
                                       IApplicationContext::kWorldMatrix
                                       | IApplicationContext::kViewMatrix
                                       | IApplicationContext::kProjectionMatrix
                                       | IApplicationContext::kShadowMatrix);
    shadowProgram->setModelViewProjectionMatrix(matrix4x4);
    const ILight *light = m_sceneRef->lightRef();
    shadowProgram->setLightColor(light->color());
    shadowProgram->setLightDirection(light->direction());
    Array<IMaterial *> materials;
    m_modelRef->getMaterialRefs(materials);
    const int nmaterials = materials.count();
    const bool isVertexShaderSkinning = m_context->isVertexShaderSkinning;
    vsize offset = 0, size = m_context->indexBuffer->strideSize();
    bindVertexBundle();
    disable(kGL_CULL_FACE);
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = materials[i];
        const int nindices = material->indexRange().count;
        if (material->hasShadow()) {
            if (isVertexShaderSkinning) {
                const IModel::MatrixBuffer *matrixBuffer = m_context->matrixBuffer;
                shadowProgram->setBoneMatrices(matrixBuffer->bytes(i), matrixBuffer->size(i));
            }
            drawElements(kGL_TRIANGLES, nindices, m_context->indexType, reinterpret_cast<const GLvoid *>(offset));
        }
        offset += nindices * size;
    }
    unbindVertexBundle();
    enable(kGL_CULL_FACE);
    shadowProgram->unbind();
}

void PMXRenderEngine::renderEdge()
{
    if (!m_modelRef || !m_modelRef->isVisible() || btFuzzyZero(Scalar(m_modelRef->edgeWidth())) || !m_context)
        return;
    EdgeProgram *edgeProgram = m_context->edgeProgram;
    edgeProgram->bind();
    float matrix4x4[16];
    const Scalar &opacity = m_modelRef->opacity();
    m_applicationContextRef->getMatrix(matrix4x4, m_modelRef,
                                       IApplicationContext::kWorldMatrix
                                       | IApplicationContext::kViewMatrix
                                       | IApplicationContext::kProjectionMatrix
                                       | IApplicationContext::kCameraMatrix);
    edgeProgram->setModelViewProjectionMatrix(matrix4x4);
    edgeProgram->setOpacity(opacity);
    Array<IMaterial *> materials;
    m_modelRef->getMaterialRefs(materials);
    const int nmaterials = materials.count();
    const bool isVertexShaderSkinning = m_context->isVertexShaderSkinning;
    IVertex::EdgeSizePrecision edgeScaleFactor = 0;
    if (isVertexShaderSkinning) {
        const ICamera *camera = m_sceneRef->cameraRef();
        edgeScaleFactor = m_modelRef->edgeScaleFactor(camera->position());
    }
    vsize offset = 0, size = m_context->indexBuffer->strideSize();
    bool isOpaque = btFuzzyZero(opacity - 1);
    if (isOpaque) {
        disable(kGL_BLEND);
    }
    cullFace(kGL_FRONT);
    bindEdgeBundle();
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = materials[i];
        const int nindices = material->indexRange().count;
        edgeProgram->setColor(material->edgeColor());
        if (material->isEdgeEnabled()) {
            if (isVertexShaderSkinning) {
                const IModel::MatrixBuffer *matrixBuffer = m_context->matrixBuffer;
                edgeProgram->setBoneMatrices(matrixBuffer->bytes(i), matrixBuffer->size(i));
                edgeProgram->setSize(Scalar(material->edgeSize() * edgeScaleFactor));
            }
            drawElements(kGL_TRIANGLES, nindices, m_context->indexType, reinterpret_cast<const GLvoid *>(offset));
        }
        offset += nindices * size;
    }
    unbindVertexBundle();
    cullFace(kGL_BACK);
    if (isOpaque) {
        enable(kGL_BLEND);
    }
    edgeProgram->unbind();
}

void PMXRenderEngine::renderZPlot()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_context)
        return;
    ExtendedZPlotProgram *zplotProgram = m_context->zplotProgram;
    zplotProgram->bind();
    float matrix4x4[16];
    m_applicationContextRef->getMatrix(matrix4x4, m_modelRef,
                                       IApplicationContext::kWorldMatrix
                                       | IApplicationContext::kViewMatrix
                                       | IApplicationContext::kProjectionMatrix
                                       | IApplicationContext::kLightMatrix);
    zplotProgram->setModelViewProjectionMatrix(matrix4x4);
    Array<IMaterial *> materials;
    m_modelRef->getMaterialRefs(materials);
    const int nmaterials = materials.count();
    const bool isVertexShaderSkinning = m_context->isVertexShaderSkinning;
    vsize offset = 0, size = m_context->indexBuffer->strideSize();
    bindVertexBundle();
    disable(kGL_CULL_FACE);
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = materials[i];
        const int nindices = material->indexRange().count;
        if (material->hasShadowMap()) {
            if (isVertexShaderSkinning) {
                const IModel::MatrixBuffer *matrixBuffer = m_context->matrixBuffer;
                zplotProgram->setBoneMatrices(matrixBuffer->bytes(i), matrixBuffer->size(i));
            }
            drawElements(kGL_TRIANGLES, nindices, m_context->indexType, reinterpret_cast<const GLvoid *>(offset));
        }
        offset += nindices * size;
    }
    unbindVertexBundle();
    enable(kGL_CULL_FACE);
    zplotProgram->unbind();
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

IEffect *PMXRenderEngine::effectRef(IEffect::ScriptOrderType /* type */) const
{
    return 0;
}

void PMXRenderEngine::setEffect(IEffect * /* effectRef */, IEffect::ScriptOrderType /* type */, void * /* userData */)
{
    /* do nothing */
}

void PMXRenderEngine::setOverridePass(IEffect::Pass * /* pass */)
{
    /* do nothing */
}

bool PMXRenderEngine::testVisible()
{
    return true;
}

bool PMXRenderEngine::createProgram(BaseShaderProgram *program,
                                    IApplicationContext::ShaderType vertexShaderType,
                                    IApplicationContext::ShaderType vertexSkinningShaderType,
                                    IApplicationContext::ShaderType fragmentShaderType,
                                    void *userData)
{
    IString *vertexShaderSource = 0;
    IString *fragmentShaderSource = 0;
    if (m_context->isVertexShaderSkinning) {
        vertexShaderSource = m_applicationContextRef->loadShaderSource(vertexSkinningShaderType, m_modelRef, userData);
    }
    else {
        vertexShaderSource = m_applicationContextRef->loadShaderSource(vertexShaderType, m_modelRef, userData);
    }
    fragmentShaderSource = m_applicationContextRef->loadShaderSource(fragmentShaderType, m_modelRef, userData);
    program->addShaderSource(vertexShaderSource, ShaderProgram::kGL_VERTEX_SHADER);
    program->addShaderSource(fragmentShaderSource, ShaderProgram::kGL_FRAGMENT_SHADER);
    bool ok = program->linkProgram();
    delete vertexShaderSource;
    delete fragmentShaderSource;
    return ok;
}

bool PMXRenderEngine::uploadMaterials(void *userData)
{
    Array<IMaterial *> materials;
    m_modelRef->getMaterialRefs(materials);
    const int nmaterials = materials.count();
    IApplicationContext::TextureDataBridge bridge(IApplicationContext::kTexture2D);
    m_context->materialTextureRefs.resize(nmaterials);
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = materials[i];
        const IString *name = material->name(IEncoding::kDefaultLanguage); (void) name;
        const int materialIndex = material->index(); (void) materialIndex;
        MaterialTextureRefs &materialPrivate = m_context->materialTextureRefs[i];
        bridge.flags = IApplicationContext::kTexture2D | IApplicationContext::kAsyncLoadingTexture;
        if (const IString *mainTexturePath = material->mainTexture()) {
            if (m_applicationContextRef->uploadTexture(mainTexturePath, bridge, userData)) {
                ITexture *textureRef = bridge.dataRef;
                materialPrivate.mainTextureRef = m_context->allocatedTextures.insert(textureRef, textureRef);
                VPVL2_VLOG(2, "Binding the texture as a main texture (material=" << internal::cstr(name, "(null)") << " index=" << materialIndex << " ID=" << bridge.dataRef << ")");
            }
            else {
                VPVL2_LOG(WARNING, "Cannot bind a main texture (material=" << internal::cstr(name, "(null)") << " index=" << materialIndex << ")");
                return false;
            }
        }
        if (const IString *sphereTexturePath = material->sphereTexture()) {
            if (m_applicationContextRef->uploadTexture(sphereTexturePath, bridge, userData)) {
                ITexture *textureRef = bridge.dataRef;
                materialPrivate.sphereTextureRef = m_context->allocatedTextures.insert(textureRef, textureRef);
                VPVL2_VLOG(2, "Binding the texture as a sphere texture: material=" << internal::cstr(name, "(null)") << " index=" << materialIndex << " ID=" << bridge.dataRef);
            }
            else {
                VPVL2_LOG(WARNING, "Cannot bind a sphere texture: material=" << internal::cstr(name, "(null)") << " index=" << materialIndex);
                return false;
            }
        }
        bridge.flags |= IApplicationContext::kToonTexture;
        if (material->isSharedToonTextureUsed()) {
            char buf[16];
            internal::snprintf(buf, sizeof(buf), "toon%02d.bmp", material->toonTextureIndex() + 1);
            IString *s = m_applicationContextRef->toUnicode(reinterpret_cast<const uint8 *>(buf));
            bridge.flags |= IApplicationContext::kSystemToonTexture;
            bool ret = m_applicationContextRef->uploadTexture(s, bridge, userData);
            internal::deleteObject(s);
            if (ret) {
                ITexture *textureRef = bridge.dataRef;
                materialPrivate.toonTextureRef = m_context->allocatedTextures.insert(textureRef, textureRef);
                VPVL2_VLOG(2, "Binding the texture as a shared toon texture: material=" << internal::cstr(name, "(null)") << " index=" << materialIndex << " ID=" << bridge.dataRef);
            }
            else {
                VPVL2_LOG(WARNING, "Cannot bind a shared toon texture: material=" << internal::cstr(name, "(null)") << " index=" << materialIndex);
                return false;
            }
        }
        else if (const IString *toonTexturePath = material->toonTexture()) {
            if (m_applicationContextRef->uploadTexture(toonTexturePath, bridge, userData)) {
                ITexture *textureRef = bridge.dataRef;
                materialPrivate.toonTextureRef = m_context->allocatedTextures.insert(textureRef, textureRef);
                VPVL2_VLOG(2, "Binding the texture as a toon texture: material=" << internal::cstr(name, "(null)") << " index=" << materialIndex << " ID=" << bridge.dataRef);
            }
            else {
                VPVL2_LOG(WARNING, "Cannot bind a toon texture: material=" << internal::cstr(name, "(null)") << " index=" << materialIndex);
                return false;
            }
        }
    }
    return true;
}

void PMXRenderEngine::createVertexBundle(GLuint dvbo)
{
    VertexBundle &buffer = m_context->buffer;
    buffer.bind(VertexBundle::kVertexBuffer, dvbo);
    bindDynamicVertexAttributePointers();
    buffer.bind(VertexBundle::kVertexBuffer, kModelStaticVertexBuffer);
    bindStaticVertexAttributePointers();
    buffer.bind(VertexBundle::kIndexBuffer, kModelIndexBuffer);
    unbindVertexBundle();
}

void PMXRenderEngine::createEdgeBundle(GLuint dvbo)
{
    VertexBundle &buffer = m_context->buffer;
    buffer.bind(VertexBundle::kVertexBuffer, dvbo);
    bindEdgeVertexAttributePointers();
    buffer.bind(VertexBundle::kVertexBuffer, kModelStaticVertexBuffer);
    bindStaticVertexAttributePointers();
    buffer.bind(VertexBundle::kIndexBuffer, kModelIndexBuffer);
    unbindVertexBundle();
}

void PMXRenderEngine::bindVertexBundle()
{
    VertexArrayObjectType vao;
    VertexBufferObjectType vbo;
    m_context->getVertexBundleType(vao, vbo);
    if (!m_context->bundles[vao]->bind()) {
        VertexBundle &buffer = m_context->buffer;
        buffer.bind(VertexBundle::kVertexBuffer, vbo);
        bindDynamicVertexAttributePointers();
        buffer.bind(VertexBundle::kVertexBuffer, kModelStaticVertexBuffer);
        bindStaticVertexAttributePointers();
        buffer.bind(VertexBundle::kIndexBuffer, kModelIndexBuffer);
    }
}

void PMXRenderEngine::bindEdgeBundle()
{
    VertexArrayObjectType vao;
    VertexBufferObjectType vbo;
    m_context->getEdgeBundleType(vao, vbo);
    if (!m_context->bundles[vao]->bind()) {
        VertexBundle &buffer = m_context->buffer;
        buffer.bind(VertexBundle::kVertexBuffer, vbo);
        bindEdgeVertexAttributePointers();
        buffer.bind(VertexBundle::kVertexBuffer, kModelStaticVertexBuffer);
        bindStaticVertexAttributePointers();
        buffer.bind(VertexBundle::kIndexBuffer, kModelIndexBuffer);
    }
}

void PMXRenderEngine::unbindVertexBundle()
{
    VertexArrayObjectType vao;
    VertexBufferObjectType vbo;
    m_context->getEdgeBundleType(vao, vbo);
    if (!m_context->bundles[vao]->unbind()) {
        VertexBundle &buffer = m_context->buffer;
        buffer.unbind(VertexBundle::kVertexBuffer);
        buffer.unbind(VertexBundle::kIndexBuffer);
    }
}

void PMXRenderEngine::bindDynamicVertexAttributePointers()
{
    const IModel::DynamicVertexBuffer *dynamicBuffer = m_context->dynamicBuffer;
    vsize offset, size;
    offset = dynamicBuffer->strideOffset(IModel::DynamicVertexBuffer::kVertexStride);
    size   = dynamicBuffer->strideSize();
    vertexAttribPointer(IModel::Buffer::kVertexStride, m_context->isVertexShaderSkinning ? 4 : 3, kGL_FLOAT, kGL_FALSE,
                        size, reinterpret_cast<const GLvoid *>(offset));
    offset = dynamicBuffer->strideOffset(IModel::DynamicVertexBuffer::kNormalStride);
    vertexAttribPointer(IModel::Buffer::kNormalStride, 3, kGL_FLOAT, kGL_FALSE,
                        size, reinterpret_cast<const GLvoid *>(offset));
    offset = dynamicBuffer->strideOffset(IModel::DynamicVertexBuffer::kUVA1Stride);
    vertexAttribPointer(IModel::Buffer::kUVA1Stride, 4, kGL_FLOAT, kGL_FALSE,
                        size, reinterpret_cast<const GLvoid *>(offset));
    enableVertexAttribArray(IModel::Buffer::kVertexStride);
    enableVertexAttribArray(IModel::Buffer::kNormalStride);
    enableVertexAttribArray(IModel::Buffer::kUVA1Stride);
}

void PMXRenderEngine::bindEdgeVertexAttributePointers()
{
    const IModel::DynamicVertexBuffer *dynamicBuffer = m_context->dynamicBuffer;
    const vsize size = dynamicBuffer->strideSize();
    vsize offset = dynamicBuffer->strideOffset(m_context->isVertexShaderSkinning ? IModel::DynamicVertexBuffer::kVertexStride
                                                                                 : IModel::DynamicVertexBuffer::kEdgeVertexStride);
    vertexAttribPointer(IModel::Buffer::kVertexStride, m_context->isVertexShaderSkinning ? 4 : 3, kGL_FLOAT, kGL_FALSE,
                        size, reinterpret_cast<const GLvoid *>(offset));
    enableVertexAttribArray(IModel::Buffer::kVertexStride);
    if (m_context->isVertexShaderSkinning) {
        offset = dynamicBuffer->strideOffset(IModel::DynamicVertexBuffer::kNormalStride);
        vertexAttribPointer(IModel::Buffer::kNormalStride, 4, kGL_FLOAT, kGL_FALSE,
                            size, reinterpret_cast<const GLvoid *>(offset));
        enableVertexAttribArray(IModel::Buffer::kNormalStride);
    }
}

void PMXRenderEngine::bindStaticVertexAttributePointers()
{
    const IModel::StaticVertexBuffer *staticBuffer = m_context->staticBuffer;
    const vsize size = staticBuffer->strideSize();
    vsize offset = staticBuffer->strideOffset(IModel::StaticVertexBuffer::kTextureCoordStride);
    vertexAttribPointer(IModel::Buffer::kTextureCoordStride, 2, kGL_FLOAT, kGL_FALSE,
                        size, reinterpret_cast<const GLvoid *>(offset));
    enableVertexAttribArray(IModel::Buffer::kTextureCoordStride);
    if (m_context->isVertexShaderSkinning) {
        offset = staticBuffer->strideOffset(IModel::StaticVertexBuffer::kBoneIndexStride);
        vertexAttribPointer(IModel::Buffer::kBoneIndexStride, 4, kGL_FLOAT, kGL_FALSE,
                            size, reinterpret_cast<const GLvoid *>(offset));
        enableVertexAttribArray(IModel::Buffer::kBoneIndexStride);
        offset = staticBuffer->strideOffset(IModel::StaticVertexBuffer::kBoneWeightStride);
        vertexAttribPointer(IModel::Buffer::kBoneWeightStride, 4, kGL_FLOAT, kGL_FALSE,
                            size, reinterpret_cast<const GLvoid *>(offset));
        enableVertexAttribArray(IModel::Buffer::kBoneWeightStride);
    }
}

} /* namespace gl2 */
} /* namespace vpvl2 */
