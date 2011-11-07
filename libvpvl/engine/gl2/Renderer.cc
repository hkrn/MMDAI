/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn                                    */
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

#include <vpvl/internal/gl2.h>

#include <btBulletDynamicsCommon.h>

#ifdef VPVL_LINK_ASSIMP
#include <aiScene.h>
#include <map>
#endif

namespace vpvl
{
namespace gl2
{
class ShaderProgram
{
public:
    ShaderProgram(IDelegate *delegate)
        : m_program(0),
          m_delegate(delegate),
          m_modelViewUniformLocation(0),
          m_projectionUniformLocation(0),
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
        m_modelViewUniformLocation = 0;
        m_projectionUniformLocation = 0;
        m_positionAttributeLocation = 0;
        m_normalAttributeLocation = 0;
    }

    virtual bool load(const char *vertexShaderSource, const char *fragmentShaderSource) {
        GLuint vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
        GLuint fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
        if (vertexShader && fragmentShader) {
            GLuint program = glCreateProgram();
            glAttachShader(program, vertexShader);
            glAttachShader(program, fragmentShader);
            glLinkProgram(program);
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
                    m_delegate->log(IDelegate::kLogWarning, "%s", m_message);
                }
                glDeleteProgram(program);
                return false;
            }
            m_modelViewUniformLocation = glGetUniformLocation(program, "modelViewMatrix");
            m_projectionUniformLocation = glGetUniformLocation(program, "projectionMatrix");
            m_positionAttributeLocation = glGetAttribLocation(program, "inPosition");
            m_normalAttributeLocation = glGetAttribLocation(program, "inNormal");
            m_program = program;
            m_delegate->log(IDelegate::kLogInfo, "Created a shader program (ID=%d)", m_program);
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
    void setModelViewMatrix(const float value[16]) {
        glUniformMatrix4fv(m_modelViewUniformLocation, 1, GL_FALSE, value);
    }
    void setProjectionMatrix(const float value[16]) {
        glUniformMatrix4fv(m_projectionUniformLocation, 1, GL_FALSE, value);
    }
    void setPosition(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_positionAttributeLocation);
        glVertexAttribPointer(m_positionAttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setNormal(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_normalAttributeLocation);
        glVertexAttribPointer(m_normalAttributeLocation, 3, GL_FLOAT, GL_FALSE, stride, ptr);
    }

protected:
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
                m_delegate->log(IDelegate::kLogWarning, "%s", m_message);
            }
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

    IDelegate *m_delegate;
    GLuint m_modelViewUniformLocation;
    GLuint m_projectionUniformLocation;
    GLuint m_positionAttributeLocation;
    GLuint m_normalAttributeLocation;
    char *m_message;
};

class EdgeProgram : public ShaderProgram {
public:
    EdgeProgram(IDelegate *delegate)
        : ShaderProgram(delegate),
          m_colorUniformLocation(0)
    {
    }
    ~EdgeProgram() {
        m_colorUniformLocation = 0;
    }

    virtual bool load(const char *vertexShaderSource, const char *fragmentShaderSource) {
        bool ret = ShaderProgram::load(vertexShaderSource, fragmentShaderSource);
        if (ret)
            m_colorUniformLocation = glGetUniformLocation(m_program, "color");
        return ret;
    }
    void setColor(const Vector3 &value) {
        glUniform4fv(m_colorUniformLocation, 1, value);
    }

private:
    GLuint m_colorUniformLocation;
};

class ShadowProgram : public ShaderProgram {
public:
    ShadowProgram(IDelegate *delegate)
        : ShaderProgram(delegate)
    {
    }
    ~ShadowProgram() {
    }
};

class ObjectProgram : public ShaderProgram {
public:
    ObjectProgram(IDelegate *delegate)
        : ShaderProgram(delegate),
          m_lightColorUniformLocation(0),
          m_lightPositionUniformLocation(0),
          m_lightAmbientUniformLocation(0),
          m_lightDiffuseUniformLocation(0),
          m_lightSpecularUniformLocation(0)
    {
    }
    ~ObjectProgram() {
        m_lightColorUniformLocation = 0;
        m_lightPositionUniformLocation = 0;
        m_lightAmbientUniformLocation = 0;
        m_lightDiffuseUniformLocation = 0;
        m_lightSpecularUniformLocation = 0;
    }

    virtual bool load(const char *vertexShaderSource, const char *fragmentShaderSource) {
        bool ret = ShaderProgram::load(vertexShaderSource, fragmentShaderSource);
        if (ret) {
            m_lightColorUniformLocation = glGetUniformLocation(m_program, "lightColor");
            m_lightPositionUniformLocation = glGetUniformLocation(m_program, "lightPosition");
            m_lightAmbientUniformLocation = glGetUniformLocation(m_program, "lightAmbient");
            m_lightDiffuseUniformLocation = glGetUniformLocation(m_program, "lightDiffuse");
            m_lightSpecularUniformLocation = glGetUniformLocation(m_program, "lightSpecular");
        }
        return ret;
    }
    void setLightColor(const Color &value) {
        glUniform4fv(m_lightColorUniformLocation, 1, value);
    }
    void setLightPosition(const Vector3 &value) {
        glUniform3fv(m_lightPositionUniformLocation, 1, value);
    }
    void setLightAmbient(const Color &value) {
        glUniform4fv(m_lightAmbientUniformLocation, 1, value);
    }
    void setLightDiffuse(const Color &value) {
        glUniform4fv(m_lightDiffuseUniformLocation, 1, value);
    }
    void setLightSpecular(const Color &value) {
        glUniform4fv(m_lightSpecularUniformLocation, 1, value);
    }

private:
    GLuint m_lightColorUniformLocation;
    GLuint m_lightPositionUniformLocation;
    GLuint m_lightAmbientUniformLocation;
    GLuint m_lightDiffuseUniformLocation;
    GLuint m_lightSpecularUniformLocation;
};

class ModelProgram : public ObjectProgram {
public:
    ModelProgram(IDelegate *delegate)
        : ObjectProgram(delegate),
          m_texCoordAttributeLocation(0),
          m_toonTexCoordAttributeLocation(0),
          m_normalMatrixUniformLocation(0),
          m_materialAmbientUniformLocation(0),
          m_materialDiffuseUniformLocation(0),
          m_materialSpecularUniformLocation(0),
          m_materialShininessUniformLocation(0),
          m_lightIntensityUniformLocation(0),
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
        m_normalMatrixUniformLocation = 0;
        m_materialAmbientUniformLocation = 0;
        m_materialDiffuseUniformLocation = 0;
        m_materialSpecularUniformLocation = 0;
        m_materialShininessUniformLocation = 0;
        m_lightIntensityUniformLocation = 0;
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
            m_normalMatrixUniformLocation = glGetUniformLocation(m_program, "normalMatrix");
            m_materialAmbientUniformLocation = glGetUniformLocation(m_program, "materialAmbient");
            m_materialDiffuseUniformLocation = glGetUniformLocation(m_program, "materialDiffuse");
            m_materialSpecularUniformLocation = glGetUniformLocation(m_program, "materialSpecular");
            m_materialShininessUniformLocation = glGetUniformLocation(m_program, "materialShininess");
            m_lightIntensityUniformLocation = glGetUniformLocation(m_program, "lightIntensity");
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
        resetTextureState();
    }
    virtual void resetTextureState() {
        glUniform1i(m_hasMainTextureUniformLocation, 0);
        glUniform1i(m_hasSubTextureUniformLocation, 0);
        glUniform1i(m_isMainSphereMapUniformLocation, 0);
        glUniform1i(m_isSubSphereMapUniformLocation, 0);
        glUniform1i(m_isMainAdditiveUniformLocation, 0);
        glUniform1i(m_isSubAdditiveUniformLocation, 0);
    }
    void setTexCoord(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_texCoordAttributeLocation);
        glVertexAttribPointer(m_texCoordAttributeLocation, 2, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setToonTexCoord(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_toonTexCoordAttributeLocation);
        glVertexAttribPointer(m_toonTexCoordAttributeLocation, 2, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setNormalMatrix(const float value[9]) {
        glUniformMatrix3fv(m_normalMatrixUniformLocation, 1, GL_FALSE, value);
    }
    void setMaterialAmbient(const Color &value) {
        glUniform4fv(m_materialAmbientUniformLocation, 1, value);
    }
    void setMaterialDiffuse(const Color &value) {
        glUniform4fv(m_materialDiffuseUniformLocation, 1, value);
    }
    void setMaterialSpecular(const Color &value) {
        glUniform4fv(m_materialSpecularUniformLocation, 1, value);
    }
    void setMaterialShininess(float value) {
        glUniform1f(m_materialShininessUniformLocation, value);
    }
    void setLightIntensity(float value) {
        glUniform1f(m_lightIntensityUniformLocation, value);
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
    void setMainTexture(GLuint value) {
        if (value) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, value);
            glUniform1i(m_mainTextureUniformLocation, 0);
            glUniform1i(m_hasMainTextureUniformLocation, 1);
        }
    }
    void setSubTexture(GLuint value) {
        if (value) {
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, value);
            glUniform1i(m_subTextureUniformLocation, 2);
            glUniform1i(m_hasSubTextureUniformLocation, 1);
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
    GLuint m_normalMatrixUniformLocation;
    GLuint m_materialAmbientUniformLocation;
    GLuint m_materialDiffuseUniformLocation;
    GLuint m_materialSpecularUniformLocation;
    GLuint m_materialShininessUniformLocation;
    GLuint m_lightIntensityUniformLocation;
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

class ExtendedModelProgram : public ModelProgram {
public:
    ExtendedModelProgram(IDelegate *delegate)
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
        resetTextureState();
        glUniformMatrix4fv(m_biasMatrixUniformLocation, 1, GL_FALSE, matrix);
    }
    virtual void resetTextureState() {
        ModelProgram::resetTextureState();
        glUniform1i(m_shadowTextureUniformLocation, 0);
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

class AssetProgram : public ObjectProgram {
public:
    AssetProgram(IDelegate *delegate)
        : ObjectProgram(delegate),
          m_texCoordAttributeLocation(0),
          m_colorAttributeLocation(0),
          m_normalMatrixUniformLocation(0),
          m_transformMatrixUniformLocation(0),
          m_materialAmbientUniformLocation(0),
          m_materialDiffuseUniformLocation(0),
          m_materialSpecularUniformLocation(0),
          m_materialShininessUniformLocation(0),
          m_hasTextureUniformLocation(0),
          m_hasColorVertexUniformLocation(0),
          m_textureUniformLocation(0),
          m_opacityUniformLocation(0)
    {
    }
    ~AssetProgram() {
        m_texCoordAttributeLocation = 0;
        m_colorAttributeLocation = 0;
        m_normalMatrixUniformLocation = 0;
        m_transformMatrixUniformLocation = 0;
        m_materialAmbientUniformLocation = 0;
        m_materialDiffuseUniformLocation = 0;
        m_materialEmissionUniformLocation = 0;
        m_materialSpecularUniformLocation = 0;
        m_materialShininessUniformLocation = 0;
        m_hasTextureUniformLocation = 0;
        m_hasColorVertexUniformLocation = 0;
        m_textureUniformLocation = 0;
        m_opacityUniformLocation = 0;
    }

    virtual bool load(const char *vertexShaderSource, const char *fragmentShaderSource) {
        bool ret = ObjectProgram::load(vertexShaderSource, fragmentShaderSource);
        if (ret) {
            m_texCoordAttributeLocation = glGetAttribLocation(m_program, "inTexCoord");
            m_colorAttributeLocation = glGetAttribLocation(m_program, "inColor");
            m_normalMatrixUniformLocation = glGetUniformLocation(m_program, "normalMatrix");
            m_transformMatrixUniformLocation = glGetUniformLocation(m_program, "transformMatrix");
            m_materialAmbientUniformLocation = glGetUniformLocation(m_program, "materialAmbient");
            m_materialDiffuseUniformLocation = glGetUniformLocation(m_program, "materialDiffuse");
            m_materialEmissionUniformLocation = glGetUniformLocation(m_program, "materialEmission");
            m_materialSpecularUniformLocation = glGetUniformLocation(m_program, "materialSpecular");
            m_materialShininessUniformLocation = glGetUniformLocation(m_program, "materialShininess");
            m_hasTextureUniformLocation = glGetUniformLocation(m_program, "hasTexture");
            m_hasColorVertexUniformLocation = glGetUniformLocation(m_program, "hasColorVertex");
            m_textureUniformLocation = glGetUniformLocation(m_program, "mainTexture");
            m_opacityUniformLocation = glGetUniformLocation(m_program, "opacity");
        }
        return ret;
    }
    virtual void bind() {
        ObjectProgram::bind();
        resetTextureState();
    }
    virtual void resetTextureState() {
        glUniform1i(m_hasTextureUniformLocation, 0);
    }

    void setTexCoord(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_texCoordAttributeLocation);
        glVertexAttribPointer(m_texCoordAttributeLocation, 2, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setColor(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_colorAttributeLocation);
        glVertexAttribPointer(m_colorAttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setHasColor(bool value) {
        glUniform1i(m_hasColorVertexUniformLocation, value ? 1 : 0);
    }
    void setNormalMatrix(const float value[16]) {
        glUniformMatrix3fv(m_normalMatrixUniformLocation, 1, GL_FALSE, value);
    }
    void setTransformMatrix(const float value[9]) {
        glUniformMatrix4fv(m_transformMatrixUniformLocation, 1, GL_FALSE, value);
    }
    void setMaterialAmbient(const Color &value) {
        glUniform4fv(m_materialAmbientUniformLocation, 1, value);
    }
    void setMaterialDiffuse(const Color &value) {
        glUniform4fv(m_materialDiffuseUniformLocation, 1, value);
    }
    void setMaterialEmission(const Color &value) {
        glUniform4fv(m_materialEmissionUniformLocation, 1, value);
    }
    void setMaterialSpecular(const Color &value) {
        glUniform4fv(m_materialSpecularUniformLocation, 1, value);
    }
    void setMaterialShininess(float value) {
        glUniform1f(m_materialShininessUniformLocation, value);
    }
    void setOpacity(float value) {
        glUniform1f(m_opacityUniformLocation, value);
    }
    void setTexture(GLuint value) {
        if (value) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, value);
            glUniform1i(m_textureUniformLocation, 0);
            glUniform1i(m_hasTextureUniformLocation, 1);
        }
    }

private:
    GLuint m_texCoordAttributeLocation;
    GLuint m_colorAttributeLocation;
    GLuint m_normalMatrixUniformLocation;
    GLuint m_transformMatrixUniformLocation;
    GLuint m_materialAmbientUniformLocation;
    GLuint m_materialDiffuseUniformLocation;
    GLuint m_materialEmissionUniformLocation;
    GLuint m_materialSpecularUniformLocation;
    GLuint m_materialShininessUniformLocation;
    GLuint m_hasTextureUniformLocation;
    GLuint m_hasColorVertexUniformLocation;
    GLuint m_textureUniformLocation;
    GLuint m_opacityUniformLocation;
};

}
}

#ifdef VPVL_LINK_ASSIMP
namespace
{
struct AssetVertex
{
    vpvl::Vector3 position;
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
}
namespace vpvl
{
struct AssetUserData
{
    std::map<std::string, GLuint> textures;
    std::map<const struct aiMesh *, AssetVertices> vertices;
    std::map<const struct aiMesh *, AssetIndices> indices;
    std::map<const struct aiMesh *, AssetVBO> vbo;
    std::map<const struct aiNode *, vpvl::gl2::AssetProgram *> programs;
};
}
namespace
{

void aiLoadAssetRecursive(const aiScene *scene, const aiNode *node, vpvl::AssetUserData *userData, vpvl::gl2::IDelegate *delegate)
{
    const unsigned int nmeshes = node->mNumMeshes;
    AssetVertex assetVertex;
    vpvl::gl2::AssetProgram *program = new vpvl::gl2::AssetProgram(delegate);
    program->load(delegate->loadShader(vpvl::gl2::IDelegate::kAssetVertexShader).c_str(),
                  delegate->loadShader(vpvl::gl2::IDelegate::kAssetFragmentShader).c_str());
    userData->programs[node] = program;
    for (unsigned int i = 0; i < nmeshes; i++) {
        const struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        const aiVector3D *vertices = mesh->mVertices;
        const aiVector3D *normals = mesh->mNormals;
        const bool hasNormals = mesh->HasNormals();
        const bool hasColors = mesh->HasVertexColors(0);
        const bool hasTexCoords = mesh->HasTextureCoords(0);
        const aiColor4D *colors = hasColors ? mesh->mColors[0] : 0;
        const aiVector3D *texcoords = hasTexCoords ? mesh->mTextureCoords[0] : 0;
        AssetVertices &assetVertices = userData->vertices[mesh];
        AssetIndices &indices = userData->indices[mesh];
        const unsigned int nfaces = mesh->mNumFaces;
        int index = 0;
        for (unsigned int j = 0; j < nfaces; j++) {
            const struct aiFace &face = mesh->mFaces[j];
            const unsigned int nindices = face.mNumIndices;
            for (unsigned int k = 0; k < nindices; k++) {
                int vertexIndex = face.mIndices[k];
                if (hasColors) {
                    const aiColor4D &c = colors[vertexIndex];
                    assetVertex.color.setValue(c.r, c.g, c.b, c.a);
                }
                else {
                    assetVertex.color.setZero();
                    assetVertex.color.setW(1.0f);
                }
                if (hasTexCoords) {
                    const aiVector3D &p = texcoords[vertexIndex];
                    assetVertex.texcoord.setValue(p.x, p.y, 0.0f);
                }
                else {
                    assetVertex.texcoord.setZero();
                }
                if (hasNormals) {
                    const aiVector3D &n = normals[vertexIndex];
                    assetVertex.normal.setValue(n.x, n.y, n.z);
                }
                else {
                    assetVertex.normal.setZero();
                }
                const aiVector3D &v = vertices[vertexIndex];
                assetVertex.position.setValue(v.x, v.y, v.z);
                assetVertex.position.setW(1.0f);
                assetVertices.push_back(assetVertex);
                indices.push_back(index);
                index++;
            }
        }
        AssetVBO &vbo = userData->vbo[mesh];
        size_t vsize = assetVertices.size() * sizeof(assetVertices[0]);
        glGenBuffers(1, &vbo.vertices);
        glBindBuffer(GL_ARRAY_BUFFER, vbo.vertices);
        glBufferData(GL_ARRAY_BUFFER, vsize, assetVertices[0].position, GL_STATIC_DRAW);
        glGenBuffers(1, &vbo.indices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.indices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), &indices[0], GL_STATIC_DRAW);
    }
    const unsigned int nChildNodes = node->mNumChildren;
    for (unsigned int i = 0; i < nChildNodes; i++)
        aiLoadAssetRecursive(scene, node->mChildren[i], userData, delegate);
}

void aiUnloadAssetRecursive(const aiScene *scene, const aiNode *node, vpvl::AssetUserData *userData)
{
    const unsigned int nmeshes = node->mNumMeshes;
    for (unsigned int i = 0; i < nmeshes; i++) {
        const struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        const AssetVBO &vbo = userData->vbo[mesh];
        glDeleteBuffers(1, &vbo.vertices);
        glDeleteBuffers(1, &vbo.indices);
    }
    delete userData->programs[node];
    const unsigned int nChildNodes = node->mNumChildren;
    for (unsigned int i = 0; i < nChildNodes; i++)
        aiUnloadAssetRecursive(scene, node->mChildren[i], userData);
}

void aiSetAssetMaterial(const aiMaterial *material, vpvl::Asset *asset, vpvl::gl2::AssetProgram *program)
{
    int textureIndex = 0;
    aiString texturePath;
    if (material->GetTexture(aiTextureType_DIFFUSE, textureIndex, &texturePath) == aiReturn_SUCCESS) {
        GLuint textureID = asset->userData()->textures[texturePath.data];
        program->setTexture(textureID);
    }
    else {
        program->setTexture(0);
    }
    aiColor4D ambient, diffuse, emission, specular;
    vpvl::Color color(0.0f, 0.0f, 0.0f, 0.0f);
    if (aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &ambient) == aiReturn_SUCCESS) {
        color.setValue(ambient.r, ambient.g, ambient.b, ambient.a);
        program->setMaterialAmbient(color);
    }
    else {
        program->setMaterialAmbient(vpvl::Color(0.2f, 0.2f, 0.2f, 1.0f));
    }
    if (aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse) == aiReturn_SUCCESS) {
        color.setValue(diffuse.r, diffuse.g, diffuse.b, diffuse.a);
        program->setMaterialDiffuse(color);
    }
    else {
        program->setMaterialDiffuse(vpvl::Color(0.8f, 0.8f, 0.8f, 1.0f));
    }
    if (aiGetMaterialColor(material, AI_MATKEY_COLOR_EMISSIVE, &emission) == aiReturn_SUCCESS) {
        color.setValue(emission.r, emission.g, emission.b, emission.a);
        program->setMaterialEmission(color);
    }
    else {
        program->setMaterialEmission(vpvl::Color(0.0f, 0.0f, 0.0f, 1.0f));
    }
    if (aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &specular) == aiReturn_SUCCESS) {
        color.setValue(specular.r, specular.g, specular.b, specular.a);
        program->setMaterialSpecular(color);
    }
    else {
        program->setMaterialSpecular(vpvl::Color(0.0f, 0.0f, 0.0f, 1.0f));
    }
    float shininess, strength;
    int ret1 = aiGetMaterialFloat(material, AI_MATKEY_SHININESS, &shininess);
    int ret2 = aiGetMaterialFloat(material, AI_MATKEY_SHININESS_STRENGTH, &strength);
    if (ret1 == aiReturn_SUCCESS && ret2 == aiReturn_SUCCESS) {
        program->setMaterialShininess(shininess * strength);
    }
    else if (ret1 == aiReturn_SUCCESS) {
        program->setMaterialShininess(shininess);
    }
    else {
        program->setMaterialShininess(15.0f);
    }
    float opacity;
    if (aiGetMaterialFloat(material, AI_MATKEY_OPACITY, &opacity) == aiReturn_SUCCESS) {
        program->setOpacity(opacity * asset->opacity());
    }
    else {
        program->setOpacity(asset->opacity());
    }
    int wireframe, twoside;
    if (aiGetMaterialInteger(material, AI_MATKEY_ENABLE_WIREFRAME, &wireframe) == aiReturn_SUCCESS && wireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if (aiGetMaterialInteger(material, AI_MATKEY_TWOSIDED, &twoside) == aiReturn_SUCCESS && twoside)
        glEnable(GL_CULL_FACE);
    else
        glDisable(GL_CULL_FACE);
}

void aiDrawAssetRecurse(const aiScene *scene, const aiNode *node, vpvl::Asset *asset, vpvl::Scene *s)
{
    const btScalar &scaleFactor = asset->scaleFactor();
    const vpvl::Bone *bone = asset->parentBone();
    float matrix4x4[16], matrix3x3[9];
    aiVector3D aiS, aiP;
    aiQuaternion aiQ;
    node->mTransformation.Decompose(aiS, aiQ, aiP);
    vpvl::Transform transform(btMatrix3x3(vpvl::Quaternion(aiQ.x, aiQ.y, aiQ.z, aiQ.w) * asset->rotation())
                              .scaled(vpvl::Vector3(aiS.x * scaleFactor, aiS.y * scaleFactor, aiS.z * scaleFactor)),
                              vpvl::Vector3(aiP.x,aiP.y, aiP.z) + asset->position());
    if (bone) {
        const vpvl::Transform &boneTransform = bone->localTransform();
        transform.setBasis(boneTransform.getBasis() * transform.getBasis());
        transform.setOrigin(boneTransform.getOrigin() + transform.getOrigin());
    }
    vpvl::AssetUserData *userData = asset->userData();
    AssetVertex v;
    const GLvoid *vertexPtr = 0;
    const GLvoid *normalPtr = reinterpret_cast<const GLvoid *>(reinterpret_cast<const uint8_t *>(&v.normal) - reinterpret_cast<const uint8_t *>(&v.position));
    const GLvoid *texcoordPtr = reinterpret_cast<const GLvoid *>(reinterpret_cast<const uint8_t *>(&v.texcoord) - reinterpret_cast<const uint8_t *>(&v.position));
    const GLvoid *colorPtr = reinterpret_cast<const GLvoid *>(reinterpret_cast<const uint8_t *>(&v.color) - reinterpret_cast<const uint8_t *>(&v.position));
    const unsigned int nmeshes = node->mNumMeshes;
    const size_t stride = sizeof(AssetVertex);
    vpvl::gl2::AssetProgram *program = userData->programs[node];
    program->bind();
    s->getModelViewMatrix(matrix4x4);
    program->setModelViewMatrix(matrix4x4);
    s->getProjectionMatrix(matrix4x4);
    program->setProjectionMatrix(matrix4x4);
    s->getNormalMatrix(matrix3x3);
    program->setNormalMatrix(matrix3x3);
    transform.getOpenGLMatrix(matrix4x4);
    program->setTransformMatrix(matrix4x4);
    program->setLightAmbient(s->lightAmbient());
    program->setLightColor(s->lightColor());
    program->setLightDiffuse(s->lightDiffuse());
    program->setLightPosition(s->lightPosition());
    program->setLightSpecular(s->lightSpecular());
    for (unsigned int i = 0; i < nmeshes; i++) {
        const struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        const AssetVBO &vbo = userData->vbo[mesh];
        const AssetIndices &indices = userData->indices[mesh];
        aiSetAssetMaterial(scene->mMaterials[mesh->mMaterialIndex], asset, program);
        glBindBuffer(GL_ARRAY_BUFFER, vbo.vertices);
        program->setPosition(vertexPtr, stride);
        program->setNormal(normalPtr, stride);
        program->setTexCoord(texcoordPtr, stride);
        program->setColor(colorPtr, stride);
        program->setHasColor(mesh->HasVertexColors(0));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.indices);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        program->resetTextureState();
    }
    program->unbind();
    const unsigned int nChildNodes = node->mNumChildren;
    for (unsigned int i = 0; i < nChildNodes; i++)
        aiDrawAssetRecurse(scene, node->mChildren[i], asset, s);
}

void aiDrawAsset(vpvl::Asset *asset, vpvl::Scene *s)
{
    const aiScene *a = asset->getScene();
    aiDrawAssetRecurse(a, a->mRootNode, asset, s);
}

}
#else
VPVL_DECLARE_HANDLE(aiNode)
VPVL_DECLARE_HANDLE(aiScene)
namespace vpvl
{
    namespace gl
    {
    VPVL_DECLARE_HANDLE(AssetInternal)
    }
}
namespace
{
void aiDrawAsset(vpvl::Asset *asset, vpvl::Scene *s)
{
    (void) asset;
    (void) s;
}
}
#endif

namespace {

const std::string CanonicalizePath(const std::string &path)
{
    const std::string from("\\"), to("/");
    std::string ret(path);
    std::string::size_type pos(path.find(from));
    while (pos != std::string::npos) {
        ret.replace(pos, from.length(), to);
        pos = ret.find(from, pos + to.length());
    }
    return ret;
}

}

namespace vpvl
{

namespace gl2
{

Renderer::Renderer(IDelegate *delegate, int width, int height, int fps)
    : m_delegate(delegate),
      m_edgeProgram(0),
      m_modelProgram(0),
      m_shadowProgram(0),
      m_scene(0),
      m_selected(0),
      m_depthTextureID(0),
      m_frameBufferID(0)
{
    m_scene = new vpvl::Scene(width, height, fps);
}

Renderer::~Renderer()
{
    glDeleteFramebuffers(1, &m_frameBufferID);
    glDeleteTextures(1, &m_depthTextureID);
    vpvl::Array<vpvl::PMDModel *> models;
    models.copy(m_scene->models());
    const int nmodels = models.count();
    for (int i = 0; i < nmodels; i++) {
        vpvl::PMDModel *model = models[i];
        unloadModel(model);
    }
    vpvl::Array<vpvl::Asset *> assets;
    assets.copy(m_assets);
    const int nassets = assets.count();
    for (int i = 0; i < nassets; i++) {
        vpvl::Asset *asset = assets[i];
        unloadAsset(asset);
    }
    models.releaseAll();
    assets.clear();
    delete m_edgeProgram;
    m_edgeProgram = 0;
    delete m_modelProgram;
    m_modelProgram = 0;
    delete m_shadowProgram;
    m_shadowProgram = 0;
    delete m_scene;
    m_scene = 0;
}

void Renderer::initializeSurface()
{
    glClearStencil(0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

bool Renderer::createPrograms()
{
    bool ret = false;
    std::string vertexShader;
    std::string fragmentShader;
    vertexShader = m_delegate->loadShader(IDelegate::kEdgeVertexShader);
    fragmentShader = m_delegate->loadShader(IDelegate::kEdgeFragmentShader);
    EdgeProgram *edgeProgram = new EdgeProgram(m_delegate);
    ret = edgeProgram->load(vertexShader.c_str(), fragmentShader.c_str());
    if (!ret) {
        delete edgeProgram;
        return ret;
    }
    if (m_depthTextureID && m_frameBufferID) {
        ExtendedModelProgram *modelProgram = new ExtendedModelProgram(m_delegate);
        vertexShader = m_delegate->loadShader(IDelegate::kModelVertexShader);
        fragmentShader = m_delegate->loadShader(IDelegate::kModelFragmentShader);
        ret = modelProgram->load(vertexShader.c_str(), fragmentShader.c_str());
        if (!ret) {
            delete edgeProgram;
            delete modelProgram;
            return ret;
        }
        ShadowProgram *shadowProgram = new ShadowProgram(m_delegate);
        vertexShader = m_delegate->loadShader(IDelegate::kShadowVertexShader);
        fragmentShader = m_delegate->loadShader(IDelegate::kShadowFragmentShader);
        ret = shadowProgram->load(vertexShader.c_str(), fragmentShader.c_str());
        if (!ret) {
            delete edgeProgram;
            delete modelProgram;
            delete shadowProgram;
            return ret;
        }
        m_modelProgram = modelProgram;
        m_shadowProgram = shadowProgram;
    }
    else {
        ModelProgram *modelProgram = new ModelProgram(m_delegate);
        vertexShader = m_delegate->loadShader(IDelegate::kModelVertexShader);
        fragmentShader = m_delegate->loadShader(IDelegate::kModelFragmentShader);
        ret = modelProgram->load(vertexShader.c_str(), fragmentShader.c_str());
        if (!ret) {
            delete edgeProgram;
            delete modelProgram;
            return ret;
        }
        m_modelProgram = modelProgram;
    }
    m_edgeProgram = edgeProgram;
    return ret;
}

bool Renderer::createShadowFrameBuffers()
{
    bool ret = true;
    int width = 1024;
    int height = width;
    glGenTextures(1, &m_depthTextureID);
    glBindTexture(GL_TEXTURE_2D, m_depthTextureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glGenFramebuffers(1, &m_frameBufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferID);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTextureID, 0);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status == GL_FRAMEBUFFER_COMPLETE) {
        m_delegate->log(IDelegate::kLogInfo,
                        "Created a framebuffer (textureID = %d, frameBufferID = %d)",
                        m_depthTextureID,
                        m_frameBufferID);
    }
    else {
        m_delegate->log(IDelegate::kLogWarning, "Failed creating a framebuffer: %d", status);
        ret = false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return ret;
}

void Renderer::resize(int width, int height)
{
    m_scene->setWidth(width);
    m_scene->setHeight(height);
}

void Renderer::loadModel(vpvl::PMDModel *model, const std::string &dir)
{
    loadModel0(new vpvl::gl2::PMDModelUserData(), model, dir);
}

void Renderer::loadModel0(vpvl::gl2::PMDModelUserData *userData, vpvl::PMDModel *model, const std::string &dir)
{
    const vpvl::MaterialList &materials = model->materials();
    const int nmaterials = materials.count();
    GLuint textureID = 0;
    PMDModelMaterialPrivate *materialPrivates = new PMDModelMaterialPrivate[nmaterials];
    bool hasSingleSphere = false, hasMultipleSphere = false;
    for (int i = 0; i < nmaterials; i++) {
        const vpvl::Material *material = materials[i];
        const std::string primary = m_delegate->toUnicode(material->mainTextureName());
        const std::string second = m_delegate->toUnicode(material->subTextureName());
        PMDModelMaterialPrivate &materialPrivate = materialPrivates[i];
        materialPrivate.mainTextureID = 0;
        materialPrivate.subTextureID = 0;
        if (!primary.empty()) {
            if (m_delegate->loadTexture(dir + "/" + primary, textureID)) {
                materialPrivate.mainTextureID = textureID;
                m_delegate->log(IDelegate::kLogInfo, "Binding the texture as a primary texture (ID=%d)", textureID);
            }
        }
        if (!second.empty()) {
            if (m_delegate->loadTexture(dir + "/" + second, textureID)) {
                materialPrivate.subTextureID = textureID;
                m_delegate->log(IDelegate::kLogInfo, "Binding the texture as a secondary texture (ID=%d)", textureID);
            }
        }
        hasSingleSphere |= material->isMainSphereModulate() && !material->isSubSphereAdd();
        hasMultipleSphere |= material->isSubSphereAdd();
    }
    userData->hasSingleSphereMap = hasSingleSphere;
    userData->hasMultipleSphereMap = hasMultipleSphere;
    m_delegate->log(IDelegate::kLogInfo,
                    "Sphere map information: hasSingleSphere=%s, hasMultipleSphere=%s",
                    hasSingleSphere ? "true" : "false",
                    hasMultipleSphere ? "true" : "false");
    glGenBuffers(kVertexBufferObjectMax, userData->vertexBufferObjects);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, userData->vertexBufferObjects[kEdgeIndices]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->edgeIndicesCount() * model->strideSize(vpvl::PMDModel::kEdgeIndicesStride),
                 model->edgeIndicesPointer(), GL_STATIC_DRAW);
    m_delegate->log(IDelegate::kLogInfo,
                    "Binding edge indices to the vertex buffer object (ID=%d)",
                    userData->vertexBufferObjects[kEdgeIndices]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, userData->vertexBufferObjects[kShadowIndices]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->indices().count() * model->strideSize(vpvl::PMDModel::kIndicesStride),
                 model->indicesPointer(), GL_STATIC_DRAW);
    m_delegate->log(IDelegate::kLogInfo,
                    "Binding indices to the vertex buffer object (ID=%d)",
                    userData->vertexBufferObjects[kShadowIndices]);
    glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelVertices]);
    glBufferData(GL_ARRAY_BUFFER, model->vertices().count() * model->strideSize(vpvl::PMDModel::kVerticesStride),
                 model->verticesPointer(), GL_DYNAMIC_DRAW);
    m_delegate->log(IDelegate::kLogInfo,
                    "Binding texture coordinates to the vertex buffer object (ID=%d)",
                    userData->vertexBufferObjects[kModelTexCoords]);
    if (m_delegate->loadToonTexture("toon0.bmp", dir, textureID)) {
        userData->toonTextureID[0] = textureID;
        glTexParameteri(textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        m_delegate->log(IDelegate::kLogInfo, "Binding the texture as a toon texture (ID=%d)", textureID);
    }
    for (int i = 0; i < vpvl::PMDModel::kSystemTextureMax - 1; i++) {
        const uint8_t *name = model->toonTexture(i);
        if (m_delegate->loadToonTexture(reinterpret_cast<const char *>(name), dir, textureID)) {
            userData->toonTextureID[i + 1] = textureID;
            glTexParameteri(textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            m_delegate->log(IDelegate::kLogInfo, "Binding the texture as a toon texture (ID=%d)", textureID);
        }
    }
    userData->materials = materialPrivates;
    model->setUserData(userData);
    m_delegate->log(IDelegate::kLogInfo, "Created the model: %s", m_delegate->toUnicode(model->name()).c_str());
    m_scene->addModel(model);
}

void Renderer::unloadModel(const vpvl::PMDModel *model)
{
    unloadModel0(static_cast<vpvl::gl2::PMDModelUserData *>(model->userData()), model);
}

void Renderer::unloadModel0(vpvl::gl2::PMDModelUserData *userData, const vpvl::PMDModel *model)
{
    if (model) {
        const vpvl::MaterialList &materials = model->materials();
        const int nmaterials = materials.count();
        for (int i = 0; i < nmaterials; i++) {
            PMDModelMaterialPrivate &materialPrivate = userData->materials[i];
            glDeleteTextures(1, &materialPrivate.mainTextureID);
            glDeleteTextures(1, &materialPrivate.subTextureID);
        }
        for (int i = 0; i < vpvl::PMDModel::kSystemTextureMax; i++) {
            glDeleteTextures(1, &userData->toonTextureID[i]);
        }
        glDeleteBuffers(kVertexBufferObjectMax, userData->vertexBufferObjects);
        delete[] userData->materials;
        delete userData;
        m_delegate->log(IDelegate::kLogInfo, "Destroyed the model: %s", m_delegate->toUnicode(model->name()).c_str());
        m_scene->removeModel(const_cast<vpvl::PMDModel *>(model));
    }
}

void Renderer::updateModelBuffer(const vpvl::PMDModel *model) const
{
    vpvl::gl2::PMDModelUserData *userData = static_cast<vpvl::gl2::PMDModelUserData *>(model->userData());
    size_t size = model->vertices().count() * model->strideSize(vpvl::PMDModel::kVerticesStride);
    glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelVertices]);
    glBufferData(GL_ARRAY_BUFFER, size, model->verticesPointer(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Renderer::drawModel(const vpvl::PMDModel *model)
{
    const vpvl::gl2::PMDModelUserData *userData = static_cast<vpvl::gl2::PMDModelUserData *>(model->userData());
    size_t stride = model->strideSize(vpvl::PMDModel::kVerticesStride), vsize = model->vertices().count();

    m_modelProgram->bind();
    glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelVertices]);
    m_modelProgram->setPosition(reinterpret_cast<const GLvoid *>(model->strideOffset(vpvl::PMDModel::kVerticesStride)), stride);
    m_modelProgram->setNormal(reinterpret_cast<const GLvoid *>(model->strideOffset(vpvl::PMDModel::kNormalsStride)), stride);
    m_modelProgram->setTexCoord(reinterpret_cast<const GLvoid *>(model->strideOffset(vpvl::PMDModel::kTextureCoordsStride)), stride);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, userData->vertexBufferObjects[kShadowIndices]);

    float matrix4x4[16], matrix3x3[9];
    m_scene->getModelViewMatrix(matrix4x4);
    m_modelProgram->setModelViewMatrix(matrix4x4);
    m_scene->getProjectionMatrix(matrix4x4);
    m_modelProgram->setProjectionMatrix(matrix4x4);
    m_scene->getNormalMatrix(matrix3x3);
    m_modelProgram->setNormalMatrix(matrix3x3);
    m_modelProgram->setLightAmbient(m_scene->lightAmbient());
    m_modelProgram->setLightColor(m_scene->lightColor());
    m_modelProgram->setLightDiffuse(m_scene->lightDiffuse());
    m_modelProgram->setLightPosition(m_scene->lightPosition());
    m_modelProgram->setLightSpecular(m_scene->lightSpecular());
    m_modelProgram->setLightIntensity(m_scene->lightIntensity());
    if (m_depthTextureID && m_frameBufferID) {
        ExtendedModelProgram *modelProgram = static_cast<ExtendedModelProgram *>(m_modelProgram);
        modelProgram->setShadowTexture(m_depthTextureID);
    }

    const bool enableToon = model->isToonEnabled();
    // toon
    if (enableToon) {
        // shadow map
        stride = model->strideSize(vpvl::PMDModel::kToonTextureStride);
        glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelToonTexCoords]);
        glBufferData(GL_ARRAY_BUFFER, vsize * stride, model->toonTextureCoordsPointer(), GL_DYNAMIC_DRAW);
        m_modelProgram->setToonTexCoord(0, stride);
    }

    const vpvl::MaterialList &materials = model->materials();
    const PMDModelMaterialPrivate *materialPrivates = userData->materials;
    const int nmaterials = materials.count();
    Color average, ambient, diffuse, specular;
    size_t offset = 0;

    for (int i = 0; i < nmaterials; i++) {
        const vpvl::Material *material = materials[i];
        const PMDModelMaterialPrivate &materialPrivate = materialPrivates[i];
        // toon
        const float alpha = material->opacity();
        ambient = material->ambient();
        ambient.setW(ambient.w() * alpha);
        diffuse = material->diffuse();
        diffuse.setW(diffuse.w() * alpha);
        specular = material->specular();
        specular.setW(specular.w() * alpha);
        m_modelProgram->setMaterialAmbient(ambient);
        m_modelProgram->setMaterialDiffuse(diffuse);
        m_modelProgram->setMaterialSpecular(specular);
        m_modelProgram->setMaterialShininess(material->shiness());
        m_modelProgram->setMainTexture(materialPrivate.mainTextureID);
        m_modelProgram->setToonTexture(userData->toonTextureID[material->toonID()]);
        m_modelProgram->setSubTexture(materialPrivate.subTextureID);
        m_modelProgram->setIsMainSphereMap(material->isMainSphereAdd() || material->isMainSphereModulate());
        m_modelProgram->setIsMainAdditive(material->isMainSphereAdd());
        m_modelProgram->setIsSubSphereMap(material->isSubSphereAdd() || material->isSubSphereModulate());
        m_modelProgram->setIsSubAdditive(material->isSubSphereAdd());
        material->opacity() < 1.0f ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);

        // draw
        const int nindices = material->countIndices();
        glDrawElements(GL_TRIANGLES, nindices, GL_UNSIGNED_SHORT, reinterpret_cast<const GLvoid *>(offset));
        offset += (nindices << 1);
        m_modelProgram->resetTextureState();
    }

    m_modelProgram->unbind();
    glEnable(GL_CULL_FACE);
}

void Renderer::drawModelEdge(const vpvl::PMDModel *model)
{
    if (model->edgeOffset() == 0.0f)
        return;
    const size_t stride = model->strideSize(vpvl::PMDModel::kEdgeVerticesStride);
    const vpvl::gl2::PMDModelUserData *userData = static_cast<vpvl::gl2::PMDModelUserData *>(model->userData());
    float modelViewMatrix[16], projectionMatrix[16];
    m_scene->getModelViewMatrix(modelViewMatrix);
    m_scene->getProjectionMatrix(projectionMatrix);
    m_edgeProgram->bind();
    size_t len = model->vertices().count() * stride;
    glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kEdgeVertices]);
    glBufferData(GL_ARRAY_BUFFER, len, model->edgeVerticesPointer(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, userData->vertexBufferObjects[kEdgeIndices]);
    m_edgeProgram->setColor(model->edgeColor());
    m_edgeProgram->setModelViewMatrix(modelViewMatrix);
    m_edgeProgram->setProjectionMatrix(projectionMatrix);
    m_edgeProgram->setPosition(0, stride);
    glCullFace(GL_FRONT);
    glDrawElements(GL_TRIANGLES, model->edgeIndicesCount(), GL_UNSIGNED_SHORT, 0);
    glCullFace(GL_BACK);
    m_edgeProgram->unbind();
}

void Renderer::drawModelShadow(const vpvl::PMDModel *model)
{
    const size_t stride = model->strideSize(vpvl::PMDModel::kVerticesStride);
    const vpvl::gl2::PMDModelUserData *userData = static_cast<vpvl::gl2::PMDModelUserData *>(model->userData());
    float modelViewMatrix[16], projectionMatrix[16];
    m_scene->getModelViewMatrix(modelViewMatrix);
    m_scene->getProjectionMatrix(projectionMatrix);
    glCullFace(GL_FRONT);
    size_t len = model->vertices().count() * stride;
    glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelVertices]);
    glBufferData(GL_ARRAY_BUFFER, len, model->verticesPointer(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, userData->vertexBufferObjects[kShadowIndices]);
    m_shadowProgram->bind();
    m_shadowProgram->setModelViewMatrix(modelViewMatrix);
    m_shadowProgram->setProjectionMatrix(projectionMatrix);
    m_shadowProgram->setPosition(0, stride);
    glDrawElements(GL_TRIANGLES, model->indices().count(), GL_UNSIGNED_SHORT, 0);
    m_shadowProgram->unbind();
}

void Renderer::loadAsset(Asset *asset, const std::string &dir)
{
#ifdef VPVL_LINK_ASSIMP
    const aiScene *scene = asset->getScene();
    const unsigned int nmaterials = scene->mNumMaterials;
    AssetUserData *userData = new AssetUserData();
    aiString texturePath;
    std::string path, canonicalized, filename;
    asset->setUserData(userData);
    for (unsigned int i = 0; i < nmaterials; i++) {
        aiMaterial *material = scene->mMaterials[i];
        aiReturn found = AI_SUCCESS;
        GLuint textureID;
        int textureIndex = 0;
        while (found == AI_SUCCESS) {
            found = material->GetTexture(aiTextureType_DIFFUSE, textureIndex, &texturePath);
            path = texturePath.data;
            if (userData->textures[path] == 0) {
                canonicalized = m_delegate->toUnicode(reinterpret_cast<const uint8_t *>(CanonicalizePath(path).c_str()));
                filename = dir + "/" + canonicalized;
                if (m_delegate->loadTexture(filename, textureID)) {
                    userData->textures[path] = textureID;
                    m_delegate->log(IDelegate::kLogInfo, "Loaded a texture: %s (ID=%d)", canonicalized.c_str(), textureID);
                }
            }
            textureIndex++;
        }
    }
    aiLoadAssetRecursive(scene, scene->mRootNode, userData, m_delegate);
    m_assets.add(asset);
#else
    (void) asset;
    (void) dir;
#endif
}

void Renderer::unloadAsset(Asset *&asset)
{
#ifdef VPVL_LINK_ASSIMP
    if (asset) {
        const aiScene *scene = asset->getScene();
        const unsigned int nmaterials = scene->mNumMaterials;
        AssetUserData *userData = asset->userData();
        aiString texturePath;
        for (unsigned int i = 0; i < nmaterials; i++) {
            aiMaterial *material = scene->mMaterials[i];
            aiReturn found = AI_SUCCESS;
            GLuint textureID;
            int textureIndex = 0;
            while (found == AI_SUCCESS) {
                found = material->GetTexture(aiTextureType_DIFFUSE, textureIndex, &texturePath);
                textureID = userData->textures[texturePath.data];
                glDeleteTextures(1, &textureID);
                userData->textures.erase(texturePath.data);
                textureIndex++;
            }
        }
        aiUnloadAssetRecursive(scene, scene->mRootNode, userData);
        delete userData;
        delete asset;
        m_assets.remove(asset);
        asset = 0;
    }
#else
    (void) asset;
#endif
}


void Renderer::clearSurface()
{
    glViewport(0, 0, m_scene->width(), m_scene->height());
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Renderer::drawShadow()
{
    size_t size = 0;
    vpvl::PMDModel **models = m_scene->getRenderingOrder(size);
    if (m_depthTextureID && m_frameBufferID) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferID);
        glViewport(0, 0, 1024, 1024);
        glClear(GL_DEPTH_BUFFER_BIT);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glCullFace(GL_FRONT);
        for (size_t i = 0; i < size; i++) {
            vpvl::PMDModel *model = models[i];
            if (model->isVisible()) {
                updateModelBuffer(model);
                drawModelShadow(model);
            }
        }
        glCullFace(GL_BACK);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    else {
        for (size_t i = 0; i < size; i++) {
            vpvl::PMDModel *model = models[i];
            if (model->isVisible())
                updateModelBuffer(model);
        }
    }
}

void Renderer::drawAssets()
{
    const int nassets = m_assets.count();
    for (int i = 0; i < nassets; i++) {
        aiDrawAsset(m_assets[i], m_scene);
    }
}

void Renderer::drawModels()
{
    size_t size = 0;
    vpvl::PMDModel **models = m_scene->getRenderingOrder(size);
    for (size_t i = 0; i < size; i++) {
        vpvl::PMDModel *model = models[i];
        if (model->isVisible()) {
            drawModel(model);
            drawModelEdge(model);
        }
    }
}

void Renderer::drawSurface()
{
    drawShadow();
    clearSurface();
    drawModels();
    drawAssets();
}

}
}
