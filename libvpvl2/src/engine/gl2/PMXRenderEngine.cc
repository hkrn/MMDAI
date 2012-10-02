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

#include "EngineCommon.h"
#include "vpvl2/internal/util.h" /* internal::snprintf */
#include "vpvl2/gl2/PMXRenderEngine.h"
#include "vpvl2/pmx/Material.h"
#include "vpvl2/pmx/Model.h"
#include "vpvl2/pmx/Vertex.h"
#ifdef VPVL2_ENABLE_OPENCL
#include "vpvl2/cl/Context.h"
#include "vpvl2/cl/PMXAccelerator.h"
#endif

namespace {

using namespace vpvl2;
using namespace vpvl2::gl2;

enum VertexBufferObjectType
{
    kModelDynamicVertexBuffer,
    kModelStaticVertexBuffer,
    kModelIndexBuffer,
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
        glVertexAttribPointer(m_boneIndicesAttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setBoneWeights(const GLvoid *ptr, GLsizei stride) {
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
        enableAttribute(m_boneIndicesAttributeLocation);
        enableAttribute(m_boneWeightsAttributeLocation);
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
          m_normalAttributeLocation(0),
          m_edgeAttributeLocation(0),
          m_boneIndicesAttributeLocation(0),
          m_boneWeightsAttributeLocation(0),
          m_boneMatricesUniformLocation(0)
    {
    }
    ~EdgeProgram() {
        m_colorUniformLocation = 0;
        m_edgeSizeUniformLocation = 0;
        m_opacityUniformLocation = 0;
        m_normalAttributeLocation = 0;
        m_edgeAttributeLocation = 0;
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
    void setNormal(const GLvoid *ptr, GLsizei stride) {
        glVertexAttribPointer(m_normalAttributeLocation, 1, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setVertexEdgeSize(const GLvoid *ptr, GLsizei stride) {
        glVertexAttribPointer(m_edgeAttributeLocation, 1, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setBoneIndices(const GLvoid *ptr, GLsizei stride) {
        glVertexAttribPointer(m_boneIndicesAttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setBoneWeights(const GLvoid *ptr, GLsizei stride) {
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
        m_normalAttributeLocation = glGetAttribLocation(m_program, "inNormal");
        m_edgeAttributeLocation = glGetAttribLocation(m_program, "inEdgeSize");
        m_boneIndicesAttributeLocation = glGetAttribLocation(m_program, "inBoneIndices");
        m_boneWeightsAttributeLocation = glGetAttribLocation(m_program, "inBoneWeights");
        m_boneMatricesUniformLocation = glGetUniformLocation(m_program, "boneMatrices");
        enableAttribute(m_normalAttributeLocation);
        enableAttribute(m_edgeAttributeLocation);
        enableAttribute(m_boneIndicesAttributeLocation);
        enableAttribute(m_boneWeightsAttributeLocation);
    }

private:
    GLuint m_colorUniformLocation;
    GLuint m_edgeSizeUniformLocation;
    GLuint m_opacityUniformLocation;
    GLuint m_normalAttributeLocation;
    GLuint m_edgeAttributeLocation;
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
        glVertexAttribPointer(m_boneIndicesAttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setBoneWeights(const GLvoid *ptr, GLsizei stride) {
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
        enableAttribute(m_boneIndicesAttributeLocation);
        enableAttribute(m_boneWeightsAttributeLocation);
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
          m_deltaAttributeLocation(0),
          m_uva0AttributeLocation(0),
          m_uva1AttributeLocation(0),
          m_cameraPositionUniformLocation(0),
          m_materialColorUniformLocation(0),
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
        m_deltaAttributeLocation = 0;
        m_uva0AttributeLocation = 0;
        m_uva1AttributeLocation = 0;
        m_cameraPositionUniformLocation = 0;
        m_materialColorUniformLocation = 0;
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
    void setDelta(const GLvoid *ptr, GLsizei stride) {
        glVertexAttribPointer(m_deltaAttributeLocation, 3, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setUVA0(const GLvoid *ptr, GLsizei stride) {
        glVertexAttribPointer(m_uva0AttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setUVA1(const GLvoid *ptr, GLsizei stride) {
        glVertexAttribPointer(m_uva1AttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
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
            default:
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
        glVertexAttribPointer(m_boneIndicesAttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setBoneWeights(const GLvoid *ptr, GLsizei stride) {
        glVertexAttribPointer(m_boneWeightsAttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setBoneMatrices(const Scalar *value, size_t size) {
        glUniformMatrix4fv(m_boneMatricesUniformLocation, size, GL_FALSE, value);
    }

protected:
    virtual void getLocations() {
        ObjectProgram::getLocations();
        m_deltaAttributeLocation = glGetAttribLocation(m_program, "inDelta");
        m_uva0AttributeLocation = glGetAttribLocation(m_program, "inUVA0");
        m_uva1AttributeLocation = glGetAttribLocation(m_program, "inUVA1");
        m_cameraPositionUniformLocation = glGetUniformLocation(m_program, "cameraPosition");
        m_materialColorUniformLocation = glGetUniformLocation(m_program, "materialColor");
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
        enableAttribute(m_deltaAttributeLocation);
        enableAttribute(m_uva0AttributeLocation);
        enableAttribute(m_uva1AttributeLocation);
        enableAttribute(m_boneIndicesAttributeLocation);
        enableAttribute(m_boneWeightsAttributeLocation);
    }

private:
    GLuint m_deltaAttributeLocation;
    GLuint m_uva0AttributeLocation;
    GLuint m_uva1AttributeLocation;
    GLuint m_cameraPositionUniformLocation;
    GLuint m_materialColorUniformLocation;
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
    PrivateContext(const IModel *model)
        : modelRef(model),
          indexBuffer(0),
          staticBuffer(0),
          dynamicBuffer(0),
          matrixBuffer(0),
          edgeProgram(0),
          modelProgram(0),
          shadowProgram(0),
          zplotProgram(0),
          materials(0),
          cullFaceState(true),
          isVertexShaderSkinning(false)
    {
        model->getIndexBuffer(indexBuffer);
        model->getStaticVertexBuffer(staticBuffer);
        model->getDynamicVertexBuffer(dynamicBuffer, indexBuffer);
        model->getMatrixBuffer(matrixBuffer, dynamicBuffer, indexBuffer);
        switch (indexBuffer->type()) {
        case IModel::IIndexBuffer::kIndex32:
            indexType = GL_UNSIGNED_INT;
            break;
        case IModel::IIndexBuffer::kIndex16:
            indexType = GL_UNSIGNED_SHORT;
            break;
        case IModel::IIndexBuffer::kIndex8:
            indexType = GL_UNSIGNED_BYTE;
            break;
        case IModel::IIndexBuffer::kMaxIndexType:
        default:
            indexType = GL_UNSIGNED_INT;
            break;
        }
#ifdef VPVL2_LINK_QT
        initializeGLFunctions();
#endif
    }
    virtual ~PrivateContext() {
        glDeleteBuffers(kVertexBufferObjectMax, vertexBufferObjects);
        delete indexBuffer;
        indexBuffer = 0;
        delete dynamicBuffer;
        dynamicBuffer = 0;
        delete staticBuffer;
        staticBuffer = 0;
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

    void releaseMaterials() {
        if (materials) {
            Array<IMaterial *> modelMaterials;
            modelRef->getMaterials(modelMaterials);
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

    const IModel *modelRef;
    IModel::IIndexBuffer *indexBuffer;
    IModel::IStaticVertexBuffer *staticBuffer;
    IModel::IDynamicVertexBuffer *dynamicBuffer;
    IModel::IMatrixBuffer *matrixBuffer;
    EdgeProgram *edgeProgram;
    ModelProgram *modelProgram;
    ShadowProgram *shadowProgram;
    ExtendedZPlotProgram *zplotProgram;
    GLuint vertexBufferObjects[kVertexBufferObjectMax];
    GLenum indexType;
    MaterialTextures *materials;
    bool cullFaceState;
    bool isVertexShaderSkinning;
};

PMXRenderEngine::PMXRenderEngine(IRenderDelegate *delegate,
                                 const Scene *scene,
                                 cl::PMXAccelerator *accelerator,
                                 IModel *modelRef)
#ifdef VPVL2_LINK_QT
    : QGLFunctions(),
      #else
    :
      #endif /* VPVL2_LINK_QT */
      m_delegateRef(delegate),
      m_sceneRef(scene),
      m_accelerator(accelerator),
      m_modelRef(modelRef),
      m_context(0)
{
    m_context = new PrivateContext(modelRef);
    if (m_accelerator)
        m_context->dynamicBuffer->setSkinningEnable(false);
#ifdef VPVL2_LINK_QT
    initializeGLFunctions();
#endif
}

PMXRenderEngine::~PMXRenderEngine()
{
    if (m_context) {
        m_context->releaseMaterials();
        delete m_context;
        m_context = 0;
    }
#ifdef VPVL2_ENABLE_OPENCL
    delete m_accelerator;
#endif
    m_delegateRef = 0;
    m_sceneRef = 0;
    m_modelRef = 0;
    m_accelerator = 0;
}

IModel *PMXRenderEngine::model() const
{
    return m_modelRef;
}

bool PMXRenderEngine::upload(const IString *dir)
{
    bool ret = true;
    void *context = 0;
    if (!m_context)
        m_context = new PrivateContext(m_modelRef);
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
    glGenBuffers(kVertexBufferObjectMax, m_context->vertexBufferObjects);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelIndexBuffer]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_context->indexBuffer->size(),
                 m_context->indexBuffer->bytes(), GL_STATIC_DRAW);
    log0(context, IRenderDelegate::kLogInfo,
         "Binding indices to the vertex buffer object (ID=%d)",
         m_context->vertexBufferObjects[kModelIndexBuffer]);
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelDynamicVertexBuffer]);
    glBufferData(GL_ARRAY_BUFFER, m_context->dynamicBuffer->size(),
                 m_context->dynamicBuffer->bytes(), GL_DYNAMIC_DRAW);
    log0(context, IRenderDelegate::kLogInfo,
         "Binding model dynamic vertex buffer to the vertex buffer object (ID=%d)",
         m_context->vertexBufferObjects[kModelDynamicVertexBuffer]);
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelStaticVertexBuffer]);
    glBufferData(GL_ARRAY_BUFFER, m_context->staticBuffer->size(),
                 m_context->staticBuffer->bytes(), GL_STATIC_DRAW);
    log0(context, IRenderDelegate::kLogInfo,
         "Binding model static vertex buffer to the vertex buffer object (ID=%d)",
         m_context->vertexBufferObjects[kModelStaticVertexBuffer]);
    //if (m_context->isVertexShaderSkinning)
    //    m_modelRef->getSkinningMesh(m_context->mesh);
    Array<IMaterial *> materials;
    m_modelRef->getMaterials(materials);
    const int nmaterials = materials.count();
    IRenderDelegate::Texture texture;
    GLuint textureID = 0;
    MaterialTextures *materialPrivates = m_context->materials = new MaterialTextures[nmaterials];
    texture.object = &textureID;
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = materials[i];
        MaterialTextures &materialPrivate = materialPrivates[i];
        materialPrivate.mainTextureID = 0;
        materialPrivate.sphereTextureID = 0;
        materialPrivate.toonTextureID = 0;
        const IString *path = 0;
        path = material->mainTexture();
        if (path) {
            if (m_delegateRef->uploadTexture(path, dir, IRenderDelegate::kTexture2D, texture, context)) {
                materialPrivate.mainTextureID = textureID = *static_cast<const GLuint *>(texture.object);
                log0(context, IRenderDelegate::kLogInfo, "Binding the texture as a main texture (ID=%d)", textureID);
            }
            else {
                return releaseContext0(context);
            }
        }
        path = material->sphereTexture();
        if (path) {
            if (m_delegateRef->uploadTexture(path, dir, IRenderDelegate::kTexture2D, texture, context)) {
                materialPrivate.sphereTextureID = textureID = *static_cast<const GLuint *>(texture.object);
                log0(context, IRenderDelegate::kLogInfo, "Binding the texture as a sphere texture (ID=%d)", textureID);
            }
            else {
                return releaseContext0(context);
            }
        }
        if (material->isSharedToonTextureUsed()) {
            char buf[16];
            internal::snprintf(buf, sizeof(buf), "toon%02d.bmp", material->toonTextureIndex() + 1);
            IString *s = m_delegateRef->toUnicode(reinterpret_cast<const uint8_t *>(buf));
            ret = m_delegateRef->uploadTexture(s, 0, IRenderDelegate::kToonTexture, texture, context);
            delete s;
            if (ret) {
                materialPrivate.toonTextureID = textureID = *static_cast<const GLuint *>(texture.object);
                log0(context, IRenderDelegate::kLogInfo, "Binding the texture as a shared toon texture (ID=%d)", textureID);
            }
            else {
                return releaseContext0(context);
            }
        }
        else {
            path = material->toonTexture();
            if (path) {
                if (m_delegateRef->uploadTexture(path, dir, IRenderDelegate::kTexture2D, texture, context)) {
                    materialPrivate.toonTextureID = textureID = *static_cast<const GLuint *>(texture.object);
                    log0(context, IRenderDelegate::kLogInfo, "Binding the texture as a static toon texture (ID=%d)", textureID);
                }
                else {
                    return releaseContext0(context);
                }
            }
        }
    }
#ifdef VPVL2_ENABLE_OPENCL
    if (m_accelerator && m_accelerator->isAvailable()) {
        GLuint vbo = m_context->vertexBufferObjects[kModelDynamicVertexBuffer];
        m_accelerator->upload(vbo, m_context->indexBuffer, context);
    }
#endif
    m_modelRef->performUpdate();
    m_modelRef->setVisible(true);
    update();
    log0(context, IRenderDelegate::kLogInfo, "Created the model: %s", m_modelRef->name()->toByteArray());
    m_delegateRef->releaseContext(m_modelRef, context);
    return ret;
}

void PMXRenderEngine::update()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_context)
        return;
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelDynamicVertexBuffer]);
    IModel::IDynamicVertexBuffer *dynamicBuffer = m_context->dynamicBuffer;
    m_modelRef->performUpdate();
    dynamicBuffer->update(m_sceneRef->camera()->position());
    glBufferSubData(GL_ARRAY_BUFFER, 0, dynamicBuffer->size(), dynamicBuffer->bytes());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //if (m_context->isVertexShaderSkinning)
    //    m_modelRef->updateSkinningMesh(m_context->mesh);
#ifdef VPVL2_ENABLE_OPENCL
    if (m_accelerator && m_accelerator->isAvailable())
        m_accelerator->update(m_context->dynamicBuffer, m_sceneRef);
#endif
}

void PMXRenderEngine::renderModel()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_context)
        return;
    ModelProgram *modelProgram = m_context->modelProgram;
    modelProgram->bind();
    IModel::IStaticVertexBuffer *staticBuffer = m_context->staticBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelStaticVertexBuffer]);
    size_t offset = staticBuffer->strideOffset(IModel::IStaticVertexBuffer::kTextureCoordStride);
    size_t size   = staticBuffer->strideSize();
    modelProgram->setTexCoord(reinterpret_cast<const GLvoid *>(offset), size);
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelDynamicVertexBuffer]);
    IModel::IDynamicVertexBuffer *dynamicBuffer = m_context->dynamicBuffer;
    offset = dynamicBuffer->strideOffset(IModel::IDynamicVertexBuffer::kVertexStride);
    size   = dynamicBuffer->strideSize();
    modelProgram->setPosition(reinterpret_cast<const GLvoid *>(offset), size);
    offset = dynamicBuffer->strideOffset(IModel::IDynamicVertexBuffer::kNormalStride);
    modelProgram->setNormal(reinterpret_cast<const GLvoid *>(offset), size);
    offset = dynamicBuffer->strideOffset(IModel::IDynamicVertexBuffer::kMorphDeltaStride);
    modelProgram->setDelta(reinterpret_cast<const GLvoid *>(offset), size);
    offset = dynamicBuffer->strideOffset(IModel::IDynamicVertexBuffer::kUVA0Stride);
    modelProgram->setUVA0(reinterpret_cast<const GLvoid *>(offset), size);
    offset = dynamicBuffer->strideOffset(IModel::IDynamicVertexBuffer::kUVA1Stride);
    modelProgram->setUVA1(reinterpret_cast<const GLvoid *>(offset), size);
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
    const Scalar &opacity = m_modelRef->opacity();
    modelProgram->setOpacity(opacity);
    Array<IMaterial *> materials;
    m_modelRef->getMaterials(materials);
    const MaterialTextures *materialPrivates = m_context->materials;
    const int nmaterials = materials.count();
    const size_t boneIndexOffset = staticBuffer->strideOffset(IModel::IStaticVertexBuffer::kBoneIndexStride),
            boneWeightOffset = staticBuffer->strideOffset(IModel::IStaticVertexBuffer::kBoneWeightStride),
            boneStride = staticBuffer->strideSize();
    const bool hasModelTransparent = !btFuzzyZero(opacity - 1.0f),
            isVertexShaderSkinning = m_context->isVertexShaderSkinning;
    const Vector3 &lc = light->color();
    Color diffuse, specular;
    offset = 0; size = m_context->indexBuffer->strideSize();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelIndexBuffer]);
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = materials[i];
        const MaterialTextures &materialPrivate = materialPrivates[i];
        const Color &ma = material->ambient(), &md = material->diffuse(), &ms = material->specular();
        diffuse.setValue(ma.x() + md.x() * lc.x(), ma.y() + md.y() * lc.y(), ma.z() + md.z() * lc.z(), md.w());
        specular.setValue(ms.x() * lc.x(), ms.y() * lc.y(), ms.z() * lc.z(), 1.0);
        modelProgram->setMaterialColor(diffuse);
        modelProgram->setMaterialSpecular(specular);
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
            IModel::IMatrixBuffer *matrixBuffer = m_context->matrixBuffer;
            modelProgram->setBoneIndices(reinterpret_cast<const GLvoid *>(boneIndexOffset), boneStride);
            modelProgram->setBoneWeights(reinterpret_cast<const GLvoid *>(boneWeightOffset), boneStride);
            modelProgram->setBoneMatrices(matrixBuffer->bytes(i), matrixBuffer->size(i));
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
        glDrawElements(GL_TRIANGLES, nindices, m_context->indexType, reinterpret_cast<const GLvoid *>(offset));
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
    if (!m_modelRef || !m_modelRef->isVisible() || !m_context)
        return;
    ShadowProgram *shadowProgram = m_context->shadowProgram;
    shadowProgram->bind();
    float matrix4x4[16];
    m_delegateRef->getMatrix(matrix4x4, m_modelRef,
                          IRenderDelegate::kWorldMatrix
                          | IRenderDelegate::kViewMatrix
                          | IRenderDelegate::kProjectionMatrix
                          | IRenderDelegate::kShadowMatrix);
    shadowProgram->setModelViewProjectionMatrix(matrix4x4);
    const ILight *light = m_sceneRef->light();
    shadowProgram->setLightColor(light->color());
    shadowProgram->setLightDirection(light->direction());
    IModel::IDynamicVertexBuffer *dynamicBuffer = m_context->dynamicBuffer;
    IModel::IStaticVertexBuffer *staticBuffer = m_context->staticBuffer;
    size_t offset = dynamicBuffer->strideOffset(IModel::IDynamicVertexBuffer::kVertexStride);
    size_t size = dynamicBuffer->strideSize();
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelDynamicVertexBuffer]);
    shadowProgram->setPosition(reinterpret_cast<const GLvoid *>(offset), size);
    glCullFace(GL_FRONT);
    Array<IMaterial *> materials;
    m_modelRef->getMaterials(materials);
    const size_t boneIndexOffset = staticBuffer->strideOffset(IModel::IStaticVertexBuffer::kBoneIndexStride),
            boneWeightOffset = staticBuffer->strideOffset(IModel::IStaticVertexBuffer::kBoneWeightStride),
            boneStride = staticBuffer->strideSize();
    const int nmaterials = materials.count();
    const bool isVertexShaderSkinning = m_context->isVertexShaderSkinning;
    offset = 0; size = m_context->indexBuffer->strideSize();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelIndexBuffer]);
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = materials[i];
        const int nindices = material->indices();
        if (material->hasShadow()) {
            if (isVertexShaderSkinning) {
                IModel::IMatrixBuffer *matrixBuffer = m_context->matrixBuffer;
                shadowProgram->setBoneIndices(reinterpret_cast<const GLvoid *>(boneIndexOffset), boneStride);
                shadowProgram->setBoneWeights(reinterpret_cast<const GLvoid *>(boneWeightOffset), boneStride);
                shadowProgram->setBoneMatrices(matrixBuffer->bytes(i), matrixBuffer->size(i));
            }
            glDrawElements(GL_TRIANGLES, nindices, m_context->indexType, reinterpret_cast<const GLvoid *>(offset));
        }
        offset += nindices * size;
    }
    glCullFace(GL_BACK);
    shadowProgram->unbind();
}

void PMXRenderEngine::renderEdge()
{
    if (!m_modelRef || !m_modelRef->isVisible() || btFuzzyZero(m_modelRef->edgeWidth()) || !m_context)
        return;
    EdgeProgram *edgeProgram = m_context->edgeProgram;
    edgeProgram->bind();
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelDynamicVertexBuffer]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelIndexBuffer]);
    float matrix4x4[16];
    m_delegateRef->getMatrix(matrix4x4, m_modelRef,
                          IRenderDelegate::kWorldMatrix
                          | IRenderDelegate::kViewMatrix
                          | IRenderDelegate::kProjectionMatrix
                          | IRenderDelegate::kCameraMatrix);
    edgeProgram->setModelViewProjectionMatrix(matrix4x4);
    edgeProgram->setOpacity(m_modelRef->opacity());
    Array<IMaterial *> materials;
    m_modelRef->getMaterials(materials);
    const int nmaterials = materials.count();
    const bool isVertexShaderSkinning = m_context->isVertexShaderSkinning;
    IModel::IDynamicVertexBuffer *dynamicBuffer = m_context->dynamicBuffer;
    IModel::IStaticVertexBuffer *staticBuffer = m_context->staticBuffer;
    size_t offset, size = dynamicBuffer->strideSize();
    Scalar edgeScaleFactor;
    if (isVertexShaderSkinning) {
        const ICamera *camera = m_sceneRef->camera();
        const size_t boneIndexOffset = staticBuffer->strideOffset(IModel::IStaticVertexBuffer::kBoneIndexStride),
                boneWeightOffset = staticBuffer->strideOffset(IModel::IStaticVertexBuffer::kBoneWeightStride),
                boneStride = staticBuffer->strideSize();
        edgeScaleFactor = m_modelRef->edgeScaleFactor(camera->position() + Vector3(0, 0, camera->distance()));
        offset = dynamicBuffer->strideOffset(IModel::IDynamicVertexBuffer::kEdgeSizeStride);
        edgeProgram->setVertexEdgeSize(reinterpret_cast<const GLvoid *>(offset), size);
        offset = dynamicBuffer->strideOffset(IModel::IDynamicVertexBuffer::kVertexStride);
        edgeProgram->setPosition(reinterpret_cast<const GLvoid *>(offset), size);
        offset = dynamicBuffer->strideOffset(IModel::IDynamicVertexBuffer::kNormalStride);
        edgeProgram->setNormal(reinterpret_cast<const GLvoid *>(offset), size);
        edgeProgram->setBoneIndices(reinterpret_cast<const GLvoid *>(boneIndexOffset), boneStride);
        edgeProgram->setBoneWeights(reinterpret_cast<const GLvoid *>(boneWeightOffset), boneStride);
    }
    else {
        offset = dynamicBuffer->strideOffset(IModel::IDynamicVertexBuffer::kEdgeVertexStride);
        edgeProgram->setPosition(reinterpret_cast<const GLvoid *>(offset), size);
    }
    offset = 0; size = m_context->indexBuffer->strideSize();
    glCullFace(GL_FRONT);
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = materials[i];
        const int nindices = material->indices();
        edgeProgram->setColor(material->edgeColor());
        if (material->isEdgeDrawn()) {
            if (isVertexShaderSkinning) {
                IModel::IMatrixBuffer *matrixBuffer = m_context->matrixBuffer;
                edgeProgram->setBoneMatrices(matrixBuffer->bytes(i), matrixBuffer->size(i));
                edgeProgram->setSize(material->edgeSize() * edgeScaleFactor);
            }
            glDrawElements(GL_TRIANGLES, nindices, m_context->indexType, reinterpret_cast<const GLvoid *>(offset));
        }
        offset += nindices * size;
    }
    glCullFace(GL_BACK);
    edgeProgram->unbind();
}

void PMXRenderEngine::renderZPlot()
{
    if (!m_modelRef || !m_modelRef->isVisible() || !m_context)
        return;
    ExtendedZPlotProgram *zplotProgram = m_context->zplotProgram;
    zplotProgram->bind();
    glBindBuffer(GL_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelDynamicVertexBuffer]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_context->vertexBufferObjects[kModelIndexBuffer]);
    IModel::IDynamicVertexBuffer *dynamicBuffer = m_context->dynamicBuffer;
    IModel::IStaticVertexBuffer *staticBuffer = m_context->staticBuffer;
    size_t offset = dynamicBuffer->strideOffset(IModel::IDynamicVertexBuffer::kVertexStride);
    size_t size   = dynamicBuffer->strideSize();
    zplotProgram->setPosition(reinterpret_cast<const GLvoid *>(offset), size);
    float matrix4x4[16];
    m_delegateRef->getMatrix(matrix4x4, m_modelRef,
                          IRenderDelegate::kWorldMatrix
                          | IRenderDelegate::kViewMatrix
                          | IRenderDelegate::kProjectionMatrix
                          | IRenderDelegate::kLightMatrix);
    zplotProgram->setModelViewProjectionMatrix(matrix4x4);
    glCullFace(GL_FRONT);
    Array<IMaterial *> materials;
    m_modelRef->getMaterials(materials);
    const size_t boneIndexOffset = staticBuffer->strideOffset(IModel::IStaticVertexBuffer::kBoneIndexStride),
            boneWeightOffset = staticBuffer->strideOffset(IModel::IStaticVertexBuffer::kBoneWeightStride),
            boneStride = staticBuffer->strideSize();
    const int nmaterials = materials.count();
    const bool isVertexShaderSkinning = m_context->isVertexShaderSkinning;
    offset = 0; size = m_context->indexBuffer->strideSize();
    for (int i = 0; i < nmaterials; i++) {
        const IMaterial *material = materials[i];
        const int nindices = material->indices();
        if (material->isShadowMapDrawn()) {
            if (isVertexShaderSkinning) {
                IModel::IMatrixBuffer *matrixBuffer = m_context->matrixBuffer;
                zplotProgram->setBoneIndices(reinterpret_cast<const GLvoid *>(boneIndexOffset), boneStride);
                zplotProgram->setBoneWeights(reinterpret_cast<const GLvoid *>(boneWeightOffset), boneStride);
                zplotProgram->setBoneMatrices(matrixBuffer->bytes(i), matrixBuffer->size(i));
            }
            glDrawElements(GL_TRIANGLES, nindices, m_context->indexType, reinterpret_cast<const GLvoid *>(offset));
        }
        offset += nindices * size;
    }
    glCullFace(GL_BACK);
    zplotProgram->unbind();
}

bool PMXRenderEngine::hasPreProcess() const
{
    return false;
}

bool PMXRenderEngine::hasPostProcess() const
{
    return false;
}

void PMXRenderEngine::preparePostProcess()
{
    /* do nothing */
}

void PMXRenderEngine::performPreProcess()
{
    /* do nothing */
}

void PMXRenderEngine::performPostProcess()
{
    /* do nothing */
}

IEffect *PMXRenderEngine::effect(IEffect::ScriptOrderType /* type */) const
{
    return 0;
}

void PMXRenderEngine::setEffect(IEffect::ScriptOrderType /* type */, IEffect * /* effect */, const IString * /* dir */)
{
    /* do nothing */
}

void PMXRenderEngine::log0(void *context, IRenderDelegate::LogLevel level, const char *format...)
{
    va_list ap;
    va_start(ap, format);
    m_delegateRef->log(context, level, format, ap);
    va_end(ap);
}

bool PMXRenderEngine::createProgram(BaseShaderProgram *program,
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

bool PMXRenderEngine::releaseContext0(void *context)
{
    delete m_context;
    m_context = 0;
    m_delegateRef->releaseContext(m_modelRef, context);
    return false;
}

} /* namespace gl2 */
} /* namespace vpvl2 */
