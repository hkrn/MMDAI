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
      m_accelerator(accelerator),
      m_model(model),
      m_context(effectContext),
      m_textures(0),
      m_cullFaceState(true),
      m_isVertexShaderSkinning(false)
{
}

PMDRenderEngine::~PMDRenderEngine()
{
    glDeleteBuffers(kVertexBufferObjectMax, m_vertexBufferObjects);
    glDeleteTextures(vpvl::PMDModel::kCustomTextureMax, m_toonTextures);
    if (m_textures) {
        const vpvl::MaterialList &modelMaterials = m_model->ptr()->materials();
        const int nmaterials = modelMaterials.count();
        for (int i = 0; i < nmaterials; i++) {
            MaterialTextures &materialPrivate = m_textures[i];
            glDeleteTextures(1, &materialPrivate.mainTextureID);
            glDeleteTextures(1, &materialPrivate.subTextureID);
        }
        delete[] m_textures;
        m_textures = 0;
    }
#ifdef VPVL2_ENABLE_OPENCL
    delete m_accelerator;
    m_accelerator = 0;
#endif
    m_delegate = 0;
    m_scene = 0;
    m_model = 0;
    m_context = 0;
    m_cullFaceState = false;
    m_isVertexShaderSkinning = false;
}

IModel *PMDRenderEngine::model() const
{
    return m_model;
}

bool PMDRenderEngine::upload(const IString *dir)
{
    void *context = 0;
    m_delegate->allocateContext(m_model, context);
    IString *source = m_delegate->loadShaderSource(IRenderDelegate::kModelEffectTechniques, m_model, context);
    CGeffect effect = cgCreateEffect(m_context, reinterpret_cast<const char *>(source->toByteArray()), 0);
    delete source;
    if (!effect) {
        return false;
    }
    m_effect.attachEffect(effect);
    cgSetParameter1i(m_effect.useToon, 1);
    cgSetParameter1i(m_effect.parthf, 0);
    cgSetParameter1i(m_effect.transp, 0);
    cgSetParameter1i(m_effect.opadd, 0);
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
    cgSetParameter1i(m_effect.vertexCount, nvertices);
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
    GLuint textureID = 0;
    MaterialTextures *materialPrivates = new MaterialTextures[nmaterials];
    cgSetParameter1i(m_effect.subsetCount, nmaterials);
    for (int i = 0; i < nmaterials; i++) {
        const Material *material = materials[i];
        IString *primary = m_delegate->toUnicode(material->mainTextureName());
        IString *second = m_delegate->toUnicode(material->subTextureName());
        MaterialTextures &materialPrivate = materialPrivates[i];
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
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    if (m_delegate->uploadToonTexture(context, "toon0.bmp", dir, &textureID)) {
        m_toonTextures[0] = textureID;
        log0(context, IRenderDelegate::kLogInfo, "Binding the texture as a toon texture (ID=%d)", textureID);
    }
    for (int i = 0; i < PMDModel::kCustomTextureMax; i++) {
        const uint8_t *name = model->toonTexture(i);
        if (m_delegate->uploadToonTexture(context, reinterpret_cast<const char *>(name), dir, &textureID)) {
            m_toonTextures[i + 1] = textureID;
            log0(context, IRenderDelegate::kLogInfo, "Binding the texture as a toon texture (ID=%d)", textureID);
        }
    }
#ifdef VPVL2_ENABLE_OPENCL
    if (m_accelerator && m_accelerator->isAvailable())
        m_accelerator->uploadModel(m_model, m_vertexBufferObjects[kModelVertices], context);
#endif
    model->updateImmediate();
    update();
    IString *modelName = m_delegate->toUnicode(model->name());
    log0(context, IRenderDelegate::kLogInfo, "Created the model: %s", modelName->toByteArray());
    delete modelName;
    m_delegate->releaseContext(m_model, context);
    return true;
}

void PMDRenderEngine::update()
{
    if (!m_model->isVisible())
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
        m_accelerator->updateModel(m_model);
#endif
    const Scene::ILight *light = m_scene->light();
    const Vector3 kOne(1, 1, 1);
    m_effect.ambient.setLightColor(kOne);
    m_effect.diffuse.setLightColor(kOne);
    m_effect.emissive.setLightColor(kZeroV3);
    m_effect.specular.setLightColor(kOne);
    m_effect.toonColor.setLightColor(kZeroV3);
    m_effect.edgeColor.setLightColor(kZeroV3);
    m_effect.specularPower.setLightValue(0);
    const Vector3 &lightDirection = light->direction();
    m_effect.position.setLightValue(-lightDirection);
    m_effect.direction.setLightValue(lightDirection);
    const Scene::ICamera *camera = m_scene->camera();
    m_effect.position.setCameraValue(camera->position());
    m_effect.direction.setCameraValue(kZeroV3); // TODO: set camera direction
}

void PMDRenderEngine::renderModel()
{
    if (!m_model->isVisible())
        return;
    const Scene::IMatrices *matrices = m_scene->matrices();
    float matrix4x4[16];
    matrices->getModelViewProjection(matrix4x4);
    m_effect.worldViewProjection.setCameraMatrix(matrix4x4);
    matrices->getLightViewProjection(matrix4x4);
    m_effect.worldViewProjection.setLightMatrix(matrix4x4);
    PMDModel *model = m_model->ptr();
    const Scene::ILight *light = m_scene->light();
    const MaterialList &materials = model->materials();
    const size_t indexStride = model->strideSize(vpvl::PMDModel::kIndicesStride);
    const Scalar &modelOpacity = m_model->opacity();
    const bool hasModelTransparent = !btFuzzyZero(modelOpacity - 1.0),
            isToonEnabled = light->isToonEnabled(),
            hasShadowMap = light->depthTexture() ? true : false;
    const int nmaterials = materials.count();
    Color diffuse;
    size_t offset = 0;
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObjects[kModelVertices]);
    glVertexPointer(3, GL_FLOAT, model->strideSize(PMDModel::kVerticesStride),
                    reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kVerticesStride)));
    glNormalPointer(GL_FLOAT, model->strideSize(PMDModel::kNormalsStride),
                    reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kNormalsStride)));
    glTexCoordPointer(2, GL_FLOAT, model->strideSize(PMDModel::kTextureCoordsStride),
                      reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kTextureCoordsStride)));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vertexBufferObjects[kModelIndices]);
    m_effect.edgeColor.setGeometryColor(m_model->edgeColor());
    for (int i = 0; i < nmaterials; i++) {
        const Material *material = materials[i];
        const MaterialTextures &texture = m_textures[i];
        const Scalar &materialOpacity = material->opacity();
        diffuse = material->diffuse();
        diffuse.setW(diffuse.w() * materialOpacity);
        m_effect.ambient.setGeometryColor(material->ambient());
        m_effect.diffuse.setGeometryColor(diffuse);
        m_effect.specular.setGeometryColor(material->specular());
        m_effect.specularPower.setGeometryValue(material->shiness());
        bool hasSphereMap = false;
        bool hasMainTexture = texture.mainTextureID > 0;
        if (hasMainTexture) {
            if (material->isMainSphereAdd()) {
                m_effect.materialSphereMap.setTexture(texture.mainTextureID);
                cgSetParameter1i(m_effect.spadd, 1);
                cgSetParameter1i(m_effect.useTexture, 0);
                hasSphereMap = true;
            }
            else if (material->isSubSphereModulate()) {
                m_effect.materialSphereMap.setTexture(texture.mainTextureID);
                cgSetParameter1i(m_effect.spadd, 0);
                cgSetParameter1i(m_effect.useTexture, 0);
                hasSphereMap = true;
            }
            else {
                m_effect.materialTexture.setTexture(texture.mainTextureID);
                cgSetParameter1i(m_effect.useTexture, 1);
            }
        }
        else {
            cgSetParameter1i(m_effect.useTexture, 0);
        }
        if (!hasSphereMap) {
            if (material->isSubSphereAdd()) {
                m_effect.materialSphereMap.setTexture(texture.subTextureID);
                cgSetParameter1i(m_effect.spadd, 1);
            }
            else if (material->isSubSphereModulate()) {
                m_effect.materialSphereMap.setTexture(texture.subTextureID);
                cgSetParameter1i(m_effect.spadd, 0);
            }
        }
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
        const char *target = hasShadowMap ? "object_ss" : "object";
        CGtechnique technique = m_effect.findTechnique(target, i, nmaterials, hasMainTexture, isToonEnabled);
        CGpass pass = cgGetFirstPass(technique);
        while (pass) {
            cgSetPassState(pass);
            glDrawElements(GL_TRIANGLES, nindices, GL_UNSIGNED_SHORT, reinterpret_cast<const GLvoid *>(offset));
            cgResetPassState(pass);
            pass = cgGetNextPass(pass);
        }
        offset += nindices * indexStride;
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    if (!m_cullFaceState) {
        glEnable(GL_CULL_FACE);
        m_cullFaceState = true;
    }
}

void PMDRenderEngine::renderEdge()
{
}

void PMDRenderEngine::renderShadow()
{
}

void PMDRenderEngine::renderZPlot()
{
}

void PMDRenderEngine::log0(void *context, IRenderDelegate::LogLevel level, const char *format ...)
{
    va_list ap;
    va_start(ap, format);
    m_delegate->log(context, level, format, ap);
    va_end(ap);
}

} /* namespace gl2 */
} /* namespace vpvl2 */
