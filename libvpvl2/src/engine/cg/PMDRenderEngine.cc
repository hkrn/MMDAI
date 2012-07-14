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
      m_delegate(delegate),
      m_scene(scene),
      m_current(0),
      m_accelerator(accelerator),
      m_model(model),
      m_context(effectContext),
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
    return m_model;
}

bool PMDRenderEngine::upload(const IString *dir)
{
    if (!m_model || !m_current)
        return false;
    void *context = 0;
    m_delegate->allocateContext(m_model, context);
    m_current->useToon.setValue(true);
    m_current->parthf.setValue(false);
    m_current->transp.setValue(false);
    m_current->opadd.setValue(false);
    PMDModel *model = m_model->ptr();
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
    log0(context, IRenderDelegate::kLogInfo,
         "Binding indices to the vertex buffer object (ID=%d)",
         m_vertexBufferObjects[kModelIndices]);
    const int nvertices = model->vertices().count();
    m_current->vertexCount.setValue(nvertices);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObjects[kModelVertices]);
    glBufferData(GL_ARRAY_BUFFER, nvertices * model->strideSize(PMDModel::kVerticesStride),
                 model->verticesPointer(), GL_DYNAMIC_DRAW);
    log0(context, IRenderDelegate::kLogInfo,
         "Binding model vertices to the vertex buffer object (ID=%d)",
         m_vertexBufferObjects[kModelVertices]);
    if (m_isVertexShaderSkinning)
        m_model->getSkinningMeshes(m_mesh);
    const MaterialList &materials = model->materials();
    const int nmaterials = materials.count();
    IRenderDelegate::Texture texture;
    GLuint textureID = 0;
    MaterialContext *materialContexts = m_materialContexts = new MaterialContext[nmaterials];
    m_current->subsetCount.setValue(nmaterials);
    texture.object = &textureID;
    for (int i = 0; i < nmaterials; i++) {
        const Material *material = materials[i];
        MaterialContext &materialContext = materialContexts[i];
        materialContext.mainTextureID = 0;
        materialContext.subTextureID = 0;
        const uint8_t *mainTextureName = material->mainTextureName();
        if (*mainTextureName) {
            IString *primary = m_delegate->toUnicode(mainTextureName);
            bool ret = m_delegate->uploadTexture(primary, dir, IRenderDelegate::kTexture2D, texture, context);
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
            IString *second = m_delegate->toUnicode(subTextureName);
            bool ret = m_delegate->uploadTexture(second, dir, IRenderDelegate::kTexture2D, texture, context);
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
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    IString *s = m_delegate->toUnicode(reinterpret_cast<const uint8_t *>("toon0.bmp"));
    m_delegate->getToonColor(s, dir, m_toonTextureColors[0], context);
    delete s;
    static const int nToonTextures = PMDModel::kCustomTextureMax - 1;
    for (int i = 0; i < nToonTextures; i++) {
        const uint8_t *name = model->toonTexture(i);
        s = m_delegate->toUnicode(name);
        m_delegate->getToonColor(s, dir, m_toonTextureColors[i + 1], context);
        delete s;
    }
    m_current->setModelMatrixParameters(m_model);
#ifdef VPVL2_ENABLE_OPENCL
    if (m_accelerator && m_accelerator->isAvailable())
        m_accelerator->uploadModel(m_model, m_vertexBufferObjects[kModelVertices], context);
#endif
    m_scene->updateModel(m_model);
    update();
    IString *modelName = m_delegate->toUnicode(model->name());
    log0(context, IRenderDelegate::kLogInfo, "Created the model: %s", modelName->toByteArray());
    delete modelName;
    m_delegate->releaseContext(m_model, context);
    return true;
}

void PMDRenderEngine::update()
{
    if (!m_model || !m_model->isVisible() || !m_current)
        return;
    PMDModel *model = m_model->ptr();
    model->setLightPosition(-m_scene->light()->direction());
    int nvertices = model->vertices().count();
    size_t strideSize = model->strideSize(PMDModel::kVerticesStride);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObjects[kModelVertices]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, nvertices * strideSize, model->verticesPointer());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    if (m_isVertexShaderSkinning)
        m_model->updateSkinningMeshes(m_mesh);
#ifdef VPVL2_ENABLE_OPENCL
    if (m_accelerator && m_accelerator->isAvailable())
        m_accelerator->updateModel(m_model, m_scene);
#endif
    m_current->updateModelGeometryParameters(m_scene, m_model);
    m_current->updateSceneParameters();
}

void PMDRenderEngine::renderModel()
{
    if (!m_model || !m_model->isVisible() || !m_current || m_current->scriptOrder() != IEffect::kStandard)
        return;
    m_current->setModelMatrixParameters(m_model);
    PMDModel *model = m_model->ptr();
    const MaterialList &materials = model->materials();
    const size_t indexStride = model->strideSize(vpvl::PMDModel::kIndicesStride);
    const Scalar &modelOpacity = m_model->opacity();
    const ILight *light = m_scene->light();
    const GLuint *depthTexturePtr = static_cast<const GLuint *>(light->depthTexture());
    const bool hasModelTransparent = !btFuzzyZero(modelOpacity - 1.0),
            hasShadowMap = depthTexturePtr ? true : false;
    const int nmaterials = materials.count();
    size_t offset = 0;
    if (depthTexturePtr && light->hasFloatTexture()) {
        const GLuint depthTexture = *depthTexturePtr;
        m_current->depthTexture.setTexture(depthTexture);
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
    m_current->edgeColor.setGeometryColor(m_model->edgeColor());
    for (int i = 0; i < nmaterials; i++) {
        const Material *material = materials[i];
        const MaterialContext &materialContext = m_materialContexts[i];
        const Scalar &materialOpacity = material->opacity();
        const Color &toonColor = m_toonTextureColors[material->toonID()];
        const Color &diffuse = material->diffuse();
        diffuseColor.setValue(diffuse.x(), diffuse.y(), diffuse.z(), diffuse.w() * materialOpacity);
        m_current->ambient.setGeometryColor(diffuseColor);
        m_current->diffuse.setGeometryColor(diffuseColor);
        m_current->emissive.setGeometryColor(material->ambient());
        m_current->specular.setGeometryColor(material->specular());
        m_current->specularPower.setGeometryValue(btMax(material->shiness(), 1.0f));
        m_current->toonColor.setGeometryColor(toonColor);
        bool useTexture = false, useSphereMap = false, spadd = false;
        if (materialContext.mainTextureID > 0) {
            if (material->isMainSphereAdd()) {
                m_current->materialSphereMap.setTexture(materialContext.mainTextureID);
                spadd = true;
                useSphereMap = true;
            }
            else if (material->isSubSphereModulate()) {
                m_current->materialSphereMap.setTexture(materialContext.mainTextureID);
                useSphereMap = true;
            }
            else {
                m_current->materialTexture.setTexture(materialContext.mainTextureID);
                useTexture = true;
            }
        }
        if (!useSphereMap) {
            if (material->isSubSphereAdd()) {
                m_current->materialSphereMap.setTexture(materialContext.subTextureID);
                spadd = true;
                useSphereMap = true;
            }
            else if (material->isSubSphereModulate()) {
                m_current->materialSphereMap.setTexture(materialContext.subTextureID);
                useSphereMap = true;
            }
        }
        m_current->useTexture.setValue(useTexture);
        m_current->useSpheremap.setValue(useSphereMap);
        m_current->spadd.setValue(spadd);
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
        CGtechnique technique = m_current->findTechnique(target, i, nmaterials, useTexture, useSphereMap, true);
        m_current->executeTechniquePasses(technique, GL_TRIANGLES, nindices, GL_UNSIGNED_SHORT, reinterpret_cast<const GLvoid *>(offset));
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
    if (!m_model || !m_model->isVisible() || !m_current || m_current->scriptOrder() != IEffect::kStandard)
        return;
    m_current->setModelMatrixParameters(m_model);
    m_current->setZeroGeometryParameters(m_model);
    PMDModel *model = m_model->ptr();
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
        CGtechnique technique = m_current->findTechnique("edge", i, nmaterials, false, false, true);
        m_current->executeTechniquePasses(technique, GL_TRIANGLES, nindices, GL_UNSIGNED_SHORT, reinterpret_cast<const GLvoid *>(offset));
        offset += nindices * indexStride;
    }
    glCullFace(GL_BACK);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void PMDRenderEngine::renderShadow()
{
    if (!m_model || !m_model->isVisible() || !m_current || m_current->scriptOrder() != IEffect::kStandard)
        return;
    m_current->setModelMatrixParameters(m_model, IRenderDelegate::kShadowMatrix);
    m_current->setZeroGeometryParameters(m_model);
    PMDModel *model = m_model->ptr();
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
        CGtechnique technique = m_current->findTechnique("shadow", i, nmaterials, false, false, true);
        m_current->executeTechniquePasses(technique, GL_TRIANGLES, nindices, GL_UNSIGNED_SHORT, reinterpret_cast<const GLvoid *>(offset));
        offset += nindices * indexStride;
    }
    glCullFace(GL_BACK);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void PMDRenderEngine::renderZPlot()
{
    if (!m_model || !m_model->isVisible() || !m_current || m_current->scriptOrder() != IEffect::kStandard)
        return;
    m_current->setModelMatrixParameters(m_model);
    m_current->setZeroGeometryParameters(m_model);
    PMDModel *model = m_model->ptr();
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
            CGtechnique technique = m_current->findTechnique("zplot", i, nmaterials, false, false, true);
            m_current->executeTechniquePasses(technique, GL_TRIANGLES, nindices, GL_UNSIGNED_SHORT, reinterpret_cast<const GLvoid *>(offset));
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
    return m_current ? m_current->hasTechniques(IEffect::kPreProcess) : false;
}

bool PMDRenderEngine::hasPostProcess() const
{
    return m_current ? m_current->hasTechniques(IEffect::kPostProcess) : false;
}

void PMDRenderEngine::preparePostProcess()
{
    if (m_current)
        m_current->executeScriptExternal();
}

void PMDRenderEngine::performPreProcess()
{
    if (m_current)
        m_current->executeProcess(m_model, IEffect::kPreProcess);
}

void PMDRenderEngine::performPostProcess()
{
    if (m_current)
        m_current->executeProcess(m_model, IEffect::kPostProcess);
}

IEffect *PMDRenderEngine::effect(IEffect::ScriptOrderType type) const
{
    EffectEngine **ee = const_cast<EffectEngine **>(m_effects.find(type));
    return ee ? (*ee)->effect() : 0;
}

void PMDRenderEngine::setEffect(IEffect::ScriptOrderType type, IEffect *effect, const IString *dir)
{
    if (type == IEffect::kStandardOffscreen) {
        const int neffects = m_oseffects.count();
        bool found = false;
        EffectEngine *ee;
        for (int i = 0; i < neffects; i++) {
            ee = m_oseffects[i];
            if (ee->effect() == effect) {
                found = true;
                break;
            }
        }
        if (found) {
            m_current = ee;
        }
        else if (effect) {
            EffectEngine *previous = m_current;
            m_current = new EffectEngine(m_scene, m_delegate);
            m_current->attachEffect(effect, dir);
            if (m_current->scriptOrder() == IEffect::kStandard)
                m_oseffects.add(m_current);
            else {
                delete m_current;
                m_current = previous;
            }
        }
    }
    else {
        EffectEngine **ee = const_cast<EffectEngine **>(m_effects.find(type));
        if (ee) {
            m_current = *ee;
        }
        else if (effect) {
            m_current = new EffectEngine(m_scene, m_delegate);
            m_current->attachEffect(effect, dir);
            m_effects.insert(type == IEffect::kAutoDetection ? m_current->scriptOrder() : type, m_current);
        }
    }
}

void PMDRenderEngine::log0(void *context, IRenderDelegate::LogLevel level, const char *format ...)
{
    va_list ap;
    va_start(ap, format);
    m_delegate->log(context, level, format, ap);
    va_end(ap);
}

bool PMDRenderEngine::releaseContext0(void *context)
{
    m_delegate->releaseContext(m_model, context);
    release();
    return false;
}

void PMDRenderEngine::release()
{
    glDeleteBuffers(kVertexBufferObjectMax, m_vertexBufferObjects);
    if (m_materialContexts) {
        const vpvl::MaterialList &modelMaterials = m_model->ptr()->materials();
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
    m_current = 0;
    m_delegate = 0;
    m_scene = 0;
    m_model = 0;
    m_context = 0;
    m_cullFaceState = false;
    m_isVertexShaderSkinning = false;
}

} /* namespace gl2 */
} /* namespace vpvl2 */
