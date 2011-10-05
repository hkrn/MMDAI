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

namespace
{

enum VertexBufferObjectType {
    kModelVertices,
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

#ifdef VPVL_LINK_ASSIMP
#include <aiScene.h>
#include <map>
namespace
{
struct AssetVertex
{
    btVector3 position;
    btVector3 normal;
    btVector3 texcoord;
    btVector4 color;
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
};
}
namespace
{

inline void aiColor2Float4(const struct aiColor4D &color, float *dest)
{
    dest[0] = color.r;
    dest[1] = color.g;
    dest[2] = color.b;
    dest[3] = color.a;
}

void aiLoadAssetRecursive(const aiScene *scene, const aiNode *node, vpvl::AssetUserData *userData)
{
    const uint32_t nMeshes = node->mNumMeshes;
    AssetVertex assetVertex;
    for (uint32_t i = 0; i < nMeshes; i++) {
        const struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        const aiVector3D *vertices = mesh->mVertices;
        const aiVector3D *normals = mesh->mNormals;
        const aiColor4D *colors = mesh->mColors[0];
        const bool hasNormals = normals ? true : false;
        const bool hasColors = colors ? true : false;
        const bool hasTexCoords = mesh->HasTextureCoords(0);
        const aiVector3D *texcoords = hasTexCoords ? mesh->mTextureCoords[0] : 0;
        AssetVertices &assetVertices = userData->vertices[mesh];
        AssetIndices &indices = userData->indices[mesh];
        const uint32_t nFaces = mesh->mNumFaces;
        uint32_t index = 0;
        for (uint32_t j = 0; j < nFaces; j++) {
            const struct aiFace &face = mesh->mFaces[j];
            const uint32_t nIndices = face.mNumIndices;
            for (uint32_t k = 0; k < nIndices; k++) {
                int vertexIndex = face.mIndices[k];
                if (hasColors) {
                    const aiColor4D &c = colors[vertexIndex];
                    assetVertex.color.setValue(c.r, c.g, c.b, c.a);
                }
                else {
                    assetVertex.color.setZero();
                    assetVertex.color.setW(0.0f);
                }
                if (hasTexCoords) {
                    const aiVector3D &p = texcoords[vertexIndex];
                    assetVertex.texcoord.setValue(p.x, 1.0f - p.y, 0.0f);
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
    const uint32_t nChildNodes = node->mNumChildren;
    for (uint32_t i = 0; i < nChildNodes; i++)
        aiLoadAssetRecursive(scene, node->mChildren[i], userData);
}

void aiUnloadAssetRecursive(const aiScene *scene, const aiNode *node, vpvl::AssetUserData *userData)
{
    const uint32_t nMeshes = node->mNumMeshes;
    for (uint32_t i = 0; i < nMeshes; i++) {
        const struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        const AssetVBO &vbo = userData->vbo[mesh];
        glDeleteBuffers(1, &vbo.vertices);
        glDeleteBuffers(1, &vbo.indices);
    }
    const uint32_t nChildNodes = node->mNumChildren;
    for (uint32_t i = 0; i < nChildNodes; i++)
        aiUnloadAssetRecursive(scene, node->mChildren[i], userData);
}

void aiSetAssetMaterial(const aiMaterial *material, vpvl::Asset *asset)
{
    int textureIndex = 0;
    aiString texturePath;
    if (material->GetTexture(aiTextureType_DIFFUSE, textureIndex, &texturePath) == aiReturn_SUCCESS) {
        GLuint textureID = asset->userData()->textures[texturePath.data];
        glBindTexture(GL_TEXTURE_2D, textureID);
    }
    else {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    aiColor4D ambient, diffuse, emission, specular;
    float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    if (aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &ambient) == aiReturn_SUCCESS) {
        aiColor2Float4(ambient, color);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, color);
    }
    else {
        float defaultAmbient[4] = { 0.8f, 0.8f, 0.8f, 0.8f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, defaultAmbient);
    }
    if (aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse) == aiReturn_SUCCESS) {
        aiColor2Float4(diffuse, color);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
    }
    else {
        float defaultDiffuse[4] = { 0.2f, 0.2f, 0.2f, 0.2f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, defaultDiffuse);
    }
    if (aiGetMaterialColor(material, AI_MATKEY_COLOR_EMISSIVE, &emission) == aiReturn_SUCCESS) {
        aiColor2Float4(emission, color);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, color);
    }
    else {
        float defaultEmission[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, defaultEmission);
    }
    if (aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &specular) == aiReturn_SUCCESS) {
        aiColor2Float4(specular, color);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, color);
    }
    else {
        float defaultSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, defaultSpecular);
    }
    float shininess, strength;
    int ret1 = aiGetMaterialFloat(material, AI_MATKEY_SHININESS, &shininess);
    int ret2 = aiGetMaterialFloat(material, AI_MATKEY_SHININESS_STRENGTH, &strength);
    if (ret1 == aiReturn_SUCCESS && ret2 == aiReturn_SUCCESS) {
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess * strength);
    }
    else if (ret1 == aiReturn_SUCCESS) {
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
    }
    else {
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);
    }
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
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

void aiDrawAssetRecurse(const aiScene *scene, const aiNode *node, vpvl::Asset *asset)
{
    struct aiMatrix4x4 m = node->mTransformation;
    const btScalar &scaleFactor = asset->scaleFactor();
    const btVector3 &pos = asset->position();
    // translate
    m.a4 = pos.x();
    m.b4 = pos.y();
    m.c4 = pos.z();
    // scale
    m.a1 = scaleFactor;
    m.b2 = scaleFactor;
    m.c3 = scaleFactor;
    m.Transpose();
    glPushMatrix();
    glMultMatrixf(reinterpret_cast<const float *>(&m));
    vpvl::AssetUserData *userData = asset->userData();
    AssetVertex v;
    const GLvoid *vertexPtr = 0;
    const GLvoid *normalPtr = reinterpret_cast<const GLvoid *>(reinterpret_cast<const uint8_t *>(&v.normal) - reinterpret_cast<const uint8_t *>(&v.position));
    const GLvoid *texcoordPtr = reinterpret_cast<const GLvoid *>(reinterpret_cast<const uint8_t *>(&v.texcoord) - reinterpret_cast<const uint8_t *>(&v.position));
    const GLvoid *colorPtr = reinterpret_cast<const GLvoid *>(reinterpret_cast<const uint8_t *>(&v.color) - reinterpret_cast<const uint8_t *>(&v.position));
    const uint32_t nMeshes = node->mNumMeshes;
    const size_t stride = sizeof(AssetVertex);
    for (uint32_t i = 0; i < nMeshes; i++) {
        const struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        const bool hasNormals = mesh->mNormals ? true : false;
        const bool hasColors = mesh->mColors[0] ? true : false;
        const bool hasTexCoords = mesh->HasTextureCoords(0);
        aiSetAssetMaterial(scene->mMaterials[mesh->mMaterialIndex], asset);
        hasNormals ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING);
        hasColors ? glEnable(GL_COLOR_MATERIAL) : glDisable(GL_COLOR_MATERIAL);
        glActiveTexture(GL_TEXTURE0);
        glClientActiveTexture(GL_TEXTURE0);
        glEnableClientState(GL_VERTEX_ARRAY);
        if (hasColors)
            glEnableClientState(GL_COLOR_ARRAY);
        if (hasNormals)
            glEnableClientState(GL_NORMAL_ARRAY);
        if (hasTexCoords)
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        const AssetVBO &vbo = userData->vbo[mesh];
        const AssetIndices &indices = userData->indices[mesh];
        glBindBuffer(GL_ARRAY_BUFFER, vbo.vertices);
        glVertexPointer(3, GL_FLOAT, stride, vertexPtr);
        glNormalPointer(GL_FLOAT, stride, normalPtr);
        glTexCoordPointer(2, GL_FLOAT, stride, texcoordPtr);
        glColorPointer(4, GL_FLOAT, stride, colorPtr);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.indices);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glDisableClientState(GL_VERTEX_ARRAY);
        if (hasColors)
            glDisableClientState(GL_COLOR_ARRAY);
        if (hasNormals)
            glDisableClientState(GL_NORMAL_ARRAY);
        if (hasTexCoords)
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    const uint32_t nChildNodes = node->mNumChildren;
    for (uint32_t i = 0; i < nChildNodes; i++)
        aiDrawAssetRecurse(scene, node->mChildren[i], asset);
    glPopMatrix();
}

void aiDrawAsset(vpvl::Asset *asset)
{
    const aiScene *a = asset->getScene();
    glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);
    aiDrawAssetRecurse(a, a->mRootNode, asset);
    glPopAttrib();
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
void aiDrawAsset(vpvl::Asset *asset)
{
    (void) asset;
}
}
#endif

namespace vpvl
{

struct PMDModelUserData {
    GLuint toonTextureID[vpvl::PMDModel::kSystemTextureMax];
    GLuint vertexBufferObjects[kVertexBufferObjectMax];
    bool hasSingleSphereMap;
    bool hasMultipleSphereMap;
    PMDModelMaterialPrivate *materials;
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
    vpvl::Array<vpvl::PMDModel *> models;
    models.copy(m_scene->models());
    const uint32_t nModels = models.count();
    for (uint32_t i = 0; i < nModels; i++) {
        vpvl::PMDModel *model = models[i];
        unloadModel(model);
    }
    vpvl::Array<vpvl::Asset *> assets;
    assets.copy(m_assets);
    const uint32_t nAssets = assets.count();
    for (uint32_t i = 0; i < nAssets; i++) {
        vpvl::Asset *asset = assets[i];
        unloadAsset(asset);
    }
    m_assets.clear();
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
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
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
                    userData->vertexBufferObjects[kModelVertices]);
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
    m_scene->addModel(model);
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
        m_scene->removeModel(const_cast<vpvl::PMDModel *>(model));
    }
}

void Renderer::updateModelBuffer(const vpvl::PMDModel *model)
{
    size_t size = model->vertices().count() * model->strideSize(vpvl::PMDModel::kVerticesStride);
    glBindBuffer(GL_ARRAY_BUFFER, model->userData()->vertexBufferObjects[kModelVertices]);
    glBufferData(GL_ARRAY_BUFFER, size, model->verticesPointer(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Renderer::drawModel(const vpvl::PMDModel *model)
{
#ifndef VPVL_COORDINATE_OPENGL
    glPushMatrix();
    glScalef(1.0f, 1.0f, -1.0f);
    glCullFace(GL_FRONT);
#endif

    const vpvl::PMDModelUserData *userData = model->userData();
    size_t stride = model->strideSize(vpvl::PMDModel::kVerticesStride), vsize = model->vertices().count();
    glActiveTexture(GL_TEXTURE0);
    glClientActiveTexture(GL_TEXTURE0);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelVertices]);
    glVertexPointer(3, GL_FLOAT, stride, reinterpret_cast<const GLvoid *>(model->strideOffset(vpvl::PMDModel::kVerticesStride)));
    glNormalPointer(GL_FLOAT, stride, reinterpret_cast<const GLvoid *>(model->strideOffset(vpvl::PMDModel::kNormalsStride)));
    glTexCoordPointer(2, GL_FLOAT, stride, reinterpret_cast<const GLvoid *>(model->strideOffset(vpvl::PMDModel::kTextureCoordsStride)));
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
        stride = model->strideSize(vpvl::PMDModel::kToonTextureStride);
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
    const PMDModelMaterialPrivate *materialPrivates = userData->materials;
    const uint32_t nMaterials = materials.count();
    btVector4 average, ambient, diffuse, specular;
    uint32_t offset = 0;
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

    const size_t stride = model->strideSize(vpvl::PMDModel::kEdgeVerticesStride);
    const vpvl::PMDModelUserData *modelPrivate = model->userData();
    glDisable(GL_LIGHTING);
    glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, modelPrivate->vertexBufferObjects[kEdgeVertices]);
    glBufferData(GL_ARRAY_BUFFER, model->vertices().count() * stride, model->edgeVerticesPointer(), GL_DYNAMIC_DRAW);
    glVertexPointer(3, GL_FLOAT, stride, 0);
    glColor4fv(static_cast<const Scalar *>(model->edgeColor()));
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
    const size_t stride = model->strideSize(vpvl::PMDModel::kVerticesStride);
    const vpvl::PMDModelUserData *modelPrivate = model->userData();
    glDisable(GL_CULL_FACE);
    glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, modelPrivate->vertexBufferObjects[kModelVertices]);
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

static const std::string CanonicalizePath(const std::string &path)
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

void Renderer::loadAsset(Asset *asset, const std::string &dir)
{
#ifdef VPVL_LINK_ASSIMP
    const aiScene *scene = asset->getScene();
    const uint32_t nMaterials = scene->mNumMaterials;
    AssetUserData *userData = new AssetUserData();
    aiString texturePath;
    std::string path, canonicalized, filename;
    asset->setUserData(userData);
    for (uint32_t i = 0; i < nMaterials; i++) {
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
    aiLoadAssetRecursive(scene, scene->mRootNode, userData);
    m_assets.add(asset);
#else
    (void) asset;
    (void) dir;
#endif
}

void Renderer::unloadAsset(Asset *asset)
{
#ifdef VPVL_LINK_ASSIMP
    if (asset) {
        const aiScene *scene = asset->getScene();
        const uint32_t nMaterials = scene->mNumMaterials;
        AssetUserData *userData = asset->userData();
        aiString texturePath;
        for (uint32_t i = 0; i < nMaterials; i++) {
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
        delete asset;
        delete userData;
        m_assets.remove(asset);
    }
#else
    (void) asset;
#endif
}

void Renderer::clearSurface()
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
}

void Renderer::preShadow()
{
    glEnable(GL_STENCIL_TEST);
    glDisable(GL_LIGHTING);
    glStencilFunc(GL_EQUAL, 1, ~0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
    glColorMask(0, 0, 0, 0);
    glDepthMask(0);
    glDisable(GL_DEPTH_TEST);
}

void Renderer::drawShadow()
{
    size_t size = 0;
    vpvl::PMDModel **models = m_scene->getRenderingOrder(size);
    for (size_t i = 0; i < size; i++) {
        vpvl::PMDModel *model = models[i];
        glPushMatrix();
        updateModelBuffer(model);
        drawModelShadow(model);
        glPopMatrix();
    }
}

void Renderer::postShadow()
{
    glColorMask(1, 1, 1, 1);
    glDepthMask(1);
    glStencilFunc(GL_EQUAL, 2, ~0);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void Renderer::drawAssets()
{
    uint32_t nAssets = m_assets.count();
    for (uint32_t i = 0; i < nAssets; i++) {
        glPushMatrix();
        aiDrawAsset(m_assets[i]);
        glPopMatrix();
    }
}

void Renderer::drawModels()
{
    size_t size = 0;
    vpvl::PMDModel **models = m_scene->getRenderingOrder(size);
    for (size_t i = 0; i < size; i++) {
        vpvl::PMDModel *model = models[i];
        glPushMatrix();
        drawModel(model);
        drawModelEdge(model);
        glPopMatrix();
    }
}

void Renderer::drawSurface()
{
    clearSurface();
    preShadow();
    drawShadow();
    postShadow();
    drawAssets();
    drawModels();
}

}
}
