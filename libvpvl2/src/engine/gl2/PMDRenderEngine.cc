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
        glEnableVertexAttribArray(m_boneIndicesAndWeightsAttributeLocation);
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
          m_edgeAttributeLocation(0),
          m_colorUniformLocation(0),
          m_opacityUniformLocation(0),
          m_boneIndicesAndWeightsAttributeLocation(0),
          m_boneMatricesUniformLocation(0)
    {
    }
    ~EdgeProgram() {
        m_normalAttributeLocation = 0;
        m_edgeAttributeLocation = 0;
        m_colorUniformLocation = 0;
        m_opacityUniformLocation = 0;
        m_boneIndicesAndWeightsAttributeLocation = 0;
        m_boneMatricesUniformLocation = 0;
    }

    void setEdge(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_edgeAttributeLocation);
        glVertexAttribPointer(m_edgeAttributeLocation, 1, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setColor(const Vector3 &value) {
        glUniform4fv(m_colorUniformLocation, 1, value);
    }
    void setNormal(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_normalAttributeLocation);
        glVertexAttribPointer(m_normalAttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setOpacity(const Scalar &value) {
        glUniform1f(m_opacityUniformLocation, value);
    }
    void setBoneIndicesAndWeights(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_boneIndicesAndWeightsAttributeLocation);
        glVertexAttribPointer(m_boneIndicesAndWeightsAttributeLocation, 3, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setBoneMatrices(const Scalar *value, size_t size) {
        glUniformMatrix4fv(m_boneMatricesUniformLocation, size, GL_FALSE, value);
    }

protected:
    virtual void getLocations() {
        BaseShaderProgram::getLocations();
        m_normalAttributeLocation = glGetAttribLocation(m_program, "inNormal");
        m_edgeAttributeLocation = glGetAttribLocation(m_program, "inEdgeOffset");
        m_colorUniformLocation = glGetUniformLocation(m_program, "color");
        m_opacityUniformLocation = glGetUniformLocation(m_program, "opacity");
        m_boneIndicesAndWeightsAttributeLocation = glGetAttribLocation(m_program, "inBoneIndicesAndWeights");
        m_boneMatricesUniformLocation = glGetUniformLocation(m_program, "boneMatrices");
    }

private:
    GLuint m_normalAttributeLocation;
    GLuint m_edgeAttributeLocation;
    GLuint m_colorUniformLocation;
    GLuint m_opacityUniformLocation;
    GLuint m_boneIndicesAndWeightsAttributeLocation;
    GLuint m_boneMatricesUniformLocation;
};

class ShadowProgram : public ObjectProgram
{
public:
    ShadowProgram(IRenderDelegate *delegate)
        : ObjectProgram(delegate),
          m_shadowMatrixUniformLocation(0),
          m_boneIndicesAndWeightsAttributeLocation(0),
          m_boneMatricesUniformLocation(0)
    {
    }
    ~ShadowProgram() {
        m_shadowMatrixUniformLocation = 0;
        m_boneIndicesAndWeightsAttributeLocation = 0;
        m_boneMatricesUniformLocation = 0;
    }

    void setShadowMatrix(const float value[16]) {
        glUniformMatrix4fv(m_shadowMatrixUniformLocation, 1, GL_FALSE, value);
    }
    void setBoneIndicesAndWeights(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_boneIndicesAndWeightsAttributeLocation);
        glVertexAttribPointer(m_boneIndicesAndWeightsAttributeLocation, 3, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setBoneMatrices(const Scalar *value, size_t size) {
        glUniformMatrix4fv(m_boneMatricesUniformLocation, size, GL_FALSE, value);
    }

protected:
    virtual void getLocations() {
        ObjectProgram::getLocations();
        m_shadowMatrixUniformLocation = glGetUniformLocation(m_program, "shadowMatrix");
        m_boneIndicesAndWeightsAttributeLocation = glGetAttribLocation(m_program, "inBoneIndicesAndWeights");
        m_boneMatricesUniformLocation = glGetUniformLocation(m_program, "boneMatrices");
    }

private:
    GLuint m_shadowMatrixUniformLocation;
    GLuint m_boneIndicesAndWeightsAttributeLocation;
    GLuint m_boneMatricesUniformLocation;
};

class ModelProgram : public ObjectProgram
{
public:
    ModelProgram(IRenderDelegate *delegate)
        : ObjectProgram(delegate),
          m_toonTexCoordAttributeLocation(0),
          m_modelViewInverseMatrixUniformLocation(0),
          m_materialAmbientUniformLocation(0),
          m_materialDiffuseUniformLocation(0),
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
        m_toonTexCoordAttributeLocation = 0;
        m_modelViewInverseMatrixUniformLocation = 0;
        m_materialAmbientUniformLocation = 0;
        m_materialDiffuseUniformLocation = 0;
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

    void setToonTexCoord(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_toonTexCoordAttributeLocation);
        glVertexAttribPointer(m_toonTexCoordAttributeLocation, 2, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setModelViewInverseMatrix(const GLfloat value[16]) {
        glUniformMatrix4fv(m_modelViewInverseMatrixUniformLocation, 1, GL_FALSE, value);
    }
    void setMaterialAmbient(const Color &value) {
        glUniform3fv(m_materialAmbientUniformLocation, 1, value);
    }
    void setMaterialDiffuse(const Color &value) {
        glUniform4fv(m_materialDiffuseUniformLocation, 1, value);
    }
    void setMaterialSpecular(const Color &value) {
        glUniform4fv(m_materialSpecularUniformLocation, 1, value);
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
        glEnableVertexAttribArray(m_boneIndicesAndWeightsAttributeLocation);
        glVertexAttribPointer(m_boneIndicesAndWeightsAttributeLocation, 3, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setBoneMatrices(const Scalar *value, size_t size) {
        glUniformMatrix4fv(m_boneMatricesUniformLocation, size, GL_FALSE, value);
    }

protected:
    virtual void getLocations() {
        ObjectProgram::getLocations();
        m_toonTexCoordAttributeLocation = glGetAttribLocation(m_program, "inToonCoord");
        m_modelViewInverseMatrixUniformLocation = glGetUniformLocation(m_program, "modelViewInverseMatrix");
        m_materialAmbientUniformLocation = glGetUniformLocation(m_program, "materialAmbient");
        m_materialDiffuseUniformLocation = glGetUniformLocation(m_program, "materialDiffuse");
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
    }

private:
    GLuint m_toonTexCoordAttributeLocation;
    GLuint m_modelViewInverseMatrixUniformLocation;
    GLuint m_materialAmbientUniformLocation;
    GLuint m_materialDiffuseUniformLocation;
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

#ifdef VPVL2_LINK_QT
    void initializeContext(const QGLContext *context) {
        initializeGLFunctions(context);
    }
#endif /* VPVL2_LINK_QT */

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
      m_delegate(delegate),
      m_scene(scene),
      m_accelerator(accelerator),
      m_model(model),
      m_context(0)
{
    m_context = new PrivateContext();
}

PMDRenderEngine::~PMDRenderEngine()
{
    if (m_context) {
        m_context->releaseMaterials(m_model->ptr());
        delete m_context;
        m_context = 0;
    }
#ifdef VPVL2_ENABLE_OPENCL
    delete m_accelerator;
    m_accelerator = 0;
#endif
    m_model = 0;
    m_delegate = 0;
    m_scene = 0;
}

IModel *PMDRenderEngine::model() const
{
    return m_model;
}

bool PMDRenderEngine::upload(const IString *dir)
{
    bool ret = true;
    void *context = 0;
    if (!m_context)
        m_context = new PrivateContext();
    m_delegate->allocateContext(m_model, context);
    EdgeProgram *edgeProgram = m_context->edgeProgram = new EdgeProgram(m_delegate);
    ModelProgram *modelProgram = m_context->modelProgram = new ModelProgram(m_delegate);
    ShadowProgram *shadowProgram = m_context->shadowProgram = new ShadowProgram(m_delegate);
    ExtendedZPlotProgram *zplotProgram = m_context->zplotProgram = new ExtendedZPlotProgram(m_delegate);
#ifdef VPVL2_LINK_QT
    const QGLContext *glContext = QGLContext::currentContext();
    initializeGLFunctions(glContext);
    m_context->initializeContext(glContext);
    edgeProgram->initializeContext(glContext);
    modelProgram->initializeContext(glContext);
    shadowProgram->initializeContext(glContext);
    zplotProgram->initializeContext(glContext);
#endif /* VPVL2_LINK_QT */
    IString *vertexShaderSource = 0;
    IString *fragmentShaderSource = 0;
    const bool isVertexShaderSkinning = m_scene->accelerationType() == Scene::kVertexShaderAccelerationType1;
    m_context->isVertexShaderSkinning = isVertexShaderSkinning;
    if (isVertexShaderSkinning)
        vertexShaderSource = m_delegate->loadShaderSource(IRenderDelegate::kEdgeWithSkinningVertexShader, m_model, context);
    else
        vertexShaderSource = m_delegate->loadShaderSource(IRenderDelegate::kEdgeVertexShader, m_model, context);
    fragmentShaderSource = m_delegate->loadShaderSource(IRenderDelegate::kEdgeFragmentShader, m_model, context);
    edgeProgram->addShaderSource(vertexShaderSource, GL_VERTEX_SHADER, context);
    edgeProgram->addShaderSource(fragmentShaderSource, GL_FRAGMENT_SHADER, context);
    ret = edgeProgram->linkProgram(context);
    delete vertexShaderSource;
    delete fragmentShaderSource;
    if (!ret) {
        delete m_context;
        m_context = 0;
        return ret;
    }
    if (isVertexShaderSkinning)
        vertexShaderSource = m_delegate->loadShaderSource(IRenderDelegate::kModelWithSkinningVertexShader, m_model, context);
    else
        vertexShaderSource = m_delegate->loadShaderSource(IRenderDelegate::kModelVertexShader, m_model, context);
    fragmentShaderSource = m_delegate->loadShaderSource(IRenderDelegate::kModelFragmentShader, m_model, context);
    modelProgram->addShaderSource(vertexShaderSource, GL_VERTEX_SHADER, context);
    modelProgram->addShaderSource(fragmentShaderSource, GL_FRAGMENT_SHADER, context);
    ret = modelProgram->linkProgram(context);
    delete vertexShaderSource;
    delete fragmentShaderSource;
    if (!ret) {
        delete m_context;
        m_context = 0;
        return ret;
    }
    if (isVertexShaderSkinning)
        vertexShaderSource = m_delegate->loadShaderSource(IRenderDelegate::kShadowWithSkinningVertexShader, m_model, context);
    else
        vertexShaderSource = m_delegate->loadShaderSource(IRenderDelegate::kShadowVertexShader, m_model, context);
    fragmentShaderSource = m_delegate->loadShaderSource(IRenderDelegate::kShadowFragmentShader, m_model, context);
    shadowProgram->addShaderSource(vertexShaderSource, GL_VERTEX_SHADER, context);
    shadowProgram->addShaderSource(fragmentShaderSource, GL_FRAGMENT_SHADER, context);
    ret = shadowProgram->linkProgram(context);
    delete vertexShaderSource;
    delete fragmentShaderSource;
    if (!ret) {
        delete m_context;
        m_context = 0;
        return ret;
    }
    if (isVertexShaderSkinning)
        vertexShaderSource = m_delegate->loadShaderSource(IRenderDelegate::kZPlotWithSkinningVertexShader, m_model, context);
    else
        vertexShaderSource = m_delegate->loadShaderSource(IRenderDelegate::kZPlotVertexShader, m_model, context);
    fragmentShaderSource = m_delegate->loadShaderSource(IRenderDelegate::kZPlotFragmentShader, m_model, context);
    zplotProgram->addShaderSource(vertexShaderSource, GL_VERTEX_SHADER, context);
    zplotProgram->addShaderSource(fragmentShaderSource, GL_FRAGMENT_SHADER, context);
    ret = zplotProgram->linkProgram(context);
    delete vertexShaderSource;
    delete fragmentShaderSource;
    if (!ret) {
        delete m_context;
        m_context = 0;
        return ret;
    }
    PMDModel *model = m_model->ptr();
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
    const int nvertices = model->vertices().count();
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelVertices]);
    glBufferData(GL_ARRAY_BUFFER, nvertices * model->strideSize(PMDModel::kVerticesStride),
                 model->verticesPointer(), GL_DYNAMIC_DRAW);
    log0(context, IRenderDelegate::kLogInfo,
         "Binding model vertices to the vertex buffer object (ID=%d)",
         m_context->vertexBufferObjects[kModelVertices]);
    if (isVertexShaderSkinning)
        m_model->getSkinningMeshes(m_context->mesh);
    const MaterialList &materials = model->materials();
    const int nmaterials = materials.count();
    GLuint textureID = 0;
    PMDModelMaterialPrivate *materialPrivates = m_context->materials = new PMDModelMaterialPrivate[nmaterials];
    bool hasSingleSphere = false, hasMultipleSphere = false;
    for (int i = 0; i < nmaterials; i++) {
        const Material *material = materials[i];
        IString *primary = m_delegate->toUnicode(material->mainTextureName());
        IString *second = m_delegate->toUnicode(material->subTextureName());
        PMDModelMaterialPrivate &materialPrivate = materialPrivates[i];
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
    if (m_delegate->uploadToonTexture(context, "toon0.bmp", dir, &textureID)) {
        m_context->toonTextures[0] = textureID;
        log0(context, IRenderDelegate::kLogInfo, "Binding the texture as a toon texture (ID=%d)", textureID);
    }
    for (int i = 0; i < PMDModel::kCustomTextureMax; i++) {
        const uint8_t *name = model->toonTexture(i);
        if (m_delegate->uploadToonTexture(context, reinterpret_cast<const char *>(name), dir, &textureID)) {
            m_context->toonTextures[i + 1] = textureID;
            log0(context, IRenderDelegate::kLogInfo, "Binding the texture as a toon texture (ID=%d)", textureID);
        }
    }
#ifdef VPVL2_ENABLE_OPENCL
    if (m_accelerator && m_accelerator->isAvailable())
        m_accelerator->uploadModel(m_model, m_context->vertexBufferObjects[kModelVertices], context);
#endif
    model->updateImmediate();
    update();
    IString *modelName = m_delegate->toUnicode(model->name());
    log0(context, IRenderDelegate::kLogInfo, "Created the model: %s", modelName->toByteArray());
    delete modelName;
    m_delegate->releaseContext(m_model, context);

    return ret;
}

void PMDRenderEngine::update()
{
    if (!m_model->isVisible() || !m_context)
        return;
    PMDModel *model = m_model->ptr();
    model->setLightPosition(-m_scene->light()->direction());
    int nvertices = model->vertices().count();
    size_t strideSize = model->strideSize(PMDModel::kVerticesStride);
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelVertices]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, nvertices * strideSize, model->verticesPointer());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    if (m_context->isVertexShaderSkinning)
        m_model->updateSkinningMeshes(m_context->mesh);
#ifdef VPVL2_ENABLE_OPENCL
    if (m_accelerator && m_accelerator->isAvailable())
        m_accelerator->updateModel(m_model);
#endif
}

void PMDRenderEngine::renderModel()
{
    if (!m_model->isVisible() || !m_context)
        return;
    PMDModel *model = m_model->ptr();
    ModelProgram *modelProgram = m_context->modelProgram;
    modelProgram->bind();
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelVertices]);
    modelProgram->setPosition(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kVerticesStride)),
                              model->strideSize(PMDModel::kVerticesStride));
    modelProgram->setNormal(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kNormalsStride)),
                            model->strideSize(PMDModel::kNormalsStride));
    modelProgram->setTexCoord(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kTextureCoordsStride)),
                              model->strideSize(PMDModel::kTextureCoordsStride));
    const Scene::IMatrices *matrices = m_scene->matrices();
    float matrix4x4[16];
    matrices->getModelViewProjection(matrix4x4);
    modelProgram->setModelViewProjectionMatrix(matrix4x4);
    matrices->getNormal(matrix4x4);
    modelProgram->setNormalMatrix(matrix4x4);
    m_scene->camera()->modelViewTransform().inverse().getOpenGLMatrix(matrix4x4);
    modelProgram->setModelViewInverseMatrix(matrix4x4);
    matrices->getLightViewProjection(matrix4x4);
    modelProgram->setLightViewProjectionMatrix(matrix4x4);
    const Scene::ILight *light = m_scene->light();
    void *texture = light->depthTexture();
    GLuint textureID = texture ? *static_cast<GLuint *>(texture) : 0;
    modelProgram->setLightColor(light->color());
    modelProgram->setLightDirection(light->direction());
    modelProgram->setToonEnable(light->isToonEnabled());
    modelProgram->setSoftShadowEnable(light->isSoftShadowEnabled());
    modelProgram->setDepthTextureSize(light->depthTextureSize());
    const Scalar &modelOpacity = m_model->opacity();
    const bool hasModelTransparent = !btFuzzyZero(modelOpacity - 1.0);
    modelProgram->setOpacity(modelOpacity);
    modelProgram->setToonTexCoord(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kToonTextureStride)),
                                  model->strideSize(PMDModel::kToonTextureStride));

    const MaterialList &materials = model->materials();
    const PMDModelMaterialPrivate *materialPrivates = m_context->materials;
    const size_t indexStride = model->strideSize(vpvl::PMDModel::kIndicesStride),
            boneOffset = model->strideOffset(PMDModel::kBoneAttributesStride),
            boneStride = model->strideSize(PMDModel::kVerticesStride);
    const int nmaterials = materials.count();
    const bool isVertexShaderSkinning = m_context->isVertexShaderSkinning;
    //const bool hasSingleSphereMap = m_context->hasSingleSphereMap;
    const bool hasMultipleSphereMap = m_context->hasMultipleSphereMap;
    Color diffuse;
    size_t offset = 0;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelIndices]);
    for (int i = 0; i < nmaterials; i++) {
        const Material *material = materials[i];
        const PMDModelMaterialPrivate &materialPrivate = materialPrivates[i];
        const Scalar &materialOpacity = material->opacity();
        const bool isMainSphereAdd = material->isMainSphereAdd();
        diffuse = material->diffuse();
        diffuse.setW(diffuse.w() * materialOpacity);
        modelProgram->setMaterialAmbient(material->ambient());
        modelProgram->setMaterialDiffuse(diffuse);
        modelProgram->setMaterialSpecular(material->specular());
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
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    modelProgram->unbind();
    if (!m_context->cullFaceState) {
        glEnable(GL_CULL_FACE);
        m_context->cullFaceState = true;
    }
}

void PMDRenderEngine::renderShadow()
{
    if (!m_model->isVisible() || !m_context)
        return;
    static const Vector3 plane(0.0f, 1.0f, 0.0f);
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelVertices]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelIndices]);
    const Scene::ILight *light = m_scene->light();
    const Vector3 &direction = light->direction();
    const Scalar dot = plane.dot(-direction);
    float shadowMatrix[16];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int index = (i << 2) + j;
            shadowMatrix[index] = plane[i] * direction[j];
            if (i == j)
                shadowMatrix[index] += dot;
        }
    }
    ShadowProgram *shadowProgram = m_context->shadowProgram;
    PMDModel *model = m_model->ptr();
    shadowProgram->bind();
    const Scene::IMatrices *matrices = m_scene->matrices();
    const MaterialList &materials = model->materials();
    const int nmaterials = materials.count();
    const bool isVertexShaderSkinning = m_scene->accelerationType() == Scene::kVertexShaderAccelerationType1;
    const size_t indexStride = model->strideSize(vpvl::PMDModel::kIndicesStride),
            boneOffset = model->strideOffset(PMDModel::kBoneAttributesStride),
            boneStride = model->strideSize(PMDModel::kVerticesStride);
    float matrix4x4[16];
    size_t offset = 0;
    matrices->getModelViewProjection(matrix4x4);
    shadowProgram->setModelViewProjectionMatrix(matrix4x4);
    shadowProgram->setShadowMatrix(shadowMatrix);
    shadowProgram->setLightColor(light->color());
    shadowProgram->setLightDirection(direction);
    shadowProgram->setPosition(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kVerticesStride)),
                               model->strideSize(PMDModel::kVerticesStride));
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
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    shadowProgram->unbind();
}

void PMDRenderEngine::renderZPlot()
{
    if (!m_model->isVisible() || !m_context)
        return;
    ExtendedZPlotProgram *zplotProgram = m_context->zplotProgram;
    PMDModel *model = m_model->ptr();
    float matrix4x4[16];
    zplotProgram->bind();
    m_scene->matrices()->getLightViewProjection(matrix4x4);
    zplotProgram->setModelViewProjectionMatrix(matrix4x4);
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelVertices]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelIndices]);
    zplotProgram->setPosition(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kVerticesStride)),
                              model->strideSize(PMDModel::kVerticesStride));
    glCullFace(GL_FRONT);
    const vpvl::MaterialList &materials = model->materials();
    const int nmaterials = materials.count();
    const bool isVertexShaderSkinning = m_scene->accelerationType() == Scene::kVertexShaderAccelerationType1;
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
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    zplotProgram->unbind();
}

void PMDRenderEngine::renderEdge()
{
    if (!m_model->isVisible() || btFuzzyZero(m_model->edgeWidth()) || !m_context)
        return;
    EdgeProgram *edgeProgram = m_context->edgeProgram;
    PMDModel *model = m_model->ptr();
    edgeProgram->bind();
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelVertices]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_context->vertexBufferObjects[kEdgeIndices]);
    edgeProgram->setColor(m_model->edgeColor());
    edgeProgram->setOpacity(m_model->opacity());
    float matrix4x4[16];
    m_scene->matrices()->getModelViewProjection(matrix4x4);
    edgeProgram->setModelViewProjectionMatrix(matrix4x4);
    edgeProgram->setPosition(reinterpret_cast<const GLvoid *>(model->strideOffset(PMDModel::kEdgeVerticesStride)),
                             model->strideSize(PMDModel::kEdgeVerticesStride));
    const bool isVertexShaderSkinning = m_scene->accelerationType() == Scene::kVertexShaderAccelerationType1;
    if (isVertexShaderSkinning) {
        /*
          FIXME: implement this
        const pmd::Model::SkinningMeshes &mesh = m_context->mesh;
        const size_t boneOffset = model->strideOffset(PMDModel::kBoneAttributesStride),
                boneStride = model->strideSize(PMDModel::kVerticesStride);
        edgeProgram->setBoneIndicesAndWeights(reinterpret_cast<const GLvoid *>(boneOffset), boneStride);
        edgeProgram->setBoneMatrices(mesh.matrices[i], mesh.bones[i].size());
        */
    }
    glCullFace(GL_FRONT);
    glDrawElements(GL_TRIANGLES, model->edgeIndicesCount(), GL_UNSIGNED_SHORT, 0);
    glCullFace(GL_BACK);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    edgeProgram->unbind();
}

void PMDRenderEngine::log0(void *context, IRenderDelegate::LogLevel level, const char *format...)
{
    va_list ap;
    va_start(ap, format);
    m_delegate->log(context, level, format, ap);
    va_end(ap);
}

} /* namespace gl2 */
} /* namespace vpvl2 */
