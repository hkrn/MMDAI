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
    : m_delegate(delegate),
      m_scene(scene),
      m_accelerator(accelerator),
      m_model(model),
      m_context(effectContext),
      m_materials(0),
      m_cullFaceState(true),
      m_isVertexShaderSkinning(false)
{
}

PMXRenderEngine::~PMXRenderEngine()
{
    glDeleteBuffers(kVertexBufferObjectMax, m_vertexBufferObjects);
    const Array<pmx::Material *> &modelMaterials = m_model->materials();
    const int nmaterials = modelMaterials.count();
    for (int i = 0; i < nmaterials; i++) {
        MaterialTextures &materialPrivate = m_materials[i];
        glDeleteTextures(1, &materialPrivate.mainTextureID);
        glDeleteTextures(1, &materialPrivate.sphereTextureID);
        glDeleteTextures(1, &materialPrivate.toonTextureID);
    }
    delete[] m_materials;
    m_materials = 0;
#ifdef VPVL2_ENABLE_OPENCL
    delete m_accelerator;
    m_accelerator = 0;
#endif
    cgDestroyEffect(m_effect);
    m_effect = 0;
    m_delegate = 0;
    m_scene = 0;
    m_model = 0;
    m_accelerator = 0;
    m_cullFaceState = false;
    m_isVertexShaderSkinning = false;
}

IModel *PMXRenderEngine::model() const
{
    return m_model;
}

bool PMXRenderEngine::upload(const IString *dir)
{
    bool ret = true;
    void *context = 0;
    m_delegate->allocateContext(m_model, context);
    IString *source = m_delegate->loadShaderSource(IRenderDelegate::kModelEffectTechniques, m_model, context);
    m_effect = cgCreateEffect(m_context, reinterpret_cast<const char *>(source->toByteArray()), 0);
    delete source;
    if (!m_effect) {
        return false;
    }
    m_parameters.attachEffect(m_effect);
    glGenBuffers(kVertexBufferObjectMax, m_vertexBufferObjects);
    size_t size = pmx::Model::strideSize(pmx::Model::kIndexStride);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vertexBufferObjects[kModelIndices]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_model->indices().count() * size, m_model->indicesPtr(), GL_STATIC_DRAW);
    log0(context, IRenderDelegate::kLogInfo,
         "Binding indices to the vertex buffer object (ID=%d)",
         m_vertexBufferObjects[kModelIndices]);
    size = pmx::Model::strideSize(pmx::Model::kVertexStride);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObjects[kModelVertices]);
    glBufferData(GL_ARRAY_BUFFER, m_model->vertices().count() * size, m_model->vertexPtr(), GL_DYNAMIC_DRAW);
    log0(context, IRenderDelegate::kLogInfo,
         "Binding model vertices to the vertex buffer object (ID=%d)",
         m_vertexBufferObjects[kModelVertices]);
    if (m_isVertexShaderSkinning)
        m_model->getSkinningMesh(m_mesh);
    const Array<pmx::Material *> &materials = m_model->materials();
    const int nmaterials = materials.count();
    GLuint textureID = 0;
    MaterialTextures *materialPrivates = m_materials = new MaterialTextures[nmaterials];
    for (int i = 0; i < nmaterials; i++) {
        const pmx::Material *material = materials[i];
        MaterialTextures &materialPrivate = materialPrivates[i];
        materialPrivate.mainTextureID = 0;
        materialPrivate.sphereTextureID = 0;
        materialPrivate.toonTextureID = 0;
        const IString *path = 0;
        path = material->mainTexture();
        if (path && m_delegate->uploadTexture(context, path, dir, &textureID, false)) {
            log0(context, IRenderDelegate::kLogInfo, "Binding the texture as a main texture (ID=%d)", textureID);
            materialPrivate.mainTextureID = textureID;
        }
        path = material->sphereTexture();
        if (path && m_delegate->uploadTexture(context, path, dir, &textureID, false)) {
            log0(context, IRenderDelegate::kLogInfo, "Binding the texture as a sphere texture (ID=%d)", textureID);
            materialPrivate.sphereTextureID = textureID;
        }
        if (material->isSharedToonTextureUsed()) {
            if (m_delegate->uploadToonTexture(context, material->toonTextureIndex(), &textureID)) {
                log0(context, IRenderDelegate::kLogInfo, "Binding the texture as a shared toon texture (ID=%d)", textureID);
                materialPrivate.toonTextureID = textureID;
            }
        }
        else {
            path = material->toonTexture();
            if (path && m_delegate->uploadTexture(context, path, dir, &textureID, true)) {
                log0(context, IRenderDelegate::kLogInfo, "Binding the texture as a static toon texture (ID=%d)", textureID);
                materialPrivate.toonTextureID = textureID;
            }
        }
    }
#ifdef VPVL2_ENABLE_OPENCL
    if (m_accelerator && m_accelerator->isAvailable())
        m_accelerator->uploadModel(m_model, m_vertexBufferObjects[kModelVertices], context);
#endif
    m_model->performUpdate(m_scene->light()->direction());
    m_model->setVisible(true);
    update();
    log0(context, IRenderDelegate::kLogInfo, "Created the model: %s", m_model->name()->toByteArray());
    m_delegate->releaseContext(m_model, context);
    return ret;
}

void PMXRenderEngine::update()
{
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
}

void PMXRenderEngine::renderModel()
{
}

void PMXRenderEngine::renderEdge()
{
}

void PMXRenderEngine::renderShadow()
{
}

void PMXRenderEngine::renderZPlot()
{
}

void PMXRenderEngine::log0(void *context, IRenderDelegate::LogLevel level, const char *format ...)
{
    va_list ap;
    va_start(ap, format);
    m_delegate->log(context, level, format, ap);
    va_end(ap);
}

} /* namespace gl2 */
} /* namespace vpvl2 */
