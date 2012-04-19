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

#ifdef VPVL2_LINK_ASSIMP
#include "vpvl2/gl2/AssetRenderEngine.h"

#include "vpvl/Bone.h"
#include "vpvl2/asset/Model.h"
#include "EngineCommon.h"

#include <map>

namespace vpvl2
{
namespace gl2
{

class AssetRenderEngine::Program : public ObjectProgram
{
public:
    Program(IRenderDelegate *delegate)
        : ObjectProgram(delegate),
          m_texCoordAttributeLocation(0),
          m_colorAttributeLocation(0),
          m_normalMatrixUniformLocation(0),
          m_transformMatrixUniformLocation(0),
          m_lightViewProjectionMatrixUniformLocation(0),
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
    ~Program() {
        m_texCoordAttributeLocation = 0;
        m_colorAttributeLocation = 0;
        m_normalMatrixUniformLocation = 0;
        m_transformMatrixUniformLocation = 0;
        m_lightViewProjectionMatrixUniformLocation = 0;
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

    virtual bool load(const char *vertexShaderSource, const char *fragmentShaderSource, void *context) {
        bool ret = ObjectProgram::load(vertexShaderSource, fragmentShaderSource, context);
        if (ret) {
            m_texCoordAttributeLocation = glGetAttribLocation(m_program, "inTexCoord");
            m_colorAttributeLocation = glGetAttribLocation(m_program, "inColor");
            m_normalMatrixUniformLocation = glGetUniformLocation(m_program, "normalMatrix");
            m_transformMatrixUniformLocation = glGetUniformLocation(m_program, "transformMatrix");
            m_lightViewProjectionMatrixUniformLocation = glGetUniformLocation(m_program, "lightViewProjectionMatrix");
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
    void setLightViewProjectionMatrix(const GLfloat value[16]) {
        glUniformMatrix4fv(m_lightViewProjectionMatrixUniformLocation, 1, GL_FALSE, value);
    }
    void setMaterialAmbient(const Color &value) {
        glUniform3fv(m_materialAmbientUniformLocation, 1, value);
    }
    void setMaterialDiffuse(const Color &value) {
        glUniform4fv(m_materialDiffuseUniformLocation, 1, value);
    }
    void setMaterialEmission(const Color &value) {
        glUniform3fv(m_materialEmissionUniformLocation, 1, value);
    }
    void setMaterialSpecular(const Color &value) {
        glUniform3fv(m_materialSpecularUniformLocation, 1, value);
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
        else {
            glUniform1i(m_hasTextureUniformLocation, 0);
        }
    }

private:
    GLuint m_texCoordAttributeLocation;
    GLuint m_colorAttributeLocation;
    GLuint m_normalMatrixUniformLocation;
    GLuint m_transformMatrixUniformLocation;
    GLuint m_lightViewProjectionMatrixUniformLocation;
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

struct AssetVertex
{
    AssetVertex() {}
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

class AssetRenderEngine::PrivateContext
{
public:
    PrivateContext() {}
    virtual ~PrivateContext() {}

    std::map<std::string, GLuint> textures;
    std::map<const struct aiMesh *, AssetVertices> vertices;
    std::map<const struct aiMesh *, AssetIndices> indices;
    std::map<const struct aiMesh *, AssetVBO> vbo;
    std::map<const struct aiNode *, AssetRenderEngine::Program *> assetPrograms;
    std::map<const struct aiNode *, ZPlotProgram *> zplotPrograms;
};

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

AssetRenderEngine::AssetRenderEngine(IRenderDelegate *delegate, const Scene *scene, asset::Model *model)
#ifdef VPVL2_LINK_QT
    : QGLFunctions(),
      #else
    :
      #endif /* VPVL2_LINK_QT */
      m_delegate(delegate),
      m_scene(scene),
      m_model(model),
      m_context(0)
{
    m_context = new PrivateContext();
}

AssetRenderEngine::~AssetRenderEngine()
{
    const aiScene *scene = m_model->ptr()->getScene();
    const unsigned int nmaterials = scene->mNumMaterials;
    aiString texturePath;
    for (unsigned int i = 0; i < nmaterials; i++) {
        aiMaterial *material = scene->mMaterials[i];
        aiReturn found = AI_SUCCESS;
        GLuint textureID;
        int textureIndex = 0;
        while (found == AI_SUCCESS) {
            found = material->GetTexture(aiTextureType_DIFFUSE, textureIndex, &texturePath);
            textureID = m_context->textures[texturePath.data];
            glDeleteTextures(1, &textureID);
            m_context->textures.erase(texturePath.data);
            textureIndex++;
        }
    }
    deleteRecurse(scene, scene->mRootNode);
    delete m_context;
    m_context = 0;
    delete m_model;
    m_model = 0;
    m_delegate = 0;
    m_scene = 0;
}

void AssetRenderEngine::renderModel()
{
    vpvl::Asset *asset = m_model->ptr();
    if (btFuzzyZero(asset->opacity()))
        return;
    const aiScene *a = asset->getScene();
    renderRecurse(a, a->mRootNode);
}

void AssetRenderEngine::renderEdge()
{
}

void AssetRenderEngine::renderShadow()
{
}

void AssetRenderEngine::renderZPlot()
{
    vpvl::Asset *asset = m_model->ptr();
    if (btFuzzyZero(asset->opacity()))
        return;
    const aiScene *a = asset->getScene();
    renderZPlotRecurse(a, a->mRootNode);
}

IModel *AssetRenderEngine::model() const
{
    return m_model;
}

bool AssetRenderEngine::upload(const std::string &dir)
{
#ifdef VPVL2_LINK_QT
    initializeGLFunctions(QGLContext::currentContext());
#endif /* VPVL2_LINK_QT */
    vpvl::Asset *asset = m_model->ptr();
    const aiScene *scene = asset->getScene();
    const unsigned int nmaterials = scene->mNumMaterials;
    void *context = 0;
    aiString texturePath;
    std::string path, canonicalized;
    m_delegate->allocateContext(m_model, context);
    for (unsigned int i = 0; i < nmaterials; i++) {
        aiMaterial *material = scene->mMaterials[i];
        aiReturn found = AI_SUCCESS;
        GLuint textureID;
        int textureIndex = 0;
        while (found == AI_SUCCESS) {
            found = material->GetTexture(aiTextureType_DIFFUSE, textureIndex, &texturePath);
            path = texturePath.data;
            if (m_context->textures[path] == 0) {
                canonicalized = m_delegate->toUnicode(reinterpret_cast<const uint8_t *>(CanonicalizePath(path).c_str()));
                if (m_delegate->uploadTexture(context, canonicalized, dir, &textureID, false)) {
                    m_context->textures[path] = textureID;
                    log0(context, IRenderDelegate::kLogInfo, "Loaded a texture: %s (ID=%d)", canonicalized.c_str(), textureID);
                }
            }
            textureIndex++;
        }
    }
    uploadRecurse(scene, scene->mRootNode, context);
    m_delegate->releaseContext(m_model, context);
    return true;
}

void AssetRenderEngine::update()
{
}

void AssetRenderEngine::uploadRecurse(const aiScene *scene, const aiNode *node, void *context)
{
    const unsigned int nmeshes = node->mNumMeshes;
    AssetVertex assetVertex;
    Program *assetProgram = new Program(m_delegate);
    ZPlotProgram *zplotProgram = new ZPlotProgram(m_delegate);
#ifdef VPVL2_LINK_QT
    assetProgram->initializeContext(QGLContext::currentContext());
    zplotProgram->initializeContext(QGLContext::currentContext());
#endif /* VPVL2_LINK_QT */
    assetProgram->load(m_delegate->loadShader(IRenderDelegate::kModelVertexShader, m_model, context).c_str(),
                       m_delegate->loadShader(IRenderDelegate::kModelFragmentShader, m_model, context).c_str(),
                       context);
    zplotProgram->load(m_delegate->loadShader(IRenderDelegate::kZPlotVertexShader, m_model, context).c_str(),
                       m_delegate->loadShader(IRenderDelegate::kZPlotFragmentShader, m_model, context).c_str(),
                       context);
    m_context->assetPrograms[node] = assetProgram;
    m_context->zplotPrograms[node] = zplotProgram;
    for (unsigned int i = 0; i < nmeshes; i++) {
        const struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        const aiVector3D *vertices = mesh->mVertices;
        const aiVector3D *normals = mesh->mNormals;
        const bool hasNormals = mesh->HasNormals();
        const bool hasColors = mesh->HasVertexColors(0);
        const bool hasTexCoords = mesh->HasTextureCoords(0);
        const aiColor4D *colors = hasColors ? mesh->mColors[0] : 0;
        const aiVector3D *texcoords = hasTexCoords ? mesh->mTextureCoords[0] : 0;
        AssetVertices &assetVertices = m_context->vertices[mesh];
        AssetIndices &indices = m_context->indices[mesh];
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
                assetVertex.position.setValue(v.x, v.y, v.z, 1.0f);
                assetVertices.push_back(assetVertex);
                indices.push_back(index);
                index++;
            }
        }
        AssetVBO &vbo = m_context->vbo[mesh];
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
        uploadRecurse(scene, node->mChildren[i], context);
}

void AssetRenderEngine::deleteRecurse(const aiScene *scene, const aiNode *node)
{
    const unsigned int nmeshes = node->mNumMeshes;
    for (unsigned int i = 0; i < nmeshes; i++) {
        const struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        const AssetVBO &vbo = m_context->vbo[mesh];
        glDeleteBuffers(1, &vbo.vertices);
        glDeleteBuffers(1, &vbo.indices);
    }
    delete m_context->assetPrograms[node];
    delete m_context->zplotPrograms[node];
    const unsigned int nChildNodes = node->mNumChildren;
    for (unsigned int i = 0; i < nChildNodes; i++)
        deleteRecurse(scene, node->mChildren[i]);
}

void AssetRenderEngine::setAssetMaterial(const aiMaterial *material, Program *program)
{
    int textureIndex = 0;
    aiString texturePath;
    if (material->GetTexture(aiTextureType_DIFFUSE, textureIndex, &texturePath) == aiReturn_SUCCESS) {
        GLuint textureID = m_context->textures[texturePath.data];
        program->setTexture(textureID);
    }
    else {
        program->setTexture(0);
    }
    aiColor4D ambient, diffuse, emission, specular;
    Color color(0.0f, 0.0f, 0.0f, 0.0f);
    if (aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &ambient) == aiReturn_SUCCESS) {
        color.setValue(ambient.r, ambient.g, ambient.b, ambient.a);
    }
    else {
        color.setValue(0.2f, 0.2f, 0.2f, 1.0f);
    }
    program->setMaterialAmbient(color);
    if (aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse) == aiReturn_SUCCESS) {
        color.setValue(diffuse.r, diffuse.g, diffuse.b, diffuse.a);
    }
    else {
        color.setValue(0.8f, 0.8f, 0.8f, 1.0f);
    }
    program->setMaterialDiffuse(color);
    if (aiGetMaterialColor(material, AI_MATKEY_COLOR_EMISSIVE, &emission) == aiReturn_SUCCESS) {
        color.setValue(emission.r, emission.g, emission.b, emission.a);
    }
    else {
        color.setValue(0.0f, 0.0f, 0.0f, 0.0f);
    }
    program->setMaterialEmission(color);
    if (aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &specular) == aiReturn_SUCCESS) {
        color.setValue(specular.r, specular.g, specular.b, specular.a);
    }
    else {
        color.setValue(0.0f, 0.0f, 0.0f, 1.0f);
    }
    program->setMaterialSpecular(color);
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
    vpvl::Asset *asset = m_model->ptr();
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

void AssetRenderEngine::renderRecurse(const aiScene *scene, const aiNode *node)
{
    vpvl::Asset *asset = m_model->ptr();
    const btScalar &scaleFactor = asset->scaleFactor();
    aiVector3D aiS, aiP;
    aiQuaternion aiQ;
    node->mTransformation.Decompose(aiS, aiQ, aiP);
    const vpvl::Vector3 scaleVector(aiS.x * scaleFactor, aiS.y * scaleFactor, aiS.z * scaleFactor);
    Transform transform(btMatrix3x3(Quaternion(aiQ.x, aiQ.y, aiQ.z, aiQ.w) * asset->rotation()).scaled(scaleVector),
                        Vector3(aiP.x,aiP.y, aiP.z) + asset->position());
    if (const vpvl::Bone *bone = asset->parentBone()) {
        const Transform &boneTransform = bone->localTransform();
        const btMatrix3x3 &boneBasis = boneTransform.getBasis();
        transform.setOrigin(boneTransform.getOrigin() + boneBasis * transform.getOrigin());
        transform.setBasis(boneBasis.scaled(scaleVector));
    }
    static const AssetVertex v;
    const GLvoid *vertexPtr = 0;
    const GLvoid *normalPtr = reinterpret_cast<const GLvoid *>(reinterpret_cast<const uint8_t *>(&v.normal) - reinterpret_cast<const uint8_t *>(&v.position));
    const GLvoid *texcoordPtr = reinterpret_cast<const GLvoid *>(reinterpret_cast<const uint8_t *>(&v.texcoord) - reinterpret_cast<const uint8_t *>(&v.position));
    const GLvoid *colorPtr = reinterpret_cast<const GLvoid *>(reinterpret_cast<const uint8_t *>(&v.color) - reinterpret_cast<const uint8_t *>(&v.position));
    const unsigned int nmeshes = node->mNumMeshes;
    const size_t stride = sizeof(AssetVertex);
    float matrix4x4[16];
    Program *program = m_context->assetPrograms[node];
    program->bind();
    const Scene::IMatrices *matrices = m_scene->matrices();
    matrices->getModelViewProjection(matrix4x4);
    program->setModelViewProjectionMatrix(matrix4x4);
    // program->setLightViewProjectionMatrix(m_scene->lightViewProjectionMatrix());
    matrices->getNormal(matrix4x4);
    program->setNormalMatrix(matrix4x4);
    transform.getOpenGLMatrix(matrix4x4);
    program->setTransformMatrix(matrix4x4);
    const Scene::ILight *light = m_scene->light();
    program->setLightColor(light->color());
    program->setLightDirection(light->direction());
    for (unsigned int i = 0; i < nmeshes; i++) {
        const struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        const AssetVBO &vbo = m_context->vbo[mesh];
        const AssetIndices &indices = m_context->indices[mesh];
        setAssetMaterial(scene->mMaterials[mesh->mMaterialIndex], program);
        glBindBuffer(GL_ARRAY_BUFFER, vbo.vertices);
        program->setPosition(vertexPtr, stride);
        program->setNormal(normalPtr, stride);
        program->setTexCoord(texcoordPtr, stride);
        program->setColor(colorPtr, stride);
        program->setHasColor(mesh->HasVertexColors(0));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.indices);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    }
    program->unbind();
    const unsigned int nChildNodes = node->mNumChildren;
    for (unsigned int i = 0; i < nChildNodes; i++)
        renderRecurse(scene, node->mChildren[i]);
}

void AssetRenderEngine::renderZPlotRecurse(const aiScene *scene, const aiNode *node)
{
    vpvl::Asset *asset = m_model->ptr();
    const btScalar &scaleFactor = asset->scaleFactor();
    aiVector3D aiS, aiP;
    aiQuaternion aiQ;
    node->mTransformation.Decompose(aiS, aiQ, aiP);
    const vpvl::Vector3 scaleVector(aiS.x * scaleFactor, aiS.y * scaleFactor, aiS.z * scaleFactor);
    Transform transform(btMatrix3x3(Quaternion(aiQ.x, aiQ.y, aiQ.z, aiQ.w) * asset->rotation()).scaled(scaleVector),
                        Vector3(aiP.x,aiP.y, aiP.z) + asset->position());
    if (const vpvl::Bone *bone = asset->parentBone()) {
        const Transform &boneTransform = bone->localTransform();
        const btMatrix3x3 &boneBasis = boneTransform.getBasis();
        transform.setOrigin(boneTransform.getOrigin() + boneBasis * transform.getOrigin());
        transform.setBasis(boneBasis.scaled(scaleVector));
    }
    const GLvoid *vertexPtr = 0;
    const unsigned int nmeshes = node->mNumMeshes;
    const size_t stride = sizeof(AssetVertex);
    float matrix4x4[16];
    transform.getOpenGLMatrix(matrix4x4);
    ZPlotProgram *program = m_context->zplotPrograms[node];
    program->bind();
    // program->setModelViewProjectionMatrix(m_scene->lightViewProjectionMatrix());
    program->setTransformMatrix(matrix4x4);
    glCullFace(GL_FRONT);
    for (unsigned int i = 0; i < nmeshes; i++) {
        const struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        const AssetVBO &vbo = m_context->vbo[mesh];
        const AssetIndices &indices = m_context->indices[mesh];
        glBindBuffer(GL_ARRAY_BUFFER, vbo.vertices);
        program->setPosition(vertexPtr, stride);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.indices);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    }
    glCullFace(GL_BACK);
    program->unbind();
    const unsigned int nChildNodes = node->mNumChildren;
    for (unsigned int i = 0; i < nChildNodes; i++)
        renderZPlotRecurse(scene, node->mChildren[i]);
}

void AssetRenderEngine::log0(void *context, IRenderDelegate::LogLevel level, const char *format...)
{
    va_list ap;
    va_start(ap, format);
    m_delegate->log(context, level, format, ap);
    va_end(ap);
}

} /* namespace gl2 */
} /* namespace vpvl2 */

#endif
