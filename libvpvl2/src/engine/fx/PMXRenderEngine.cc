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

#include "vpvl2/fx/PMXRenderEngine.h"

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/util.h" /* internal::snprintf */
#include "vpvl2/cl/PMXAccelerator.h"

namespace vpvl2
{
namespace fx
{
using namespace extensions::gl;

class PMXRenderEngine::PrivateEffectEngine : public EffectEngine {
public:
    enum DrawType {
        kVertex,
        kEdge
    };

    PrivateEffectEngine(PMXRenderEngine *renderEngineRef, const IApplicationContext::FunctionResolver *resolver)
        : EffectEngine(renderEngineRef->sceneRef(), renderEngineRef->applicationContextRef()),
          drawRangeElementsBaseVertex(reinterpret_cast<PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC>(resolver->resolveSymbol("glDrawRangeElementsBaseVertex"))),
          drawRangeElements(reinterpret_cast<PFNGLDRAWRANGEELEMENTSPROC>(resolver->resolveSymbol("glDrawRangeElements"))),
          m_parentRenderEngineRef(renderEngineRef),
          m_drawType(kVertex)
    {
    }
    ~PrivateEffectEngine() {
        m_parentRenderEngineRef = 0;
    }

    void setDrawType(DrawType value) {
        m_drawType = value;
    }

protected:
    typedef void (GLAPIENTRY * PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, void* indices, GLint basevertex);
    typedef void (GLAPIENTRY * PFNGLDRAWRANGEELEMENTSPROC) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
    PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC drawRangeElementsBaseVertex;
    PFNGLDRAWRANGEELEMENTSPROC drawRangeElements;

    void drawPrimitives(const DrawPrimitiveCommand &command) const {
        if (drawRangeElementsBaseVertex) {
            drawRangeElementsBaseVertex(command.mode, command.start, command.end, command.count, command.type,
                                        const_cast<uint8 *>(command.ptr) + command.offset * command.stride, 0);
        }
        else {
            drawRangeElements(command.mode, command.start, command.end, command.count,
                              command.type, command.ptr + command.offset * command.stride);
        }
    }
    void rebindVertexBundle() {
        switch (m_drawType) {
        case kVertex:
            m_parentRenderEngineRef->bindVertexBundle();
            break;
        case kEdge:
            m_parentRenderEngineRef->bindEdgeBundle();
            break;
        default:
            break;
        }
    }

private:
    PMXRenderEngine *m_parentRenderEngineRef;
    DrawType m_drawType;

    VPVL2_DISABLE_COPY_AND_ASSIGN(PrivateEffectEngine)
};

PMXRenderEngine::PMXRenderEngine(IApplicationContext *applicationContextRef,
                                 Scene *scene,
                                 cl::PMXAccelerator *accelerator,
                                 IModel *modelRef)

    : cullFace(reinterpret_cast<PFNGLCULLFACEPROC>(applicationContextRef->sharedFunctionResolverInstance()->resolveSymbol("glCullFace"))),
      enable(reinterpret_cast<PFNGLENABLEPROC>(applicationContextRef->sharedFunctionResolverInstance()->resolveSymbol("glEnable"))),
      disable(reinterpret_cast<PFNGLDISABLEPROC>(applicationContextRef->sharedFunctionResolverInstance()->resolveSymbol("glDisable"))),
      genQueries(reinterpret_cast<PFNGLGENQUERIESPROC>(applicationContextRef->sharedFunctionResolverInstance()->resolveSymbol("glGenQueries"))),
      beginQuery(reinterpret_cast<PFNGLBEGINQUERYPROC>(applicationContextRef->sharedFunctionResolverInstance()->resolveSymbol("glBeginQuery"))),
      endQuery(reinterpret_cast<PFNGLENDQUERYPROC>(applicationContextRef->sharedFunctionResolverInstance()->resolveSymbol("glEndQuery"))),
      getQueryObjectiv(reinterpret_cast<PFNGLGETQUERYOBJECTIVPROC>(applicationContextRef->sharedFunctionResolverInstance()->resolveSymbol("glGetQueryObjectiv"))),
      deleteQueries(reinterpret_cast<PFNGLDELETEQUERIESPROC>(applicationContextRef->sharedFunctionResolverInstance()->resolveSymbol("glDeleteQueries"))),
      m_currentEffectEngineRef(0),
      m_accelerator(accelerator),
      m_applicationContextRef(applicationContextRef),
      m_sceneRef(scene),
      m_modelRef(modelRef),
      m_staticBuffer(0),
      m_dynamicBuffer(0),
      m_matrixBuffer(0),
      m_indexBuffer(0),
      m_bundle(applicationContextRef->sharedFunctionResolverInstance()),
      m_defaultEffect(0),
      m_overridePass(0),
      m_indexType(kGL_UNSIGNED_INT),
      m_aabbMin(SIMD_INFINITY, SIMD_INFINITY, SIMD_INFINITY),
      m_aabbMax(-SIMD_INFINITY, -SIMD_INFINITY, -SIMD_INFINITY),
      m_isVertexShaderSkinning(m_sceneRef->accelerationType() == Scene::kVertexShaderAccelerationType1),
      m_cullFaceState(true),
      m_updateEvenBuffer(true)
{
    m_modelRef->getIndexBuffer(m_indexBuffer);
    m_modelRef->getStaticVertexBuffer(m_staticBuffer);
    m_modelRef->getDynamicVertexBuffer(m_dynamicBuffer, m_indexBuffer);
    if (m_isVertexShaderSkinning) {
        m_modelRef->getMatrixBuffer(m_matrixBuffer, m_dynamicBuffer, m_indexBuffer);
    }
    switch (m_indexBuffer->type()) {
    case IModel::IndexBuffer::kIndex8:
        m_indexType = kGL_UNSIGNED_BYTE;
        break;
    case IModel::IndexBuffer::kIndex16:
        m_indexType = kGL_UNSIGNED_SHORT;
        break;
    case IModel::IndexBuffer::kIndex32:
    case IModel::IndexBuffer::kMaxIndexType:
    default:
        break;
    }
    for (int i = 0; i < kMaxVertexArrayObjectType; i++) {
        m_layouts[i] = new VertexBundleLayout(applicationContextRef->sharedFunctionResolverInstance());
    }
#ifdef VPVL2_ENABLE_OPENCL
    if (m_isVertexShaderSkinning || (m_accelerator && m_accelerator->isAvailable())) {
#else
    if (m_isVertexShaderSkinning) {
#endif
        m_dynamicBuffer->setSkinningEnable(false);
    }
}

PMXRenderEngine::~PMXRenderEngine()
{
    release();
}

IModel *PMXRenderEngine::parentModelRef() const
{
    return m_modelRef && m_modelRef->parentSceneRef() ? m_modelRef : 0;
}

bool PMXRenderEngine::upload(void *userData)
{
    if (!uploadMaterials(userData)) {
        return false;
    }
    pushAnnotationGroup("PMXRenderEngine#upload", m_applicationContextRef->sharedFunctionResolverInstance());
    m_bundle.create(VertexBundle::kVertexBuffer, kModelDynamicVertexBufferEven, VertexBundle::kGL_DYNAMIC_DRAW, 0, m_dynamicBuffer->size());
    labelVertexBuffer(kModelDynamicVertexBufferEven, "ModelDynamicVertexBufferEven");
    m_bundle.create(VertexBundle::kVertexBuffer, kModelDynamicVertexBufferOdd, VertexBundle::kGL_DYNAMIC_DRAW, 0, m_dynamicBuffer->size());
    labelVertexBuffer(kModelDynamicVertexBufferOdd, "ModelDynamicVertexBufferOdd");
    VPVL2_VLOG(2, "Binding model dynamic vertex buffer to the vertex buffer object: size=" << m_dynamicBuffer->size());
    m_bundle.create(VertexBundle::kVertexBuffer, kModelStaticVertexBuffer, VertexBundle::kGL_STATIC_DRAW, 0, m_staticBuffer->size());
    labelVertexBuffer(kModelStaticVertexBuffer, "ModelStaticVertexBuffer");
    m_bundle.bind(VertexBundle::kVertexBuffer, kModelStaticVertexBuffer);
    void *address = m_bundle.map(VertexBundle::kVertexBuffer, 0, m_staticBuffer->size());
    m_staticBuffer->update(address);
    VPVL2_VLOG(2, "Binding model static vertex buffer to the vertex buffer object: ptr=" << address << " size=" << m_staticBuffer->size());
    m_bundle.unmap(VertexBundle::kVertexBuffer, address);
    m_bundle.unbind(VertexBundle::kVertexBuffer);
    m_bundle.create(VertexBundle::kIndexBuffer, kModelIndexBuffer, VertexBundle::kGL_STATIC_DRAW, m_indexBuffer->bytes(), m_indexBuffer->size());
    labelVertexBuffer(kModelIndexBuffer, "ModelIndexBuffer");
    VPVL2_VLOG(2, "Binding indices to the vertex buffer object: ptr=" << m_indexBuffer->bytes() << " size=" << m_indexBuffer->size());
    VertexBundleLayout *bundleME = m_layouts[kVertexArrayObjectEven];
    if (bundleME->create() && bundleME->bind()) {
        VPVL2_VLOG(2, "Binding an vertex array object for even frame: " << bundleME->name());
        createVertexBundle(kModelDynamicVertexBufferEven);
        labelVertexArray(bundleME, "VertexArrayObjectEven");
    }
    bundleME->unbind();
    VertexBundleLayout *bundleMO = m_layouts[kVertexArrayObjectOdd];
    if (bundleMO->create() && bundleMO->bind()) {
        VPVL2_VLOG(2, "Binding an vertex array object for odd frame: " << bundleMO->name());
        createVertexBundle(kModelDynamicVertexBufferOdd);
        labelVertexArray(bundleMO, "VertexArrayObjectOdd");
    }
    bundleMO->unbind();
    VertexBundleLayout *bundleEE = m_layouts[kEdgeVertexArrayObjectEven];
    if (bundleEE->create() && bundleEE->bind()) {
        VPVL2_VLOG(2, "Binding an edge vertex array object for even frame: " << bundleEE->name());
        createEdgeBundle(kModelDynamicVertexBufferEven);
        labelVertexArray(bundleEE, "EdgeVertexArrayObjectEven");
    }
    bundleEE->unbind();
    VertexBundleLayout *bundleEO = m_layouts[kEdgeVertexArrayObjectOdd];
    if (bundleEO->create() && bundleEO->bind()) {
        VPVL2_VLOG(2, "Binding an edge vertex array object for odd frame: " << bundleEO->name());
        createEdgeBundle(kModelDynamicVertexBufferOdd);
        labelVertexArray(bundleEO, "VertexArrayObjectEven");
    }
    bundleEO->unbind();
    m_bundle.unbind(VertexBundle::kVertexBuffer);
    m_bundle.unbind(VertexBundle::kIndexBuffer);
#ifdef VPVL2_ENABLE_OPENCL
    if (m_accelerator && m_accelerator->isAvailable()) {
        m_accelerator->release(m_accelerationBuffers);
        m_accelerationBuffers.append(cl::PMXAccelerator::VertexBufferBridge(m_bundle.findName(kModelDynamicVertexBufferEven)));
        m_accelerationBuffers.append(cl::PMXAccelerator::VertexBufferBridge(m_bundle.findName(kModelDynamicVertexBufferOdd)));
        m_accelerator->upload(m_accelerationBuffers, m_indexBuffer);
    }
#endif
    m_sceneRef->updateModel(m_modelRef);
    m_modelRef->setVisible(true);
    update(); // for updating even frame
    update(); // for updating odd frame
    popAnnotationGroup(m_applicationContextRef->sharedFunctionResolverInstance());
    VPVL2_VLOG(2, "Created the model: " << internal::cstr(m_modelRef->name(IEncoding::kDefaultLanguage), "(null)"));
    return true;
}

void PMXRenderEngine::update()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_currentEffectEngineRef) {
        return;
    }
    pushAnnotationGroup("PMXRenderEngine#update", m_applicationContextRef->sharedFunctionResolverInstance());
    VertexBufferObjectType vbo = m_updateEvenBuffer ? kModelDynamicVertexBufferEven : kModelDynamicVertexBufferOdd;
    annotate("update: model=%s type=%d", m_modelRef->name(IEncoding::kDefaultLanguage)->toByteArray(), vbo);
    m_bundle.bind(VertexBundle::kVertexBuffer, vbo);
    if (void *address = m_bundle.map(VertexBundle::kVertexBuffer, 0, m_dynamicBuffer->size())) {
        m_dynamicBuffer->update(address, m_sceneRef->cameraRef()->position(), m_aabbMin, m_aabbMax);
        if (m_isVertexShaderSkinning) {
            m_matrixBuffer->update(address);
        }
        m_bundle.unmap(VertexBundle::kVertexBuffer, address);
    }
    m_bundle.unbind(VertexBundle::kVertexBuffer);
#ifdef VPVL2_ENABLE_OPENCL
    if (m_accelerator && m_accelerator->isAvailable()) {
        const cl::PMXAccelerator::VertexBufferBridge &buffer = m_accelerationBuffers[m_updateEvenBuffer ? 0 : 1];
        m_accelerator->update(m_dynamicBuffer, buffer, m_aabbMin, m_aabbMax);
    }
#endif
    m_modelRef->setAabb(m_aabbMin, m_aabbMax);
    m_currentEffectEngineRef->updateModelLightParameters(m_sceneRef, m_modelRef);
    m_currentEffectEngineRef->updateSceneParameters();
    m_updateEvenBuffer = m_updateEvenBuffer ? false :true;
    if (m_currentEffectEngineRef) {
        m_currentEffectEngineRef->parthf.setValue(false);
        m_currentEffectEngineRef->transp.setValue(false);
        m_currentEffectEngineRef->opadd.setValue(false);
        m_currentEffectEngineRef->vertexCount.setValue(m_modelRef->count(IModel::kVertex));
        m_currentEffectEngineRef->subsetCount.setValue(m_modelRef->count(IModel::kMaterial));
        m_currentEffectEngineRef->setModelMatrixParameters(m_modelRef);
    }
    popAnnotationGroup(m_applicationContextRef->sharedFunctionResolverInstance());
}

void PMXRenderEngine::setUpdateOptions(int options)
{
    m_dynamicBuffer->setParallelUpdateEnable(internal::hasFlagBits(options, kParallelUpdate));
}

void PMXRenderEngine::renderModel()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_currentEffectEngineRef || !m_currentEffectEngineRef->isStandardEffect()) {
        return;
    }
    pushAnnotationGroup("PMXRenderEngine#renderModel", m_applicationContextRef->sharedFunctionResolverInstance());
    m_currentEffectEngineRef->setModelMatrixParameters(m_modelRef);
    const Scalar &modelOpacity = m_modelRef->opacity();
    const bool hasModelTransparent = !btFuzzyZero(modelOpacity - 1.0f);
    Array<IMaterial *> materials;
    m_modelRef->getMaterialRefs(materials);
    const int nmaterials = materials.count();
    bool hasShadowMap = false;
    if (const IShadowMap *shadowMap = m_sceneRef->shadowMapRef()) {
        const void *textureRef = shadowMap->textureRef();
        const GLuint depthTexture = *static_cast<const GLuint *>(textureRef);
        m_currentEffectEngineRef->depthTexture.setTexture(depthTexture);
        m_currentEffectEngineRef->selfShadow.updateParameter(shadowMap);
        hasShadowMap = true;
    }
    bindVertexBundle();
    EffectEngine::DrawPrimitiveCommand command;
    getDrawPrimitivesCommand(command);
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = materials[i];
        const char *const target = hasShadowMap && material->isSelfShadowEnabled() ? "object_ss" : "object";
        const MaterialContext &materialContext = m_materialContexts[i];
        IMaterial::SphereTextureRenderMode renderMode;
        bool hasMainTexture, hasSphereMap;
        updateMaterialParameters(material, materialContext, i, renderMode, hasMainTexture, hasSphereMap);
        if (IEffect::Technique *technique = m_currentEffectEngineRef->findTechnique(target, i, nmaterials, hasMainTexture, hasSphereMap, true)) {
            updateDrawPrimitivesCommand(material, command);
            if (!hasModelTransparent && m_cullFaceState && material->isCullingDisabled()) {
                disable(kGL_CULL_FACE);
                m_cullFaceState = false;
            }
            else if (!m_cullFaceState && !material->isCullingDisabled()) {
                enable(kGL_CULL_FACE);
                m_cullFaceState = true;
            }
            bool rendering;
            technique->setOverridePass(m_overridePass, rendering);
            if (rendering) {
                annotateMaterial("renderModel", material);
                pushAnnotationGroup("PMXRenderEngine::PrivateEffectEngine#executeTechniquePasses", m_applicationContextRef->sharedFunctionResolverInstance());
                m_currentEffectEngineRef->executeTechniquePasses(technique, command, 0);
                popAnnotationGroup(m_applicationContextRef->sharedFunctionResolverInstance());
            }
        }
        command.offset += command.count;
    }
    unbindVertexBundle();
    if (!m_cullFaceState) {
        enable(kGL_CULL_FACE);
        m_cullFaceState = true;
    }
    popAnnotationGroup(m_applicationContextRef->sharedFunctionResolverInstance());
}

void PMXRenderEngine::renderEdge()
{
    if (!m_modelRef || !m_modelRef->isVisible() || btFuzzyZero(m_modelRef->edgeWidth())
            || !m_currentEffectEngineRef || m_currentEffectEngineRef->scriptOrder() != IEffect::kStandard) {
        return;
    }
    pushAnnotationGroup("PMXRenderEngine#renderEdge", m_applicationContextRef->sharedFunctionResolverInstance());
    m_currentEffectEngineRef->setModelMatrixParameters(m_modelRef);
    m_currentEffectEngineRef->setZeroGeometryParameters(m_modelRef);
    Array<IMaterial *> materials;
    m_modelRef->getMaterialRefs(materials);
    const int nmaterials = materials.count();
    cullFace(kGL_FRONT);
    bindEdgeBundle();
    EffectEngine::DrawPrimitiveCommand command;
    getDrawPrimitivesCommand(command);
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = materials[i];
        const int nindices = material->indexRange().count;
        if (material->isEdgeEnabled()) {
            if (IEffect::Technique *technique = m_currentEffectEngineRef->findTechnique("edge", i, nmaterials, false, false, true)) {
                bool rendering;
                technique->setOverridePass(m_overridePass, rendering);
                if (rendering) {
                    updateDrawPrimitivesCommand(material, command);
                    annotateMaterial("renderEdge", material);
                    m_currentEffectEngineRef->edgeColor.setGeometryColor(material->edgeColor());
                    pushAnnotationGroup("PMXRenderEngine::PrivateEffectEngine#executeTechniquePasses", m_applicationContextRef->sharedFunctionResolverInstance());
                    m_currentEffectEngineRef->executeTechniquePasses(technique, command, 0);
                    popAnnotationGroup(m_applicationContextRef->sharedFunctionResolverInstance());
                }
            }
        }
        command.offset += nindices;
    }
    unbindVertexBundle();
    cullFace(kGL_BACK);
    popAnnotationGroup(m_applicationContextRef->sharedFunctionResolverInstance());
}

void PMXRenderEngine::renderShadow()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_currentEffectEngineRef || m_currentEffectEngineRef->scriptOrder() != IEffect::kStandard) {
        return;
    }
    pushAnnotationGroup("PMXRenderEngine#renderShadow", m_applicationContextRef->sharedFunctionResolverInstance());
    m_currentEffectEngineRef->setModelMatrixParameters(m_modelRef, IApplicationContext::kShadowMatrix);
    m_currentEffectEngineRef->setZeroGeometryParameters(m_modelRef);
    Array<IMaterial *> materials;
    m_modelRef->getMaterialRefs(materials);
    const int nmaterials = materials.count();
    cullFace(kGL_FRONT);
    bindVertexBundle();
    EffectEngine::DrawPrimitiveCommand command;
    getDrawPrimitivesCommand(command);
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = materials[i];
        const int nindices = material->indexRange().count;
        if (material->hasShadow()) {
            if (IEffect::Technique *technique = m_currentEffectEngineRef->findTechnique("shadow", i, nmaterials, false, false, true)) {
                bool rendering;
                technique->setOverridePass(m_overridePass, rendering);
                if (rendering) {
                    IMaterial::SphereTextureRenderMode renderMode;
                    bool hasMainTexture, hasSphereMap;
                    updateDrawPrimitivesCommand(material, command);
                    updateMaterialParameters(material, m_materialContexts[i], i, renderMode, hasMainTexture, hasSphereMap);
                    annotateMaterial("renderShadow", material);
                    pushAnnotationGroup("PMXRenderEngine::PrivateEffectEngine#executeTechniquePasses", m_applicationContextRef->sharedFunctionResolverInstance());
                    m_currentEffectEngineRef->executeTechniquePasses(technique, command, 0);
                    popAnnotationGroup(m_applicationContextRef->sharedFunctionResolverInstance());
                }
            }
        }
        command.offset += nindices;
    }
    unbindVertexBundle();
    cullFace(kGL_BACK);
    popAnnotationGroup(m_applicationContextRef->sharedFunctionResolverInstance());
}

void PMXRenderEngine::renderZPlot()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_currentEffectEngineRef || m_currentEffectEngineRef->scriptOrder() != IEffect::kStandard) {
        return;
    }
    pushAnnotationGroup("PMXRenderEngine#renderZPlot", m_applicationContextRef->sharedFunctionResolverInstance());
    m_currentEffectEngineRef->setModelMatrixParameters(m_modelRef);
    m_currentEffectEngineRef->setZeroGeometryParameters(m_modelRef);
    Array<IMaterial *> materials;
    m_modelRef->getMaterialRefs(materials);
    const int nmaterials = materials.count();
    disable(kGL_CULL_FACE);
    bindVertexBundle();
    EffectEngine::DrawPrimitiveCommand command;
    getDrawPrimitivesCommand(command);
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = materials[i];
        const int nindices = material->indexRange().count;
        if (material->hasShadowMap()) {
            if (IEffect::Technique *technique = m_currentEffectEngineRef->findTechnique("zplot", i, nmaterials, false, false, true)) {
                bool rendering;
                technique->setOverridePass(m_overridePass, rendering);
                if (rendering) {
                    IMaterial::SphereTextureRenderMode renderMode;
                    bool hasMainTexture, hasSphereMap;
                    updateDrawPrimitivesCommand(material, command);
                    updateMaterialParameters(material, m_materialContexts[i], i, renderMode, hasMainTexture, hasSphereMap);
                    annotateMaterial("renderZplot", material);
                    pushAnnotationGroup("PMXRenderEngine::PrivateEffectEngine#executeTechniquePasses", m_applicationContextRef->sharedFunctionResolverInstance());
                    m_currentEffectEngineRef->executeTechniquePasses(technique, command, 0);
                    popAnnotationGroup(m_applicationContextRef->sharedFunctionResolverInstance());
                }
            }
        }
        command.offset += nindices;
    }
    unbindVertexBundle();
    enable(kGL_CULL_FACE);
    popAnnotationGroup(m_applicationContextRef->sharedFunctionResolverInstance());
}

bool PMXRenderEngine::hasPreProcess() const
{
    return m_currentEffectEngineRef ? m_currentEffectEngineRef->hasTechniques(IEffect::kPreProcess) : false;
}

bool PMXRenderEngine::hasPostProcess() const
{
    return m_currentEffectEngineRef ? m_currentEffectEngineRef->hasTechniques(IEffect::kPostProcess) : false;
}

void PMXRenderEngine::preparePostProcess()
{
    if (m_currentEffectEngineRef) {
        pushAnnotationGroup("PMXRenderEngine#preparePostProcess", m_applicationContextRef->sharedFunctionResolverInstance());
        m_currentEffectEngineRef->executeScriptExternal();
        popAnnotationGroup(m_applicationContextRef->sharedFunctionResolverInstance());
    }
}

void PMXRenderEngine::performPreProcess()
{
    if (m_currentEffectEngineRef) {
        pushAnnotationGroup("PMXRenderEngine#performPreProcess", m_applicationContextRef->sharedFunctionResolverInstance());
        m_currentEffectEngineRef->executeProcess(m_modelRef, 0, IEffect::kPreProcess);
        popAnnotationGroup(m_applicationContextRef->sharedFunctionResolverInstance());
    }
}

void PMXRenderEngine::performPostProcess(IEffect *nextPostEffect)
{
    if (m_currentEffectEngineRef) {
        pushAnnotationGroup("PMXRenderEngine#performPostProcess", m_applicationContextRef->sharedFunctionResolverInstance());
        m_currentEffectEngineRef->executeProcess(m_modelRef, nextPostEffect, IEffect::kPostProcess);
        popAnnotationGroup(m_applicationContextRef->sharedFunctionResolverInstance());
    }
}

IEffect *PMXRenderEngine::effectRef(IEffect::ScriptOrderType type) const
{
    if (type == IEffect::kDefault) {
        return m_defaultEffect;
    }
    else {
        const PrivateEffectEngine *const *ee = m_effectEngines.find(type);
        return ee ? (*ee)->effect() : 0;
    }
}

void PMXRenderEngine::setEffect(IEffect *effectRef, IEffect::ScriptOrderType type, void *userData)
{
    pushAnnotationGroup("PMXRenderEngine#setEffect", m_applicationContextRef->sharedFunctionResolverInstance());
    if (type == IEffect::kStandardOffscreen) {
        const int neffects = m_oseffects.count();
        bool found = false;
        PrivateEffectEngine *ee = 0;
        for (int i = 0; i < neffects; i++) {
            ee = m_oseffects[i];
            if (ee->effect() == effectRef) {
                found = true;
                break;
            }
        }
        if (found) {
            m_currentEffectEngineRef = ee;
        }
        else if (effectRef) {
            PrivateEffectEngine *previous = m_currentEffectEngineRef;
            m_currentEffectEngineRef = new PrivateEffectEngine(this, m_applicationContextRef->sharedFunctionResolverInstance());
            m_currentEffectEngineRef->setEffect(effectRef, userData, false);
            if (m_currentEffectEngineRef->scriptOrder() == IEffect::kStandard) {
                Array<IMaterial *> materials;
                m_modelRef->getMaterialRefs(materials);
                const int nmaterials = materials.count();
                /* copy current material textures/spheres parameters to offscreen effect */
                for (int i = 0; i < nmaterials; i++) {
                    const IMaterial *material = materials[i];
                    const MaterialContext &materialContext = m_materialContexts[i];
                    if (const ITexture *mainTexture = materialContext.mainTextureRef) {
                        m_currentEffectEngineRef->materialTexture.setTexture(material, mainTexture);
                    }
                    if (const ITexture *toonTexture = materialContext.toonTextureRef) {
                        m_currentEffectEngineRef->materialToonTexture.setTexture(material, toonTexture);
                    }
                    if (const ITexture *sphereTexture = materialContext.sphereTextureRef) {
                        m_currentEffectEngineRef->materialSphereMap.setTexture(material, sphereTexture);
                    }
                }
                m_oseffects.append(m_currentEffectEngineRef);
            }
            else {
                internal::deleteObject(m_currentEffectEngineRef);
                m_currentEffectEngineRef = previous;
            }
        }
    }
    else {
        IEffect::ScriptOrderType findType = (type == IEffect::kAutoDetection && effectRef) ? effectRef->scriptOrderType() : type;
        if (PrivateEffectEngine *const *ee = m_effectEngines.find(findType)) {
            m_currentEffectEngineRef = *ee;
        }
        else {
            /* set default standard effect (reference) if effect is null */
            bool wasEffectNull = false;
            if (!effectRef) {
                m_defaultEffect = m_sceneRef->createDefaultStandardEffect(m_applicationContextRef);
                effectRef = m_defaultEffect;// static_cast<Effect *>(m_defaultEffect);
                wasEffectNull = true;
            }
            m_currentEffectEngineRef = new PrivateEffectEngine(this, m_applicationContextRef->sharedFunctionResolverInstance());
            m_currentEffectEngineRef->setEffect(effectRef, userData, wasEffectNull);
            m_effectEngines.insert(type == IEffect::kAutoDetection ? m_currentEffectEngineRef->scriptOrder() : type, m_currentEffectEngineRef);
            /* set default standard effect as secondary effect */
            if (!wasEffectNull && m_currentEffectEngineRef->scriptOrder() == IEffect::kStandard) {
                m_defaultEffect = m_sceneRef->createDefaultStandardEffect(m_applicationContextRef);
                m_currentEffectEngineRef->setDefaultStandardEffectRef(m_defaultEffect);
            }
        }
    }
    popAnnotationGroup(m_applicationContextRef->sharedFunctionResolverInstance());
}

void PMXRenderEngine::setOverridePass(IEffect::Pass *pass)
{
    m_overridePass = pass;
}

bool PMXRenderEngine::testVisible()
{
    const IApplicationContext::FunctionResolver *resolver = m_applicationContextRef->sharedFunctionResolverInstance();
    pushAnnotationGroup("PMXRenderEngine#testVisible", resolver);
    GLenum target = kGL_NONE;
    bool visible = true;
    if (resolver->hasExtension("ARB_occlusion_query2")) {
        target = kGL_ANY_SAMPLES_PASSED;
    }
    else if (resolver->hasExtension("ARB_occlusion_query")) {
        target = kGL_SAMPLES_PASSED;
    }
    if (target != kGL_NONE) {
        GLuint query = 0;
        genQueries(1, &query);
        beginQuery(target, query);
        renderEdge();
        endQuery(target);
        GLint result = 0;
        getQueryObjectiv(query, kGL_QUERY_RESULT, &result);
        visible = result != 0;
        deleteQueries(1, &query);
    }
    popAnnotationGroup(resolver);
    return visible;
}

void PMXRenderEngine::bindVertexBundle()
{
    pushAnnotationGroup("PMXRenderEngine#bindVertexBundle", m_applicationContextRef->sharedFunctionResolverInstance());
    VertexArrayObjectType vao;
    VertexBufferObjectType vbo;
    getVertexBundleType(vao, vbo);
    m_currentEffectEngineRef->setDrawType(PrivateEffectEngine::kVertex);
    if (!m_layouts[vao]->bind()) {
        m_bundle.bind(VertexBundle::kVertexBuffer, vbo);
        bindDynamicVertexAttributePointers(IModel::Buffer::kVertexStride);
        m_bundle.bind(VertexBundle::kVertexBuffer, kModelStaticVertexBuffer);
        bindStaticVertexAttributePointers();
        m_bundle.bind(VertexBundle::kIndexBuffer, kModelIndexBuffer);
    }
    popAnnotationGroup(m_applicationContextRef->sharedFunctionResolverInstance());
}

void PMXRenderEngine::bindEdgeBundle()
{
    VertexArrayObjectType vao;
    VertexBufferObjectType vbo;
    pushAnnotationGroup("PMXRenderEngine#bindEdgeVertexBundle", m_applicationContextRef->sharedFunctionResolverInstance());
    getEdgeBundleType(vao, vbo);
    m_currentEffectEngineRef->setDrawType(PrivateEffectEngine::kEdge);
    if (!m_layouts[vao]->bind()) {
        m_bundle.bind(VertexBundle::kVertexBuffer, vbo);
        bindDynamicVertexAttributePointers(IModel::Buffer::kEdgeVertexStride);
        m_bundle.bind(VertexBundle::kVertexBuffer, kModelStaticVertexBuffer);
        bindStaticVertexAttributePointers();
        m_bundle.bind(VertexBundle::kIndexBuffer, kModelIndexBuffer);
    }
    popAnnotationGroup(m_applicationContextRef->sharedFunctionResolverInstance());
}

bool PMXRenderEngine::uploadMaterials(void *userData)
{
    pushAnnotationGroup("PMXRenderEngine#uploadMaterials", m_applicationContextRef->sharedFunctionResolverInstance());
    Array<IMaterial *> materials;
    m_modelRef->getMaterialRefs(materials);
    const int nmaterials = materials.count();
    IApplicationContext::TextureDataBridge bridge(IApplicationContext::kTexture2D | IApplicationContext::kAsyncLoadingTexture);
    m_materialContexts.resize(nmaterials);
    EffectEngine *engine = 0;
    if (PrivateEffectEngine *const *enginePtr = m_effectEngines.find(IEffect::kStandard)) {
        engine = *enginePtr;
        if (engine->materialTexture.isMipmapEnabled()) {
            bridge.flags |= IApplicationContext::kGenerateTextureMipmap;
        }
    }
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = materials[i];
        const IString *name = material->name(IEncoding::kJapanese); (void) name;
        const int materialIndex = material->index(); (void) materialIndex;
        MaterialContext &materialPrivate = m_materialContexts[i];
        ITexture *textureRef = 0;
        annotateMaterial("uploadMaterial", material);
        if (const IString *mainTexturePath = material->mainTexture()) {
            if (m_applicationContextRef->uploadTexture(mainTexturePath, bridge, userData)) {
                textureRef = bridge.dataRef;
                materialPrivate.mainTextureRef = m_allocatedTextures.insert(textureRef, textureRef);
                if (engine) {
                    engine->materialTexture.setTexture(material, textureRef);
                    VPVL2_VLOG(2, "Binding the texture as a main texture: material=" << internal::cstr(name, "(null)") << " index=" << materialIndex << " ID=" << bridge.dataRef);
                }
            }
            else {
                VPVL2_LOG(WARNING, "Cannot bind a main texture: material=" << internal::cstr(name, "(null)") << " index=" << materialIndex);
                popAnnotationGroup(m_applicationContextRef->sharedFunctionResolverInstance());
                return false;
            }
        }
        if (const IString *sphereTexturePath = material->sphereTexture()) {
            if (m_applicationContextRef->uploadTexture(sphereTexturePath, bridge, userData)) {
                textureRef = bridge.dataRef;
                materialPrivate.sphereTextureRef = m_allocatedTextures.insert(textureRef, textureRef);
                if (engine) {
                    engine->materialSphereMap.setTexture(material, textureRef);
                    VPVL2_VLOG(2, "Binding the texture as a sphere texture: material=" << internal::cstr(name, "(null)") << " index=" << materialIndex << " ID=" << bridge.dataRef);
                }
            }
            else {
                VPVL2_LOG(WARNING, "Cannot bind a sphere texture: material=" << internal::cstr(name, "(null)") << " index=" << materialIndex);
                popAnnotationGroup(m_applicationContextRef->sharedFunctionResolverInstance());
                return false;
            }
        }
        if (material->isSharedToonTextureUsed()) {
            char buf[16];
            int index = material->toonTextureIndex();
            if (index == 0) {
                internal::snprintf(buf, sizeof(buf), "toon%d.bmp", index);
            }
            else {
                internal::snprintf(buf, sizeof(buf), "toon%02d.bmp", index);
            }
            if (IString *toonTexturePath = m_applicationContextRef->toUnicode(reinterpret_cast<const uint8 *>(buf))) {
                uploadToonTexture(material, toonTexturePath, engine, materialPrivate, true, userData);
                internal::deleteObject(toonTexturePath);
            }
        }
        else if (const IString *toonTexturePath = material->toonTexture()) {
            uploadToonTexture(material, toonTexturePath, engine, materialPrivate, false, userData);
        }
    }
    popAnnotationGroup(m_applicationContextRef->sharedFunctionResolverInstance());
    return true;
}

void PMXRenderEngine::release()
{
    for (int i = 0; i < kMaxVertexArrayObjectType; i++) {
        internal::deleteObject(m_layouts[i]);
    }
    m_allocatedTextures.releaseAll();
    m_effectEngines.releaseAll();
    m_oseffects.releaseAll();
    internal::deleteObject(m_staticBuffer);
    internal::deleteObject(m_dynamicBuffer);
    internal::deleteObject(m_indexBuffer);
    internal::deleteObject(m_defaultEffect);
#ifdef VPVL2_ENABLE_OPENCL
    internal::deleteObject(m_accelerator);
#endif
    m_aabbMin.setZero();
    m_aabbMax.setZero();
    m_currentEffectEngineRef = 0;
    m_applicationContextRef = 0;
    m_sceneRef = 0;
    m_modelRef = 0;
    m_accelerator = 0;
    m_cullFaceState = false;
}

void PMXRenderEngine::createVertexBundle(GLuint dvbo)
{
    pushAnnotationGroup("PMXRenderEngine#createVertexBundle", m_applicationContextRef->sharedFunctionResolverInstance());
    annotate("createVertexBundle: model=%s dvbo=%i", m_modelRef->name(IEncoding::kDefaultLanguage)->toByteArray(), dvbo);
    m_bundle.bind(VertexBundle::kVertexBuffer, dvbo);
    bindDynamicVertexAttributePointers(IModel::Buffer::kVertexStride);
    IEffect *effectRef = m_currentEffectEngineRef->effect();
    int maxNumVertexAttributes = IEffect::kUVA1VertexAttribute + m_modelRef->maxUVCount();
    for (int i = IEffect::kUVA1VertexAttribute; i < maxNumVertexAttributes; i++) {
        IEffect::VertexAttributeType attribType = static_cast<IEffect::VertexAttributeType>(int(IModel::DynamicVertexBuffer::kUVA1Stride) + i);
        effectRef->activateVertexAttribute(attribType);
    }
    m_bundle.bind(VertexBundle::kVertexBuffer, kModelStaticVertexBuffer);
    bindStaticVertexAttributePointers();
    m_bundle.bind(VertexBundle::kIndexBuffer, kModelIndexBuffer);
    unbindVertexBundle();
    popAnnotationGroup(m_applicationContextRef->sharedFunctionResolverInstance());
}

void PMXRenderEngine::createEdgeBundle(GLuint dvbo)
{
    pushAnnotationGroup("PMXRenderEngine#createEdgeBundle", m_applicationContextRef->sharedFunctionResolverInstance());
    annotate("createEdgeBundle: model=%s dvbo=%i", m_modelRef->name(IEncoding::kDefaultLanguage)->toByteArray(), dvbo);
    m_bundle.bind(VertexBundle::kVertexBuffer, dvbo);
    bindDynamicVertexAttributePointers(IModel::Buffer::kEdgeVertexStride);
    IEffect *effectRef = m_currentEffectEngineRef->effect();
    int maxNumVertexAttributes = IEffect::kUVA1VertexAttribute + m_modelRef->maxUVCount();
    for (int i = IEffect::kUVA1VertexAttribute; i < maxNumVertexAttributes; i++) {
        IEffect::VertexAttributeType attribType = static_cast<IEffect::VertexAttributeType>(int(IModel::DynamicVertexBuffer::kUVA1Stride) + i);
        effectRef->activateVertexAttribute(attribType);
    }
    m_bundle.bind(VertexBundle::kIndexBuffer, kModelIndexBuffer);
    unbindVertexBundle();
    popAnnotationGroup(m_applicationContextRef->sharedFunctionResolverInstance());
}

void PMXRenderEngine::unbindVertexBundle()
{
    VertexArrayObjectType vao;
    VertexBufferObjectType vbo;
    getVertexBundleType(vao, vbo);
    if (!m_layouts[vao]->unbind()) {
        IEffect *effectRef = m_currentEffectEngineRef->effect();
        for (int i = 0; i < int(IEffect::kUVA1VertexAttribute); i++) {
            effectRef->deactivateVertexAttribute(static_cast<IEffect::VertexAttributeType>(i));
        }
        int maxNumVertexAttributes = IEffect::kUVA1VertexAttribute + m_modelRef->maxUVCount();
        for (int i = IEffect::kUVA1VertexAttribute; i < maxNumVertexAttributes; i++) {
            IEffect::VertexAttributeType attribType = static_cast<IEffect::VertexAttributeType>(int(IModel::DynamicVertexBuffer::kUVA1Stride) + i);
            effectRef->deactivateVertexAttribute(attribType);
        }
        m_bundle.unbind(VertexBundle::kVertexBuffer);
        m_bundle.unbind(VertexBundle::kIndexBuffer);
    }
}

void PMXRenderEngine::bindDynamicVertexAttributePointers(IModel::IndexBuffer::StrideType type)
{
    pushAnnotationGroup("PMXRenderEngine#bindDynamicVertexAttributePointers", m_applicationContextRef->sharedFunctionResolverInstance());
    const vsize size = m_dynamicBuffer->strideSize();
    vsize offset = m_dynamicBuffer->strideOffset(type);
    IEffect *effectRef = m_currentEffectEngineRef->effect();
    effectRef->setVertexAttributePointer(IEffect::kPositionVertexAttribute, IEffect::Parameter::kFloat4, size, reinterpret_cast<const GLvoid *>(offset));
    effectRef->activateVertexAttribute(IEffect::kPositionVertexAttribute);
    offset = m_dynamicBuffer->strideOffset(IModel::DynamicVertexBuffer::kNormalStride);
    effectRef->setVertexAttributePointer(IEffect::kNormalVertexAttribute, IEffect::Parameter::kFloat4, size, reinterpret_cast<const GLvoid *>(offset));
    effectRef->activateVertexAttribute(IEffect::kNormalVertexAttribute);
    int maxNumVertexAttributes = IEffect::kUVA1VertexAttribute + m_modelRef->maxUVCount();
    for (int i = IEffect::kUVA1VertexAttribute; i < maxNumVertexAttributes; i++) {
        IEffect::VertexAttributeType attribType = static_cast<IEffect::VertexAttributeType>(int(IModel::DynamicVertexBuffer::kUVA1Stride) + i);
        IModel::Buffer::StrideType strideType = static_cast<IModel::Buffer::StrideType>(int(IModel::Buffer::kUVA1Stride) + i);
        offset = m_dynamicBuffer->strideOffset(strideType);
        effectRef->setVertexAttributePointer(attribType, IEffect::Parameter::kFloat4, size, reinterpret_cast<const GLvoid *>(offset));
        effectRef->activateVertexAttribute(attribType);
    }
    popAnnotationGroup(m_applicationContextRef->sharedFunctionResolverInstance());
}

void PMXRenderEngine::bindStaticVertexAttributePointers()
{
    pushAnnotationGroup("PMXRenderEngine#bindStaticVertexAttributePointers", m_applicationContextRef->sharedFunctionResolverInstance());
    const vsize size = m_staticBuffer->strideSize();
    vsize offset = m_staticBuffer->strideOffset(IModel::StaticVertexBuffer::kTextureCoordStride);
    IEffect *effectRef = m_currentEffectEngineRef->effect();
    effectRef->setVertexAttributePointer(IEffect::kTextureCoordVertexAttribute, IEffect::Parameter::kFloat4, size, reinterpret_cast<const GLvoid *>(offset));
    effectRef->activateVertexAttribute(IEffect::kTextureCoordVertexAttribute);
    if (m_isVertexShaderSkinning) {
        offset = m_staticBuffer->strideOffset(IModel::StaticVertexBuffer::kBoneIndexStride);
        effectRef->setVertexAttributePointer(IEffect::kBoneIndexVertexAttribute, IEffect::Parameter::kFloat4, size, reinterpret_cast<const GLvoid *>(offset));
        effectRef->activateVertexAttribute(IEffect::kBoneIndexVertexAttribute);
        offset = m_staticBuffer->strideOffset(IModel::StaticVertexBuffer::kBoneWeightStride);
        effectRef->setVertexAttributePointer(IEffect::kBoneWeightVertexAttribute, IEffect::Parameter::kFloat4, size, reinterpret_cast<const GLvoid *>(offset));
        effectRef->activateVertexAttribute(IEffect::kBoneWeightVertexAttribute);
    }
    popAnnotationGroup(m_applicationContextRef->sharedFunctionResolverInstance());
}

void PMXRenderEngine::getVertexBundleType(VertexArrayObjectType &vao, VertexBufferObjectType &vbo) const
{
    if (m_updateEvenBuffer) {
        vao = kVertexArrayObjectOdd;
        vbo = kModelDynamicVertexBufferOdd;
    }
    else {
        vao = kVertexArrayObjectEven;
        vbo = kModelDynamicVertexBufferEven;
    }
}

void PMXRenderEngine::getEdgeBundleType(VertexArrayObjectType &vao, VertexBufferObjectType &vbo) const
{
    if (m_updateEvenBuffer) {
        vao = kEdgeVertexArrayObjectOdd;
        vbo = kModelDynamicVertexBufferOdd;
    }
    else {
        vao = kEdgeVertexArrayObjectEven;
        vbo = kModelDynamicVertexBufferEven;
    }
}

void PMXRenderEngine::getDrawPrimitivesCommand(EffectEngine::DrawPrimitiveCommand &command) const
{
    command.type = m_indexType;
    command.stride = m_indexBuffer->strideSize();
}

void PMXRenderEngine::updateDrawPrimitivesCommand(const IMaterial *material, EffectEngine::DrawPrimitiveCommand &command) const
{
    const IMaterial::IndexRange &range = material->indexRange();
    command.start = range.start;
    command.end = range.end;
    command.count = range.count;
}

void PMXRenderEngine::updateMaterialParameters(const IMaterial *material,
                                               const MaterialContext &context,
                                               int materialIndex,
                                               IMaterial::SphereTextureRenderMode &renderMode,
                                               bool &hasMainTexture,
                                               bool &hasSphereMap)
{
    const Color &toonColor = context.toonTextureColor;
    const Color &diffuse = material->diffuse();
    renderMode = material->sphereTextureRenderMode();
    hasMainTexture = context.mainTextureRef > 0;
    hasSphereMap = context.sphereTextureRef > 0 && renderMode != IMaterial::kNone;
    m_currentEffectEngineRef->ambient.setGeometryColor(diffuse);
    m_currentEffectEngineRef->diffuse.setGeometryColor(diffuse);
    m_currentEffectEngineRef->emissive.setGeometryColor(material->ambient());
    m_currentEffectEngineRef->specular.setGeometryColor(material->specular());
    m_currentEffectEngineRef->specularPower.setGeometryValue(btMax(material->shininess(), 1.0f));
    m_currentEffectEngineRef->toonColor.setGeometryColor(toonColor);
    m_currentEffectEngineRef->edgeColor.setGeometryColor(material->edgeColor());
    m_currentEffectEngineRef->edgeWidth.setValue(material->edgeSize());
    m_currentEffectEngineRef->materialTexture.updateParameter(material);
    m_currentEffectEngineRef->materialToonTexture.updateParameter(material);
    m_currentEffectEngineRef->materialSphereMap.updateParameter(material);
    m_currentEffectEngineRef->spadd.setValue(renderMode == IMaterial::kAddTexture);
    m_currentEffectEngineRef->spsub.setValue(renderMode == IMaterial::kSubTexture);
    m_currentEffectEngineRef->useTexture.setValue(hasMainTexture);
    m_currentEffectEngineRef->useToon.setValue(context.toonTextureRef > 0);
    m_currentEffectEngineRef->useSpheremap.setValue(hasSphereMap);
    if (m_isVertexShaderSkinning) {
        const float32 *ptr = m_matrixBuffer->bytes(materialIndex);
        const size_t size = m_matrixBuffer->size(materialIndex);
        m_currentEffectEngineRef->boneMatrices.setValues(ptr, size);
    }
}

void PMXRenderEngine::uploadToonTexture(const IMaterial *material,
                                        const IString *toonTexturePath,
                                        EffectEngine *engine,
                                        MaterialContext &context,
                                        bool shared,
                                        void *userData)
{
    const char *name = internal::cstr(material->name(IEncoding::kDefaultLanguage), "(null)");
    const int index = material->index();
    m_applicationContextRef->getToonColor(toonTexturePath, context.toonTextureColor, userData);
    const Color &c = context.toonTextureColor; (void) c;
    VPVL2_VLOG(2, "Fetched color from toon texture: material=" << name << " index=" << index << " shared=" << shared << " R=" << c.x() << " G=" << c.y() << " B=" << c.z());
    IApplicationContext::TextureDataBridge bridge(IApplicationContext::kTexture2D | IApplicationContext::kToonTexture | IApplicationContext::kAsyncLoadingTexture);
    ITexture *textureRef = 0;
    if (shared) {
        /* only load system default toon texture */
        bridge.flags |= IApplicationContext::kSystemToonTexture;
    }
    m_applicationContextRef->uploadTexture(toonTexturePath, bridge, userData);
    textureRef = bridge.dataRef;
    if (!textureRef) {
        bridge.flags |= IApplicationContext::kSystemToonTexture;
        m_applicationContextRef->uploadTexture(toonTexturePath, bridge, userData);
        textureRef = bridge.dataRef;
    }
    if (textureRef) {
        context.toonTextureRef = m_allocatedTextures.insert(textureRef, textureRef);
        if (engine) {
            engine->materialToonTexture.setTexture(material, textureRef);
            VPVL2_VLOG(2, "Binding the texture as a toon texture: material=" << name << " index=" << index << " shared=" << shared << " ID=" << bridge.dataRef);
        }
    }
}

void PMXRenderEngine::labelVertexArray(const VertexBundleLayout *layout, const char *name)
{
    char buffer[1024];
    internal::snprintf(buffer, sizeof(buffer), "name=%s model=%s", name, internal::cstr(m_modelRef->name(IEncoding::kDefaultLanguage), "(null)"));
    annotateObject(VertexBundleLayout::kGL_VERTEX_ARRAY, layout->name(), buffer, m_applicationContextRef->sharedFunctionResolverInstance());
}

void PMXRenderEngine::labelVertexBuffer(GLenum key, const char *name)
{
    char buffer[1024];
    internal::snprintf(buffer, sizeof(buffer), "name=%s model=%s", name, internal::cstr(m_modelRef->name(IEncoding::kDefaultLanguage), "(null)"));
    annotateObject(key == VertexBundle::kIndexBuffer ? VertexBundle::kGL_ELEMENT_ARRAY_BUFFER : VertexBundle::kGL_ARRAY_BUFFER, m_bundle.findName(key), buffer, m_applicationContextRef->sharedFunctionResolverInstance());
}

void PMXRenderEngine::annotateMaterial(const char *name, const IMaterial *material)
{
    annotate("%s: model=%s material=%s index=%i", name, internal::cstr(m_modelRef->name(IEncoding::kDefaultLanguage), "(null)"), internal::cstr(material->name(IEncoding::kDefaultLanguage), "(null)"), material->index());
}

void PMXRenderEngine::annotate(const char * const format, ...)
{
    char buffer[1024];
    va_list ap;
    va_start(ap, format);
    vsnprintf(buffer, sizeof(buffer), format, ap);
    va_end(ap);
    annotateString(buffer, m_applicationContextRef->sharedFunctionResolverInstance());
}

} /* namespace fx */
} /* namespace vpvl2 */
