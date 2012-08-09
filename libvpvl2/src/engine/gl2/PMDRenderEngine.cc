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

#include "vpvl2/IRenderDelegate.h"
#include "vpvl2/gl2/PMDRenderEngine.h"
#include "vpvl2/pmd/Model.h"
#ifdef VPVL2_ENABLE_OPENCL
#include "vpvl2/cl/Context.h"
#include "vpvl2/cl/PMDAccelerator.h"
#endif

#include "EngineCommon.h"

namespace {

using namespace vpvl2;
using namespace vpvl2::gl2;

class ExtendedZPlotProgram : public ZPlotProgram
{
public:
    ExtendedZPlotProgram(IRenderDelegate *delegate)
        : ZPlotProgram(delegate),
          m_boneIndicesAndWeightsAttributeLocation(0),
          m_boneMatricesUniformLocation(0)
    {
    }
    ~ExtendedZPlotProgram() {
        m_boneIndicesAndWeightsAttributeLocation = 0;
        m_boneMatricesUniformLocation = 0;
    }

    void setBoneIndicesAndWeights(const GLvoid *ptr, GLsizei stride) {
        glVertexAttribPointer(m_boneIndicesAndWeightsAttributeLocation, 3, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setBoneMatrices(const Scalar *value, size_t size) {
        glUniformMatrix4fv(m_boneMatricesUniformLocation, size, GL_FALSE, value);
    }

protected:
    virtual void getLocations() {
        ZPlotProgram::getLocations();
        m_boneIndicesAndWeightsAttributeLocation = glGetAttribLocation(m_program, "inBoneIndicesAndWeights");
        m_boneMatricesUniformLocation = glGetUniformLocation(m_program, "boneMatrices");
        enableAttribute(m_boneIndicesAndWeightsAttributeLocation);
    }

private:
    GLuint m_boneIndicesAndWeightsAttributeLocation;
    GLuint m_boneMatricesUniformLocation;
};

class EdgeProgram : public BaseShaderProgram
{
public:
    EdgeProgram(IRenderDelegate *delegate)
        : BaseShaderProgram(delegate),
          m_normalAttributeLocation(0),
          m_edgeOffsetAttributeLocation(0),
          m_colorUniformLocation(0),
          m_opacityUniformLocation(0),
          m_edgeWidthUniformLocation(0),
          m_boneIndicesAndWeightsAttributeLocation(0),
          m_boneMatricesUniformLocation(0)
    {
    }
    ~EdgeProgram() {
        m_normalAttributeLocation = 0;
        m_edgeOffsetAttributeLocation = 0;
        m_colorUniformLocation = 0;
        m_opacityUniformLocation = 0;
        m_edgeWidthUniformLocation = 0;
        m_boneIndicesAndWeightsAttributeLocation = 0;
        m_boneMatricesUniformLocation = 0;
    }

    void setEdgeOffset(const GLvoid *ptr, GLsizei stride) {
        glVertexAttribPointer(m_edgeOffsetAttributeLocation, 1, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setColor(const Vector3 &value) {
        glUniform4fv(m_colorUniformLocation, 1, value);
    }
    void setNormal(const GLvoid *ptr, GLsizei stride) {
        glVertexAttribPointer(m_normalAttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setOpacity(const Scalar &value) {
        glUniform1f(m_opacityUniformLocation, value);
    }
    void setEdgeWidth(const Scalar &value) {
        glUniform1f(m_edgeWidthUniformLocation, value);
    }
    void setBoneIndicesAndWeights(const GLvoid *ptr, GLsizei stride) {
        glVertexAttribPointer(m_boneIndicesAndWeightsAttributeLocation, 3, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setBoneMatrices(const Scalar *value, size_t size) {
        glUniformMatrix4fv(m_boneMatricesUniformLocation, size, GL_FALSE, value);
    }

protected:
    virtual void getLocations() {
        BaseShaderProgram::getLocations();
        m_normalAttributeLocation = glGetAttribLocation(m_program, "inNormal");
        m_edgeOffsetAttributeLocation = glGetAttribLocation(m_program, "inEdgeOffset");
        m_colorUniformLocation = glGetUniformLocation(m_program, "color");
        m_opacityUniformLocation = glGetUniformLocation(m_program, "opacity");
        m_edgeWidthUniformLocation = glGetUniformLocation(m_program, "edgeWidth");
        m_boneIndicesAndWeightsAttributeLocation = glGetAttribLocation(m_program, "inBoneIndicesAndWeights");
        m_boneMatricesUniformLocation = glGetUniformLocation(m_program, "boneMatrices");
        enableAttribute(m_edgeOffsetAttributeLocation);
        enableAttribute(m_normalAttributeLocation);
        enableAttribute(m_boneIndicesAndWeightsAttributeLocation);
    }

private:
    GLuint m_normalAttributeLocation;
    GLuint m_edgeOffsetAttributeLocation;
    GLuint m_colorUniformLocation;
    GLuint m_opacityUniformLocation;
    GLuint m_edgeWidthUniformLocation;
    GLuint m_boneIndicesAndWeightsAttributeLocation;
    GLuint m_boneMatricesUniformLocation;
};

class ShadowProgram : public ObjectProgram
{
public:
    ShadowProgram(IRenderDelegate *delegate)
        : ObjectProgram(delegate),
          m_boneIndicesAndWeightsAttributeLocation(0),
          m_boneMatricesUniformLocation(0)
    {
    }
    ~ShadowProgram() {
        m_boneIndicesAndWeightsAttributeLocation = 0;
        m_boneMatricesUniformLocation = 0;
    }

    void setBoneIndicesAndWeights(const GLvoid *ptr, GLsizei stride) {
        glVertexAttribPointer(m_boneIndicesAndWeightsAttributeLocation, 3, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setBoneMatrices(const Scalar *value, size_t size) {
        glUniformMatrix4fv(m_boneMatricesUniformLocation, size, GL_FALSE, value);
    }

protected:
    virtual void getLocations() {
        ObjectProgram::getLocations();
        m_boneIndicesAndWeightsAttributeLocation = glGetAttribLocation(m_program, "inBoneIndicesAndWeights");
        m_boneMatricesUniformLocation = glGetUniformLocation(m_program, "boneMatrices");
        enableAttribute(m_boneIndicesAndWeightsAttributeLocation);
    }

private:
    GLuint m_boneIndicesAndWeightsAttributeLocation;
    GLuint m_boneMatricesUniformLocation;
};

class ModelProgram : public ObjectProgram
{
public:
    ModelProgram(IRenderDelegate *delegate)
        : ObjectProgram(delegate),
          m_cameraPositionUniformLocation(0),
          m_materialColorUniformLocation(0),
          m_materialSpecularUniformLocation(0),
          m_materialShininessUniformLocation(0),
          m_hasSubTextureUniformLocation(0),
          m_isMainSphereMapUniformLocation(0),
          m_isSubSphereMapUniformLocation(0),
          m_isMainAdditiveUniformLocation(0),
          m_isSubAdditiveUniformLocation(0),
          m_subTextureUniformLocation(0),
          m_toonTextureUniformLocation(0),
          m_useToonUniformLocation(0),
          m_boneIndicesAndWeightsAttributeLocation(0),
          m_boneMatricesUniformLocation(0)
    {
    }
    ~ModelProgram() {
        m_cameraPositionUniformLocation = 0;
        m_materialColorUniformLocation = 0;
        m_materialSpecularUniformLocation = 0;
        m_materialShininessUniformLocation = 0;
        m_hasSubTextureUniformLocation = 0;
        m_isMainSphereMapUniformLocation = 0;
        m_isSubSphereMapUniformLocation = 0;
        m_isMainAdditiveUniformLocation = 0;
        m_isSubAdditiveUniformLocation = 0;
        m_subTextureUniformLocation = 0;
        m_toonTextureUniformLocation = 0;
        m_useToonUniformLocation = 0;
        m_boneIndicesAndWeightsAttributeLocation = 0;
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
    void setIsMainSphereMap(bool value) {
        glUniform1i(m_isMainSphereMapUniformLocation, value ? 1 : 0);
    }
    void setIsSubSphereMap(bool value) {
        glUniform1i(m_isSubSphereMapUniformLocation, value ? 1 : 0);
    }
    void setIsMainAdditive(bool value) {
        glUniform1i(m_isMainAdditiveUniformLocation, value ? 1 : 0);
    }
    void setIsSubAdditive(bool value) {
        glUniform1i(m_isSubAdditiveUniformLocation, value ? 1 : 0);
    }
    void setToonEnable(bool value) {
        glUniform1i(m_useToonUniformLocation, value ? 1 : 0);
    }
    void setSubTexture(GLuint value) {
        if (value) {
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, value);
            glUniform1i(m_subTextureUniformLocation, 2);
            glUniform1i(m_hasSubTextureUniformLocation, 1);
        }
        else {
            glUniform1i(m_hasSubTextureUniformLocation, 0);
        }
    }
    void setToonTexture(GLuint value) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, value);
        glUniform1i(m_toonTextureUniformLocation, 1);
    }
    void setBoneIndicesAndWeights(const GLvoid *ptr, GLsizei stride) {
        glVertexAttribPointer(m_boneIndicesAndWeightsAttributeLocation, 3, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setBoneMatrices(const Scalar *value, size_t size) {
        glUniformMatrix4fv(m_boneMatricesUniformLocation, size, GL_FALSE, value);
    }

protected:
    virtual void getLocations() {
        ObjectProgram::getLocations();
        m_cameraPositionUniformLocation = glGetUniformLocation(m_program, "cameraPosition");
        m_materialColorUniformLocation = glGetUniformLocation(m_program, "materialColor");
        m_materialSpecularUniformLocation = glGetUniformLocation(m_program, "materialSpecular");
        m_materialShininessUniformLocation = glGetUniformLocation(m_program, "materialShininess");
        m_hasSubTextureUniformLocation = glGetUniformLocation(m_program, "hasSubTexture");
        m_isMainSphereMapUniformLocation = glGetUniformLocation(m_program, "isMainSphereMap");
        m_isSubSphereMapUniformLocation = glGetUniformLocation(m_program, "isSubSphereMap");
        m_isMainAdditiveUniformLocation = glGetUniformLocation(m_program, "isMainAdditive");
        m_isSubAdditiveUniformLocation = glGetUniformLocation(m_program, "isSubAdditive");
        m_subTextureUniformLocation = glGetUniformLocation(m_program, "subTexture");
        m_toonTextureUniformLocation = glGetUniformLocation(m_program, "toonTexture");
        m_useToonUniformLocation = glGetUniformLocation(m_program, "useToon");
        m_boneIndicesAndWeightsAttributeLocation = glGetAttribLocation(m_program, "inBoneIndicesAndWeights");
        m_boneMatricesUniformLocation = glGetUniformLocation(m_program, "boneMatrices");
        enableAttribute(m_boneIndicesAndWeightsAttributeLocation);
    }

private:
    GLuint m_cameraPositionUniformLocation;
    GLuint m_materialColorUniformLocation;
    GLuint m_materialSpecularUniformLocation;
    GLuint m_materialShininessUniformLocation;
    GLuint m_hasSubTextureUniformLocation;
    GLuint m_isMainSphereMapUniformLocation;
    GLuint m_isSubSphereMapUniformLocation;
    GLuint m_isMainAdditiveUniformLocation;
    GLuint m_isSubAdditiveUniformLocation;
    GLuint m_subTextureUniformLocation;
    GLuint m_toonTextureUniformLocation;
    GLuint m_useToonUniformLocation;
    GLuint m_boneIndicesAndWeightsAttributeLocation;
    GLuint m_boneMatricesUniformLocation;
};

enum VertexBufferObjectType
{
    kModelVertices,
    kEdgeIndices,
    kModelIndices,
    kVertexBufferObjectMax
};

struct PMDModelMaterialPrivate
{
    GLuint mainTextureID;
    GLuint subTextureID;
};

}

namespace vpvl2
{
namespace gl2
{

using namespace vpvl;

class PMDRenderEngine::PrivateContext
        #ifdef VPVL2_LINK_QT
        : protected QGLFunctions
        #endif
{
public:
    PrivateContext()
        : edgeProgram(0),
          modelProgram(0),
          shadowProgram(0),
          zplotProgram(0),
          materials(0),
          hasSingleSphereMap(false),
          hasMultipleSphereMap(false),
          cullFaceState(true),
          isVertexShaderSkinning(false)
    {
#ifdef VPVL2_LINK_QT
        initializeGLFunctions();
#endif
    }
    virtual ~PrivateContext() {
        glDeleteTextures(PMDModel::kCustomTextureMax, toonTextures);
        glDeleteBuffers(kVertexBufferObjectMax, vertexBufferObjects);
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

    void releaseMaterials(vpvl::PMDModel *model) {
        if (materials) {
            const MaterialList &modelMaterials = model->materials();
            const int nmaterials = modelMaterials.count();
            for (int i = 0; i < nmaterials; i++) {
                PMDModelMaterialPrivate &materialPrivate = materials[i];
                glDeleteTextures(1, &materialPrivate.mainTextureID);
                glDeleteTextures(1, &materialPrivate.subTextureID);
            }
            delete[] materials;
            materials = 0;
        }
    }

    EdgeProgram *edgeProgram;
    ModelProgram *modelProgram;
    ShadowProgram *shadowProgram;
    ExtendedZPlotProgram *zplotProgram;
    PMDModelMaterialPrivate *materials;
    pmd::Model::SkinningMeshes mesh;
    GLuint toonTextures[PMDModel::kCustomTextureMax];
    GLuint vertexBufferObjects[kVertexBufferObjectMax];
    bool hasSingleSphereMap;
    bool hasMultipleSphereMap;
    bool cullFaceState;
    bool isVertexShaderSkinning;
};

PMDRenderEngine::PMDRenderEngine(IRenderDelegate *delegate,
                                 const Scene *scene,
                                 cl::PMDAccelerator *accelerator,
                                 pmd::Model *model)
#ifdef VPVL_LINK_QT
    : QGLFunctions(),
      #else
    :
      #endif /* VPVL_LINK_QT */
      m_delegateRef(delegate),
      m_sceneRef(scene),
      m_acceleratorRef(accelerator),
      m_modelRef(model),
      m_context(0)
{
    m_context = new PrivateContext();
#ifdef VPVL2_LINK_QT
    initializeGLFunctions();
#endif
}

PMDRenderEngine::~PMDRenderEngine()
{
    if (m_context) {
        m_context->releaseMaterials(m_modelRef->ptr());
        delete m_context;
        m_context = 0;
    }
#ifdef VPVL2_ENABLE_OPENCL
    delete m_accelerator;
    m_accelerator = 0;
#endif
    m_modelRef = 0;
    m_delegateRef = 0;
    m_sceneRef = 0;
}

IModel *PMDRenderEngine::model() const
{
    return m_modelRef;
}

bool PMDRenderEngine::upload(const IString *dir)
{
    bool ret = true;
    void *context = 0;
    if (!m_context)
        m_context = new PrivateContext();
    m_delegateRef->allocateContext(m_modelRef, context);
    EdgeProgram *edgeProgram = m_context->edgeProgram = new EdgeProgram(m_delegateRef);
    ModelProgram *modelProgram = m_context->modelProgram = new ModelProgram(m_delegateRef);
    ShadowProgram *shadowProgram = m_context->shadowProgram = new ShadowProgram(m_delegateRef);
    ExtendedZPlotProgram *zplotProgram = m_context->zplotProgram = new ExtendedZPlotProgram(m_delegateRef);
    m_context->isVertexShaderSkinning = m_sceneRef->accelerationType() == Scene::kVertexShaderAccelerationType1;
    if (!createProgram(edgeProgram, dir,
                       IRenderDelegate::kEdgeVertexShader,
                       IRenderDelegate::kEdgeWithSkinningVertexShader,
                       IRenderDelegate::kEdgeFragmentShader,
                       context)) {
        return releaseContext0(context);
    }
    if (!createProgram(modelProgram, dir,
                       IRenderDelegate::kModelVertexShader,
                       IRenderDelegate::kModelWithSkinningVertexShader,
                       IRenderDelegate::kModelFragmentShader,
                       context)) {
        return releaseContext0(context);
    }
    if (!createProgram(shadowProgram, dir,
                       IRenderDelegate::kShadowVertexShader,
                       IRenderDelegate::kShadowWithSkinningVertexShader,
                       IRenderDelegate::kShadowFragmentShader,
                       context)) {
        return releaseContext0(context);
    }
    if (!createProgram(zplotProgram, dir,
                       IRenderDelegate::kZPlotVertexShader,
                       IRenderDelegate::kZPlotWithSkinningVertexShader,
                       IRenderDelegate::kZPlotFragmentShader,
                       context)) {
        return releaseContext0(context);
    }
    PMDModel *model = m_modelRef->ptr();
    glGenBuffers(kVertexBufferObjectMax, m_context->vertexBufferObjects);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_context->vertexBufferObjects[kEdgeIndices]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->edgeIndicesCount() * model->strideSize(PMDModel::kEdgeIndicesStride),
                 model->edgeIndicesPointer(), GL_STATIC_DRAW);
    log0(context, IRenderDelegate::kLogInfo,
         "Binding edge indices to the vertex buffer object (ID=%d)",
         m_context->vertexBufferObjects[kEdgeIndices]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelIndices]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->indices().count() * model->strideSize(PMDModel::kIndicesStride),
                 model->indicesPointer(), GL_STATIC_DRAW);
    log0(context, IRenderDelegate::kLogInfo,
         "Binding indices to the vertex buffer object (ID=%d)",
         m_context->vertexBufferObjects[kModelIndices]);
    const vpvl::VertexList &vertices = model->vertices();
    const int nvertices = vertices.count();
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelVertices]);
    glBufferData(GL_ARRAY_BUFFER, nvertices * model->strideSize(PMDModel::kVerticesStride),
                 model->verticesPointer(), GL_DYNAMIC_DRAW);
    log0(context, IRenderDelegate::kLogInfo,
         "Binding model vertices to the vertex buffer object (ID=%d)",
         m_context->vertexBufferObjects[kModelVertices]);
    if (m_context->isVertexShaderSkinning) {
        m_modelRef->getSkinningMeshes(m_context->mesh);
    }
    const MaterialList &materials = model->materials();
    const int nmaterials = materials.count();
    IRenderDelegate::Texture texture;
    GLuint textureID = 0;
    PMDModelMaterialPrivate *materialPrivates = m_context->materials = new PMDModelMaterialPrivate[nmaterials];
    bool hasSingleSphere = false, hasMultipleSphere = false;
    texture.object = &textureID;
    for (int i = 0; i < nmaterials; i++) {
        const Material *material = materials[i];
        PMDModelMaterialPrivate &materialPrivate = materialPrivates[i];
        materialPrivate.mainTextureID = 0;
        materialPrivate.subTextureID = 0;
        const uint8_t *mainTextureName = material->mainTextureName();
        if (*mainTextureName) {
            IString *primary = m_delegateRef->toUnicode(mainTextureName);
            ret = m_delegateRef->uploadTexture(primary, dir, IRenderDelegate::kTexture2D, texture, context);
            delete primary;
            if (ret) {
                materialPrivate.mainTextureID = textureID = *static_cast<const GLuint *>(texture.object);
                log0(context, IRenderDelegate::kLogInfo, "Binding the texture as a primary texture (ID=%d)", textureID);
            }
            else {
                return releaseContext0(context);
            }
        }
        const uint8_t *subTextureName = material->subTextureName();
        if (*subTextureName) {
            IString *second = m_delegateRef->toUnicode(subTextureName);
            ret = m_delegateRef->uploadTexture(second, dir, IRenderDelegate::kTexture2D, texture, context);
            delete second;
            if (ret) {
                materialPrivate.subTextureID = textureID = *static_cast<const GLuint *>(texture.object);
                log0(context, IRenderDelegate::kLogInfo, "Binding the texture as a secondary texture (ID=%d)", textureID);
            }
            else {
                return releaseContext0(context);
            }
        }
        hasSingleSphere |= material->isMainSphereModulate() && !material->isSubSphereAdd();
        hasMultipleSphere |= material->isSubSphereAdd();
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    log0(context, IRenderDelegate::kLogInfo,
         "Sphere map information: hasSingleSphere=%s, hasMultipleSphere=%s",
         hasSingleSphere ? "true" : "false",
         hasMultipleSphere ? "true" : "false");
    m_context->hasSingleSphereMap = hasSingleSphere;
    m_context->hasMultipleSphereMap = hasMultipleSphere;
    IString *s = m_delegateRef->toUnicode(reinterpret_cast<const uint8_t *>("toon0.bmp"));
    ret = m_delegateRef->uploadTexture(s, dir, IRenderDelegate::kToonTexture, texture, context);
    delete s;
    if (ret) {
        m_context->toonTextures[0] = textureID = *static_cast<const GLuint *>(texture.object);
        log0(context, IRenderDelegate::kLogInfo, "Binding the texture as a toon texture (ID=%d)", textureID);
    }
    else {
        return releaseContext0(context);
    }
    static const int nToonTextures = PMDModel::kCustomTextureMax - 1;
    for (int i = 0; i < nToonTextures; i++) {
        const uint8_t *name = model->toonTexture(i);
        s = m_delegateRef->toUnicode(name);
        ret = m_delegateRef->uploadTexture(s, dir, IRenderDelegate::kToonTexture, texture, context);
        delete s;
        if (ret) {
            m_context->toonTextures[i + 1] = textureID = *static_cast<const GLuint *>(texture.object);
            log0(context, IRenderDelegate::kLogInfo, "Binding the texture as a toon texture (ID=%d)", textureID);
        }
        else {
            return releaseContext0(context);
        }
    }
#ifdef VPVL2_ENABLE_OPENCL
    if (m_accelerator && m_accelerator->isAvailable())
        m_accelerator->uploadModel(m_model, m_context->vertexBufferObjects[kModelVertices], context);
#endif
    update();
    IString *modelName = m_delegateRef->toUnicode(model->name());
    log0(context, IRenderDelegate::kLogInfo, "Created the model: %s", modelName->toByteArray());
    delete modelName;
    m_delegateRef->releaseContext(m_modelRef, context);

    return ret;
}

void PMDRenderEngine::update()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_context)
        return;
    PMDModel *model = m_modelRef->ptr();
    model->setLightPosition(-m_sceneRef->light()->direction());
    int nvertices = model->vertices().count();
    size_t strideSize = model->strideSize(PMDModel::kVerticesStride);
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelVertices]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, nvertices * strideSize, model->verticesPointer());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    if (m_context->isVertexShaderSkinning) {
        m_modelRef->updateSkinningMeshes(m_context->mesh);
        m_modelRef->overrideEdgeVerticesOffset();
    }
#ifdef VPVL2_ENABLE_OPENCL
    if (m_accelerator && m_accelerator->isAvailable())
        m_accelerator->updateModel(m_model, m_scene);
#endif
}

void PMDRenderEngine::renderModel()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_context)
        return;
    PMDModel *model = m_modelRef->ptr();
    ModelProgram *modelProgram = m_context->modelProgram;
    modelProgram->bind();
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelVertices]);
    modelProgram->setPosition(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kVerticesStride)),
                              model->strideSize(PMDModel::kVerticesStride));
    modelProgram->setNormal(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kNormalsStride)),
                            model->strideSize(PMDModel::kNormalsStride));
    modelProgram->setTexCoord(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kTextureCoordsStride)),
                              model->strideSize(PMDModel::kTextureCoordsStride));
    float matrix4x4[16];
    m_delegateRef->getMatrix(matrix4x4, m_modelRef,
                          IRenderDelegate::kWorldMatrix
                          | IRenderDelegate::kViewMatrix
                          | IRenderDelegate::kProjectionMatrix
                          | IRenderDelegate::kCameraMatrix);
    modelProgram->setModelViewProjectionMatrix(matrix4x4);
    m_delegateRef->getMatrix(matrix4x4, m_modelRef,
                          IRenderDelegate::kWorldMatrix
                          | IRenderDelegate::kViewMatrix
                          | IRenderDelegate::kCameraMatrix);
    modelProgram->setNormalMatrix(matrix4x4);
    m_delegateRef->getMatrix(matrix4x4, m_modelRef,
                          IRenderDelegate::kWorldMatrix
                          | IRenderDelegate::kViewMatrix
                          | IRenderDelegate::kProjectionMatrix
                          | IRenderDelegate::kLightMatrix);
    modelProgram->setLightViewProjectionMatrix(matrix4x4);
    const ILight *light = m_sceneRef->light();
    void *texture = light->depthTexture();
    GLuint textureID = texture ? *static_cast<GLuint *>(texture) : 0;
    modelProgram->setLightColor(light->color());
    modelProgram->setLightDirection(light->direction());
    modelProgram->setToonEnable(light->isToonEnabled());
    modelProgram->setSoftShadowEnable(light->isSoftShadowEnabled());
    modelProgram->setDepthTextureSize(light->depthTextureSize());
    modelProgram->setCameraPosition(m_sceneRef->camera()->position());
    const Scalar &modelOpacity = m_modelRef->opacity();
    const bool hasModelTransparent = !btFuzzyZero(modelOpacity - 1.0);
    modelProgram->setOpacity(modelOpacity);
    const MaterialList &materials = model->materials();
    const PMDModelMaterialPrivate *materialPrivates = m_context->materials;
    const size_t indexStride = model->strideSize(vpvl::PMDModel::kIndicesStride),
            boneOffset = model->strideOffset(PMDModel::kBoneAttributesStride),
            boneStride = model->strideSize(PMDModel::kVerticesStride);
    const int nmaterials = materials.count();
    const bool isVertexShaderSkinning = m_context->isVertexShaderSkinning;
    //const bool hasSingleSphereMap = m_context->hasSingleSphereMap;
    const bool hasMultipleSphereMap = m_context->hasMultipleSphereMap;
    const Vector3 &lc = light->color();
    Color diffuse, specular;
    size_t offset = 0;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelIndices]);
    for (int i = 0; i < nmaterials; i++) {
        const Material *material = materials[i];
        const PMDModelMaterialPrivate &materialPrivate = materialPrivates[i];
        const Scalar &materialOpacity = material->opacity();
        const bool isMainSphereAdd = material->isMainSphereAdd();
        const Color &ma = material->ambient(), &md = material->diffuse(), &ms = material->specular();
        diffuse.setValue(ma.x() + md.x() * lc.x(), ma.y() + md.y() * lc.y(), ma.z() + md.z() * lc.z(), md.w() * materialOpacity);
        specular.setValue(ms.x() * lc.x(), ms.y() * lc.y(), ms.z() * lc.z(), 1.0);
        modelProgram->setMaterialColor(diffuse);
        modelProgram->setMaterialSpecular(specular);
        modelProgram->setMaterialShininess(material->shiness());
        modelProgram->setMainTexture(materialPrivate.mainTextureID);
        modelProgram->setToonTexture(m_context->toonTextures[material->toonID()]);
        modelProgram->setIsMainSphereMap(isMainSphereAdd || material->isMainSphereModulate());
        modelProgram->setIsMainAdditive(isMainSphereAdd);
        if (hasMultipleSphereMap) {
            const bool isSubSphereAdd = material->isSubSphereAdd();
            modelProgram->setIsSubSphereMap(isSubSphereAdd || material->isSubSphereModulate());
            modelProgram->setIsSubAdditive(isSubSphereAdd);
            modelProgram->setSubTexture(materialPrivate.subTextureID);
        }
        if (texture && !btFuzzyZero(materialOpacity - 0.98)) {
            modelProgram->setDepthTexture(textureID);
        }
        else {
            modelProgram->setDepthTexture(0);
        }
        if (isVertexShaderSkinning) {
            const pmd::Model::SkinningMeshes &mesh = m_context->mesh;
            modelProgram->setBoneIndicesAndWeights(reinterpret_cast<const GLvoid *>(boneOffset), boneStride);
            modelProgram->setBoneMatrices(mesh.matrices[i], mesh.bones[i].size());
        }
        if ((hasModelTransparent && m_context->cullFaceState) ||
                (!btFuzzyZero(materialOpacity - 1.0f) && m_context->cullFaceState)) {
            glDisable(GL_CULL_FACE);
            m_context->cullFaceState = false;
        }
        else if (!m_context->cullFaceState) {
            glEnable(GL_CULL_FACE);
            m_context->cullFaceState = true;
        }
        const int nindices = material->countIndices();
        glDrawElements(GL_TRIANGLES, nindices, GL_UNSIGNED_SHORT, reinterpret_cast<const GLvoid *>(offset));
        offset += nindices * indexStride;
    }
    modelProgram->unbind();
    if (!m_context->cullFaceState) {
        glEnable(GL_CULL_FACE);
        m_context->cullFaceState = true;
    }
}

void PMDRenderEngine::renderShadow()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_context)
        return;
    ShadowProgram *shadowProgram = m_context->shadowProgram;
    PMDModel *model = m_modelRef->ptr();
    shadowProgram->bind();
    const MaterialList &materials = model->materials();
    const int nmaterials = materials.count();
    const bool isVertexShaderSkinning = m_sceneRef->accelerationType() == Scene::kVertexShaderAccelerationType1;
    const size_t indexStride = model->strideSize(vpvl::PMDModel::kIndicesStride),
            boneOffset = model->strideOffset(PMDModel::kBoneAttributesStride),
            boneStride = model->strideSize(PMDModel::kVerticesStride);
    float matrix4x4[16];
    size_t offset = 0;
    m_delegateRef->getMatrix(matrix4x4, m_modelRef,
                          IRenderDelegate::kWorldMatrix
                          | IRenderDelegate::kViewMatrix
                          | IRenderDelegate::kProjectionMatrix
                          | IRenderDelegate::kShadowMatrix);
    shadowProgram->setModelViewProjectionMatrix(matrix4x4);
    ILight *light = m_sceneRef->light();
    shadowProgram->setLightColor(light->color());
    shadowProgram->setLightDirection(light->direction());
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelVertices]);
    shadowProgram->setPosition(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kVerticesStride)),
                               model->strideSize(PMDModel::kVerticesStride));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelIndices]);
    glCullFace(GL_FRONT);
    for (int i = 0; i < nmaterials; i++) {
        const Material *material = materials[i];
        if (isVertexShaderSkinning) {
            const pmd::Model::SkinningMeshes &mesh = m_context->mesh;
            shadowProgram->setBoneIndicesAndWeights(reinterpret_cast<const GLvoid *>(boneOffset), boneStride);
            shadowProgram->setBoneMatrices(mesh.matrices[i], mesh.bones[i].size());
        }
        const int nindices = material->countIndices();
        glDrawElements(GL_TRIANGLES, nindices, GL_UNSIGNED_SHORT, reinterpret_cast<const GLvoid *>(offset));
        offset += nindices * indexStride;
    }
    glCullFace(GL_BACK);
    shadowProgram->unbind();
}

void PMDRenderEngine::renderZPlot()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_context)
        return;
    ExtendedZPlotProgram *zplotProgram = m_context->zplotProgram;
    PMDModel *model = m_modelRef->ptr();
    float matrix4x4[16];
    zplotProgram->bind();
    m_delegateRef->getMatrix(matrix4x4, m_modelRef,
                          IRenderDelegate::kViewMatrix
                          | IRenderDelegate::kProjectionMatrix
                          | IRenderDelegate::kLightMatrix);
    zplotProgram->setModelViewProjectionMatrix(matrix4x4);
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelVertices]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelIndices]);
    zplotProgram->setPosition(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kVerticesStride)),
                              model->strideSize(PMDModel::kVerticesStride));
    glCullFace(GL_FRONT);
    const vpvl::MaterialList &materials = model->materials();
    const int nmaterials = materials.count();
    const bool isVertexShaderSkinning = m_sceneRef->accelerationType() == Scene::kVertexShaderAccelerationType1;
    const size_t boneOffset = model->strideOffset(PMDModel::kBoneAttributesStride),
            boneStride = model->strideSize(PMDModel::kVerticesStride);
    size_t offset = 0, size = model->strideSize(vpvl::PMDModel::kIndicesStride);
    for (int i = 0; i < nmaterials; i++) {
        const vpvl::Material *material = materials[i];
        const int nindices = material->countIndices();
        if (!btFuzzyZero(material->opacity() - 0.98)) {
            if (isVertexShaderSkinning) {
                const pmd::Model::SkinningMeshes &mesh = m_context->mesh;
                zplotProgram->setBoneIndicesAndWeights(reinterpret_cast<const GLvoid *>(boneOffset), boneStride);
                zplotProgram->setBoneMatrices(mesh.matrices[i], mesh.bones[i].size());
            }
            glDrawElements(GL_TRIANGLES, nindices, GL_UNSIGNED_SHORT, reinterpret_cast<const GLvoid *>(offset));
        }
        offset += nindices * size;
    }
    glCullFace(GL_BACK);
    zplotProgram->unbind();
}

void PMDRenderEngine::renderEdge()
{
    if (!m_modelRef || !m_modelRef->isVisible() || btFuzzyZero(m_modelRef->edgeWidth()) || !m_context)
        return;
    EdgeProgram *edgeProgram = m_context->edgeProgram;
    PMDModel *model = m_modelRef->ptr();
    edgeProgram->bind();
    edgeProgram->setColor(m_modelRef->edgeColor());
    edgeProgram->setOpacity(m_modelRef->opacity());
    float matrix4x4[16];
    m_delegateRef->getMatrix(matrix4x4, m_modelRef,
                          IRenderDelegate::kWorldMatrix
                          | IRenderDelegate::kViewMatrix
                          | IRenderDelegate::kProjectionMatrix
                          | IRenderDelegate::kCameraMatrix);
    edgeProgram->setModelViewProjectionMatrix(matrix4x4);
    glCullFace(GL_FRONT);
    glDisable(GL_BLEND);
    const bool isVertexShaderSkinning = m_sceneRef->accelerationType() == Scene::kVertexShaderAccelerationType1;
    if (isVertexShaderSkinning) {
        const pmd::Model::SkinningMeshes &mesh = m_context->mesh;
        const size_t boneOffset = model->strideOffset(PMDModel::kBoneAttributesStride),
                boneStride = model->strideSize(PMDModel::kVerticesStride);
        const vpvl::MaterialList &materials = model->materials();
        const ICamera *camera = m_sceneRef->camera();
        const Vector3 &cameraPosition = camera->position() + Vector3(0, 0, camera->distance());
        const int nmaterials = materials.count();
        glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelVertices]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelIndices]);
        edgeProgram->setPosition(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kVerticesStride)),
                                 model->strideSize(PMDModel::kVerticesStride));
        edgeProgram->setNormal(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kNormalsStride)),
                               model->strideSize(PMDModel::kVerticesStride));
        edgeProgram->setEdgeOffset(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kEdgeVerticesStride)),
                                   model->strideSize(PMDModel::kVerticesStride));
        edgeProgram->setBoneIndicesAndWeights(reinterpret_cast<const GLvoid *>(boneOffset), boneStride);
        edgeProgram->setEdgeWidth(m_modelRef->edgeScaleFactor(cameraPosition) * m_modelRef->edgeWidth());
        size_t offset = 0, size = model->strideSize(vpvl::PMDModel::kIndicesStride);
        for (int i = 0; i < nmaterials; i++) {
            const vpvl::Material *material = materials[i];
            const int nindices = material->countIndices();
            edgeProgram->setBoneMatrices(mesh.matrices[i], mesh.bones[i].size());
            glDrawElements(GL_TRIANGLES, nindices, GL_UNSIGNED_SHORT, reinterpret_cast<const GLvoid *>(offset));
            offset += nindices * size;
        }
    }
    else {
        glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelVertices]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_context->vertexBufferObjects[kEdgeIndices]);
        edgeProgram->setPosition(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kEdgeVerticesStride)),
                                 model->strideSize(PMDModel::kEdgeVerticesStride));
        glDrawElements(GL_TRIANGLES, model->edgeIndicesCount(), GL_UNSIGNED_SHORT, 0);
    }
    glEnable(GL_BLEND);
    glCullFace(GL_BACK);
    edgeProgram->unbind();
}

bool PMDRenderEngine::hasPreProcess() const
{
    return false;
}

bool PMDRenderEngine::hasPostProcess() const
{
    return false;
}

void PMDRenderEngine::preparePostProcess()
{
    /* do nothing */
}

void PMDRenderEngine::performPreProcess()
{
    /* do nothing */
}

void PMDRenderEngine::performPostProcess()
{
    /* do nothing */
}

IEffect *PMDRenderEngine::effect(IEffect::ScriptOrderType /* type */) const
{
    return 0;
}

void PMDRenderEngine::setEffect(IEffect::ScriptOrderType /* type */, IEffect * /* effect */, const IString * /* dir */)
{
    /* do nothing */
}

void PMDRenderEngine::log0(void *context, IRenderDelegate::LogLevel level, const char *format...)
{
    va_list ap;
    va_start(ap, format);
    m_delegateRef->log(context, level, format, ap);
    va_end(ap);
}

bool PMDRenderEngine::createProgram(BaseShaderProgram *program,
                                    const IString *dir,
                                    IRenderDelegate::ShaderType vertexShaderType,
                                    IRenderDelegate::ShaderType vertexSkinningShaderType,
                                    IRenderDelegate::ShaderType fragmentShaderType,
                                    void *context)
{
    IString *vertexShaderSource = 0;
    IString *fragmentShaderSource = 0;
    if (m_context->isVertexShaderSkinning)
        vertexShaderSource = m_delegateRef->loadShaderSource(vertexSkinningShaderType, m_modelRef, dir, context);
    else
        vertexShaderSource = m_delegateRef->loadShaderSource(vertexShaderType, m_modelRef, dir, context);
    fragmentShaderSource = m_delegateRef->loadShaderSource(fragmentShaderType, m_modelRef, dir, context);
    program->addShaderSource(vertexShaderSource, GL_VERTEX_SHADER, context);
    program->addShaderSource(fragmentShaderSource, GL_FRAGMENT_SHADER, context);
    bool ok = program->linkProgram(context);
    delete vertexShaderSource;
    delete fragmentShaderSource;
    return ok;
}

bool PMDRenderEngine::releaseContext0(void *context)
{
    m_delegateRef->releaseContext(m_modelRef, context);
    return false;
}

} /* namespace gl2 */
} /* namespace vpvl2 */
