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
#include "vpvl2/gl2/PMXRenderEngine.h"
#include "vpvl2/pmx/Material.h"
#include "vpvl2/pmx/Model.h"
#include "vpvl2/pmx/Vertex.h"
#ifdef VPVL2_ENABLE_OPENCL
#include "vpvl2/cl/Context.h"
#include "vpvl2/cl/PMXAccelerator.h"
#endif

#include "EngineCommon.h"

namespace {

using namespace vpvl2;
using namespace vpvl2::gl2;

enum VertexBufferObjectType
{
    kModelVertices,
    kModelIndices,
    kVertexBufferObjectMax
};

struct MaterialTextures
{
    GLuint mainTextureID;
    GLuint sphereTextureID;
    GLuint toonTextureID;
};

class ExtendedZPlotProgram : public ZPlotProgram
{
public:
    ExtendedZPlotProgram(IRenderDelegate *delegate)
        : ZPlotProgram(delegate),
          m_boneIndicesAttributeLocation(0),
          m_boneWeightsAttributeLocation(0),
          m_boneMatricesUniformLocation(0)
    {
    }
    ~ExtendedZPlotProgram() {
        m_boneIndicesAttributeLocation = 0;
        m_boneWeightsAttributeLocation = 0;
        m_boneMatricesUniformLocation = 0;
    }

    void setBoneIndices(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_boneIndicesAttributeLocation);
        glVertexAttribPointer(m_boneIndicesAttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setBoneWeights(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_boneWeightsAttributeLocation);
        glVertexAttribPointer(m_boneWeightsAttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setBoneMatrices(const Scalar *value, size_t size) {
        glUniformMatrix4fv(m_boneMatricesUniformLocation, size, GL_FALSE, value);
    }

protected:
    virtual void getLocations() {
        ZPlotProgram::getLocations();
        m_boneIndicesAttributeLocation = glGetAttribLocation(m_program, "inBoneIndices");
        m_boneWeightsAttributeLocation = glGetAttribLocation(m_program, "inBoneWeights");
        m_boneMatricesUniformLocation = glGetUniformLocation(m_program, "boneMatrices");
    }

private:
    GLuint m_boneIndicesAttributeLocation;
    GLuint m_boneWeightsAttributeLocation;
    GLuint m_boneMatricesUniformLocation;
};

class EdgeProgram : public BaseShaderProgram
{
public:
    EdgeProgram(IRenderDelegate *delegate)
        : BaseShaderProgram(delegate),
          m_colorUniformLocation(0),
          m_edgeSizeUniformLocation(0),
          m_opacityUniformLocation(0),
          m_boneIndicesAttributeLocation(0),
          m_boneWeightsAttributeLocation(0),
          m_boneMatricesUniformLocation(0)
    {
    }
    ~EdgeProgram() {
        m_colorUniformLocation = 0;
        m_edgeSizeUniformLocation = 0;
        m_opacityUniformLocation = 0;
        m_boneIndicesAttributeLocation = 0;
        m_boneWeightsAttributeLocation = 0;
        m_boneMatricesUniformLocation = 0;
    }

    void setColor(const Color &value) {
        glUniform4fv(m_colorUniformLocation, 1, value);
    }
    void setSize(const Scalar &value) {
        glUniform1f(m_edgeSizeUniformLocation, value);
    }
    void setOpacity(const Scalar &value) {
        glUniform1f(m_opacityUniformLocation, value);
    }
    void setBoneIndices(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_boneIndicesAttributeLocation);
        glVertexAttribPointer(m_boneIndicesAttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setBoneWeights(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_boneWeightsAttributeLocation);
        glVertexAttribPointer(m_boneWeightsAttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setBoneMatrices(const Scalar *value, size_t size) {
        glUniformMatrix4fv(m_boneMatricesUniformLocation, size, GL_FALSE, value);
    }

protected:
    virtual void getLocations() {
        BaseShaderProgram::getLocations();
        m_colorUniformLocation = glGetUniformLocation(m_program, "color");
        m_edgeSizeUniformLocation = glGetUniformLocation(m_program, "edgeSize");
        m_opacityUniformLocation = glGetUniformLocation(m_program, "opacity");
        m_boneIndicesAttributeLocation = glGetAttribLocation(m_program, "inBoneIndices");
        m_boneWeightsAttributeLocation = glGetAttribLocation(m_program, "inBoneWeights");
        m_boneMatricesUniformLocation = glGetUniformLocation(m_program, "boneMatrices");
    }

private:
    GLuint m_colorUniformLocation;
    GLuint m_edgeSizeUniformLocation;
    GLuint m_opacityUniformLocation;
    GLuint m_boneIndicesAttributeLocation;
    GLuint m_boneWeightsAttributeLocation;
    GLuint m_boneMatricesUniformLocation;
};

class ShadowProgram : public ObjectProgram
{
public:
    ShadowProgram(IRenderDelegate *delegate)
        : ObjectProgram(delegate),
          m_shadowMatrixUniformLocation(0),
          m_boneIndicesAttributeLocation(0),
          m_boneWeightsAttributeLocation(0),
          m_boneMatricesUniformLocation(0)
    {
    }
    ~ShadowProgram() {
        m_shadowMatrixUniformLocation = 0;
        m_boneIndicesAttributeLocation = 0;
        m_boneWeightsAttributeLocation = 0;
        m_boneMatricesUniformLocation = 0;
    }

    void setShadowMatrix(const float value[16]) {
        glUniformMatrix4fv(m_shadowMatrixUniformLocation, 1, GL_FALSE, value);
    }
    void setBoneIndices(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_boneIndicesAttributeLocation);
        glVertexAttribPointer(m_boneIndicesAttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setBoneWeights(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_boneWeightsAttributeLocation);
        glVertexAttribPointer(m_boneWeightsAttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setBoneMatrices(const Scalar *value, size_t size) {
        glUniformMatrix4fv(m_boneMatricesUniformLocation, size, GL_FALSE, value);
    }

protected:
    virtual void getLocations() {
        ObjectProgram::getLocations();
        m_shadowMatrixUniformLocation = glGetUniformLocation(m_program, "shadowMatrix");
        m_boneIndicesAttributeLocation = glGetAttribLocation(m_program, "inBoneIndices");
        m_boneWeightsAttributeLocation = glGetAttribLocation(m_program, "inBoneWeights");
        m_boneMatricesUniformLocation = glGetUniformLocation(m_program, "boneMatrices");
    }

private:
    GLuint m_shadowMatrixUniformLocation;
    GLuint m_boneIndicesAttributeLocation;
    GLuint m_boneWeightsAttributeLocation;
    GLuint m_boneMatricesUniformLocation;
};

class ModelProgram : public ObjectProgram
{
public:
    ModelProgram(IRenderDelegate *delegate)
        : ObjectProgram(delegate),
          m_toonTexCoordAttributeLocation(0),
          m_uva1AttributeLocation(0),
          m_cameraPositionUniformLocation(0),
          m_materialAmbientUniformLocation(0),
          m_materialDiffuseUniformLocation(0),
          m_materialSpecularUniformLocation(0),
          m_materialShininessUniformLocation(0),
          m_mainTextureBlendUniformLocation(0),
          m_sphereTextureBlendUniformLocation(0),
          m_toonTextureBlendUniformLocation(0),
          m_sphereTextureUniformLocation(0),
          m_hasSphereTextureUniformLocation(0),
          m_isSPHTextureUniformLocation(0),
          m_isSPATextureUniformLocation(0),
          m_isSubTextureUniformLocation(0),
          m_toonTextureUniformLocation(0),
          m_hasToonTextureUniformLocation(0),
          m_useToonUniformLocation(0),
          m_boneIndicesAttributeLocation(0),
          m_boneWeightsAttributeLocation(0),
          m_boneMatricesUniformLocation(0)
    {
    }
    ~ModelProgram() {
        m_toonTexCoordAttributeLocation = 0;
        m_uva1AttributeLocation = 0;
        m_cameraPositionUniformLocation = 0;
        m_materialAmbientUniformLocation = 0;
        m_materialDiffuseUniformLocation = 0;
        m_materialSpecularUniformLocation = 0;
        m_materialShininessUniformLocation = 0;
        m_mainTextureBlendUniformLocation = 0;
        m_sphereTextureBlendUniformLocation = 0;
        m_toonTextureBlendUniformLocation = 0;
        m_sphereTextureUniformLocation = 0;
        m_hasSphereTextureUniformLocation = 0;
        m_isSPHTextureUniformLocation = 0;
        m_isSPATextureUniformLocation = 0;
        m_isSubTextureUniformLocation = 0;
        m_toonTextureUniformLocation = 0;
        m_hasToonTextureUniformLocation = 0;
        m_useToonUniformLocation = 0;
        m_boneIndicesAttributeLocation = 0;
        m_boneWeightsAttributeLocation = 0;
        m_boneMatricesUniformLocation = 0;
    }

    void setCameraPosition(const Vector3 &value) {
        glUniform3fv(m_cameraPositionUniformLocation, 1, value);
    }
    void setToonTexCoord(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_toonTexCoordAttributeLocation);
        glVertexAttribPointer(m_toonTexCoordAttributeLocation, 2, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setUVA1(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_uva1AttributeLocation);
        glVertexAttribPointer(m_uva1AttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
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
    void setMainTextureBlend(const Color &value) {
        glUniform4fv(m_mainTextureBlendUniformLocation, 1, value);
    }
    void setSphereTextureBlend(const Color &value) {
        glUniform4fv(m_sphereTextureBlendUniformLocation, 1, value);
    }
    void setToonTextureBlend(const Color &value) {
        glUniform4fv(m_toonTextureBlendUniformLocation, 1, value);
    }
    void setToonEnable(bool value) {
        glUniform1i(m_useToonUniformLocation, value ? 1 : 0);
    }
    void setSphereTexture(GLuint value, pmx::Material::SphereTextureRenderMode mode) {
        if (value) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, value);
            glUniform1i(m_sphereTextureUniformLocation, 1);
            switch (mode) {
            case pmx::Material::kNone:
                glUniform1i(m_hasSphereTextureUniformLocation, 0);
                glUniform1i(m_isSPHTextureUniformLocation, 0);
                glUniform1i(m_isSPATextureUniformLocation, 0);
                glUniform1i(m_isSubTextureUniformLocation, 0);
                break;
            case pmx::Material::kMultTexture:
                glUniform1i(m_hasSphereTextureUniformLocation, 1);
                glUniform1i(m_isSPHTextureUniformLocation, 1);
                glUniform1i(m_isSPATextureUniformLocation, 0);
                glUniform1i(m_isSubTextureUniformLocation, 0);
                break;
            case pmx::Material::kAddTexture:
                glUniform1i(m_hasSphereTextureUniformLocation, 1);
                glUniform1i(m_isSPHTextureUniformLocation, 0);
                glUniform1i(m_isSPATextureUniformLocation, 1);
                glUniform1i(m_isSubTextureUniformLocation, 0);
                break;
            case pmx::Material::kSubTexture:
                glUniform1i(m_hasSphereTextureUniformLocation, 1);
                glUniform1i(m_isSPHTextureUniformLocation, 0);
                glUniform1i(m_isSPATextureUniformLocation, 0);
                glUniform1i(m_isSubTextureUniformLocation, 1);
                break;
            }
        }
        else {
            glUniform1i(m_hasSphereTextureUniformLocation, 0);
        }
    }
    void setToonTexture(GLuint value) {
        if (value) {
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, value);
            glUniform1i(m_toonTextureUniformLocation, 2);
            glUniform1i(m_hasToonTextureUniformLocation, 1);
        }
        else {
            glUniform1i(m_hasToonTextureUniformLocation, 0);
        }
    }
    void setBoneIndices(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_boneIndicesAttributeLocation);
        glVertexAttribPointer(m_boneIndicesAttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setBoneWeights(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_boneWeightsAttributeLocation);
        glVertexAttribPointer(m_boneWeightsAttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setBoneMatrices(const Scalar *value, size_t size) {
        glUniformMatrix4fv(m_boneMatricesUniformLocation, size, GL_FALSE, value);
    }

protected:
    virtual void getLocations() {
        ObjectProgram::getLocations();
        m_toonTexCoordAttributeLocation = glGetAttribLocation(m_program, "inToonCoord");
        m_uva1AttributeLocation = glGetAttribLocation(m_program, "inUVA1");
        m_cameraPositionUniformLocation = glGetUniformLocation(m_program, "cameraPosition");
        m_materialAmbientUniformLocation = glGetUniformLocation(m_program, "materialAmbient");
        m_materialDiffuseUniformLocation = glGetUniformLocation(m_program, "materialDiffuse");
        m_materialSpecularUniformLocation = glGetUniformLocation(m_program, "materialSpecular");
        m_materialShininessUniformLocation = glGetUniformLocation(m_program, "materialShininess");
        m_mainTextureBlendUniformLocation = glGetUniformLocation(m_program, "mainTextureBlend");
        m_sphereTextureBlendUniformLocation = glGetUniformLocation(m_program, "sphereTextureBlend");
        m_toonTextureBlendUniformLocation = glGetUniformLocation(m_program, "toonTextureBlend");
        m_sphereTextureUniformLocation = glGetUniformLocation(m_program, "sphereTexture");
        m_hasSphereTextureUniformLocation = glGetUniformLocation(m_program, "hasSphereTexture");
        m_isSPHTextureUniformLocation = glGetUniformLocation(m_program, "isSPHTexture");
        m_isSPATextureUniformLocation = glGetUniformLocation(m_program, "isSPATexture");
        m_isSubTextureUniformLocation = glGetUniformLocation(m_program, "isSubTexture");
        m_toonTextureUniformLocation = glGetUniformLocation(m_program, "toonTexture");
        m_hasToonTextureUniformLocation = glGetUniformLocation(m_program, "hasToonTexture");
        m_useToonUniformLocation = glGetUniformLocation(m_program, "useToon");
        m_boneIndicesAttributeLocation = glGetAttribLocation(m_program, "inBoneIndices");
        m_boneWeightsAttributeLocation = glGetAttribLocation(m_program, "inBoneWeights");
        m_boneMatricesUniformLocation = glGetUniformLocation(m_program, "boneMatrices");
    }

private:
    GLuint m_toonTexCoordAttributeLocation;
    GLuint m_uva1AttributeLocation;
    GLuint m_cameraPositionUniformLocation;
    GLuint m_materialAmbientUniformLocation;
    GLuint m_materialDiffuseUniformLocation;
    GLuint m_materialSpecularUniformLocation;
    GLuint m_materialShininessUniformLocation;
    GLuint m_mainTextureBlendUniformLocation;
    GLuint m_sphereTextureBlendUniformLocation;
    GLuint m_toonTextureBlendUniformLocation;
    GLuint m_sphereTextureUniformLocation;
    GLuint m_hasSphereTextureUniformLocation;
    GLuint m_isSPHTextureUniformLocation;
    GLuint m_isSPATextureUniformLocation;
    GLuint m_isSubTextureUniformLocation;
    GLuint m_toonTextureUniformLocation;
    GLuint m_hasToonTextureUniformLocation;
    GLuint m_useToonUniformLocation;
    GLuint m_boneIndicesAttributeLocation;
    GLuint m_boneWeightsAttributeLocation;
    GLuint m_boneMatricesUniformLocation;
};

}

namespace vpvl2
{
namespace gl2
{

class PMXRenderEngine::PrivateContext
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
          cullFaceState(true),
          isVertexShaderSkinning(false)
    {
    }
    virtual ~PrivateContext() {
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

    void releaseMaterials(pmx::Model *model) {
        if (materials) {
            const Array<pmx::Material *> &modelMaterials = model->materials();
            const int nmaterials = modelMaterials.count();
            for (int i = 0; i < nmaterials; i++) {
                MaterialTextures &materialPrivate = materials[i];
                glDeleteTextures(1, &materialPrivate.mainTextureID);
                glDeleteTextures(1, &materialPrivate.sphereTextureID);
                glDeleteTextures(1, &materialPrivate.toonTextureID);
            }
            delete[] materials;
            materials = 0;
        }
    }

    EdgeProgram *edgeProgram;
    ModelProgram *modelProgram;
    ShadowProgram *shadowProgram;
    ExtendedZPlotProgram *zplotProgram;
    pmx::Model::SkinningMeshes mesh;
    GLuint vertexBufferObjects[kVertexBufferObjectMax];
    MaterialTextures *materials;
    bool cullFaceState;
    bool isVertexShaderSkinning;
};

PMXRenderEngine::PMXRenderEngine(IRenderDelegate *delegate,
                                 const Scene *scene,
                                 cl::PMXAccelerator *accelerator,
                                 pmx::Model *model)
#ifdef VPVL2_LINK_QT
    : QGLFunctions(),
      #else
    :
      #endif /* VPVL2_LINK_QT */
      m_delegate(delegate),
      m_scene(scene),
      m_accelerator(accelerator),
      m_model(model),
      m_context(0)
{
    m_context = new PrivateContext();
}

PMXRenderEngine::~PMXRenderEngine()
{
    if (m_context) {
        m_context->releaseMaterials(m_model);
        delete m_context;
        m_context = 0;
    }
#ifdef VPVL2_ENABLE_OPENCL
    delete m_accelerator;
    m_accelerator = 0;
#endif
    m_delegate = 0;
    m_scene = 0;
    m_model = 0;
    m_accelerator = 0;
}

IModel *PMXRenderEngine::model() const
{
    return m_model;
}

bool PMXRenderEngine::upload(const IString *dir)
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
        vertexShaderSource = m_delegate->loadShaderSource(IRenderDelegate::kEdgeWithSkinningVertexShader, m_model, dir, context);
    else
        vertexShaderSource = m_delegate->loadShaderSource(IRenderDelegate::kEdgeVertexShader, m_model, dir, context);
    fragmentShaderSource = m_delegate->loadShaderSource(IRenderDelegate::kEdgeFragmentShader, m_model, dir, context);
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
        vertexShaderSource = m_delegate->loadShaderSource(IRenderDelegate::kModelWithSkinningVertexShader, m_model, dir, context);
    else
        vertexShaderSource = m_delegate->loadShaderSource(IRenderDelegate::kModelVertexShader, m_model, dir, context);
    fragmentShaderSource = m_delegate->loadShaderSource(IRenderDelegate::kModelFragmentShader, m_model, dir, context);
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
        vertexShaderSource = m_delegate->loadShaderSource(IRenderDelegate::kShadowWithSkinningVertexShader, m_model, dir, context);
    else
        vertexShaderSource = m_delegate->loadShaderSource(IRenderDelegate::kShadowVertexShader, m_model, dir, context);
    fragmentShaderSource = m_delegate->loadShaderSource(IRenderDelegate::kShadowFragmentShader, m_model, dir, context);
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
        vertexShaderSource = m_delegate->loadShaderSource(IRenderDelegate::kZPlotWithSkinningVertexShader, m_model, dir, context);
    else
        vertexShaderSource = m_delegate->loadShaderSource(IRenderDelegate::kZPlotVertexShader, m_model, dir, context);
    fragmentShaderSource = m_delegate->loadShaderSource(IRenderDelegate::kZPlotFragmentShader, m_model, dir, context);
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
    glGenBuffers(kVertexBufferObjectMax, m_context->vertexBufferObjects);
    size_t size = pmx::Model::strideSize(pmx::Model::kIndexStride);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelIndices]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_model->indices().count() * size, m_model->indicesPtr(), GL_STATIC_DRAW);
    log0(context, IRenderDelegate::kLogInfo,
         "Binding indices to the vertex buffer object (ID=%d)",
         m_context->vertexBufferObjects[kModelIndices]);
    size = pmx::Model::strideSize(pmx::Model::kVertexStride);
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelVertices]);
    glBufferData(GL_ARRAY_BUFFER, m_model->vertices().count() * size, m_model->vertexPtr(), GL_DYNAMIC_DRAW);
    log0(context, IRenderDelegate::kLogInfo,
         "Binding model vertices to the vertex buffer object (ID=%d)",
         m_context->vertexBufferObjects[kModelVertices]);
    if (isVertexShaderSkinning)
        m_model->getSkinningMesh(m_context->mesh);
    const Array<pmx::Material *> &materials = m_model->materials();
    const int nmaterials = materials.count();
    GLuint textureID = 0;
    MaterialTextures *materialPrivates = m_context->materials = new MaterialTextures[nmaterials];
    for (int i = 0; i < nmaterials; i++) {
        const pmx::Material *material = materials[i];
        MaterialTextures &materialPrivate = materialPrivates[i];
        materialPrivate.mainTextureID = 0;
        materialPrivate.sphereTextureID = 0;
        materialPrivate.toonTextureID = 0;
        const IString *path = 0;
        path = material->mainTexture();
        if (path && m_delegate->uploadTexture(context, path, dir, IRenderDelegate::kTexture2D, &textureID)) {
            log0(context, IRenderDelegate::kLogInfo, "Binding the texture as a main texture (ID=%d)", textureID);
            materialPrivate.mainTextureID = textureID;
        }
        path = material->sphereTexture();
        if (path && m_delegate->uploadTexture(context, path, dir, IRenderDelegate::kTexture2D, &textureID)) {
            log0(context, IRenderDelegate::kLogInfo, "Binding the texture as a sphere texture (ID=%d)", textureID);
            materialPrivate.sphereTextureID = textureID;
        }
        if (material->isSharedToonTextureUsed()) {
            char buf[16];
            snprintf(buf, sizeof(buf), "toon%d.bmp", material->toonTextureIndex());
            IString *s = m_delegate->toUnicode(reinterpret_cast<const uint8_t *>(buf));
            if (m_delegate->uploadTexture(context, s, 0, IRenderDelegate::kToonTexture, &textureID)) {
                log0(context, IRenderDelegate::kLogInfo, "Binding the texture as a shared toon texture (ID=%d)", textureID);
                materialPrivate.toonTextureID = textureID;
            }
            delete s;
        }
        else {
            path = material->toonTexture();
            if (path && m_delegate->uploadTexture(context, path, dir, IRenderDelegate::kTexture2D, &textureID)) {
                log0(context, IRenderDelegate::kLogInfo, "Binding the texture as a static toon texture (ID=%d)", textureID);
                materialPrivate.toonTextureID = textureID;
            }
        }
    }
#ifdef VPVL2_ENABLE_OPENCL
    if (m_accelerator && m_accelerator->isAvailable())
        m_accelerator->uploadModel(m_model, m_context->vertexBufferObjects[kModelVertices], context);
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
    if (!m_context)
        return;
    size_t size = pmx::Model::strideSize(pmx::Model::kVertexStride);
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelVertices]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_model->vertices().count() * size, m_model->vertexPtr());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    if (m_context->isVertexShaderSkinning)
        m_model->updateSkinningMesh(m_context->mesh);
#ifdef VPVL2_ENABLE_OPENCL
    if (m_accelerator && m_accelerator->isAvailable())
        m_accelerator->updateModel(m_model, m_scene);
#endif
}

void PMXRenderEngine::renderModel()
{
    if (!m_model->isVisible() || !m_context)
        return;
    ModelProgram *modelProgram = m_context->modelProgram;
    modelProgram->bind();
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelVertices]);
    size_t offset = pmx::Model::strideOffset(pmx::Model::kVertexStride);
    size_t size   = pmx::Model::strideSize(pmx::Model::kVertexStride);
    modelProgram->setPosition(reinterpret_cast<const GLvoid *>(offset), size);
    offset = pmx::Model::strideOffset(pmx::Model::kNormalStride);
    size   = pmx::Model::strideSize(pmx::Model::kNormalStride);
    modelProgram->setNormal(reinterpret_cast<const GLvoid *>(offset), size);
    offset = pmx::Model::strideOffset(pmx::Model::kTexCoordStride);
    size   = pmx::Model::strideSize(pmx::Model::kTexCoordStride);
    modelProgram->setTexCoord(reinterpret_cast<const GLvoid *>(offset), size);
    offset = pmx::Model::strideOffset(pmx::Model::kToonCoordStride);
    size   = pmx::Model::strideSize(pmx::Model::kToonCoordStride);
    modelProgram->setToonTexCoord(reinterpret_cast<const GLvoid *>(offset), size);
    offset = pmx::Model::strideOffset(pmx::Model::kUVA1Stride);
    size   = pmx::Model::strideSize(pmx::Model::kUVA1Stride);
    modelProgram->setUVA1(reinterpret_cast<const GLvoid *>(offset), size);
    float matrix4x4[16];
    m_delegate->getMatrix(matrix4x4, m_model,
                          IRenderDelegate::kWorldMatrix
                          | IRenderDelegate::kViewMatrix
                          | IRenderDelegate::kProjectionMatrix
                          | IRenderDelegate::kCameraMatrix);
    modelProgram->setModelViewProjectionMatrix(matrix4x4);
    m_delegate->getMatrix(matrix4x4, m_model,
                          IRenderDelegate::kWorldMatrix
                          | IRenderDelegate::kViewMatrix
                          | IRenderDelegate::kInverseMatrix
                          | IRenderDelegate::kTransposeMatrix
                          | IRenderDelegate::kCameraMatrix);
    modelProgram->setNormalMatrix(matrix4x4);
    m_delegate->getMatrix(matrix4x4, m_model,
                          IRenderDelegate::kWorldMatrix
                          | IRenderDelegate::kViewMatrix
                          | IRenderDelegate::kProjectionMatrix
                          | IRenderDelegate::kLightMatrix);
    modelProgram->setLightViewProjectionMatrix(matrix4x4);
    const Scene::ILight *light = m_scene->light();
    void *texture = light->depthTexture();
    GLuint textureID = texture ? *static_cast<GLuint *>(texture) : 0;
    modelProgram->setLightColor(light->color());
    modelProgram->setLightDirection(light->direction());
    modelProgram->setToonEnable(light->isToonEnabled());
    modelProgram->setSoftShadowEnable(light->isSoftShadowEnabled());
    modelProgram->setDepthTextureSize(light->depthTextureSize());
    modelProgram->setCameraPosition(m_scene->camera()->position());
    const Scalar &opacity = m_model->opacity();
    modelProgram->setOpacity(opacity);
    const Array<pmx::Material *> &materials = m_model->materials();
    const MaterialTextures *materialPrivates = m_context->materials;
    const int nmaterials = materials.count();
    const size_t boneIndexOffset = m_model->strideOffset(pmx::Model::kBoneIndexStride),
            boneWeightOffset = m_model->strideOffset(pmx::Model::kBoneWeightStride),
            boneStride = m_model->strideSize(pmx::Model::kVertexStride);
    const bool hasModelTransparent = !btFuzzyZero(opacity - 1.0),
            isVertexShaderSkinning = m_context->isVertexShaderSkinning;
    offset = 0; size = pmx::Model::strideSize(pmx::Model::kIndexStride);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelIndices]);
    for (int i = 0; i < nmaterials; i++) {
        const pmx::Material *material = materials[i];
        const MaterialTextures &materialPrivate = materialPrivates[i];
        modelProgram->setMaterialAmbient(material->ambient());
        modelProgram->setMaterialDiffuse(material->diffuse());
        modelProgram->setMaterialSpecular(material->specular());
        modelProgram->setMaterialShininess(material->shininess());
        modelProgram->setMainTextureBlend(material->mainTextureBlend());
        modelProgram->setSphereTextureBlend(material->sphereTextureBlend());
        modelProgram->setToonTextureBlend(material->toonTextureBlend());
        modelProgram->setMainTexture(materialPrivate.mainTextureID);
        modelProgram->setSphereTexture(materialPrivate.sphereTextureID, material->sphereTextureRenderMode());
        modelProgram->setToonTexture(materialPrivate.toonTextureID);
        if (texture && material->isSelfShadowDrawn())
            modelProgram->setDepthTexture(textureID);
        else
            modelProgram->setDepthTexture(0);
        if (isVertexShaderSkinning) {
            const pmx::Model::SkinningMeshes &mesh = m_context->mesh;
            modelProgram->setBoneIndices(reinterpret_cast<const GLvoid *>(boneIndexOffset), boneStride);
            modelProgram->setBoneWeights(reinterpret_cast<const GLvoid *>(boneWeightOffset), boneStride);
            modelProgram->setBoneMatrices(mesh.matrices[i], mesh.bones[i].size());
        }
        if ((!hasModelTransparent && m_context->cullFaceState) ||
                (material->isCullFaceDisabled() && m_context->cullFaceState)) {
            glDisable(GL_CULL_FACE);
            m_context->cullFaceState = false;
        }
        else if (!m_context->cullFaceState) {
            glEnable(GL_CULL_FACE);
            m_context->cullFaceState = true;
        }
        const int nindices = material->indices();
        glDrawElements(GL_TRIANGLES, nindices, GL_UNSIGNED_INT, reinterpret_cast<const GLvoid *>(offset));
        offset += nindices * size;
    }
    modelProgram->unbind();
    if (!m_context->cullFaceState) {
        glEnable(GL_CULL_FACE);
        m_context->cullFaceState = true;
    }
}

void PMXRenderEngine::renderShadow()
{
    if (!m_model->isVisible() || !m_context)
        return;
    ShadowProgram *shadowProgram = m_context->shadowProgram;
    shadowProgram->bind();
    float matrix4x4[16];
    m_delegate->getMatrix(matrix4x4, m_model,
                          IRenderDelegate::kWorldMatrix
                          | IRenderDelegate::kViewMatrix
                          | IRenderDelegate::kProjectionMatrix
                          | IRenderDelegate::kShadowMatrix);
    shadowProgram->setModelViewProjectionMatrix(matrix4x4);
    const Scene::ILight *light = m_scene->light();
    shadowProgram->setLightColor(light->color());
    shadowProgram->setLightDirection(light->direction());
    size_t offset = pmx::Model::strideOffset(pmx::Model::kVertexStride);
    size_t size = pmx::Model::strideSize(pmx::Model::kVertexStride);
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelVertices]);
    shadowProgram->setPosition(reinterpret_cast<const GLvoid *>(offset), size);
    glCullFace(GL_FRONT);
    const Array<pmx::Material *> &materials = m_model->materials();
    const size_t boneIndexOffset = m_model->strideOffset(pmx::Model::kBoneIndexStride),
            boneWeightOffset = m_model->strideOffset(pmx::Model::kBoneWeightStride),
            boneStride = m_model->strideSize(pmx::Model::kVertexStride);
    const int nmaterials = materials.count();
    const bool isVertexShaderSkinning = m_context->isVertexShaderSkinning;
    offset = 0; size = pmx::Model::strideSize(pmx::Model::kIndexStride);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelIndices]);
    for (int i = 0; i < nmaterials; i++) {
        const pmx::Material *material = materials[i];
        const int nindices = material->indices();
        if (material->hasShadow()) {
            if (isVertexShaderSkinning) {
                const pmx::Model::SkinningMeshes &mesh = m_context->mesh;
                shadowProgram->setBoneIndices(reinterpret_cast<const GLvoid *>(boneIndexOffset), boneStride);
                shadowProgram->setBoneWeights(reinterpret_cast<const GLvoid *>(boneWeightOffset), boneStride);
                shadowProgram->setBoneMatrices(mesh.matrices[i], mesh.bones[i].size());
            }
            glDrawElements(GL_TRIANGLES, nindices, GL_UNSIGNED_INT, reinterpret_cast<const GLvoid *>(offset));
        }
        offset += nindices * size;
    }
    glCullFace(GL_BACK);
    shadowProgram->unbind();
}

void PMXRenderEngine::renderEdge()
{
    if (!m_model->isVisible() || !m_context)
        return;
    EdgeProgram *edgeProgram = m_context->edgeProgram;
    edgeProgram->bind();
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelVertices]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelIndices]);
    size_t offset = pmx::Model::strideOffset(pmx::Model::kEdgeVertexStride);
    size_t size   = pmx::Model::strideSize(pmx::Model::kEdgeVertexStride);
    edgeProgram->setPosition(reinterpret_cast<const GLvoid *>(offset), size);
    float matrix4x4[16];
    m_delegate->getMatrix(matrix4x4, m_model,
                          IRenderDelegate::kWorldMatrix
                          | IRenderDelegate::kViewMatrix
                          | IRenderDelegate::kProjectionMatrix
                          | IRenderDelegate::kCameraMatrix);
    edgeProgram->setModelViewProjectionMatrix(matrix4x4);
    edgeProgram->setOpacity(m_model->opacity());
    const Array<pmx::Material *> &materials = m_model->materials();
    const size_t boneIndexOffset = m_model->strideOffset(pmx::Model::kBoneIndexStride),
            boneWeightOffset = m_model->strideOffset(pmx::Model::kBoneWeightStride),
            boneStride = m_model->strideSize(pmx::Model::kVertexStride);
    const int nmaterials = materials.count();
    const bool isVertexShaderSkinning = m_context->isVertexShaderSkinning;
    offset = 0; size = pmx::Model::strideSize(pmx::Model::kIndexStride);
    glCullFace(GL_FRONT);
    for (int i = 0; i < nmaterials; i++) {
        const pmx::Material *material = materials[i];
        const int nindices = material->indices();
        edgeProgram->setColor(material->edgeColor());
        if (material->isEdgeDrawn()) {
            if (isVertexShaderSkinning) {
                const pmx::Model::SkinningMeshes &mesh = m_context->mesh;
                edgeProgram->setBoneIndices(reinterpret_cast<const GLvoid *>(boneIndexOffset), boneStride);
                edgeProgram->setBoneWeights(reinterpret_cast<const GLvoid *>(boneWeightOffset), boneStride);
                edgeProgram->setBoneMatrices(mesh.matrices[i], mesh.bones[i].size());
                edgeProgram->setSize(material->edgeSize());
            }
            glDrawElements(GL_TRIANGLES, nindices, GL_UNSIGNED_INT, reinterpret_cast<const GLvoid *>(offset));
        }
        offset += nindices * size;
    }
    glCullFace(GL_BACK);
    edgeProgram->unbind();
}

void PMXRenderEngine::renderZPlot()
{
    if (!m_model->isVisible() || !m_context)
        return;
    ExtendedZPlotProgram *zplotProgram = m_context->zplotProgram;
    zplotProgram->bind();
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelVertices]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelIndices]);
    size_t offset = pmx::Model::strideOffset(pmx::Model::kVertexStride);
    size_t size   = pmx::Model::strideSize(pmx::Model::kVertexStride);
    zplotProgram->setPosition(reinterpret_cast<const GLvoid *>(offset), size);
    float matrix4x4[16];
    m_delegate->getMatrix(matrix4x4, m_model,
                          IRenderDelegate::kWorldMatrix
                          | IRenderDelegate::kViewMatrix
                          | IRenderDelegate::kProjectionMatrix
                          | IRenderDelegate::kLightMatrix);
    zplotProgram->setModelViewProjectionMatrix(matrix4x4);
    glCullFace(GL_FRONT);
    const Array<pmx::Material *> &materials = m_model->materials();
    const size_t boneIndexOffset = m_model->strideOffset(pmx::Model::kBoneIndexStride),
            boneWeightOffset = m_model->strideOffset(pmx::Model::kBoneWeightStride),
            boneStride = m_model->strideSize(pmx::Model::kVertexStride);
    const int nmaterials = materials.count();
    const bool isVertexShaderSkinning = m_context->isVertexShaderSkinning;
    offset = 0; size = pmx::Model::strideSize(pmx::Model::kIndexStride);
    for (int i = 0; i < nmaterials; i++) {
        const pmx::Material *material = materials[i];
        const int nindices = material->indices();
        if (material->isShadowMapDrawn()) {
            if (isVertexShaderSkinning) {
                const pmx::Model::SkinningMeshes &mesh = m_context->mesh;
                zplotProgram->setBoneIndices(reinterpret_cast<const GLvoid *>(boneIndexOffset), boneStride);
                zplotProgram->setBoneWeights(reinterpret_cast<const GLvoid *>(boneWeightOffset), boneStride);
                zplotProgram->setBoneMatrices(mesh.matrices[i], mesh.bones[i].size());
            }
            glDrawElements(GL_TRIANGLES, nindices, GL_UNSIGNED_INT, reinterpret_cast<const GLvoid *>(offset));
        }
        offset += nindices * size;
    }
    glCullFace(GL_BACK);
    zplotProgram->unbind();
}

void PMXRenderEngine::log0(void *context, IRenderDelegate::LogLevel level, const char *format...)
{
    va_list ap;
    va_start(ap, format);
    m_delegate->log(context, level, format, ap);
    va_end(ap);
}

} /* namespace gl2 */
} /* namespace vpvl2 */
