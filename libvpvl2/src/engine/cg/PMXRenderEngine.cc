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

#include "vpvl2/cg/PMXRenderEngine.h"

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/util.h" /* internal::snprintf */

#ifdef VPVL2_ENABLE_OPENCL
#include "vpvl2/cl/Context.h"
#include "vpvl2/cl/PMXAccelerator.h"
#endif

namespace vpvl2
{
namespace cg
{

class PMXRenderEngine::PrivateEffectEngine : public EffectEngine {
public:
    enum DrawType {
        kVertex,
        kEdge
    };

    PrivateEffectEngine(PMXRenderEngine *renderEngine,
                        Effect *effectRef,
                        const IString *dir,
                        bool isDefaultStandardEffect)
        : EffectEngine(renderEngine->sceneRef(),
                       effectRef,
                       renderEngine->renderContextRef(),
                       dir,
                       isDefaultStandardEffect),
          m_parentRenderEngine(renderEngine),
          m_drawType(kVertex)
    {
    }
    ~PrivateEffectEngine()
    {
    }

    void setDrawType(DrawType value) {
        m_drawType = value;
    }

protected:
    void drawPrimitives(const DrawPrimitiveCommand &command) const {
        if (GLEW_ARB_draw_elements_base_vertex) {
            glDrawRangeElementsBaseVertex(command.mode, command.start, command.end, command.count, command.type,
                                          const_cast<uint8_t *>(command.ptr) + command.offset * command.stride, 0);
        }
        else {
            glDrawRangeElements(command.mode, command.start, command.end, command.count,
                                command.type, command.ptr + command.offset * command.stride);
        }
    }
    void rebindVertexBundle() {
        switch (m_drawType) {
        case kVertex:
            m_parentRenderEngine->bindVertexBundle();
            break;
        case kEdge:
            m_parentRenderEngine->bindEdgeBundle();
            break;
        default:
            break;
        }
    }

private:
    PMXRenderEngine *m_parentRenderEngine;
    DrawType m_drawType;

    VPVL2_DISABLE_COPY_AND_ASSIGN(PrivateEffectEngine)
};

PMXRenderEngine::PMXRenderEngine(IRenderContext *renderContextRef,
                                 Scene *scene,
                                 cl::PMXAccelerator *accelerator,
                                 IModel *modelRef)

    : m_renderContextRef(renderContextRef),
      m_sceneRef(scene),
      m_currentEffectEngineRef(0),
      m_accelerator(accelerator),
      m_modelRef(modelRef),
      m_staticBuffer(0),
      m_dynamicBuffer(0),
      m_indexBuffer(0),
      m_materialContexts(0),
      m_bundle(renderContextRef),
      m_indexType(GL_UNSIGNED_INT),
      m_aabbMin(SIMD_INFINITY, SIMD_INFINITY, SIMD_INFINITY),
      m_aabbMax(-SIMD_INFINITY, -SIMD_INFINITY, -SIMD_INFINITY),
      m_cullFaceState(true),
      m_updateEvenBuffer(true),
      m_isVertexShaderSkinning(false)
{
    m_modelRef->getIndexBuffer(m_indexBuffer);
    m_modelRef->getStaticVertexBuffer(m_staticBuffer);
    m_modelRef->getDynamicVertexBuffer(m_dynamicBuffer, m_indexBuffer);
    m_modelRef->getMaterialRefs(m_materials);
    switch (m_indexBuffer->type()) {
    case IModel::IIndexBuffer::kIndex8:
        m_indexType = GL_UNSIGNED_BYTE;
        break;
    case IModel::IIndexBuffer::kIndex16:
        m_indexType = GL_UNSIGNED_SHORT;
        break;
    case IModel::IIndexBuffer::kIndex32:
    case IModel::IIndexBuffer::kMaxIndexType:
    default:
        break;
    }
#ifdef VPVL2_ENABLE_OPENCL
    if (m_accelerator && m_accelerator->isAvailable())
        m_dynamicBuffer->setSkinningEnable(false);
#endif /* VPVL2_ENABLE_OPENCL */
}

PMXRenderEngine::~PMXRenderEngine()
{
    release();
}

IModel *PMXRenderEngine::parentModelRef() const
{
    return m_modelRef && m_modelRef->parentSceneRef() ? m_modelRef : 0;
}

bool PMXRenderEngine::upload(const IString *dir)
{
    void *userData = 0;
    m_renderContextRef->allocateUserData(m_modelRef, userData);
    m_renderContextRef->startProfileSession(IRenderContext::kProfileUploadModelProcess, m_modelRef);
    if (!uploadMaterials(dir, userData)) {
        return releaseUserData0(userData);
    }
    glGenBuffers(kMaxVertexBufferObjectType, m_vertexBufferObjects);
    GLuint dvbo0 = m_vertexBufferObjects[kModelDynamicVertexBufferEven];
    glBindBuffer(GL_ARRAY_BUFFER, dvbo0);
    glBufferData(GL_ARRAY_BUFFER, m_dynamicBuffer->size(), 0, GL_DYNAMIC_DRAW);
    log0(userData, IRenderContext::kLogInfo,
         "Binding model dynamic vertex buffer to the vertex buffer object (ID=%d)", dvbo0);
    GLuint dvbo1 = m_vertexBufferObjects[kModelDynamicVertexBufferOdd];
    glBindBuffer(GL_ARRAY_BUFFER, dvbo1);
    glBufferData(GL_ARRAY_BUFFER, m_dynamicBuffer->size(), 0, GL_DYNAMIC_DRAW);
    log0(userData, IRenderContext::kLogInfo,
         "Binding model dynamic vertex buffer to the vertex buffer object (ID=%d)", dvbo1);
    GLuint svbo = m_vertexBufferObjects[kModelStaticVertexBuffer];
    glBindBuffer(GL_ARRAY_BUFFER, svbo);
    glBufferData(GL_ARRAY_BUFFER, m_staticBuffer->size(), 0, GL_STATIC_DRAW);
    void *address = m_bundle.mapBuffer(GL_ARRAY_BUFFER, 0, m_staticBuffer->size());
    m_staticBuffer->update(address);
    m_bundle.unmapBuffer(GL_ARRAY_BUFFER, address);
    log0(userData, IRenderContext::kLogInfo,
         "Binding model static vertex buffer to the vertex buffer object (ID=%d)", svbo);
    GLuint ibo = m_vertexBufferObjects[kModelIndexBuffer];
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer->size(), m_indexBuffer->bytes(), GL_STATIC_DRAW);
    log0(userData, IRenderContext::kLogInfo, "Binding indices to the vertex buffer object (ID=%d)", ibo);
    m_bundle.allocateVertexArrayObjects(m_vertexArrayObjects, kMaxVertexArrayObjectType);
    GLuint vao = m_vertexArrayObjects[kVertexArrayObjectEven];
    if (m_bundle.bindVertexArrayObject(vao)) {
        log0(userData, IRenderContext::kLogInfo, "Binding an vertex array object for even frame (ID=%d)", vao);
    }
    createVertexBundle(dvbo0, svbo, ibo);
    vao = m_vertexArrayObjects[kVertexArrayObjectOdd];
    if (m_bundle.bindVertexArrayObject(vao)) {
        log0(userData, IRenderContext::kLogInfo, "Binding an vertex array object for odd frame (ID=%d)", vao);
        createVertexBundle(dvbo1, svbo, ibo);
    }
    vao = m_vertexArrayObjects[kEdgeVertexArrayObjectEven];
    if (m_bundle.bindVertexArrayObject(vao)) {
        log0(userData, IRenderContext::kLogInfo, "Binding an edge vertex array object for even frame (ID=%d)", vao);
    }
    createEdgeBundle(dvbo0, svbo, ibo);
    vao = m_vertexArrayObjects[kEdgeVertexArrayObjectOdd];
    if (m_bundle.bindVertexArrayObject(vao)) {
        log0(userData, IRenderContext::kLogInfo, "Binding an edge vertex array object for odd frame (ID=%d)", vao);
        createEdgeBundle(dvbo1, svbo, ibo);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#ifdef VPVL2_ENABLE_OPENCL
    if (m_accelerator && m_accelerator->isAvailable()) {
        m_accelerator->release(m_accelerationBuffers);
        m_accelerationBuffers.add(cl::PMXAccelerator::Buffer(m_vertexBufferObjects[kModelDynamicVertexBufferEven]));
        m_accelerationBuffers.add(cl::PMXAccelerator::Buffer(m_vertexBufferObjects[kModelDynamicVertexBufferOdd]));
        m_accelerator->upload(m_accelerationBuffers, m_indexBuffer, userData);
    }
#endif
    m_sceneRef->updateModel(m_modelRef);
    m_modelRef->setVisible(true);
    update(); // for updating even frame
    update(); // for updating odd frame
    log0(userData, IRenderContext::kLogInfo, "Created the model: %s", m_modelRef->name()->toByteArray());
    m_renderContextRef->stopProfileSession(IRenderContext::kProfileUploadModelProcess, m_modelRef);
    m_renderContextRef->releaseUserData(m_modelRef, userData);
    return true;
}

void PMXRenderEngine::update()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_currentEffectEngineRef)
        return;
    m_renderContextRef->startProfileSession(IRenderContext::kProfileUpdateModelProcess, m_modelRef);
    VertexBufferObjectType vbo = m_updateEvenBuffer
            ? kModelDynamicVertexBufferEven : kModelDynamicVertexBufferOdd;
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObjects[vbo]);
    void *address = m_bundle.mapBuffer(GL_ARRAY_BUFFER, 0, m_dynamicBuffer->size());
    m_modelRef->performUpdate();
    m_dynamicBuffer->update(address, m_sceneRef->camera()->position(), m_aabbMin, m_aabbMax);
    m_bundle.unmapBuffer(GL_ARRAY_BUFFER, address);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
#ifdef VPVL2_ENABLE_OPENCL
    if (m_accelerator && m_accelerator->isAvailable()) {
        const cl::PMXAccelerator::Buffer &buffer = m_accelerationBuffers[m_updateEvenBuffer ? 0 : 1];
        m_accelerator->update(m_dynamicBuffer, m_sceneRef, buffer, m_aabbMin, m_aabbMax);
    }
#endif
    m_modelRef->setAabb(m_aabbMin, m_aabbMax);
    m_currentEffectEngineRef->updateModelGeometryParameters(m_sceneRef, m_modelRef);
    m_currentEffectEngineRef->updateSceneParameters();
    m_updateEvenBuffer = m_updateEvenBuffer ? false :true;
    m_renderContextRef->stopProfileSession(IRenderContext::kProfileUpdateModelProcess, m_modelRef);
    if (m_currentEffectEngineRef) {
        m_currentEffectEngineRef->useToon.setValue(true);
        m_currentEffectEngineRef->parthf.setValue(false);
        m_currentEffectEngineRef->transp.setValue(false);
        m_currentEffectEngineRef->opadd.setValue(false);
        m_currentEffectEngineRef->vertexCount.setValue(m_modelRef->count(IModel::kVertex));
        m_currentEffectEngineRef->subsetCount.setValue(m_modelRef->count(IModel::kMaterial));
        m_currentEffectEngineRef->setModelMatrixParameters(m_modelRef);
    }
}

void PMXRenderEngine::renderModel()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_currentEffectEngineRef || !m_currentEffectEngineRef->isStandardEffect())
        return;
    m_renderContextRef->startProfileSession(IRenderContext::kProfileRenderModelProcess, m_modelRef);
    m_currentEffectEngineRef->setModelMatrixParameters(m_modelRef);
    const Scalar &modelOpacity = m_modelRef->opacity();
    const ILight *light = m_sceneRef->light();
    const GLuint *depthTexturePtr = static_cast<const GLuint *>(light->depthTexture());
    const bool hasModelTransparent = !btFuzzyZero(modelOpacity - 1.0f),
            hasShadowMap = depthTexturePtr ? true : false;
    const int nmaterials = m_materials.count();
    if (depthTexturePtr && light->hasFloatTexture()) {
        const GLuint depthTexture = *depthTexturePtr;
        m_currentEffectEngineRef->depthTexture.setTexture(depthTexturePtr, depthTexture);
    }
    m_currentEffectEngineRef->edgeColor.setGeometryColor(m_modelRef->edgeColor());
    bindVertexBundle();
    EffectEngine::DrawPrimitiveCommand command;
    getDrawPrimitivesCommand(command);
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = m_materials[i];
        const MaterialContext &materialContext = m_materialContexts[i];
        const Color &toonColor = materialContext.toonTextureColor;
        const Color &diffuse = material->diffuse();
        m_currentEffectEngineRef->ambient.setGeometryColor(diffuse);
        m_currentEffectEngineRef->diffuse.setGeometryColor(diffuse);
        m_currentEffectEngineRef->emissive.setGeometryColor(material->ambient());
        m_currentEffectEngineRef->specular.setGeometryColor(material->specular());
        m_currentEffectEngineRef->specularPower.setGeometryValue(btMax(material->shininess(), 1.0f));
        m_currentEffectEngineRef->toonColor.setGeometryColor(toonColor);
        bool hasMainTexture = materialContext.mainTextureID > 0;
        bool hasSphereMap = materialContext.sphereTextureID > 0;
        m_currentEffectEngineRef->materialTexture.updateParameter(material);
        m_currentEffectEngineRef->materialSphereMap.updateParameter(material);
        m_currentEffectEngineRef->spadd.setValue(material->sphereTextureRenderMode() == IMaterial::kAddTexture);
        m_currentEffectEngineRef->useTexture.setValue(hasMainTexture);
        if (!hasModelTransparent && m_cullFaceState && material->isCullFaceDisabled()) {
            glDisable(GL_CULL_FACE);
            m_cullFaceState = false;
        }
        else if (!m_cullFaceState && !material->isCullFaceDisabled()) {
            glEnable(GL_CULL_FACE);
            m_cullFaceState = true;
        }
        const char *const target = hasShadowMap && material->isSelfShadowDrawn() ? "object_ss" : "object";
        CGtechnique technique = m_currentEffectEngineRef->findTechnique(target, i, nmaterials, hasMainTexture, hasSphereMap, true);
        updateDrawPrimitivesCommand(material, command);
        m_renderContextRef->startProfileSession(IRenderContext::kProfileRenderModelMaterialDrawCall, material);
        m_currentEffectEngineRef->executeTechniquePasses(technique, 0, command);
        m_renderContextRef->stopProfileSession(IRenderContext::kProfileRenderModelMaterialDrawCall, material);
        command.offset += command.count;
    }
    unbindVertexBundle();
    if (!m_cullFaceState) {
        glEnable(GL_CULL_FACE);
        m_cullFaceState = true;
    }
    m_renderContextRef->stopProfileSession(IRenderContext::kProfileRenderModelProcess, m_modelRef);
}

void PMXRenderEngine::renderEdge()
{
    if (!m_modelRef || !m_modelRef->isVisible() || btFuzzyZero(m_modelRef->edgeWidth())
            || !m_currentEffectEngineRef || m_currentEffectEngineRef->scriptOrder() != IEffect::kStandard)
        return;
    m_renderContextRef->startProfileSession(IRenderContext::kProfileRenderEdgeProcess, m_modelRef);
    m_currentEffectEngineRef->setModelMatrixParameters(m_modelRef);
    m_currentEffectEngineRef->setZeroGeometryParameters(m_modelRef);
    const int nmaterials = m_materials.count();
    glCullFace(GL_FRONT);
    bindEdgeBundle();
    EffectEngine::DrawPrimitiveCommand command;
    getDrawPrimitivesCommand(command);
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = m_materials[i];
        const int nindices = material->indexRange().count;
        if (material->isEdgeDrawn()) {
            CGtechnique technique = m_currentEffectEngineRef->findTechnique("edge", i, nmaterials, false, false, true);
            updateDrawPrimitivesCommand(material, command);
            m_currentEffectEngineRef->edgeColor.setGeometryColor(material->edgeColor());
            m_renderContextRef->startProfileSession(IRenderContext::kProfileRenderEdgeMateiralDrawCall, material);
            m_currentEffectEngineRef->executeTechniquePasses(technique, 0, command);
            m_renderContextRef->stopProfileSession(IRenderContext::kProfileRenderEdgeMateiralDrawCall, material);
        }
        command.offset += nindices;
    }
    unbindVertexBundle();
    glCullFace(GL_BACK);
    m_renderContextRef->stopProfileSession(IRenderContext::kProfileRenderEdgeProcess, m_modelRef);
}

void PMXRenderEngine::renderShadow()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_currentEffectEngineRef || m_currentEffectEngineRef->scriptOrder() != IEffect::kStandard)
        return;
    m_renderContextRef->startProfileSession(IRenderContext::kProfileRenderShadowProcess, m_modelRef);
    m_currentEffectEngineRef->setModelMatrixParameters(m_modelRef, IRenderContext::kShadowMatrix);
    m_currentEffectEngineRef->setZeroGeometryParameters(m_modelRef);
    const int nmaterials = m_materials.count();
    glCullFace(GL_FRONT);
    bindVertexBundle();
    EffectEngine::DrawPrimitiveCommand command;
    getDrawPrimitivesCommand(command);
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = m_materials[i];
        const int nindices = material->indexRange().count;
        CGtechnique technique = m_currentEffectEngineRef->findTechnique("shadow", i, nmaterials, false, false, true);
        updateDrawPrimitivesCommand(material, command);
        m_renderContextRef->startProfileSession(IRenderContext::kProfileRenderShadowMaterialDrawCall, material);
        m_currentEffectEngineRef->executeTechniquePasses(technique, 0, command);
        m_renderContextRef->stopProfileSession(IRenderContext::kProfileRenderShadowMaterialDrawCall, material);
        command.offset += nindices;
    }
    unbindVertexBundle();
    glCullFace(GL_BACK);
    m_renderContextRef->stopProfileSession(IRenderContext::kProfileRenderShadowProcess, m_modelRef);
}

void PMXRenderEngine::renderZPlot()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_currentEffectEngineRef || m_currentEffectEngineRef->scriptOrder() != IEffect::kStandard)
        return;
    m_renderContextRef->startProfileSession(IRenderContext::kProfileRenderZPlotProcess, m_modelRef);
    m_currentEffectEngineRef->setModelMatrixParameters(m_modelRef);
    m_currentEffectEngineRef->setZeroGeometryParameters(m_modelRef);
    const int nmaterials = m_materials.count();
    glDisable(GL_CULL_FACE);
    bindVertexBundle();
    EffectEngine::DrawPrimitiveCommand command;
    getDrawPrimitivesCommand(command);
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = m_materials[i];
        const int nindices = material->indexRange().count;
        if (material->isShadowMapDrawn()) {
            CGtechnique technique = m_currentEffectEngineRef->findTechnique("zplot", i, nmaterials, false, false, true);
            updateDrawPrimitivesCommand(material, command);
            m_renderContextRef->startProfileSession(IRenderContext::kProfileRenderZPlotMaterialDrawCall, material);
            m_currentEffectEngineRef->executeTechniquePasses(technique, 0, command);
            m_renderContextRef->stopProfileSession(IRenderContext::kProfileRenderZPlotMaterialDrawCall, material);
        }
        command.offset += nindices;
    }
    unbindVertexBundle();
    glEnable(GL_CULL_FACE);
    m_renderContextRef->stopProfileSession(IRenderContext::kProfileRenderZPlotProcess, m_modelRef);
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
    if (m_currentEffectEngineRef)
        m_currentEffectEngineRef->executeScriptExternal();
}

void PMXRenderEngine::performPreProcess()
{
    if (m_currentEffectEngineRef)
        m_currentEffectEngineRef->executeProcess(m_modelRef, 0, IEffect::kPreProcess);
}

void PMXRenderEngine::performPostProcess(IEffect *nextPostEffect)
{
    if (m_currentEffectEngineRef)
        m_currentEffectEngineRef->executeProcess(m_modelRef, nextPostEffect, IEffect::kPostProcess);
}

IEffect *PMXRenderEngine::effect(IEffect::ScriptOrderType type) const
{
    const PrivateEffectEngine *const *ee = m_effectEngines.find(type);
    return ee ? (*ee)->effect() : 0;
}

void PMXRenderEngine::setEffect(IEffect::ScriptOrderType type, IEffect *effect, const IString *dir)
{
    Effect *effectRef = static_cast<Effect *>(effect);
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
            m_currentEffectEngineRef = new PrivateEffectEngine(this, effectRef, dir, false);
            if (m_currentEffectEngineRef->scriptOrder() == IEffect::kStandard) {
                m_oseffects.add(m_currentEffectEngineRef);
            }
            else {
                delete m_currentEffectEngineRef;
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
                effectRef = static_cast<Effect *>(m_sceneRef->createDefaultStandardEffectRef(m_renderContextRef));
                wasEffectNull = true;
            }
            m_currentEffectEngineRef = new PrivateEffectEngine(this, effectRef, dir, wasEffectNull);
            m_effectEngines.insert(type == IEffect::kAutoDetection ? m_currentEffectEngineRef->scriptOrder() : type, m_currentEffectEngineRef);
            /* set default standard effect as secondary effect */
            if (!wasEffectNull && m_currentEffectEngineRef->scriptOrder() == IEffect::kStandard) {
                m_currentEffectEngineRef->setDefaultStandardEffectRef(m_sceneRef->createDefaultStandardEffectRef(m_renderContextRef));
            }
        }
    }
}

void PMXRenderEngine::bindVertexBundle()
{
    VertexArrayObjectType vao;
    VertexBufferObjectType vbo;
    getVertexBundleType(vao, vbo);
    m_currentEffectEngineRef->setDrawType(PrivateEffectEngine::kVertex);
    if (!m_bundle.bindVertexArrayObject(m_vertexArrayObjects[vao])) {
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObjects[vbo]);
        bindDynamicVertexAttributePointers(IModel::IBuffer::kVertexStride);
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObjects[kModelStaticVertexBuffer]);
        bindStaticVertexAttributePointers();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vertexBufferObjects[kModelIndexBuffer]);
    }
}

void PMXRenderEngine::bindEdgeBundle()
{
    VertexArrayObjectType vao;
    VertexBufferObjectType vbo;
    getEdgeBundleType(vao, vbo);
    m_currentEffectEngineRef->setDrawType(PrivateEffectEngine::kEdge);
    if (!m_bundle.bindVertexArrayObject(m_vertexArrayObjects[vao])) {
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObjects[vbo]);
        bindDynamicVertexAttributePointers(IModel::IBuffer::kEdgeVertexStride);
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObjects[kModelStaticVertexBuffer]);
        bindStaticVertexAttributePointers();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vertexBufferObjects[kModelIndexBuffer]);
    }
}

void PMXRenderEngine::log0(void *userData, IRenderContext::LogLevel level, const char *format ...)
{
    va_list ap;
    va_start(ap, format);
    m_renderContextRef->log(userData, level, format, ap);
    va_end(ap);
}

bool PMXRenderEngine::uploadMaterials(const IString *dir, void *userData)
{
    Array<IMaterial *> materials;
    m_modelRef->getMaterialRefs(materials);
    const int nmaterials = materials.count();
    IRenderContext::Texture texture;
    MaterialContext *materialPrivates = m_materialContexts = new MaterialContext[nmaterials];
    int flags = IRenderContext::kTexture2D;
    EffectEngine *engine = 0;
    if (PrivateEffectEngine *const *enginePtr = m_effectEngines.find(IEffect::kStandard)) {
        engine = *enginePtr;
        flags |= engine->materialTexture.isMipmapEnabled() ? IRenderContext::kGenerateTextureMipmap : 0;
    }
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = materials[i];
        MaterialContext &materialPrivate = materialPrivates[i];
        const IString *path = 0;
        GLuint textureID;
        path = material->mainTexture();
        if (path && path->size() > 0) {
            if (m_renderContextRef->uploadTexture(path, dir, flags, texture, userData)) {
                materialPrivate.mainTextureID = textureID = static_cast<GLuint>(texture.object);
                if (engine)
                    engine->materialTexture.setTexture(material, textureID);
                log0(userData, IRenderContext::kLogInfo, "Binding the texture as a main texture (ID=%d)", textureID);
            }
            else {
                return false;
            }
        }
        path = material->sphereTexture();
        if (path && path->size() > 0) {
            if (m_renderContextRef->uploadTexture(path, dir, IRenderContext::kTexture2D, texture, userData)) {
                materialPrivate.sphereTextureID = textureID = static_cast<GLuint>(texture.object);
                if (engine)
                    engine->materialSphereMap.setTexture(material, textureID);
                log0(userData, IRenderContext::kLogInfo, "Binding the texture as a sphere texture (ID=%d)", textureID);
            }
            else {
                return false;
            }
        }
        if (material->isSharedToonTextureUsed()) {
            char buf[16];
            int index = material->toonTextureIndex();
            if (index == 0)
                internal::snprintf(buf, sizeof(buf), "toon%d.bmp", index);
            else
                internal::snprintf(buf, sizeof(buf), "toon%02d.bmp", index);
            IString *s = m_renderContextRef->toUnicode(reinterpret_cast<const uint8_t *>(buf));
            m_renderContextRef->getToonColor(s, dir, materialPrivate.toonTextureColor, userData);
            delete s;
        }
        else {
            path = material->toonTexture();
            if (path) {
                m_renderContextRef->getToonColor(path, dir, materialPrivate.toonTextureColor, userData);
            }
        }
    }
    return true;
}

bool PMXRenderEngine::releaseUserData0(void *userData)
{
    m_renderContextRef->releaseUserData(m_modelRef, userData);
    release();
    return false;
}

void PMXRenderEngine::release()
{
    glDeleteBuffers(kMaxVertexBufferObjectType, m_vertexBufferObjects);
    m_bundle.releaseVertexArrayObjects(m_vertexArrayObjects, kMaxVertexArrayObjectType);
    if (m_materialContexts) {
        Array<IMaterial *> modelMaterials;
        m_modelRef->getMaterialRefs(modelMaterials);
        const int nmaterials = modelMaterials.count();
        for (int i = 0; i < nmaterials; i++) {
            MaterialContext &materialPrivate = m_materialContexts[i];
            glDeleteTextures(1, &materialPrivate.mainTextureID);
            glDeleteTextures(1, &materialPrivate.sphereTextureID);
        }
        delete[] m_materialContexts;
        m_materialContexts = 0;
    }
    delete m_staticBuffer;
    m_staticBuffer = 0;
    delete m_dynamicBuffer;
    m_dynamicBuffer = 0;
    delete m_indexBuffer;
    m_indexBuffer = 0;
#ifdef VPVL2_ENABLE_OPENCL
    delete m_accelerator;
#endif
    m_effectEngines.releaseAll();
    m_oseffects.releaseAll();
    m_aabbMin.setZero();
    m_aabbMax.setZero();
    m_currentEffectEngineRef = 0;
    m_renderContextRef = 0;
    m_sceneRef = 0;
    m_modelRef = 0;
    m_accelerator = 0;
    m_cullFaceState = false;
    m_isVertexShaderSkinning = false;
}

void PMXRenderEngine::createVertexBundle(GLuint dvbo, GLuint svbo, GLuint ibo)
{
    glBindBuffer(GL_ARRAY_BUFFER, dvbo);
    bindDynamicVertexAttributePointers(IModel::IBuffer::kVertexStride);
    glBindBuffer(GL_ARRAY_BUFFER, svbo);
    bindStaticVertexAttributePointers();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    unbindVertexBundle();
}

void PMXRenderEngine::createEdgeBundle(GLuint dvbo, GLuint /* svbo */, GLuint ibo)
{
    glBindBuffer(GL_ARRAY_BUFFER, dvbo);
    bindDynamicVertexAttributePointers(IModel::IBuffer::kEdgeVertexStride);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glEnableClientState(GL_VERTEX_ARRAY);
    unbindVertexBundle();
}

void PMXRenderEngine::unbindVertexBundle()
{
    if (!m_bundle.unbindVertexArrayObject()) {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}

void PMXRenderEngine::bindDynamicVertexAttributePointers(IModel::IIndexBuffer::StrideType type)
{
    size_t offset, size;
    offset = m_dynamicBuffer->strideOffset(type);
    size   = m_dynamicBuffer->strideSize();
    glVertexPointer(3, GL_FLOAT, size, reinterpret_cast<const GLvoid *>(offset));
    offset = m_dynamicBuffer->strideOffset(IModel::IDynamicVertexBuffer::kNormalStride);
    glNormalPointer(GL_FLOAT, size, reinterpret_cast<const GLvoid *>(offset));
}

void PMXRenderEngine::bindStaticVertexAttributePointers()
{
    size_t offset, size;
    offset = m_staticBuffer->strideOffset(IModel::IStaticVertexBuffer::kTextureCoordStride);
    size   = m_staticBuffer->strideSize();
    glTexCoordPointer(2, GL_FLOAT, size, reinterpret_cast<const GLvoid *>(offset));
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

} /* namespace gl2 */
} /* namespace vpvl2 */
