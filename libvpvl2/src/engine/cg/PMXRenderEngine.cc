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
                                 pmx::Model *model)

#ifdef VPVL2_LINK_QT
    : QGLFunctions(),
      #else
    :
      #endif /* VPVL2_LINK_QT */
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

PMXRenderEngine::~PMXRenderEngine()
{
    release();
}

IModel *PMXRenderEngine::model() const
{
    return m_model;
}

bool PMXRenderEngine::upload(const IString *dir)
{
    if (!m_model || !m_current)
        return false;
    void *context = 0;
    m_delegate->allocateContext(m_model, context);
    m_current->useToon.setValue(true);
    m_current->parthf.setValue(false);
    m_current->transp.setValue(false);
    m_current->opadd.setValue(false);
    glGenBuffers(kVertexBufferObjectMax, m_vertexBufferObjects);
    size_t size = pmx::Model::strideSize(pmx::Model::kIndexStride);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vertexBufferObjects[kModelIndices]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_model->indices().count() * size, m_model->indicesPtr(), GL_STATIC_DRAW);
    log0(context, IRenderDelegate::kLogInfo,
         "Binding indices to the vertex buffer object (ID=%d)",
         m_vertexBufferObjects[kModelIndices]);
    size = pmx::Model::strideSize(pmx::Model::kVertexStride);
    const int nvertices = m_model->vertices().count();
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObjects[kModelVertices]);
    glBufferData(GL_ARRAY_BUFFER, nvertices * size, m_model->vertexPtr(), GL_DYNAMIC_DRAW);
    log0(context, IRenderDelegate::kLogInfo,
         "Binding model vertices to the vertex buffer object (ID=%d)",
         m_vertexBufferObjects[kModelVertices]);
    m_current->vertexCount.setValue(nvertices);
    if (m_isVertexShaderSkinning)
        m_model->getSkinningMesh(m_mesh);
    const Array<pmx::Material *> &materials = m_model->materials();
    const int nmaterials = materials.count();
    IRenderDelegate::Texture texture;
    GLuint textureID = 0;
    MaterialContext *materialPrivates = m_materialContexts = new MaterialContext[nmaterials];
    m_current->subsetCount.setValue(nmaterials);
    texture.object = &textureID;
    for (int i = 0; i < nmaterials; i++) {
        const pmx::Material *material = materials[i];
        MaterialContext &materialPrivate = materialPrivates[i];
        const IString *path = 0;
        path = material->mainTexture();
        if (path && m_delegate->uploadTexture(path, dir, IRenderDelegate::kTexture2D, texture, context)) {
            materialPrivate.mainTextureID = textureID = *static_cast<const GLuint *>(texture.object);
            log0(context, IRenderDelegate::kLogInfo, "Binding the texture as a main texture (ID=%d)", textureID);
        }
        path = material->sphereTexture();
        if (path && m_delegate->uploadTexture(path, dir, IRenderDelegate::kTexture2D, texture, context)) {
            materialPrivate.sphereTextureID = textureID = *static_cast<const GLuint *>(texture.object);
            log0(context, IRenderDelegate::kLogInfo, "Binding the texture as a sphere texture (ID=%d)", textureID);
        }
        if (material->isSharedToonTextureUsed()) {
            char buf[16];
            snprintf(buf, sizeof(buf), "toon%d.bmp", material->toonTextureIndex());
            IString *s = m_delegate->toUnicode(reinterpret_cast<const uint8_t *>(buf));
            m_delegate->getToonColor(s, dir, materialPrivate.toonTextureColor, context);
            delete s;
        }
        else {
            path = material->toonTexture();
            if (path) {
                m_delegate->getToonColor(path, dir, materialPrivate.toonTextureColor, context);
            }
        }
    }
#ifdef VPVL2_ENABLE_OPENCL
    if (m_accelerator && m_accelerator->isAvailable())
        m_accelerator->uploadModel(m_model, m_vertexBufferObjects[kModelVertices], context);
#endif
    m_scene->updateModel(m_model);
    m_model->setVisible(true);
    update();
    log0(context, IRenderDelegate::kLogInfo, "Created the model: %s", m_model->name()->toByteArray());
    m_delegate->releaseContext(m_model, context);
    return true;
}

void PMXRenderEngine::update()
{
    if (!m_model->isVisible() || !m_current)
        return;
    size_t size = pmx::Model::strideSize(pmx::Model::kVertexStride);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObjects[kModelVertices]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_model->vertices().count() * size, m_model->vertexPtr());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    if (m_isVertexShaderSkinning)
        m_model->updateSkinningMesh(m_mesh);
#ifdef VPVL2_ENABLE_OPENCL
    if (m_accelerator && m_accelerator->isAvailable())
        m_accelerator->updateModel(m_model, m_scene);
#endif
    m_current->updateModelGeometryParameters(m_scene, m_model);
    m_current->updateSceneParameters();
}

void PMXRenderEngine::renderModel()
{
    if (!m_model->isVisible() || !m_current || m_current->scriptOrder() != IEffect::kStandard)
        return;
    m_current->setModelMatrixParameters(m_model);
    const Array<pmx::Material *> &materials = m_model->materials();
    const size_t indexStride = m_model->strideSize(pmx::Model::kIndexStride);
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
    glVertexPointer(3, GL_FLOAT, m_model->strideSize(pmx::Model::kVertexStride),
                    reinterpret_cast<const GLvoid *>(m_model->strideOffset(pmx::Model::kVertexStride)));
    glEnableClientState(GL_VERTEX_ARRAY);
    glNormalPointer(GL_FLOAT, m_model->strideSize(pmx::Model::kNormalStride),
                    reinterpret_cast<const GLvoid *>(m_model->strideOffset(pmx::Model::kNormalStride)));
    glEnableClientState(GL_NORMAL_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, m_model->strideSize(pmx::Model::kTexCoordStride),
                      reinterpret_cast<const GLvoid *>(m_model->strideOffset(pmx::Model::kTexCoordStride)));
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    m_current->edgeColor.setGeometryColor(m_model->edgeColor());
    for (int i = 0; i < nmaterials; i++) {
        const pmx::Material *material = materials[i];
        const MaterialContext &materialContext = m_materialContexts[i];
        const Color &toonColor = materialContext.toonTextureColor;
        const Color &diffuse = material->diffuse();
        m_current->ambient.setGeometryColor(diffuse);
        m_current->diffuse.setGeometryColor(diffuse);
        m_current->emissive.setGeometryColor(material->ambient());
        m_current->specular.setGeometryColor(material->specular());
        m_current->specularPower.setGeometryValue(btMax(material->shininess(), 1.0f));
        m_current->toonColor.setGeometryColor(toonColor);
        GLuint mainTexture = materialContext.mainTextureID;
        GLuint sphereTexture = materialContext.sphereTextureID;
        bool hasMainTexture = mainTexture > 0;
        bool hasSphereMap = sphereTexture > 0;
        m_current->materialTexture.setTexture(mainTexture);
        m_current->materialSphereMap.setTexture(sphereTexture);
        m_current->spadd.setValue(material->sphereTextureRenderMode() == pmx::Material::kAddTexture);
        m_current->useTexture.setValue(hasMainTexture);
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
        CGtechnique technique = m_current->findTechnique(target, i, nmaterials, hasMainTexture, hasSphereMap, true);
        m_current->executeTechniquePasses(technique, GL_TRIANGLES, nindices, GL_UNSIGNED_INT, reinterpret_cast<const GLvoid *>(offset));
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
    if (!m_model->isVisible() || !m_current || m_current->scriptOrder() != IEffect::kStandard)
        return;
    m_current->setModelMatrixParameters(m_model);
    m_current->setZeroGeometryParameters(m_model);
    const Array<pmx::Material *> &materials = m_model->materials();
    const size_t indexStride = m_model->strideSize(pmx::Model::kIndexStride);
    const int nmaterials = materials.count();
    size_t offset = 0;
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObjects[kModelVertices]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vertexBufferObjects[kModelIndices]);
    glVertexPointer(3, GL_FLOAT, m_model->strideSize(pmx::Model::kEdgeVertexStride),
                    reinterpret_cast<const GLvoid *>(m_model->strideOffset(pmx::Model::kEdgeVertexStride)));
    glEnableClientState(GL_VERTEX_ARRAY);
    glCullFace(GL_FRONT);
    for (int i = 0; i < nmaterials; i++) {
        const pmx::Material *material = materials[i];
        const int nindices = material->indices();
        if (material->isEdgeDrawn()) {
            CGtechnique technique = m_current->findTechnique("edge", i, nmaterials, false, false, true);
            m_current->edgeColor.setGeometryColor(material->edgeColor());
            m_current->executeTechniquePasses(technique, GL_TRIANGLES, nindices, GL_UNSIGNED_INT, reinterpret_cast<const GLvoid *>(offset));
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
    if (!m_model->isVisible() || !m_current || m_current->scriptOrder() != IEffect::kStandard)
        return;
    m_current->setModelMatrixParameters(m_model, IRenderDelegate::kShadowMatrix);
    m_current->setZeroGeometryParameters(m_model);
    const Array<pmx::Material *> &materials = m_model->materials();
    const size_t indexStride = m_model->strideSize(pmx::Model::kIndexStride);
    const int nmaterials = materials.count();
    size_t offset = 0;
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObjects[kModelVertices]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vertexBufferObjects[kModelIndices]);
    glVertexPointer(3, GL_FLOAT, m_model->strideSize(pmx::Model::kVertexStride),
                    reinterpret_cast<const GLvoid *>(m_model->strideOffset(pmx::Model::kVertexStride)));
    glEnableClientState(GL_VERTEX_ARRAY);
    glCullFace(GL_FRONT);
    for (int i = 0; i < nmaterials; i++) {
        const pmx::Material *material = materials[i];
        const int nindices = material->indices();
        CGtechnique technique = m_current->findTechnique("shadow", i, nmaterials, false, false, true);
        m_current->executeTechniquePasses(technique, GL_TRIANGLES, nindices, GL_UNSIGNED_INT, reinterpret_cast<const GLvoid *>(offset));
        offset += nindices * indexStride;
    }
    glCullFace(GL_BACK);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void PMXRenderEngine::renderZPlot()
{
    if (!m_model->isVisible() || !m_current || m_current->scriptOrder() != IEffect::kStandard)
        return;
    m_current->setModelMatrixParameters(m_model);
    m_current->setZeroGeometryParameters(m_model);
    const Array<pmx::Material *> &materials = m_model->materials();
    const size_t indexStride = m_model->strideSize(pmx::Model::kIndexStride);
    const int nmaterials = materials.count();
    size_t offset = 0;
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObjects[kModelVertices]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vertexBufferObjects[kModelIndices]);
    glVertexPointer(3, GL_FLOAT, m_model->strideSize(pmx::Model::kVertexStride),
                    reinterpret_cast<const GLvoid *>(m_model->strideOffset(pmx::Model::kVertexStride)));
    glEnableClientState(GL_VERTEX_ARRAY);
    glCullFace(GL_FRONT);
    for (int i = 0; i < nmaterials; i++) {
        const pmx::Material *material = materials[i];
        const int nindices = material->indices();
        if (material->isShadowMapDrawn()) {
            CGtechnique technique = m_current->findTechnique("zplot", i, nmaterials, false, false, true);
            m_current->executeTechniquePasses(technique, GL_TRIANGLES, nindices, GL_UNSIGNED_INT, reinterpret_cast<const GLvoid *>(offset));
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
    return m_current ? m_current->hasTechniques(IEffect::kPreProcess) : false;
}

bool PMXRenderEngine::hasPostProcess() const
{
    return m_current ? m_current->hasTechniques(IEffect::kPostProcess) : false;
}

void PMXRenderEngine::preparePostProcess()
{
    if (m_current)
        m_current->executeScriptExternal();
}

void PMXRenderEngine::performPreProcess()
{
    if (m_current)
        m_current->executeProcess(m_model, IEffect::kPreProcess);
}

void PMXRenderEngine::performPostProcess()
{
    if (m_current)
        m_current->executeProcess(m_model, IEffect::kPostProcess);
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
            m_current = ee;
        }
        else if (einstance) {
            EffectEngine *previous = m_current;
            m_current = new EffectEngine(m_scene, dir, einstance, m_delegate);
            if (m_current->scriptOrder() == IEffect::kStandard) {
                m_oseffects.add(m_current);
            }
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
        else if (einstance) {
            m_current = new EffectEngine(m_scene, dir, einstance, m_delegate);
            m_effects.insert(type == IEffect::kAutoDetection ? m_current->scriptOrder() : type, m_current);
        }
    }
}

void PMXRenderEngine::log0(void *context, IRenderDelegate::LogLevel level, const char *format ...)
{
    va_list ap;
    va_start(ap, format);
    m_delegate->log(context, level, format, ap);
    va_end(ap);
}

bool PMXRenderEngine::releaseContext0(void *context)
{
    m_delegate->releaseContext(m_model, context);
    release();
    return false;
}

void PMXRenderEngine::release()
{
    glDeleteBuffers(kVertexBufferObjectMax, m_vertexBufferObjects);
    if (m_materialContexts) {
        const Array<pmx::Material *> &modelMaterials = m_model->materials();
        const int nmaterials = modelMaterials.count();
        for (int i = 0; i < nmaterials; i++) {
            MaterialContext &materialPrivate = m_materialContexts[i];
            glDeleteTextures(1, &materialPrivate.mainTextureID);
            glDeleteTextures(1, &materialPrivate.sphereTextureID);
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
    m_accelerator = 0;
    m_cullFaceState = false;
    m_isVertexShaderSkinning = false;
}

} /* namespace gl2 */
} /* namespace vpvl2 */
