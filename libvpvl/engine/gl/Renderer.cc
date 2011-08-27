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
#include <vpvl/gl/Renderer.h>

#include <btBulletDynamicsCommon.h>

namespace vpvl
{

enum __vpvlVertexBufferObjectType {
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

struct __vpvlPMDModelMaterialPrivate {
    GLuint primaryTextureID;
    GLuint secondTextureID;
};

struct PMDModelUserData {
    GLuint toonTextureID[vpvl::PMDModel::kSystemTextureMax];
    GLuint vertexBufferObjects[kVertexBufferObjectMax];
    bool hasSingleSphereMap;
    bool hasMultipleSphereMap;
    __vpvlPMDModelMaterialPrivate *materials;
};

struct XModelUserData {
    GLuint listID;
    vpvl::Hash<vpvl::HashString, GLuint> textures;
};

namespace gl
{

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
#ifndef VPVL_USE_ALLEGRO5
    err = glewInit();
    return err == GLEW_OK;
#else
    (void) err;
    return true;
#endif
}

Renderer::Renderer(IDelegate *delegate, int width, int height, int fps)
    : m_scene(0),
      m_selected(0),
      m_debugDrawer(0),
      m_delegate(delegate)
{
    m_debugDrawer = new DebugDrawer();
    m_scene = new vpvl::Scene(width, height, fps);
}

Renderer::~Renderer()
{
    delete m_debugDrawer;
    delete m_scene;
}

void Renderer::initializeSurface()
{
    glClearStencil(0);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GEQUAL, 0.05f);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    glLightfv(GL_LIGHT0, GL_POSITION, static_cast<const btScalar *>(m_scene->lightPosition()));
    glLightfv(GL_LIGHT0, GL_DIFFUSE, static_cast<const btScalar *>(m_scene->lightDiffuse()));
    glLightfv(GL_LIGHT0, GL_AMBIENT, static_cast<const btScalar *>(m_scene->lightAmbient()));
    glLightfv(GL_LIGHT0, GL_SPECULAR, static_cast<const btScalar *>(m_scene->lightSpecular()));
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
    __vpvlPMDModelMaterialPrivate *materialPrivates = new __vpvlPMDModelMaterialPrivate[nMaterials];
    bool hasSingleSphere = false, hasMultipleSphere = false;
    for (uint32_t i = 0; i < nMaterials; i++) {
        const vpvl::Material *material = materials[i];
        const std::string primary = m_delegate->toUnicode(material->mainTextureName());
        const std::string second = m_delegate->toUnicode(material->subTextureName());
        __vpvlPMDModelMaterialPrivate &materialPrivate = materialPrivates[i];
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
        m_delegate->log(IDelegate::kLogInfo, "Binding the texture as a toon texture (ID=%d)", textureID);
    }
    for (uint32_t i = 0; i < vpvl::PMDModel::kSystemTextureMax - 1; i++) {
        const uint8_t *name = model->toonTexture(i);
        if (m_delegate->loadToonTexture(reinterpret_cast<const char *>(name), dir, textureID)) {
            userData->toonTextureID[i + 1] = textureID;
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
            __vpvlPMDModelMaterialPrivate &materialPrivate = userData->materials[i];
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
    size_t stride = model->stride(vpvl::PMDModel::kNormalsStride), vsize = model->vertices().count();
    glActiveTexture(GL_TEXTURE0);
    glClientActiveTexture(GL_TEXTURE0);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelVertices]);
    glVertexPointer(3, GL_FLOAT, model->stride(vpvl::PMDModel::kVerticesStride), 0);
    glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelNormals]);
    glBufferData(GL_ARRAY_BUFFER, vsize * stride, model->normalsPointer(), GL_DYNAMIC_DRAW);
    glNormalPointer(GL_FLOAT, stride, 0);
    glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelTexCoords]);
    glTexCoordPointer(2, GL_FLOAT, model->stride(vpvl::PMDModel::kTextureCoordsStride), 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, userData->vertexBufferObjects[kShadowIndices]);

    const bool enableToon = true;
    // toon
    if (enableToon) {
        glActiveTexture(GL_TEXTURE1);
        glEnable(GL_TEXTURE_2D);
        glClientActiveTexture(GL_TEXTURE1);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelToonTexCoords]);
        // shadow map
        stride = model->stride(vpvl::PMDModel::kToonTextureStride);
        if (false)
            glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_DYNAMIC_DRAW);
        else
            glBufferData(GL_ARRAY_BUFFER, vsize * stride, model->toonTextureCoordsPointer(), GL_DYNAMIC_DRAW);
        glTexCoordPointer(2, GL_FLOAT, stride, 0);
        glActiveTexture(GL_TEXTURE0);
        glClientActiveTexture(GL_TEXTURE0);
    }
    bool hasSingleSphereMap = false, hasMultipleSphereMap = false;
    // first sphere map
    if (userData->hasSingleSphereMap) {
        glEnable(GL_TEXTURE_2D);
        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        glDisable(GL_TEXTURE_2D);
        hasSingleSphereMap = true;
    }
    // second sphere map
    if (userData->hasMultipleSphereMap) {
        glActiveTexture(GL_TEXTURE2);
        glEnable(GL_TEXTURE_2D);
        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        glDisable(GL_TEXTURE_2D);
        glActiveTexture(GL_TEXTURE0);
        hasMultipleSphereMap = true;
    }

    const vpvl::MaterialList &materials = model->materials();
    const __vpvlPMDModelMaterialPrivate *materialPrivates = userData->materials;
    const uint32_t nMaterials = materials.count();
    btVector4 average, ambient, diffuse, specular;
    uint32_t offset = 0;
    for (uint32_t i = 0; i < nMaterials; i++) {
        const vpvl::Material *material = materials[i];
        const __vpvlPMDModelMaterialPrivate &materialPrivate = materialPrivates[i];
        // toon
        const float alpha = material->opacity();
        if (enableToon) {
            average = material->averageColor();
            average.setW(average.w() * alpha);
            specular = material->specular();
            specular.setW(specular.w() * alpha);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, static_cast<const GLfloat *>(average));
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, static_cast<const GLfloat *>(specular));
        }
        else {
            ambient = material->ambient();
            ambient.setW(ambient.w() * alpha);
            diffuse = material->diffuse();
            diffuse.setW(diffuse.w() * alpha);
            specular = material->specular();
            specular.setW(specular.w() * alpha);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, static_cast<const GLfloat *>(ambient));
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, static_cast<const GLfloat *>(diffuse));
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, static_cast<const GLfloat *>(specular));
        }
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material->shiness());
        material->opacity() < 1.0f ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);
        glActiveTexture(GL_TEXTURE0);
        // has texture
        if (materialPrivate.primaryTextureID > 0) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, materialPrivate.primaryTextureID);
            if (hasSingleSphereMap) {
                // is sphere map
                if (material->isMultiplicationSphereMain() || material->isAdditionalSphereMain()) {
                    // is second sphere map
                    if (material->isAdditionalSphereMain())
                        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
                    glEnable(GL_TEXTURE_GEN_S);
                    glEnable(GL_TEXTURE_GEN_T);
                }
                else {
                    glDisable(GL_TEXTURE_GEN_S);
                    glDisable(GL_TEXTURE_GEN_T);
                }
            }
        }
        else {
            glDisable(GL_TEXTURE_2D);
        }
        // toon
        if (enableToon) {
            const GLuint textureID = userData->toonTextureID[material->toonID()];
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        if (hasMultipleSphereMap) {
            // second sphere
            glActiveTexture(GL_TEXTURE2);
            glEnable(GL_TEXTURE_2D);
            if (materialPrivate.secondTextureID > 0) {
                // is second sphere
                if (material->isAdditionalSphereSub())
                    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
                else
                    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
                glBindTexture(GL_TEXTURE_2D, materialPrivate.secondTextureID);
                glEnable(GL_TEXTURE_GEN_S);
                glEnable(GL_TEXTURE_GEN_T);
            }
            else {
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }
        // draw
        const uint32_t nIndices = material->countIndices();
        glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_SHORT, reinterpret_cast<GLvoid *>(offset));
        offset += (nIndices << 1);
        // is aux sphere map
        if (material->isAdditionalSphereMain()) {
            glActiveTexture(GL_TEXTURE0);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    // toon
    if (enableToon) {
        glClientActiveTexture(GL_TEXTURE0);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        // first sphere map
        if (hasSingleSphereMap) {
            glActiveTexture(GL_TEXTURE0);
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
        }
        glClientActiveTexture(GL_TEXTURE1);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        // second sphere map
        if (hasMultipleSphereMap) {
            glActiveTexture(GL_TEXTURE2);
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
        }
    }
    else {
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        // first sphere map
        if (hasSingleSphereMap) {
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
        }
        // second sphere map
        if (hasMultipleSphereMap) {
            glActiveTexture(GL_TEXTURE2);
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
        }
    }
    glActiveTexture(GL_TEXTURE0);
    // first or second sphere map
    if (hasSingleSphereMap || hasMultipleSphereMap) {
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
    }
    // toon
    if (enableToon) {
        glActiveTexture(GL_TEXTURE1);
        glDisable(GL_TEXTURE_2D);
    }
    // second sphere map
    if (hasMultipleSphereMap) {
        glActiveTexture(GL_TEXTURE2);
        glDisable(GL_TEXTURE_2D);
    }
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);
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
    btVector4 color;

    if (model == m_selected)
        color.setValue(1.0f, 0.0f, 0.0f, alpha);
    else
        color.setValue(0.0f, 0.0f, 0.0f, alpha);

    glDisable(GL_LIGHTING);
    glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, modelPrivate->vertexBufferObjects[kEdgeVertices]);
    glBufferData(GL_ARRAY_BUFFER, model->vertices().count() * stride, model->edgeVerticesPointer(), GL_DYNAMIC_DRAW);
    glVertexPointer(3, GL_FLOAT, stride, 0);
    glColor4fv(static_cast<const btScalar *>(color));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelPrivate->vertexBufferObjects[kEdgeIndices]);
    glDrawElements(GL_TRIANGLES, model->edgeIndicesCount(), GL_UNSIGNED_SHORT, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_LIGHTING);

#ifdef VPVL_COORDINATE_OPENGL
    glCullFace(GL_BACK);
#else
    glPopMatrix();
    glCullFace(GL_FRONT);
#endif
}

void Renderer::drawModelShadow(const vpvl::PMDModel *model)
{
    const size_t stride = model->stride(vpvl::PMDModel::kVerticesStride);
    const vpvl::PMDModelUserData *modelPrivate = model->userData();
    glDisable(GL_CULL_FACE);
    glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, modelPrivate->vertexBufferObjects[kModelVertices]);
    glBufferData(GL_ARRAY_BUFFER, model->vertices().count() * stride, model->verticesPointer(), GL_DYNAMIC_DRAW);
    glVertexPointer(3, GL_FLOAT, stride, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelPrivate->vertexBufferObjects[kShadowIndices]);
    glDrawElements(GL_TRIANGLES, model->indices().count(), GL_UNSIGNED_SHORT, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableClientState(GL_VERTEX_ARRAY);
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
    // qDebug("Generated a OpenGL list (ID=%d)", userData->listID);
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
    float matrix[16];
    glViewport(0, 0, m_scene->width(), m_scene->height());
    glMatrixMode(GL_PROJECTION);
    m_scene->getProjectionMatrix(matrix);
    glLoadMatrixf(matrix);
    glMatrixMode(GL_MODELVIEW);
    m_scene->getModelViewMatrix(matrix);
    glLoadMatrixf(matrix);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    // initialize rendering states
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 1, ~0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
    glColorMask(0, 0, 0, 0);
    glDepthMask(0);
    glDisable(GL_DEPTH_TEST);
    glPushMatrix();
    // render shadow before drawing models
    size_t size = 0;
    vpvl::PMDModel **models = m_scene->getRenderingOrder(size);
    for (size_t i = 0; i < size; i++) {
        vpvl::PMDModel *model = models[i];
        drawModelShadow(model);
    }
    glPopMatrix();
    glColorMask(1, 1, 1, 1);
    glDepthMask(1);
    glStencilFunc(GL_EQUAL, 2, ~0);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
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
