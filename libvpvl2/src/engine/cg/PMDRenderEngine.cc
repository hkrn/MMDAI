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

#include "vpvl2/cg/PMDRenderEngine.h"
#include "vpvl2/vpvl2.h"

#ifdef VPVL2_ENABLE_OPENCL
#include "vpvl2/cl/Context.h"
#include "vpvl2/cl/PMDAccelerator.h"
#endif

using namespace vpvl;

namespace vpvl2
{
namespace cg
{

PMDRenderEngine::PMDRenderEngine(IRenderDelegate *delegate,
                                 const Scene *scene,
                                 CGcontext effectContext,
                                 cl::PMDAccelerator *accelerator,
                                 pmd::Model *model)
#ifdef VPVL_LINK_QT
    : QGLFunctions(),
      #else
    :
      #endif /* VPVL_LINK_QT */
      m_delegateRef(delegate),
      m_sceneRef(scene),
      m_currentRef(0),
      m_accelerator(accelerator),
      m_modelRef(model),
      m_contextRef(effectContext),
      m_materialContexts(0),
      m_cullFaceState(true),
      m_isVertexShaderSkinning(false)
{
#ifdef VPVL2_LINK_QT
    initializeGLFunctions();
#endif /* VPVL2_LINK_QT */
}

PMDRenderEngine::~PMDRenderEngine()
{
    release();
}

IModel *PMDRenderEngine::model() const
{
    return m_modelRef;
}

bool PMDRenderEngine::upload(const IString *dir)
{
    void *context = 0;
    m_delegateRef->allocateContext(m_modelRef, context);
    PMDModel *model = m_modelRef->ptr();
    glGenBuffers(kVertexBufferObjectMax, m_vertexBufferObjects);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vertexBufferObjects[kEdgeIndices]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->edgeIndicesCount() * model->strideSize(PMDModel::kEdgeIndicesStride),
                 model->edgeIndicesPointer(), GL_STATIC_DRAW);
    log0(context, IRenderDelegate::kLogInfo,
         "Binding edge indices to the vertex buffer object (ID=%d)",
         m_vertexBufferObjects[kEdgeIndices]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vertexBufferObjects[kModelIndices]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->indices().count() * model->strideSize(PMDModel::kIndicesStride),
                 model->indicesPointer(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    log0(context, IRenderDelegate::kLogInfo,
         "Binding indices to the vertex buffer object (ID=%d)",
         m_vertexBufferObjects[kModelIndices]);
    const int nvertices = model->vertices().count();
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObjects[kModelVertices]);
    glBufferData(GL_ARRAY_BUFFER, nvertices * model->strideSize(PMDModel::kVerticesStride),
                 model->verticesPointer(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    log0(context, IRenderDelegate::kLogInfo,
         "Binding model vertices to the vertex buffer object (ID=%d)",
         m_vertexBufferObjects[kModelVertices]);
    if (m_isVertexShaderSkinning)
        m_modelRef->getSkinningMeshes(m_mesh);
    const MaterialList &materials = model->materials();
    const int nmaterials = materials.count();
    IRenderDelegate::Texture texture;
    GLuint textureID = 0;
    MaterialContext *materialContexts = m_materialContexts = new MaterialContext[nmaterials];
    texture.object = &textureID;
    for (int i = 0; i < nmaterials; i++) {
        const Material *material = materials[i];
        MaterialContext &materialContext = materialContexts[i];
        materialContext.mainTextureID = 0;
        materialContext.subTextureID = 0;
        const uint8_t *mainTextureName = material->mainTextureName();
        if (*mainTextureName) {
            IString *primary = m_delegateRef->toUnicode(mainTextureName);
            bool ret = m_delegateRef->uploadTexture(primary, dir, IRenderDelegate::kTexture2D, texture, context);
            delete primary;
            if (ret) {
                materialContext.mainTextureID = textureID = *static_cast<const GLuint *>(texture.object);
                log0(context, IRenderDelegate::kLogInfo, "Binding the texture as a primary texture (ID=%d)", textureID);
            }
            else {
                return releaseContext0(context);
            }
        }
        const uint8_t *subTextureName = material->subTextureName();
        if (*subTextureName) {
            IString *second = m_delegateRef->toUnicode(subTextureName);
            bool ret = m_delegateRef->uploadTexture(second, dir, IRenderDelegate::kTexture2D, texture, context);
            delete second;
            if (ret) {
                materialContext.subTextureID = textureID = *static_cast<const GLuint *>(texture.object);
                log0(context, IRenderDelegate::kLogInfo, "Binding the texture as a secondary texture (ID=%d)", textureID);
            }
            else {
                return releaseContext0(context);
            }
        }
    }
    IString *s = m_delegateRef->toUnicode(reinterpret_cast<const uint8_t *>("toon0.bmp"));
    m_delegateRef->getToonColor(s, dir, m_toonTextureColors[0], context);
    delete s;
    static const int nToonTextures = PMDModel::kCustomTextureMax - 1;
    for (int i = 0; i < nToonTextures; i++) {
        const uint8_t *name = model->toonTexture(i);
        s = m_delegateRef->toUnicode(name);
        m_delegateRef->getToonColor(s, dir, m_toonTextureColors[i + 1], context);
        delete s;
    }
#ifdef VPVL2_ENABLE_OPENCL
    if (m_accelerator && m_accelerator->isAvailable())
        m_accelerator->uploadModel(m_modelRef, m_vertexBufferObjects[kModelVertices], context);
#endif
    m_sceneRef->updateModel(m_modelRef);
    update();
    IString *modelName = m_delegateRef->toUnicode(model->name());
    log0(context, IRenderDelegate::kLogInfo, "Created the model: %s", modelName->toByteArray());
    delete modelName;
    m_delegateRef->releaseContext(m_modelRef, context);
    return true;
}

void PMDRenderEngine::update()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_currentRef)
        return;
    PMDModel *model = m_modelRef->ptr();
    model->setLightPosition(-m_sceneRef->light()->direction());
    int nvertices = model->vertices().count();
    size_t strideSize = model->strideSize(PMDModel::kVerticesStride);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObjects[kModelVertices]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, nvertices * strideSize, model->verticesPointer());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    if (m_isVertexShaderSkinning)
        m_modelRef->updateSkinningMeshes(m_mesh);
#ifdef VPVL2_ENABLE_OPENCL
    if (m_accelerator && m_accelerator->isAvailable())
        m_accelerator->updateModel(m_modelRef, m_sceneRef);
#endif
    m_currentRef->updateModelGeometryParameters(m_sceneRef, m_modelRef);
    m_currentRef->updateSceneParameters();
}

void PMDRenderEngine::renderModel()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_currentRef || !m_currentRef->validateStandard())
        return;
    m_currentRef->setModelMatrixParameters(m_modelRef);
    PMDModel *model = m_modelRef->ptr();
    const MaterialList &materials = model->materials();
    const size_t indexStride = model->strideSize(vpvl::PMDModel::kIndicesStride);
    const Scalar &modelOpacity = m_modelRef->opacity();
    const ILight *light = m_sceneRef->light();
    const GLuint *depthTexturePtr = static_cast<const GLuint *>(light->depthTexture());
    const bool hasModelTransparent = !btFuzzyZero(modelOpacity - 1.0),
            hasShadowMap = depthTexturePtr ? true : false;
    const int nmaterials = materials.count();
    size_t offset = 0;
    if (depthTexturePtr && light->hasFloatTexture()) {
        const GLuint depthTexture = *depthTexturePtr;
        m_currentRef->depthTexture.setTexture(depthTexture);
    }
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObjects[kModelVertices]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vertexBufferObjects[kModelIndices]);
    glVertexPointer(3, GL_FLOAT, model->strideSize(PMDModel::kVerticesStride),
                    reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kVerticesStride)));
    glEnableClientState(GL_VERTEX_ARRAY);
    glNormalPointer(GL_FLOAT, model->strideSize(PMDModel::kNormalsStride),
                    reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kNormalsStride)));
    glEnableClientState(GL_NORMAL_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, model->strideSize(PMDModel::kTextureCoordsStride),
                      reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kTextureCoordsStride)));
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    Color diffuseColor(0, 0, 0, 1);
    m_currentRef->edgeColor.setGeometryColor(m_modelRef->edgeColor());
    for (int i = 0; i < nmaterials; i++) {
        const Material *material = materials[i];
        const MaterialContext &materialContext = m_materialContexts[i];
        const Scalar &materialOpacity = material->opacity();
        const Color &toonColor = m_toonTextureColors[material->toonID()];
        const Color &diffuse = material->diffuse();
        diffuseColor.setValue(diffuse.x(), diffuse.y(), diffuse.z(), diffuse.w() * materialOpacity);
        m_currentRef->ambient.setGeometryColor(diffuseColor);
        m_currentRef->diffuse.setGeometryColor(diffuseColor);
        m_currentRef->emissive.setGeometryColor(material->ambient());
        m_currentRef->specular.setGeometryColor(material->specular());
        m_currentRef->specularPower.setGeometryValue(btMax(material->shiness(), 1.0f));
        m_currentRef->toonColor.setGeometryColor(toonColor);
        bool useTexture = false, useSphereMap = false, spadd = false;
        if (materialContext.mainTextureID > 0) {
            if (material->isMainSphereAdd()) {
                m_currentRef->materialSphereMap.setTexture(materialContext.mainTextureID);
                spadd = true;
                useSphereMap = true;
            }
            else if (material->isSubSphereModulate()) {
                m_currentRef->materialSphereMap.setTexture(materialContext.mainTextureID);
                useSphereMap = true;
            }
            else {
                m_currentRef->materialTexture.setTexture(materialContext.mainTextureID);
                useTexture = true;
            }
        }
        if (!useSphereMap) {
            if (material->isSubSphereAdd()) {
                m_currentRef->materialSphereMap.setTexture(materialContext.subTextureID);
                spadd = true;
                useSphereMap = true;
            }
            else if (material->isSubSphereModulate()) {
                m_currentRef->materialSphereMap.setTexture(materialContext.subTextureID);
                useSphereMap = true;
            }
        }
        m_currentRef->useTexture.setValue(useTexture);
        m_currentRef->useSpheremap.setValue(useSphereMap);
        m_currentRef->spadd.setValue(spadd);
        if ((hasModelTransparent && m_cullFaceState) ||
                (!btFuzzyZero(materialOpacity - 1.0f) && m_cullFaceState)) {
            glDisable(GL_CULL_FACE);
            m_cullFaceState = false;
        }
        else if (!m_cullFaceState) {
            glEnable(GL_CULL_FACE);
            m_cullFaceState = true;
        }
        const int nindices = material->countIndices();
        const char *const target = hasShadowMap ? "object_ss" : "object";
        CGtechnique technique = m_currentRef->findTechnique(target, i, nmaterials, useTexture, useSphereMap, true);
        m_currentRef->executeTechniquePasses(technique, GL_TRIANGLES, nindices, GL_UNSIGNED_SHORT, reinterpret_cast<const GLvoid *>(offset));
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

void PMDRenderEngine::renderEdge()
{
    if (!m_modelRef || !m_modelRef->isVisible() || btFuzzyZero(m_modelRef->edgeWidth())
            || !m_currentRef || m_currentRef->scriptOrder() != IEffect::kStandard)
        return;
    m_currentRef->setModelMatrixParameters(m_modelRef);
    m_currentRef->setZeroGeometryParameters(m_modelRef);
    PMDModel *model = m_modelRef->ptr();
    const MaterialList &materials = model->materials();
    const size_t indexStride = model->strideSize(vpvl::PMDModel::kIndicesStride);
    const int nmaterials = materials.count();
    size_t offset = 0;
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObjects[kModelVertices]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vertexBufferObjects[kModelIndices]);
    glVertexPointer(3, GL_FLOAT, model->strideSize(PMDModel::kEdgeVerticesStride),
                    reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kEdgeVerticesStride)));
    glEnableClientState(GL_VERTEX_ARRAY);
    glCullFace(GL_FRONT);
    for (int i = 0; i < nmaterials; i++) {
        const Material *material = materials[i];
        const int nindices = material->countIndices();
        CGtechnique technique = m_currentRef->findTechnique("edge", i, nmaterials, false, false, true);
        m_currentRef->executeTechniquePasses(technique, GL_TRIANGLES, nindices, GL_UNSIGNED_SHORT, reinterpret_cast<const GLvoid *>(offset));
        offset += nindices * indexStride;
    }
    glCullFace(GL_BACK);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void PMDRenderEngine::renderShadow()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_currentRef || m_currentRef->scriptOrder() != IEffect::kStandard)
        return;
    m_currentRef->setModelMatrixParameters(m_modelRef, IRenderDelegate::kShadowMatrix);
    m_currentRef->setZeroGeometryParameters(m_modelRef);
    PMDModel *model = m_modelRef->ptr();
    const MaterialList &materials = model->materials();
    const size_t indexStride = model->strideSize(vpvl::PMDModel::kIndicesStride);
    const int nmaterials = materials.count();
    size_t offset = 0;
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObjects[kModelVertices]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vertexBufferObjects[kModelIndices]);
    glVertexPointer(3, GL_FLOAT, model->strideSize(PMDModel::kVerticesStride),
                    reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kVerticesStride)));
    glEnableClientState(GL_VERTEX_ARRAY);
    glCullFace(GL_FRONT);
    for (int i = 0; i < nmaterials; i++) {
        const Material *material = materials[i];
        const int nindices = material->countIndices();
        CGtechnique technique = m_currentRef->findTechnique("shadow", i, nmaterials, false, false, true);
        m_currentRef->executeTechniquePasses(technique, GL_TRIANGLES, nindices, GL_UNSIGNED_SHORT, reinterpret_cast<const GLvoid *>(offset));
        offset += nindices * indexStride;
    }
    glCullFace(GL_BACK);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void PMDRenderEngine::renderZPlot()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_currentRef || m_currentRef->scriptOrder() != IEffect::kStandard)
        return;
    m_currentRef->setModelMatrixParameters(m_modelRef);
    m_currentRef->setZeroGeometryParameters(m_modelRef);
    PMDModel *model = m_modelRef->ptr();
    const MaterialList &materials = model->materials();
    const size_t indexStride = model->strideSize(vpvl::PMDModel::kIndicesStride);
    const int nmaterials = materials.count();
    size_t offset = 0;
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObjects[kModelVertices]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vertexBufferObjects[kModelIndices]);
    glVertexPointer(3, GL_FLOAT, model->strideSize(PMDModel::kVerticesStride),
                    reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kVerticesStride)));
    glEnableClientState(GL_VERTEX_ARRAY);
    glCullFace(GL_FRONT);
    for (int i = 0; i < nmaterials; i++) {
        const Material *material = materials[i];
        const int nindices = material->countIndices();
        if (!btFuzzyZero(material->opacity() - 0.98)) {
            CGtechnique technique = m_currentRef->findTechnique("zplot", i, nmaterials, false, false, true);
            m_currentRef->executeTechniquePasses(technique, GL_TRIANGLES, nindices, GL_UNSIGNED_SHORT, reinterpret_cast<const GLvoid *>(offset));
        }
        offset += nindices * indexStride;
    }
    glCullFace(GL_BACK);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

bool PMDRenderEngine::hasPreProcess() const
{
    return m_currentRef ? m_currentRef->hasTechniques(IEffect::kPreProcess) : false;
}

bool PMDRenderEngine::hasPostProcess() const
{
    return m_currentRef ? m_currentRef->hasTechniques(IEffect::kPostProcess) : false;
}

void PMDRenderEngine::preparePostProcess()
{
    if (m_currentRef)
        m_currentRef->executeScriptExternal();
}

void PMDRenderEngine::performPreProcess()
{
    if (m_currentRef)
        m_currentRef->executeProcess(m_modelRef, IEffect::kPreProcess);
}

void PMDRenderEngine::performPostProcess()
{
    if (m_currentRef)
        m_currentRef->executeProcess(m_modelRef, IEffect::kPostProcess);
}

IEffect *PMDRenderEngine::effect(IEffect::ScriptOrderType type) const
{
    const EffectEngine *const *ee = m_effects.find(type);
    return ee ? (*ee)->effect() : 0;
}

void PMDRenderEngine::setEffect(IEffect::ScriptOrderType type, IEffect *effect, const IString *dir)
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

void PMDRenderEngine::log0(void *context, IRenderDelegate::LogLevel level, const char *format ...)
{
    va_list ap;
    va_start(ap, format);
    m_delegateRef->log(context, level, format, ap);
    va_end(ap);
}

bool PMDRenderEngine::releaseContext0(void *context)
{
    m_delegateRef->releaseContext(m_modelRef, context);
    release();
    return false;
}

void PMDRenderEngine::release()
{
    glDeleteBuffers(kVertexBufferObjectMax, m_vertexBufferObjects);
    if (m_materialContexts) {
        const vpvl::MaterialList &modelMaterials = m_modelRef->ptr()->materials();
        const int nmaterials = modelMaterials.count();
        for (int i = 0; i < nmaterials; i++) {
            MaterialContext &materialPrivate = m_materialContexts[i];
            glDeleteTextures(1, &materialPrivate.mainTextureID);
            glDeleteTextures(1, &materialPrivate.subTextureID);
        }
        delete[] m_materialContexts;
        m_materialContexts = 0;
    }
#ifdef VPVL2_ENABLE_OPENCL
    delete m_accelerator;
    m_accelerator = 0;
#endif
    m_effects.releaseAll();
    m_oseffects.releaseAll();
    m_currentRef = 0;
    m_delegateRef = 0;
    m_sceneRef = 0;
    m_modelRef = 0;
    m_contextRef = 0;
    m_cullFaceState = false;
    m_isVertexShaderSkinning = false;
}

} /* namespace gl2 */
} /* namespace vpvl2 */
