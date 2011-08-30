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

#include <vpvl/vpvl.h>
#include <vpvl/gl2/Renderer.h>

#include <btBulletDynamicsCommon.h>

namespace {

enum VertexBufferObjectType {
    kModelVertices,
    kModelNormals,
    kModelColors,
    kModelTexCoords,
    kModelToonTexCoords,
    kEdgeVertices,
    kEdgeIndices,
    kShadowIndices,
    kVertexBufferObjectMax
};

struct PMDModelMaterialPrivate {
    GLuint primaryTextureID;
    GLuint secondTextureID;
};

}

namespace vpvl
{

struct PMDModelUserData {
    GLuint toonTextureID[vpvl::PMDModel::kSystemTextureMax];
    GLuint vertexBufferObjects[kVertexBufferObjectMax];
    bool hasSingleSphereMap;
    bool hasMultipleSphereMap;
    PMDModelMaterialPrivate *materials;
};

struct XModelUserData {
    GLuint listID;
    vpvl::Hash<vpvl::HashString, GLuint> textures;
};

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
          m_normalAttributeLocation(0)
    {
    }
    ~ShaderProgram() {
        if (m_program) {
            glDeleteProgram(m_program);
            m_program = 0;
        }
        m_modelViewUniformLocation = 0;
        m_projectionUniformLocation = 0;
        m_positionAttributeLocation = 0;
        m_normalAttributeLocation = 0;
    }

    bool load(const char *vertexShaderSource, const char *fragmentShaderSource) {
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
                    char *message = new char[len];
                    glGetProgramInfoLog(program, len, NULL, message);
                    m_delegate->log(IDelegate::kLogWarning, "%s", message);
                    delete[] message;
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
    void bind() {
        glUseProgram(m_program);
    }
    void unbind() {
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
        glVertexAttribPointer(m_positionAttributeLocation, 3, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setNormal(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_normalAttributeLocation);
        glVertexAttribPointer(m_normalAttributeLocation, 3, GL_FLOAT, GL_FALSE, stride, ptr);
    }

protected:
    GLuint m_program;

private:
    GLuint compileShader(const char *source, GLenum type) const {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, NULL);
        glCompileShader(shader);
        GLint compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint len;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
            if (len > 0) {
                char *message = new char[len];
                glGetShaderInfoLog(shader, len, NULL, message);
                m_delegate->log(IDelegate::kLogWarning, "%s", message);
                delete[] message;
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

    bool load(const char *vertexShaderSource, const char *fragmentShaderSource) {
        bool ret = ShaderProgram::load(vertexShaderSource, fragmentShaderSource);
        if (ret)
            m_colorUniformLocation = glGetUniformLocation(m_program, "color");
        return ret;
    }
    void setColor(const btVector3 &value) {
        glUniform4fv(m_colorUniformLocation, 1, value);
    }

private:
    GLuint m_colorUniformLocation;
};

class ShadowProgram : public ShaderProgram {
public:
    ShadowProgram(IDelegate *delegate)
        : ShaderProgram(delegate),
          m_lightColorUniformLocation(0),
          m_lightPositionUniformLocation(0),
          m_lightAmbientUniformLocation(0),
          m_lightDiffuseUniformLocation(0),
          m_lightSpecularUniformLocation(0)
    {
    }
    ~ShadowProgram() {
        m_lightColorUniformLocation = 0;
        m_lightPositionUniformLocation = 0;
        m_lightAmbientUniformLocation = 0;
        m_lightDiffuseUniformLocation = 0;
        m_lightSpecularUniformLocation = 0;
    }

    bool load(const char *vertexShaderSource, const char *fragmentShaderSource) {
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
    void setLightColor(const btVector3 &value) {
        glUniform4fv(m_lightColorUniformLocation, 1, value);
    }
    void setLightPosition(const btVector3 &value) {
        glUniform3fv(m_lightPositionUniformLocation, 1, value);
    }
    void setLightAmbient(const btVector3 &value) {
        glUniform4fv(m_lightAmbientUniformLocation, 1, value);
    }
    void setLightDiffuse(const btVector3 &value) {
        glUniform4fv(m_lightDiffuseUniformLocation, 1, value);
    }
    void setLightSpecular(const btVector3 &value) {
        glUniform4fv(m_lightSpecularUniformLocation, 1, value);
    }

private:
    GLuint m_lightColorUniformLocation;
    GLuint m_lightPositionUniformLocation;
    GLuint m_lightAmbientUniformLocation;
    GLuint m_lightDiffuseUniformLocation;
    GLuint m_lightSpecularUniformLocation;
};

class ModelProgram : public ShadowProgram {
public:
    ModelProgram(IDelegate *delegate)
        : ShadowProgram(delegate),
          m_texCoordAttributeLocation(0),
          m_toonTexCoordAttributeLocation(0),
          m_normalMatrixUniformLocation(0),
          m_materialAmbientUniformLocation(0),
          m_materialDiffuseUniformLocation(0),
          m_materialSpecularUniformLocation(0),
          m_materialShininessUniformLocation(0),
          m_hasSingleSphereMapUniformLocation(0),
          m_hasMultipleSphereMapUniformLocation(0),
          m_hasMainTextureUniformLocation(0),
          m_hasSubTextureUniformLocation(0),
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
        m_hasSingleSphereMapUniformLocation = 0;
        m_hasMultipleSphereMapUniformLocation = 0;
        m_hasMainTextureUniformLocation = 0;
        m_hasSubTextureUniformLocation = 0;
        m_mainTextureUniformLocation = 0;
        m_subTextureUniformLocation = 0;
        m_toonTextureUniformLocation = 0;
    }

    bool load(const char *vertexShaderSource, const char *fragmentShaderSource) {
        bool ret = ShadowProgram::load(vertexShaderSource, fragmentShaderSource);
        if (ret) {
            m_texCoordAttributeLocation = glGetAttribLocation(m_program, "inTexCoord");
            m_toonTexCoordAttributeLocation = glGetAttribLocation(m_program, "inToonTexCoord");
            m_normalMatrixUniformLocation = glGetUniformLocation(m_program, "normalMatrix");
            m_materialAmbientUniformLocation = glGetUniformLocation(m_program, "materialAmbient");
            m_materialDiffuseUniformLocation = glGetUniformLocation(m_program, "materialDiffuse");
            m_materialSpecularUniformLocation = glGetUniformLocation(m_program, "materialSpecular");
            m_materialShininessUniformLocation = glGetUniformLocation(m_program, "materialShininess");
            m_hasSingleSphereMapUniformLocation = glGetUniformLocation(m_program, "hasSingleSphereMap");
            m_hasMultipleSphereMapUniformLocation = glGetUniformLocation(m_program, "hasMultipleSphereMap");
            m_hasMainTextureUniformLocation = glGetUniformLocation(m_program, "hasMainTexture");
            m_hasSubTextureUniformLocation = glGetUniformLocation(m_program, "hasSubTexture");
            m_isMainAdditiveUniformLocation = glGetUniformLocation(m_program, "isMainAdditive");
            m_isSubAdditiveUniformLocation = glGetUniformLocation(m_program, "isSubAdditive");
            m_mainTextureUniformLocation = glGetUniformLocation(m_program, "mainTexture");
            m_subTextureUniformLocation = glGetUniformLocation(m_program, "subTexture");
            m_toonTextureUniformLocation = glGetUniformLocation(m_program, "toonTexture");
        }
        return ret;
    }
    void bind() {
        ShadowProgram::bind();
        resetTextureState();
    }
    void resetTextureState() {
        glUniform1i(m_hasMainTextureUniformLocation, 0);
        glUniform1i(m_hasSubTextureUniformLocation, 0);
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
    void setNormalMatrix(const float value[16]) {
        glUniformMatrix4fv(m_normalMatrixUniformLocation, 1, GL_TRUE, value);
    }
    void setMaterialAmbient(const btVector3 &value) {
        glUniform4fv(m_materialAmbientUniformLocation, 1, value);
    }
    void setMaterialDiffuse(const btVector3 &value) {
        glUniform4fv(m_materialDiffuseUniformLocation, 1, value);
    }
    void setMaterialSpecular(const btVector3 &value) {
        glUniform4fv(m_materialSpecularUniformLocation, 1, value);
    }
    void setMaterialShininess(float value) {
        glUniform1f(m_materialShininessUniformLocation, value);
    }
    void setHasSingleSphereMap(bool value) {
        glUniform1i(m_hasSingleSphereMapUniformLocation, value ? 1 : 0);
    }
    void setHasMultipleSphereMap(bool value) {
        glUniform1i(m_hasMultipleSphereMapUniformLocation, value ? 1 : 0);
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
    GLuint m_hasSingleSphereMapUniformLocation;
    GLuint m_hasMultipleSphereMapUniformLocation;
    GLuint m_hasMainTextureUniformLocation;
    GLuint m_hasSubTextureUniformLocation;
    GLuint m_isMainAdditiveUniformLocation;
    GLuint m_isSubAdditiveUniformLocation;
    GLuint m_mainTextureUniformLocation;
    GLuint m_subTextureUniformLocation;
    GLuint m_toonTextureUniformLocation;
};

class DebugDrawer : public btIDebugDraw
{
public:
    DebugDrawer() : m_world(0) {}
    virtual ~DebugDrawer() {}

    void draw3dText(const btVector3 & /* location */, const char *textString) {
        fprintf(stderr, "[INFO]: %s\n", textString);
    }
    void drawContactPoint(const btVector3 &PointOnB, const btVector3 &normalOnB, btScalar distance, int /* lifeTime */, const btVector3 &color) {
        const btVector3 to = PointOnB + normalOnB * distance;
        glBegin(GL_LINES);
        glColor3fv(color);
        glVertex3fv(PointOnB);
        glVertex3fv(to);
        glEnd();
    }
    void drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color) {
        glBegin(GL_LINES);
        glColor3fv(color);
        glVertex3fv(from);
        glVertex3fv(to);
        glEnd();
    }
    void drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &fromColor, const btVector3 &toColor) {
        glBegin(GL_LINES);
        glColor3fv(fromColor);
        glVertex3fv(from);
        glColor3fv(toColor);
        glVertex3fv(to);
        glEnd();
    }
    void reportErrorWarning(const char *warningString) {
        fprintf(stderr, "[ERROR]: %s\n", warningString);
    }
    int getDebugMode() const {
        return m_flags;
    }
    void setDebugMode(int debugMode) {
        m_flags = debugMode;
    }

    void render() {
        if (m_world)
            m_world->debugDrawWorld();
    }
    void setWorld(btDynamicsWorld *value) {
        m_world = value;
    }

private:
    btDynamicsWorld *m_world;
    int m_flags;
};

bool Renderer::initializeGLEW(GLenum &err)
{
    err = glewInit();
    return err == GLEW_OK;
}

Renderer::Renderer(IDelegate *delegate, int width, int height, int fps)
    : m_delegate(delegate),
      m_edgeProgram(0),
      m_modelProgram(0),
      m_shadowProgram(0),
      m_scene(0),
      m_selected(0),
      m_debugDrawer(0)
{
    m_edgeProgram = new EdgeProgram(delegate);
    m_modelProgram = new ModelProgram(delegate);
    m_shadowProgram = new ShadowProgram(delegate);
    m_debugDrawer = new DebugDrawer();
    m_scene = new vpvl::Scene(width, height, fps);
}

Renderer::~Renderer()
{
    delete m_edgeProgram;
    m_edgeProgram = 0;
    delete m_modelProgram;
    m_modelProgram = 0;
    delete m_shadowProgram;
    m_shadowProgram = 0;
    delete m_debugDrawer;
    m_debugDrawer = 0;
    delete m_scene;
    m_scene = 0;
}

void Renderer::initializeSurface()
{
    glClearStencil(0);
    glEnable(GL_MULTISAMPLE);
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
    ret = m_edgeProgram->load(vertexShader.c_str(), fragmentShader.c_str());
    if (!ret)
        return ret;
    vertexShader = m_delegate->loadShader(IDelegate::kModelVertexShader);
    fragmentShader = m_delegate->loadShader(IDelegate::kModelFragmentShader);
    ret = m_modelProgram->load(vertexShader.c_str(), fragmentShader.c_str());
    if (!ret)
        return ret;
    vertexShader = m_delegate->loadShader(IDelegate::kShadowVertexShader);
    fragmentShader = m_delegate->loadShader(IDelegate::kShadowFragmentShader);
    ret = m_shadowProgram->load(vertexShader.c_str(), fragmentShader.c_str());
    return ret;
}

void Renderer::resize(int width, int height)
{
    m_scene->setWidth(width);
    m_scene->setHeight(height);
}

void Renderer::getObjectCoordinate(int px, int py, btVector3 &coordinate) const
{
    double modelViewMatrixd[16], projectionMatrixd[16], winX = 0, winY = 0, x = 0, y = 0, z = 0;
    float modelViewMatrixf[16], projectionMatrixf[16], winZ = 0;
    int view[4];
    m_scene->getModelViewMatrix(modelViewMatrixf);
    m_scene->getProjectionMatrix(projectionMatrixf);
    for (int i = 0; i < 16; i++) {
        modelViewMatrixd[i] = modelViewMatrixf[i];
        projectionMatrixd[i] = projectionMatrixf[i];
    }
    glGetIntegerv(GL_VIEWPORT, view);
    winX = px;
    winY = view[3] - py;
    glReadPixels(static_cast<GLint>(winX), static_cast<GLint>(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
    gluUnProject(winX, winY, winZ, modelViewMatrixd, projectionMatrixd, view, &x, &y, &z);
    coordinate.setValue(static_cast<btScalar>(x), static_cast<btScalar>(y), static_cast<btScalar>(z));
}

void Renderer::setDebugDrawer(btDynamicsWorld *world)
{
    static_cast<DebugDrawer *>(m_debugDrawer)->setWorld(world);
    world->setDebugDrawer(m_debugDrawer);
}

void Renderer::loadModel(vpvl::PMDModel *model, const std::string &dir)
{
    const vpvl::MaterialList &materials = model->materials();
    const uint32_t nMaterials = materials.count();
    GLuint textureID = 0;
    vpvl::PMDModelUserData *userData = new vpvl::PMDModelUserData;
    PMDModelMaterialPrivate *materialPrivates = new PMDModelMaterialPrivate[nMaterials];
    bool hasSingleSphere = false, hasMultipleSphere = false;
    for (uint32_t i = 0; i < nMaterials; i++) {
        const vpvl::Material *material = materials[i];
        const std::string primary = m_delegate->toUnicode(material->mainTextureName());
        const std::string second = m_delegate->toUnicode(material->subTextureName());
        PMDModelMaterialPrivate &materialPrivate = materialPrivates[i];
        materialPrivate.primaryTextureID = 0;
        materialPrivate.secondTextureID = 0;
        if (!primary.empty()) {
            if (m_delegate->loadTexture(dir + "/" + primary, textureID)) {
                materialPrivate.primaryTextureID = textureID;
                m_delegate->log(IDelegate::kLogInfo, "Binding the texture as a primary texture (ID=%d)", textureID);
            }
        }
        if (!second.empty()) {
            if (m_delegate->loadTexture(dir + "/" + second, textureID)) {
                materialPrivate.secondTextureID = textureID;
                m_delegate->log(IDelegate::kLogInfo, "Binding the texture as a secondary texture (ID=%d)", textureID);
            }
        }
        hasSingleSphere |= material->isMultiplicationSphereMain() && !material->isAdditionalSphereSub();
        hasMultipleSphere |= material->isAdditionalSphereSub();
    }
    userData->hasSingleSphereMap = hasSingleSphere;
    userData->hasMultipleSphereMap = hasMultipleSphere;
    m_delegate->log(IDelegate::kLogInfo,
                    "Sphere map information: hasSingleSphere=%s, hasMultipleSphere=%s",
                    hasSingleSphere ? "true" : "false",
                    hasMultipleSphere ? "true" : "false");
    glGenBuffers(kVertexBufferObjectMax, userData->vertexBufferObjects);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, userData->vertexBufferObjects[kEdgeIndices]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->edgeIndicesCount() * model->stride(vpvl::PMDModel::kEdgeIndicesStride),
                 model->edgeIndicesPointer(), GL_STATIC_DRAW);
    m_delegate->log(IDelegate::kLogInfo,
                    "Binding edge indices to the vertex buffer object (ID=%d)",
                    userData->vertexBufferObjects[kEdgeIndices]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, userData->vertexBufferObjects[kShadowIndices]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->indices().count() * model->stride(vpvl::PMDModel::kIndicesStride),
                 model->indicesPointer(), GL_STATIC_DRAW);
    m_delegate->log(IDelegate::kLogInfo,
                    "Binding indices to the vertex buffer object (ID=%d)",
                    userData->vertexBufferObjects[kShadowIndices]);
    glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelTexCoords]);
    glBufferData(GL_ARRAY_BUFFER, model->vertices().count() * model->stride(vpvl::PMDModel::kTextureCoordsStride),
                 model->textureCoordsPointer(), GL_STATIC_DRAW);
    m_delegate->log(IDelegate::kLogInfo,
                    "Binding texture coordinates to the vertex buffer object (ID=%d)",
                    userData->vertexBufferObjects[kModelTexCoords]);
    if (m_delegate->loadToonTexture("toon0.bmp", dir, textureID)) {
        userData->toonTextureID[0] = textureID;
        glTexParameteri(textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        m_delegate->log(IDelegate::kLogInfo, "Binding the texture as a toon texture (ID=%d)", textureID);
    }
    for (uint32_t i = 0; i < vpvl::PMDModel::kSystemTextureMax - 1; i++) {
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
}

void Renderer::unloadModel(const vpvl::PMDModel *model)
{
    if (model) {
        const vpvl::MaterialList &materials = model->materials();
        const uint32_t nMaterials = materials.count();
        vpvl::PMDModelUserData *userData = model->userData();
        for (uint32_t i = 0; i < nMaterials; i++) {
            PMDModelMaterialPrivate &materialPrivate = userData->materials[i];
            glDeleteTextures(1, &materialPrivate.primaryTextureID);
            glDeleteTextures(1, &materialPrivate.secondTextureID);
        }
        for (uint32_t i = 1; i < vpvl::PMDModel::kSystemTextureMax; i++) {
            glDeleteTextures(1, &userData->toonTextureID[i]);
        }
        glDeleteBuffers(kVertexBufferObjectMax, userData->vertexBufferObjects);
        delete[] userData->materials;
        delete userData;
        m_delegate->log(IDelegate::kLogInfo, "Destroyed the model: %s", m_delegate->toUnicode(model->name()).c_str());
    }
}

void Renderer::drawModel(const vpvl::PMDModel *model)
{
#ifndef VPVL_COORDINATE_OPENGL
    glPushMatrix();
    glScalef(1.0f, 1.0f, -1.0f);
    glCullFace(GL_FRONT);
#endif

    const vpvl::PMDModelUserData *userData = model->userData();
    size_t stride = model->stride(vpvl::PMDModel::kVerticesStride), vsize = model->vertices().count();
    m_modelProgram->bind();
    glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelVertices]);
    m_modelProgram->setPosition(0, stride);
    stride = model->stride(vpvl::PMDModel::kNormalsStride);
    glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelNormals]);
    glBufferData(GL_ARRAY_BUFFER, vsize * stride, model->normalsPointer(), GL_DYNAMIC_DRAW);
    m_modelProgram->setNormal(0, stride);
    stride = model->stride(vpvl::PMDModel::kTextureCoordsStride);
    glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelTexCoords]);
    m_modelProgram->setTexCoord(0, stride);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, userData->vertexBufferObjects[kShadowIndices]);

    float matrix[16];
    m_scene->getModelViewMatrix(matrix);
    m_modelProgram->setModelViewMatrix(matrix);
    m_scene->getProjectionMatrix(matrix);
    m_modelProgram->setProjectionMatrix(matrix);
    m_scene->getInvertedModelViewMatrix(matrix);
    m_modelProgram->setNormalMatrix(matrix);
    m_modelProgram->setLightAmbient(m_scene->lightAmbient());
    m_modelProgram->setLightColor(m_scene->lightColor());
    m_modelProgram->setLightDiffuse(m_scene->lightDiffuse());
    m_modelProgram->setLightPosition(m_scene->lightPosition());
    m_modelProgram->setLightSpecular(m_scene->lightSpecular());

    const bool enableToon = true;
    // toon
    if (enableToon) {
        // shadow map
        stride = model->stride(vpvl::PMDModel::kToonTextureStride);
        glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelToonTexCoords]);
        glBufferData(GL_ARRAY_BUFFER, vsize * stride, model->toonTextureCoordsPointer(), GL_DYNAMIC_DRAW);
        m_modelProgram->setToonTexCoord(0, stride);
    }

    const vpvl::MaterialList &materials = model->materials();
    const PMDModelMaterialPrivate *materialPrivates = userData->materials;
    const uint32_t nMaterials = materials.count();
    btVector3 average, ambient, diffuse, specular;
    uint32_t offset = 0;
    m_modelProgram->setHasSingleSphereMap(userData->hasSingleSphereMap);
    m_modelProgram->setHasMultipleSphereMap(userData->hasMultipleSphereMap);

    for (uint32_t i = 0; i < nMaterials; i++) {
        const vpvl::Material *material = materials[i];
        const PMDModelMaterialPrivate &materialPrivate = materialPrivates[i];
        // toon
        const float alpha = material->opacity();
        if (enableToon) {
            average = material->averageColor();
            average.setW(average.w() * alpha);
            specular = material->specular();
            specular.setW(specular.w() * alpha);
            m_modelProgram->setMaterialAmbient(average);
            m_modelProgram->setMaterialDiffuse(average);
            m_modelProgram->setMaterialSpecular(specular);
        }
        else {
            ambient = material->ambient();
            ambient.setW(ambient.w() * alpha);
            diffuse = material->diffuse();
            diffuse.setW(diffuse.w() * alpha);
            specular = material->specular();
            specular.setW(specular.w() * alpha);
            m_modelProgram->setMaterialAmbient(ambient);
            m_modelProgram->setMaterialDiffuse(diffuse);
            m_modelProgram->setMaterialSpecular(specular);
        }
        m_modelProgram->setMaterialShininess(material->shiness());
        m_modelProgram->setMainTexture(materialPrivate.primaryTextureID);
        m_modelProgram->setToonTexture(userData->toonTextureID[material->toonID()]);
        m_modelProgram->setSubTexture(materialPrivate.secondTextureID);
        m_modelProgram->setIsMainAdditive(material->isAdditionalSphereMain());
        m_modelProgram->setIsSubAdditive(material->isAdditionalSphereSub());
        material->opacity() < 1.0f ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);

        // draw
        const uint32_t nIndices = material->countIndices();
        glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_SHORT, reinterpret_cast<const GLvoid *>(offset));
        offset += (nIndices << 1);
        m_modelProgram->resetTextureState();
    }

    m_modelProgram->unbind();
    glEnable(GL_CULL_FACE);

#ifndef VPVL_COORDINATE_OPENGL
    glPopMatrix();
    glCullFace(GL_BACK);
#endif
}

void Renderer::drawModelEdge(const vpvl::PMDModel *model)
{
#ifdef VPVL_COORDINATE_OPENGL
    glCullFace(GL_FRONT);
#else
    glPushMatrix();
    glScalef(1.0f, 1.0f, -1.0f);
    glCullFace(GL_BACK);
#endif

    const float alpha = 1.0f;
    const size_t stride = model->stride(vpvl::PMDModel::kEdgeVerticesStride);
    const vpvl::PMDModelUserData *modelPrivate = model->userData();
    btVector3 color(0, 0, 0);
    color.setW(1);

    float modelViewMatrix[16], projectionMatrix[16];
    m_scene->getModelViewMatrix(modelViewMatrix);
    m_scene->getProjectionMatrix(projectionMatrix);

    if (model == m_selected)
        color *= alpha;

    m_edgeProgram->bind();
    size_t len = model->vertices().count() * stride;
    glBindBuffer(GL_ARRAY_BUFFER, modelPrivate->vertexBufferObjects[kEdgeVertices]);
    glBufferData(GL_ARRAY_BUFFER, len, model->edgeVerticesPointer(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelPrivate->vertexBufferObjects[kEdgeIndices]);
    m_edgeProgram->setColor(color);
    m_edgeProgram->setModelViewMatrix(modelViewMatrix);
    m_edgeProgram->setProjectionMatrix(projectionMatrix);
    m_edgeProgram->setPosition(0, stride);
    glDrawElements(GL_TRIANGLES, model->edgeIndicesCount(), GL_UNSIGNED_SHORT, 0);
    m_edgeProgram->unbind();

#ifdef VPVL_COORDINATE_OPENGL
    glCullFace(GL_BACK);
#else
    glPopMatrix();
    glCullFace(GL_FRONT);
#endif
}

void Renderer::drawModelShadow(const vpvl::PMDModel *model)
{
    glBindBuffer(GL_ARRAY_BUFFER, model->userData()->vertexBufferObjects[kModelVertices]);
    glBufferData(GL_ARRAY_BUFFER, model->vertices().count() * model->stride(vpvl::PMDModel::kVerticesStride), model->verticesPointer(), GL_DYNAMIC_DRAW);
    return;

    const size_t stride = model->stride(vpvl::PMDModel::kVerticesStride);
    const vpvl::PMDModelUserData *modelPrivate = model->userData();

    float modelViewMatrix[16], projectionMatrix[16];
    m_scene->getModelViewMatrix(modelViewMatrix);
    m_scene->getProjectionMatrix(projectionMatrix);

    glDisable(GL_CULL_FACE);
    size_t len = model->vertices().count() * stride;
    glBindBuffer(GL_ARRAY_BUFFER, modelPrivate->vertexBufferObjects[kModelVertices]);
    glBufferData(GL_ARRAY_BUFFER, len, model->verticesPointer(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelPrivate->vertexBufferObjects[kShadowIndices]);
    m_shadowProgram->bind();
    m_shadowProgram->setLightAmbient(m_scene->lightAmbient());
    m_shadowProgram->setLightColor(m_scene->lightColor());
    m_shadowProgram->setLightDiffuse(m_scene->lightDiffuse());
    m_shadowProgram->setLightPosition(m_scene->lightPosition());
    m_shadowProgram->setLightSpecular(m_scene->lightSpecular());
    m_shadowProgram->setModelViewMatrix(modelViewMatrix);
    m_shadowProgram->setProjectionMatrix(projectionMatrix);
    m_shadowProgram->setPosition(0, stride);
    glDrawElements(GL_TRIANGLES, model->indices().count(), GL_UNSIGNED_SHORT, 0);
    m_shadowProgram->unbind();
    glEnable(GL_CULL_FACE);
}

void Renderer::drawModelBones(bool drawSpheres, bool drawLines)
{
    if (m_selected)
        drawModelBones(m_selected, drawSpheres, drawLines);
}

void Renderer::drawModelBones(const vpvl::PMDModel *model, bool drawSpheres, bool drawLines)
{
    const vpvl::BoneList &bones = model->bones();
    btVector3 color;
    uint32_t nBones = bones.count();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glPushMatrix();

    for (uint32_t i = 0; i < nBones; i++) {
        const vpvl::Bone *bone = bones[i], *parent = bone->parent();
        vpvl::Bone::Type type = bone->type();
        if (type == vpvl::Bone::kIKTarget && parent && parent->isSimulated())
            continue;
        const btTransform &transform = bone->localTransform();
        if (drawSpheres && type != vpvl::Bone::kInvisible) {
            float scale;
            if (bone->isSimulated()) {
                color.setValue(0.8f, 0.8f, 0.0f);
                scale = 0.1f;
            }
            else {
                switch (type) {
                case vpvl::Bone::kIKDestination:
                    color.setValue(0.7f, 0.2f, 0.2f);
                    scale = 0.25f;
                    break;
                case vpvl::Bone::kUnderIK:
                    color.setValue(0.8f, 0.5f, 0.0f);
                    scale = 0.15f;
                    break;
                case vpvl::Bone::kIKTarget:
                    color.setValue(1.0f, 0.0f, 0.0f);
                    scale = 0.15f;
                    break;
                case vpvl::Bone::kUnderRotate:
                case vpvl::Bone::kTwist:
                case vpvl::Bone::kFollowRotate:
                    color.setValue(0.0f, 0.8f, 0.2f);
                    scale = 0.15f;
                    break;
                default:
                    if (bone->hasMotionIndependency()) {
                        color.setValue(0.0f, 1.0f, 1.0f);
                        scale = 0.25f;
                    } else {
                        color.setValue(0.0f, 0.5f, 1.0f);
                        scale = 0.15f;
                    }
                    break;
                }
            }
            m_debugDrawer->drawSphere(transform.getOrigin(), scale, color);
        }
        if (!drawLines || !parent || type == vpvl::Bone::kIKDestination)
            continue;
        if (type == vpvl::Bone::kInvisible) {
            color.setValue(0.5f, 0.4f, 0.5f);
        }
        else if (bone->isSimulated()) {
            color.setValue(0.7f, 0.7f, 0.0f);
        }
        else if (type == vpvl::Bone::kUnderIK || type == vpvl::Bone::kIKTarget) {
            color.setValue(0.8f, 0.5f, 0.3f);
        }
        else {
            color.setValue(0.5f, 0.6f, 1.0f);
        }
        m_debugDrawer->drawLine(parent->localTransform().getOrigin(), transform.getOrigin(), color);
    }

    glPopMatrix();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void Renderer::drawBoneTransform(vpvl::Bone *bone)
{
    if (bone) {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glPushMatrix();
        btTransform t = bone->localTransform();
        btScalar orthoLen = 1.0f;
        if (bone->hasParent()) {
            btTransform pt = bone->parent()->localTransform();
            orthoLen = btMin(orthoLen, pt.getOrigin().distance(t.getOrigin()));
        }
        m_debugDrawer->drawTransform(t, orthoLen);
        glPopMatrix();
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
    }
}

static void DrawAsset(const vpvl::XModel *model, const btVector4 &indices, int index)
{
    const Array<btVector3> &vertices = model->vertices();
    const Array<btVector3> &textureCoords = model->textureCoords();
    const Array<btVector3> &normals = model->normals();
    const Array<btVector4> &colors = model->colors();
    const btTransform transform = model->transform();
    const int x = static_cast<const int>(indices[index]);
    if (textureCoords.count() > x)
        glTexCoord2fv(textureCoords[x]);
    else if (textureCoords.count() > 0)
        glTexCoord2f(0, 0);
    if (colors.count() > x)
        glColor4fv(colors[x]);
    else if (colors.count() > 0)
        glColor3f(0, 0, 0);
    if (normals.count() > x)
        glNormal3fv(transform.getBasis() * normals[x]);
    else if (normals.count() > 0)
        glNormal3f(0, 0, 0);
    glVertex3fv(transform * vertices[x] * model->scale());
}

void Renderer::loadAsset(vpvl::XModel *model, const std::string &dir)
{
    vpvl::XModelUserData *userData = new vpvl::XModelUserData;
    userData->listID = glGenLists(1);
    glNewList(userData->listID, GL_COMPILE);
    m_delegate->log(IDelegate::kLogInfo, "Generated a OpenGL list (ID=%d)", userData->listID);
#ifndef VPVL_COORDINATE_OPENGL
    glPushMatrix();
    glScalef(1.0f, 1.0f, -1.0f);
    glCullFace(GL_FRONT);
#endif
    const Array<vpvl::XModelFaceIndex> &faces = model->faces();
    const bool hasMaterials = model->countMatreials() > 0;
    uint32_t nFaces = faces.count();
    uint32_t prevIndex = -1;
    glEnable(GL_TEXTURE_2D);
    for (uint32_t i = 0; i < nFaces; i++) {
        const vpvl::XModelFaceIndex &face = faces[i];
        const btVector4 &value = face.value;
        const uint32_t count = face.count;
        const uint32_t currentIndex = face.index;
        if (hasMaterials && prevIndex != currentIndex) {
            const vpvl::XMaterial *material = model->materialAt(currentIndex);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, static_cast<const GLfloat *>(material->color()));
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, static_cast<const GLfloat *>(material->emmisive()));
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, static_cast<const GLfloat *>(material->specular()));
            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material->power());
            const std::string textureName = m_delegate->toUnicode(reinterpret_cast<const uint8_t *>(material->textureName()));
            if (!textureName.empty()) {
                HashString key(material->textureName());
                GLuint *textureID = userData->textures[key];
                if (!textureID) {
                    GLuint value;
                    if (m_delegate->loadTexture(dir + "/" + textureName, value)) {
                        userData->textures.insert(key, value);
                        glBindTexture(GL_TEXTURE_2D, value);
                        m_delegate->log(IDelegate::kLogInfo, "Binding the texture as a texture (ID=%d)", value);
                    }
                }
                else {
                    glBindTexture(GL_TEXTURE_2D, *textureID);
                }
            }
            else {
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            prevIndex = currentIndex;
        }
        glBegin(GL_TRIANGLES);
        switch (count) {
        case 3:
            DrawAsset(model, value, 1);
            DrawAsset(model, value, 0);
            DrawAsset(model, value, 2);
            break;
        case 4:
            DrawAsset(model, value, 1);
            DrawAsset(model, value, 0);
            DrawAsset(model, value, 2);
            DrawAsset(model, value, 3);
            DrawAsset(model, value, 2);
            DrawAsset(model, value, 0);
            break;
        default:
            throw new std::bad_exception();
        }
        glEnd();
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
#ifndef VPVL_COORDINATE_OPENGL
    glPopMatrix();
    glCullFace(GL_BACK);
#endif
    glEndList();
    model->setUserData(userData);
    m_assets.add(model);
}

void Renderer::unloadAsset(const vpvl::XModel *model)
{
    if (model) {
        m_assets.remove(const_cast<vpvl::XModel *>(model));
        vpvl::XModelUserData *userData = model->userData();
        glDeleteLists(userData->listID, 1);
        vpvl::Hash<vpvl::HashString, GLuint> &textures = userData->textures;
        uint32_t nTextures = textures.count();
        for (uint32_t i = 0; i < nTextures; i++)
            glDeleteTextures(1, textures.value(i));
        delete userData;
    }
}

void Renderer::drawAsset(const vpvl::XModel *model)
{
    if (model)
        glCallList(model->userData()->listID);
}

void Renderer::drawSurface()
{
    glViewport(0, 0, m_scene->width(), m_scene->height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    // initialize rendering states
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 1, ~0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
    glColorMask(0, 0, 0, 0);
    glDepthMask(0);
    glDisable(GL_DEPTH_TEST);
    // render shadow before drawing models
    size_t size = 0;
    vpvl::PMDModel **models = m_scene->getRenderingOrder(size);
    for (size_t i = 0; i < size; i++) {
        vpvl::PMDModel *model = models[i];
        drawModelShadow(model);
    }
    glColorMask(1, 1, 1, 1);
    glDepthMask(1);
    glStencilFunc(GL_EQUAL, 2, ~0);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_DEPTH_TEST);
    // render all assets
    // TODO: merge drawing models
    uint32_t nAssets = m_assets.count();
    for (uint32_t i = 0; i < nAssets; i++) {
        drawAsset(m_assets[i]);
    }
    // render model and edge
    for (size_t i = 0; i < size; i++) {
        vpvl::PMDModel *model = models[i];
        drawModel(model);
        drawModelEdge(model);
    }
}

}
}
