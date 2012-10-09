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

#include "vpvl2/cg/PMXRenderEngine.h"

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/util.h" /* internal::snprintf */
#include "vpvl2/pmx/Material.h"
#include "vpvl2/pmx/Model.h"
#include "vpvl2/pmx/Vertex.h"

#ifdef VPVL2_ENABLE_OPENCL
#include "vpvl2/cl/Context.h"
#include "vpvl2/cl/PMXAccelerator.h"
#endif

namespace vpvl2
{
namespace cg
{

PMXRenderEngine::PMXRenderEngine(IRenderDelegate *delegate,
                                 const Scene *scene,
                                 CGcontext effectContext,
                                 cl::PMXAccelerator *accelerator,
                                 IModel *modelRef)

#ifdef VPVL2_LINK_QT
    : QGLFunctions(),
      #else
    :
      #endif /* VPVL2_LINK_QT */
      m_sceneRef(scene),
      m_delegateRef(delegate),
      m_contextRef(effectContext),
      m_currentRef(0),
      m_accelerator(accelerator),
      m_modelRef(modelRef),
      m_staticBuffer(0),
      m_dynamicBuffer(0),
      m_indexBuffer(0),
      m_materialContexts(0),
      m_indexType(GL_UNSIGNED_INT),
      m_aabbMin(kZeroV3),
      m_aabbMax(kZeroV3),
      m_cullFaceState(true),
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
    if (m_accelerator && m_accelerator->isAvailable())
        m_dynamicBuffer->setSkinningEnable(false);
#ifdef VPVL2_LINK_QT
    initializeGLFunctions();
#endif /* VPVL2_LINK_QT */
}

PMXRenderEngine::~PMXRenderEngine()
{
    release();
}

IModel *PMXRenderEngine::model() const
{
    return m_modelRef;
}

bool PMXRenderEngine::upload(const IString *dir)
{
    void *context = 0;
    m_delegateRef->allocateContext(m_modelRef, context);
    glGenBuffers(kVertexBufferObjectMax, m_vertexBufferObjects);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vertexBufferObjects[kModelIndexBuffer]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer->size(),
                 m_indexBuffer->bytes(), GL_STATIC_DRAW);
    log0(context, IRenderDelegate::kLogInfo,
         "Binding indices to the vertex buffer object (ID=%d)",
         m_vertexBufferObjects[kModelIndexBuffer]);
    GLuint dvbo = m_vertexBufferObjects[kModelDynamicVertexBuffer];
    glBindBuffer(GL_ARRAY_BUFFER, dvbo);
    glBufferData(GL_ARRAY_BUFFER, m_dynamicBuffer->size(),
                 m_dynamicBuffer->bytes(), GL_DYNAMIC_DRAW);
    log0(context, IRenderDelegate::kLogInfo,
         "Binding model dynamic vertex buffer to the vertex buffer object (ID=%d)", dvbo);
    GLuint svbo = m_vertexBufferObjects[kModelStaticVertexBuffer];
    glBindBuffer(GL_ARRAY_BUFFER, svbo);
    glBufferData(GL_ARRAY_BUFFER, m_staticBuffer->size(),
                 m_staticBuffer->bytes(), GL_STATIC_DRAW);
    log0(context, IRenderDelegate::kLogInfo,
         "Binding model static vertex buffer to the vertex buffer object (ID=%d)", svbo);
    Array<IMaterial *> materials;
    m_modelRef->getMaterialRefs(materials);
    const int nmaterials = materials.count();
    IRenderDelegate::Texture texture;
    GLuint textureID = 0;
    MaterialContext *materialPrivates = m_materialContexts = new MaterialContext[nmaterials];
    texture.object = &textureID;
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = materials[i];
        MaterialContext &materialPrivate = materialPrivates[i];
        const IString *path = 0;
        path = material->mainTexture();
        if (path && m_delegateRef->uploadTexture(path, dir, IRenderDelegate::kTexture2D, texture, context)) {
            materialPrivate.mainTextureID = textureID = *static_cast<const GLuint *>(texture.object);
            log0(context, IRenderDelegate::kLogInfo, "Binding the texture as a main texture (ID=%d)", textureID);
        }
        path = material->sphereTexture();
        if (path && m_delegateRef->uploadTexture(path, dir, IRenderDelegate::kTexture2D, texture, context)) {
            materialPrivate.sphereTextureID = textureID = *static_cast<const GLuint *>(texture.object);
            log0(context, IRenderDelegate::kLogInfo, "Binding the texture as a sphere texture (ID=%d)", textureID);
        }
        if (material->isSharedToonTextureUsed()) {
            char buf[16];
            int index = material->toonTextureIndex();
            if (index == 0)
                internal::snprintf(buf, sizeof(buf), "toon%d.bmp", index);
            else
                internal::snprintf(buf, sizeof(buf), "toon%02d.bmp", index);
            IString *s = m_delegateRef->toUnicode(reinterpret_cast<const uint8_t *>(buf));
            m_delegateRef->getToonColor(s, dir, materialPrivate.toonTextureColor, context);
            delete s;
        }
        else {
            path = material->toonTexture();
            if (path) {
                m_delegateRef->getToonColor(path, dir, materialPrivate.toonTextureColor, context);
            }
        }
    }
#ifdef VPVL2_ENABLE_OPENCL
    if (m_accelerator && m_accelerator->isAvailable()) {
        GLuint vbo = m_vertexBufferObjects[kModelDynamicVertexBuffer];
        m_accelerator->upload(vbo, m_indexBuffer, context);
    }
#endif
    m_sceneRef->updateModel(m_modelRef);
    m_modelRef->setVisible(true);
    update();
    log0(context, IRenderDelegate::kLogInfo, "Created the model: %s", m_modelRef->name()->toByteArray());
    m_delegateRef->releaseContext(m_modelRef, context);
    return true;
}

void PMXRenderEngine::update()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_currentRef)
        return;
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObjects[kModelDynamicVertexBuffer]);
    m_modelRef->performUpdate();
    const ICamera *camera = m_sceneRef->camera();
    m_dynamicBuffer->update(camera->position(), m_aabbMin, m_aabbMax);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_dynamicBuffer->size(), m_dynamicBuffer->bytes());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
#ifdef VPVL2_ENABLE_OPENCL
    if (m_accelerator && m_accelerator->isAvailable())
        m_accelerator->update(m_dynamicBuffer, m_sceneRef, m_aabbMin, m_aabbMax);
#endif
    m_currentRef->updateModelGeometryParameters(m_sceneRef, m_modelRef);
    m_currentRef->updateSceneParameters();
}

void PMXRenderEngine::renderModel()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_currentRef || !m_currentRef->validateStandard())
        return;
    m_currentRef->setModelMatrixParameters(m_modelRef);
    const size_t indexStride = m_indexBuffer->strideSize();
    const Scalar &modelOpacity = m_modelRef->opacity();
    const ILight *light = m_sceneRef->light();
    const GLuint *depthTexturePtr = static_cast<const GLuint *>(light->depthTexture());
    const bool hasModelTransparent = !btFuzzyZero(modelOpacity - 1.0f),
            hasShadowMap = depthTexturePtr ? true : false;
    const int nmaterials = m_materials.count();
    if (depthTexturePtr && light->hasFloatTexture()) {
        const GLuint depthTexture = *depthTexturePtr;
        m_currentRef->depthTexture.setTexture(depthTexture);
    }
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObjects[kModelStaticVertexBuffer]);
    size_t offset = m_staticBuffer->strideOffset(IModel::IStaticVertexBuffer::kTextureCoordStride);
    size_t size   = m_staticBuffer->strideSize();
    glTexCoordPointer(2, GL_FLOAT, size, reinterpret_cast<const GLvoid *>(offset));
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    offset = m_dynamicBuffer->strideOffset(IModel::IDynamicVertexBuffer::kVertexStride);
    size   = m_dynamicBuffer->strideSize();
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObjects[kModelDynamicVertexBuffer]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vertexBufferObjects[kModelIndexBuffer]);
    glVertexPointer(3, GL_FLOAT, size, reinterpret_cast<const GLvoid *>(offset));
    glEnableClientState(GL_VERTEX_ARRAY);
    offset = m_dynamicBuffer->strideOffset(IModel::IDynamicVertexBuffer::kNormalStride);
    glNormalPointer(GL_FLOAT, size, reinterpret_cast<const GLvoid *>(offset));
    glEnableClientState(GL_NORMAL_ARRAY);
    m_currentRef->edgeColor.setGeometryColor(m_modelRef->edgeColor());
    offset = 0;
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = m_materials[i];
        const MaterialContext &materialContext = m_materialContexts[i];
        const Color &toonColor = materialContext.toonTextureColor;
        const Color &diffuse = material->diffuse();
        m_currentRef->ambient.setGeometryColor(diffuse);
        m_currentRef->diffuse.setGeometryColor(diffuse);
        m_currentRef->emissive.setGeometryColor(material->ambient());
        m_currentRef->specular.setGeometryColor(material->specular());
        m_currentRef->specularPower.setGeometryValue(btMax(material->shininess(), 1.0f));
        m_currentRef->toonColor.setGeometryColor(toonColor);
        GLuint mainTexture = materialContext.mainTextureID;
        GLuint sphereTexture = materialContext.sphereTextureID;
        bool hasMainTexture = mainTexture > 0;
        bool hasSphereMap = sphereTexture > 0;
        m_currentRef->materialTexture.setTexture(mainTexture);
        m_currentRef->materialSphereMap.setTexture(sphereTexture);
        m_currentRef->spadd.setValue(material->sphereTextureRenderMode() == pmx::Material::kAddTexture);
        m_currentRef->useTexture.setValue(hasMainTexture);
        if ((hasModelTransparent && m_cullFaceState) ||
                (material->isCullFaceDisabled() && m_cullFaceState)) {
            glDisable(GL_CULL_FACE);
            m_cullFaceState = false;
        }
        else if (!m_cullFaceState) {
            glEnable(GL_CULL_FACE);
            m_cullFaceState = true;
        }
        const int nindices = material->indices();
        const char *const target = hasShadowMap && material->isSelfShadowDrawn() ? "object_ss" : "object";
        CGtechnique technique = m_currentRef->findTechnique(target, i, nmaterials, hasMainTexture, hasSphereMap, true);
        m_currentRef->executeTechniquePasses(technique, GL_TRIANGLES, nindices, m_indexType, reinterpret_cast<const GLvoid *>(offset));
        offset += nindices * indexStride;
    }
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    if (!m_cullFaceState) {
        glEnable(GL_CULL_FACE);
        m_cullFaceState = true;
    }
}

void PMXRenderEngine::renderEdge()
{
    if (!m_modelRef || !m_modelRef->isVisible() || btFuzzyZero(m_modelRef->edgeWidth())
            || !m_currentRef || m_currentRef->scriptOrder() != IEffect::kStandard)
        return;
    m_currentRef->setModelMatrixParameters(m_modelRef);
    m_currentRef->setZeroGeometryParameters(m_modelRef);
    const size_t indexStride = m_indexBuffer->strideSize();
    const int nmaterials = m_materials.count();
    size_t offset = m_dynamicBuffer->strideOffset(IModel::IStaticVertexBuffer::kEdgeVertexStride);
    size_t size   = m_dynamicBuffer->strideSize();
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObjects[kModelDynamicVertexBuffer]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vertexBufferObjects[kModelIndexBuffer]);
    glVertexPointer(3, GL_FLOAT, size, reinterpret_cast<const GLvoid *>(offset));
    glEnableClientState(GL_VERTEX_ARRAY);
    glCullFace(GL_FRONT);
    offset = 0;
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = m_materials[i];
        const int nindices = material->indices();
        if (material->isEdgeDrawn()) {
            CGtechnique technique = m_currentRef->findTechnique("edge", i, nmaterials, false, false, true);
            m_currentRef->edgeColor.setGeometryColor(material->edgeColor());
            m_currentRef->executeTechniquePasses(technique, GL_TRIANGLES, nindices, m_indexType, reinterpret_cast<const GLvoid *>(offset));
        }
        offset += nindices * indexStride;
    }
    glCullFace(GL_BACK);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void PMXRenderEngine::renderShadow()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_currentRef || m_currentRef->scriptOrder() != IEffect::kStandard)
        return;
    m_currentRef->setModelMatrixParameters(m_modelRef, IRenderDelegate::kShadowMatrix);
    m_currentRef->setZeroGeometryParameters(m_modelRef);
    const size_t indexStride = m_indexBuffer->strideSize();
    const int nmaterials = m_materials.count();
    size_t offset = m_dynamicBuffer->strideOffset(IModel::IStaticVertexBuffer::kEdgeVertexStride);
    size_t size   = m_dynamicBuffer->strideSize();
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObjects[kModelDynamicVertexBuffer]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vertexBufferObjects[kModelIndexBuffer]);
    glVertexPointer(3, GL_FLOAT, size, reinterpret_cast<const GLvoid *>(offset));
    glEnableClientState(GL_VERTEX_ARRAY);
    glCullFace(GL_FRONT);
    offset = 0;
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = m_materials[i];
        const int nindices = material->indices();
        CGtechnique technique = m_currentRef->findTechnique("shadow", i, nmaterials, false, false, true);
        m_currentRef->executeTechniquePasses(technique, GL_TRIANGLES, nindices, m_indexType, reinterpret_cast<const GLvoid *>(offset));
        offset += nindices * indexStride;
    }
    glCullFace(GL_BACK);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void PMXRenderEngine::renderZPlot()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_currentRef || m_currentRef->scriptOrder() != IEffect::kStandard)
        return;
    m_currentRef->setModelMatrixParameters(m_modelRef);
    m_currentRef->setZeroGeometryParameters(m_modelRef);
    const size_t indexStride = m_indexBuffer->strideSize();
    const int nmaterials = m_materials.count();
    size_t offset = m_dynamicBuffer->strideOffset(IModel::IStaticVertexBuffer::kEdgeVertexStride);
    size_t size   = m_dynamicBuffer->strideSize();
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObjects[kModelDynamicVertexBuffer]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vertexBufferObjects[kModelIndexBuffer]);
    glVertexPointer(3, GL_FLOAT, size, reinterpret_cast<const GLvoid *>(offset));
    glEnableClientState(GL_VERTEX_ARRAY);
    glCullFace(GL_FRONT);
    offset = 0;
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = m_materials[i];
        const int nindices = material->indices();
        if (material->isShadowMapDrawn()) {
            CGtechnique technique = m_currentRef->findTechnique("zplot", i, nmaterials, false, false, true);
            m_currentRef->executeTechniquePasses(technique, GL_TRIANGLES, nindices, m_indexType, reinterpret_cast<const GLvoid *>(offset));
        }
        offset += nindices * indexStride;
    }
    glCullFace(GL_BACK);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

bool PMXRenderEngine::hasPreProcess() const
{
    return m_currentRef ? m_currentRef->hasTechniques(IEffect::kPreProcess) : false;
}

bool PMXRenderEngine::hasPostProcess() const
{
    return m_currentRef ? m_currentRef->hasTechniques(IEffect::kPostProcess) : false;
}

void PMXRenderEngine::preparePostProcess()
{
    if (m_currentRef)
        m_currentRef->executeScriptExternal();
}

void PMXRenderEngine::performPreProcess()
{
    if (m_currentRef)
        m_currentRef->executeProcess(m_modelRef, IEffect::kPreProcess);
}

void PMXRenderEngine::performPostProcess()
{
    if (m_currentRef)
        m_currentRef->executeProcess(m_modelRef, IEffect::kPostProcess);
}

IEffect *PMXRenderEngine::effect(IEffect::ScriptOrderType type) const
{
    const EffectEngine *const *ee = m_effects.find(type);
    return ee ? (*ee)->effect() : 0;
}

void PMXRenderEngine::setEffect(IEffect::ScriptOrderType type, IEffect *effect, const IString *dir)
{
    Effect *einstance = static_cast<Effect *>(effect);
    if (type == IEffect::kStandardOffscreen) {
        const int neffects = m_oseffects.count();
        bool found = false;
        EffectEngine *ee;
        for (int i = 0; i < neffects; i++) {
            ee = m_oseffects[i];
            if (ee->effect() == einstance) {
                found = true;
                break;
            }
        }
        if (found) {
            m_currentRef = ee;
        }
        else if (einstance) {
            EffectEngine *previous = m_currentRef;
            m_currentRef = new EffectEngine(m_sceneRef, dir, einstance, m_delegateRef);
            if (m_currentRef->scriptOrder() == IEffect::kStandard) {
                m_oseffects.add(m_currentRef);
            }
            else {
                delete m_currentRef;
                m_currentRef = previous;
            }
        }
    }
    else {
        EffectEngine **ee = const_cast<EffectEngine **>(m_effects.find(type));
        if (ee) {
            m_currentRef = *ee;
        }
        else if (einstance) {
            m_currentRef = new EffectEngine(m_sceneRef, dir, einstance, m_delegateRef);
            m_effects.insert(type == IEffect::kAutoDetection ? m_currentRef->scriptOrder() : type, m_currentRef);
        }
    }
    if (m_currentRef) {
        m_currentRef->useToon.setValue(true);
        m_currentRef->parthf.setValue(false);
        m_currentRef->transp.setValue(false);
        m_currentRef->opadd.setValue(false);
        m_currentRef->vertexCount.setValue(m_modelRef->count(IModel::kVertex));
        m_currentRef->subsetCount.setValue(m_modelRef->count(IModel::kMaterial));
        m_currentRef->setModelMatrixParameters(m_modelRef);
    }
}

void PMXRenderEngine::log0(void *context, IRenderDelegate::LogLevel level, const char *format ...)
{
    va_list ap;
    va_start(ap, format);
    m_delegateRef->log(context, level, format, ap);
    va_end(ap);
}

bool PMXRenderEngine::releaseContext0(void *context)
{
    m_delegateRef->releaseContext(m_modelRef, context);
    release();
    return false;
}

void PMXRenderEngine::release()
{
    glDeleteBuffers(kVertexBufferObjectMax, m_vertexBufferObjects);
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
    m_effects.releaseAll();
    m_oseffects.releaseAll();
    m_aabbMin.setZero();
    m_aabbMax.setZero();
    m_currentRef = 0;
    m_delegateRef = 0;
    m_sceneRef = 0;
    m_modelRef = 0;
    m_accelerator = 0;
    m_cullFaceState = false;
    m_isVertexShaderSkinning = false;
}

} /* namespace gl2 */
} /* namespace vpvl2 */
