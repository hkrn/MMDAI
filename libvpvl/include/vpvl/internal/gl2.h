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

#ifndef VPVL_INTERNAL_GL2_H_
#define VPVL_INTERNAL_GL2_H_

#include <vpvl/vpvl.h>

#include <vpvl/gl2/Renderer.h>
#include <map>

#ifdef VPVL_ENABLE_OPENCL
#ifdef __APPLE__
#include <OpenCL/cl.h>
#include <OpenCL/cl_gl.h>
#include <OpenCL/cl_gl_ext.h>
#else
#include <CL/cl.h>
#include <CL/cl_gl.h>
#endif /* __APPLE__ */
#endif

namespace vpvl
{

struct PMDModelUserData
{
    int unused;
};

namespace gl2
{

const GLsizei kShadowMappingTextureWidth = 1024;
const GLsizei kShadowMappingTextureHeight = 1024;

#ifdef VPVL_LINK_QT
class ShaderProgram : protected QGLFunctions
        #else
class ShaderProgram
        #endif
{
public:
    ShaderProgram(Renderer::IDelegate *delegate)
        : m_program(0),
          m_delegate(delegate),
          m_modelViewProjectionUniformLocation(0),
          m_positionAttributeLocation(0),
          m_normalAttributeLocation(0),
          m_message(0)
    {
    }
    virtual ~ShaderProgram() {
        if (m_program) {
            glDeleteProgram(m_program);
            m_program = 0;
        }
        delete[] m_message;
        m_message = 0;
        m_modelViewProjectionUniformLocation = 0;
        m_positionAttributeLocation = 0;
        m_normalAttributeLocation = 0;
    }

#ifdef VPVL_LINK_QT
    virtual void initializeContext(const QGLContext *context) {
        initializeGLFunctions(context);
    }
#endif

    virtual bool load(const char *vertexShaderSource, const char *fragmentShaderSource) {
        GLuint vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
        GLuint fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
        if (vertexShader && fragmentShader) {
            GLuint program = glCreateProgram();
            glAttachShader(program, vertexShader);
            glAttachShader(program, fragmentShader);
            glLinkProgram(program);
            glValidateProgram(program);
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            GLint linked;
            glGetProgramiv(program, GL_LINK_STATUS, &linked);
            if (!linked) {
                GLint len;
                glGetShaderiv(program, GL_INFO_LOG_LENGTH, &len);
                if (len > 0) {
                    delete[] m_message;
                    m_message = new char[len];
                    glGetProgramInfoLog(program, len, NULL, m_message);
                    log0(Renderer::kLogWarning, "%s", m_message);
                }
                glDeleteProgram(program);
                return false;
            }
            m_modelViewProjectionUniformLocation = glGetUniformLocation(program, "modelViewProjectionMatrix");
            m_positionAttributeLocation = glGetAttribLocation(program, "inPosition");
            m_normalAttributeLocation = glGetAttribLocation(program, "inNormal");
            m_program = program;
            log0(Renderer::kLogInfo, "Created a shader program (ID=%d)", m_program);
            return true;
        }
        else {
            return false;
        }
    }
    virtual void bind() {
        glUseProgram(m_program);
    }
    virtual void unbind() {
        glUseProgram(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    void setModelViewProjectionMatrix(const float value[16]) {
        glUniformMatrix4fv(m_modelViewProjectionUniformLocation, 1, GL_FALSE, value);
    }
    void setPosition(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_positionAttributeLocation);
        glVertexAttribPointer(m_positionAttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setNormal(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_normalAttributeLocation);
        glVertexAttribPointer(m_normalAttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
    }

protected:
    void log0(Renderer::LogLevel level, const char *format...) {
        va_list ap;
        va_start(ap, format);
        m_delegate->log(level, format, ap);
        va_end(ap);
    }

    GLuint m_program;

private:
    GLuint compileShader(const char *source, GLenum type) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, NULL);
        glCompileShader(shader);
        GLint compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint len;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
            if (len > 0) {
                delete[] m_message;
                m_message = new char[len];
                glGetShaderInfoLog(shader, len, NULL, m_message);
                log0(Renderer::kLogWarning, "%s", m_message);
            }
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

    Renderer::IDelegate *m_delegate;
    GLuint m_modelViewProjectionUniformLocation;
    GLuint m_positionAttributeLocation;
    GLuint m_normalAttributeLocation;
    char *m_message;
};

class EdgeProgram : public ShaderProgram
{
public:
    EdgeProgram(Renderer::IDelegate *delegate)
        : ShaderProgram(delegate),
          m_boneAttributesAttributeLocation(0),
          m_edgeAttributeLocation(0),
          m_boneMatricesUniformLocation(0),
          m_colorUniformLocation(0)
    {
    }
    ~EdgeProgram() {
        m_boneAttributesAttributeLocation = 0;
        m_edgeAttributeLocation = 0;
        m_boneMatricesUniformLocation = 0;
        m_colorUniformLocation = 0;
    }

    virtual bool load(const char *vertexShaderSource, const char *fragmentShaderSource) {
        bool ret = ShaderProgram::load(vertexShaderSource, fragmentShaderSource);
        if (ret) {
            m_boneAttributesAttributeLocation = glGetAttribLocation(m_program, "inBoneAttributes");
            m_edgeAttributeLocation = glGetAttribLocation(m_program, "inEdgeOffset");
            m_boneMatricesUniformLocation = glGetUniformLocation(m_program, "boneMatrices");
            m_colorUniformLocation = glGetUniformLocation(m_program, "color");
        }
        return ret;
    }
    void setBoneAttributes(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_boneAttributesAttributeLocation);
        glVertexAttribPointer(m_boneAttributesAttributeLocation, 3, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setEdge(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_edgeAttributeLocation);
        glVertexAttribPointer(m_edgeAttributeLocation, 1, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setBoneMatrices(const GLfloat *ptr, GLsizei size) {
        glUniformMatrix4fv(m_boneMatricesUniformLocation, size, GL_FALSE, ptr);
    }
    void setColor(const Vector3 &value) {
        glUniform4fv(m_colorUniformLocation, 1, value);
    }

private:
    GLuint m_boneAttributesAttributeLocation;
    GLuint m_edgeAttributeLocation;
    GLuint m_boneMatricesUniformLocation;
    GLuint m_colorUniformLocation;
};

class ZPlotProgram : public ShaderProgram
{
public:
    ZPlotProgram(Renderer::IDelegate *delegate)
        : ShaderProgram(delegate)
    {
    }
    ~ZPlotProgram() {
    }
};

class ObjectProgram : public ShaderProgram
{
public:
    ObjectProgram(Renderer::IDelegate *delegate)
        : ShaderProgram(delegate),
          m_lightColorUniformLocation(0),
          m_lightPositionUniformLocation(0)
    {
    }
    ~ObjectProgram() {
        m_lightColorUniformLocation = 0;
        m_lightPositionUniformLocation = 0;
    }

    virtual bool load(const char *vertexShaderSource, const char *fragmentShaderSource) {
        bool ret = ShaderProgram::load(vertexShaderSource, fragmentShaderSource);
        if (ret) {
            m_lightColorUniformLocation = glGetUniformLocation(m_program, "lightColor");
            m_lightPositionUniformLocation = glGetUniformLocation(m_program, "lightPosition");
        }
        return ret;
    }
    void setLightColor(const Color &value) {
        glUniform3fv(m_lightColorUniformLocation, 1, value);
    }
    void setLightPosition(const Vector3 &value) {
        glUniform3fv(m_lightPositionUniformLocation, 1, value);
    }

private:
    GLuint m_lightColorUniformLocation;
    GLuint m_lightPositionUniformLocation;
};

class ShadowProgram : public ObjectProgram
{
public:
    ShadowProgram(Renderer::IDelegate *delegate)
        : ObjectProgram(delegate),
          m_shadowMatrixUniformLocation(0)
    {
    }
    ~ShadowProgram() {
        m_shadowMatrixUniformLocation = 0;
    }

    virtual bool load(const char *vertexShaderSource, const char *fragmentShaderSource) {
        bool ret = ObjectProgram::load(vertexShaderSource, fragmentShaderSource);
        if (ret) {
            m_shadowMatrixUniformLocation = glGetUniformLocation(m_program, "shadowMatrix");
        }
        return ret;
    }
    void setShadowMatrix(const float value[16]) {
        glUniformMatrix4fv(m_shadowMatrixUniformLocation, 1, GL_FALSE, value);
    }

private:
    GLuint m_shadowMatrixUniformLocation;
};

class ModelProgram : public ObjectProgram
{
public:
    ModelProgram(Renderer::IDelegate *delegate)
        : ObjectProgram(delegate),
          m_texCoordAttributeLocation(0),
          m_toonTexCoordAttributeLocation(0),
          m_boneAttributesAttributeLocation(0),
          m_boneMatricesUniformLocation(0),
          m_lightViewMatrixUniformLocation(0),
          m_normalMatrixUniformLocation(0),
          m_materialAmbientUniformLocation(0),
          m_materialDiffuseUniformLocation(0),
          m_hasMainTextureUniformLocation(0),
          m_hasSubTextureUniformLocation(0),
          m_isMainSphereMapUniformLocation(0),
          m_isSubSphereMapUniformLocation(0),
          m_isMainAdditiveUniformLocation(0),
          m_isSubAdditiveUniformLocation(0),
          m_mainTextureUniformLocation(0),
          m_subTextureUniformLocation(0),
          m_toonTextureUniformLocation(0)
    {
    }
    ~ModelProgram() {
        m_texCoordAttributeLocation = 0;
        m_toonTexCoordAttributeLocation = 0;
        m_boneAttributesAttributeLocation = 0;
        m_boneMatricesUniformLocation = 0;
        m_lightViewMatrixUniformLocation = 0;
        m_normalMatrixUniformLocation = 0;
        m_materialAmbientUniformLocation = 0;
        m_materialDiffuseUniformLocation = 0;
        m_hasMainTextureUniformLocation = 0;
        m_hasSubTextureUniformLocation = 0;
        m_isMainSphereMapUniformLocation = 0;
        m_isSubSphereMapUniformLocation = 0;
        m_isMainAdditiveUniformLocation = 0;
        m_isSubAdditiveUniformLocation = 0;
        m_mainTextureUniformLocation = 0;
        m_subTextureUniformLocation = 0;
        m_toonTextureUniformLocation = 0;
    }

    virtual bool load(const char *vertexShaderSource, const char *fragmentShaderSource) {
        bool ret = ObjectProgram::load(vertexShaderSource, fragmentShaderSource);
        if (ret) {
            m_texCoordAttributeLocation = glGetAttribLocation(m_program, "inTexCoord");
            m_toonTexCoordAttributeLocation = glGetAttribLocation(m_program, "inToonTexCoord");
            m_boneAttributesAttributeLocation = glGetAttribLocation(m_program, "inBoneAttributes");
            m_boneMatricesUniformLocation = glGetUniformLocation(m_program, "boneMatrices");
            m_lightViewMatrixUniformLocation = glGetUniformLocation(m_program, "lightViewMatrix");
            m_normalMatrixUniformLocation = glGetUniformLocation(m_program, "normalMatrix");
            m_materialAmbientUniformLocation = glGetUniformLocation(m_program, "materialAmbient");
            m_materialDiffuseUniformLocation = glGetUniformLocation(m_program, "materialDiffuse");
            m_hasMainTextureUniformLocation = glGetUniformLocation(m_program, "hasMainTexture");
            m_hasSubTextureUniformLocation = glGetUniformLocation(m_program, "hasSubTexture");
            m_isMainSphereMapUniformLocation = glGetUniformLocation(m_program, "isMainSphereMap");
            m_isSubSphereMapUniformLocation = glGetUniformLocation(m_program, "isSubSphereMap");
            m_isMainAdditiveUniformLocation = glGetUniformLocation(m_program, "isMainAdditive");
            m_isSubAdditiveUniformLocation = glGetUniformLocation(m_program, "isSubAdditive");
            m_mainTextureUniformLocation = glGetUniformLocation(m_program, "mainTexture");
            m_subTextureUniformLocation = glGetUniformLocation(m_program, "subTexture");
            m_toonTextureUniformLocation = glGetUniformLocation(m_program, "toonTexture");
        }
        return ret;
    }
    virtual void bind() {
        ObjectProgram::bind();
    }
    void setTexCoord(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_texCoordAttributeLocation);
        glVertexAttribPointer(m_texCoordAttributeLocation, 2, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setToonTexCoord(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_toonTexCoordAttributeLocation);
        glVertexAttribPointer(m_toonTexCoordAttributeLocation, 2, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setBoneAttributes(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_boneAttributesAttributeLocation);
        glVertexAttribPointer(m_boneAttributesAttributeLocation, 3, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setBoneMatrices(const GLfloat *ptr, GLsizei size) {
        glUniformMatrix4fv(m_boneMatricesUniformLocation, size, GL_FALSE, ptr);
    }
    void setNormalMatrix(const float value[9]) {
        glUniformMatrix3fv(m_normalMatrixUniformLocation, 1, GL_FALSE, value);
    }
    void setMaterialAmbient(const Color &value) {
        glUniform3fv(m_materialAmbientUniformLocation, 1, value);
    }
    void setMaterialDiffuse(const Color &value) {
        glUniform4fv(m_materialDiffuseUniformLocation, 1, value);
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
    void setLightPosition(const Vector3 &value) {
        float matrix[16];
        glUniformMatrix4fv(m_lightViewMatrixUniformLocation, 1, GL_FALSE, matrix);
        ObjectProgram::setLightPosition(value);
    }
    void setMainTexture(GLuint value) {
        if (value) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, value);
            glUniform1i(m_mainTextureUniformLocation, 0);
            glUniform1i(m_hasMainTextureUniformLocation, 1);
        }
        else {
            glUniform1i(m_hasMainTextureUniformLocation, 0);
        }
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

private:
    GLuint m_texCoordAttributeLocation;
    GLuint m_toonTexCoordAttributeLocation;
    GLuint m_boneAttributesAttributeLocation;
    GLuint m_boneMatricesUniformLocation;
    GLuint m_lightViewMatrixUniformLocation;
    GLuint m_normalMatrixUniformLocation;
    GLuint m_materialAmbientUniformLocation;
    GLuint m_materialDiffuseUniformLocation;
    GLuint m_hasMainTextureUniformLocation;
    GLuint m_hasSubTextureUniformLocation;
    GLuint m_isMainSphereMapUniformLocation;
    GLuint m_isSubSphereMapUniformLocation;
    GLuint m_isMainAdditiveUniformLocation;
    GLuint m_isSubAdditiveUniformLocation;
    GLuint m_mainTextureUniformLocation;
    GLuint m_subTextureUniformLocation;
    GLuint m_toonTextureUniformLocation;
};

class ExtendedModelProgram : public ModelProgram
{
public:
    ExtendedModelProgram(Renderer::IDelegate *delegate)
        : ModelProgram(delegate),
          m_biasMatrixUniformLocation(0),
          m_shadowTextureUniformLocation(0)
    {
    }
    ~ExtendedModelProgram() {
        m_biasMatrixUniformLocation = 0;
        m_shadowTextureUniformLocation = 0;
    }

    virtual bool load(const char *vertexShaderSource, const char *fragmentShaderSource) {
        bool ret = ModelProgram::load(vertexShaderSource, fragmentShaderSource);
        if (ret) {
            m_biasMatrixUniformLocation = glGetUniformLocation(m_program, "biasMatrix");
            m_shadowTextureUniformLocation = glGetUniformLocation(m_program, "shadowTexture");
        }
        return ret;
    }
    virtual void bind() {
        static const float matrix[] = {
            0.5f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.5f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.5f, 0.0f,
            0.5f, 0.5f, 0.5f, 1.0f
        };
        ModelProgram::bind();
        glUniformMatrix4fv(m_biasMatrixUniformLocation, 1, GL_FALSE, matrix);
    }

    void setShadowTexture(GLuint value) {
        if (value) {
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, value);
            glUniform1i(m_shadowTextureUniformLocation, 3);
        }
    }

private:
    GLuint m_biasMatrixUniformLocation;
    GLuint m_shadowTextureUniformLocation;
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

#ifdef VPVL_LINK_QT
class PMDModelUserData : public vpvl::PMDModel::UserData, protected QGLFunctions
        #else
class PMDModelUserData : public vpvl::PMDModel::UserData
        #endif
{
public:
    PMDModelUserData()
        : PMDModel::UserData(),
          edgeProgram(0),
          modelProgram(0),
          shadowProgram(0),
          zplotProgram(0),
          hasSingleSphereMap(false),
          hasMultipleSphereMap(false),
          materials(0)
    #ifdef VPVL_ENABLE_OPENCL
        ,
          vertexBufferForCL(0),
          boneMatricesBuffer(0),
          originMatricesBuffer(0),
          outputMatricesBuffer(0),
          weightsBuffer(0),
          bone1IndicesBuffer(0),
          bone2IndicesBuffer(0),
          weights(0),
          boneTransform(0),
          originTransform(0),
          bone1Indices(0),
          bone2Indices(0),

          isBufferAllocated(false)
    #endif
    {
    }
    ~PMDModelUserData() {
        for (int i = 0; i < PMDModel::kCustomTextureMax; i++) {
            glDeleteTextures(1, &toonTextureID[i]);
        }
        glDeleteBuffers(kVertexBufferObjectMax, vertexBufferObjects);
        delete edgeProgram;
        edgeProgram = 0;
        delete modelProgram;
        modelProgram = 0;
        delete shadowProgram;
        shadowProgram = 0;
        delete zplotProgram;
        zplotProgram = 0;
#ifdef VPVL_ENABLE_OPENCL
        delete[] boneTransform;
        delete[] originTransform;
        delete[] bone1Indices;
        delete[] bone2Indices;
        delete[] weights;
        clReleaseMemObject(vertexBufferForCL);
        clReleaseMemObject(boneMatricesBuffer);
        clReleaseMemObject(originMatricesBuffer);
        clReleaseMemObject(outputMatricesBuffer);
        clReleaseMemObject(bone1IndicesBuffer);
        clReleaseMemObject(bone2IndicesBuffer);
        clReleaseMemObject(weightsBuffer);
        isBufferAllocated = false;
#endif
    }

#ifdef VPVL_LINK_QT
    virtual void initializeContext(const QGLContext *context) {
        initializeGLFunctions(context);
    }
#endif

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
    ZPlotProgram *zplotProgram;
    GLuint toonTextureID[PMDModel::kCustomTextureMax];
    GLuint vertexBufferObjects[kVertexBufferObjectMax];
    bool hasSingleSphereMap;
    bool hasMultipleSphereMap;
    PMDModelMaterialPrivate *materials;
#ifdef VPVL_ENABLE_OPENCL
    cl_mem vertexBufferForCL;
    cl_mem boneMatricesBuffer;
    cl_mem originMatricesBuffer;
    cl_mem outputMatricesBuffer;
    cl_mem weightsBuffer;
    cl_mem bone1IndicesBuffer;
    cl_mem bone2IndicesBuffer;
    size_t localWGSizeForUpdateBoneMatrices;
    size_t localWGSizeForPerformSkinning;
    float *weights;
    float *boneTransform;
    float *originTransform;
    int *bone1Indices;
    int *bone2Indices;
    bool isBufferAllocated;
#endif
};

#ifdef VPVL_LINK_ASSIMP
struct AssetVertex
{
    vpvl::Vector4 position;
    vpvl::Vector3 normal;
    vpvl::Vector3 texcoord;
    vpvl::Color color;
};
struct AssetVBO
{
    GLuint vertices;
    GLuint indices;
};
typedef btAlignedObjectArray<AssetVertex> AssetVertices;
typedef btAlignedObjectArray<uint32_t> AssetIndices;

class AssetProgram;

class AssetUserData : public vpvl::Asset::UserData
{
public:
    AssetUserData() : Asset::UserData() {}
    ~AssetUserData() {}

    std::map<std::string, GLuint> textures;
    std::map<const struct aiMesh *, AssetVertices> vertices;
    std::map<const struct aiMesh *, AssetIndices> indices;
    std::map<const struct aiMesh *, AssetVBO> vbo;
    std::map<const struct aiNode *, vpvl::gl2::AssetProgram *> programs;
};
#endif

}

}

#endif
