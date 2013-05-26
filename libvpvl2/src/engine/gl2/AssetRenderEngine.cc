/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2013  hkrn                                    */
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
#include "vpvl2/internal/util.h"

#ifdef VPVL2_LINK_ASSIMP
#include "EngineCommon.h"
#include "vpvl2/gl2/AssetRenderEngine.h"

#include "vpvl2/IBone.h"
#include "vpvl2/asset/Model.h"
#include "vpvl2/extensions/gl/VertexBundle.h"
#include "vpvl2/extensions/gl/VertexBundleLayout.h"

#include <map>

namespace vpvl2
{
namespace gl2
{
using namespace extensions::gl;

class AssetRenderEngine::Program : public ObjectProgram
{
public:
    Program()
        : ObjectProgram(),
          m_modelMatrixUniformLocation(0),
          m_viewProjectionMatrixUniformLocation(0),
          m_cameraPositionUniformLocation(0),
          m_materialColorUniformLocation(0),
          m_materialDiffuseUniformLocation(0),
          m_materialSpecularUniformLocation(0),
          m_materialShininessUniformLocation(0),
          m_hasSubTextureUniformLocation(0),
          m_isMainSphereMapUniformLocation(0),
          m_isSubSphereMapUniformLocation(0),
          m_isMainAdditiveUniformLocation(0),
          m_isSubAdditiveUniformLocation(0),
          m_subTextureUniformLocation(0)
    {
    }
    ~Program() {
        m_cameraPositionUniformLocation = 0;
        m_modelMatrixUniformLocation = 0;
        m_viewProjectionMatrixUniformLocation = 0;
        m_materialColorUniformLocation = 0;
        m_materialDiffuseUniformLocation = 0;
        m_materialSpecularUniformLocation = 0;
        m_materialShininessUniformLocation = 0;
        m_hasSubTextureUniformLocation = 0;
        m_isMainSphereMapUniformLocation = 0;
        m_isSubSphereMapUniformLocation = 0;
        m_isMainAdditiveUniformLocation = 0;
        m_isSubAdditiveUniformLocation = 0;
        m_subTextureUniformLocation = 0;
    }

    void setCameraPosition(const Vector3 &value) {
        glUniform3fv(m_cameraPositionUniformLocation, 1, value);
    }
    void setModelMatrix(const Scalar *value) {
        glUniformMatrix4fv(m_modelMatrixUniformLocation, 1, GL_FALSE, value);
    }
    void setViewProjectionMatrix(const Scalar *value) {
        glUniformMatrix4fv(m_viewProjectionMatrixUniformLocation, 1, GL_FALSE, value);
    }
    void setMaterialColor(const Color &value) {
        glUniform3fv(m_materialColorUniformLocation, 1, value);
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
    void setSubTexture(const ITexture *value) {
        if (value) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(value->data()));
            glUniform1i(m_subTextureUniformLocation, 1);
            glUniform1i(m_hasSubTextureUniformLocation, 1);
        }
        else {
            glUniform1i(m_hasSubTextureUniformLocation, 0);
        }
    }

protected:
    virtual void getUniformLocations() {
        ObjectProgram::getUniformLocations();
        m_cameraPositionUniformLocation = glGetUniformLocation(m_program, "cameraPosition");
        m_modelMatrixUniformLocation = glGetUniformLocation(m_program, "modelMatrix");
        m_viewProjectionMatrixUniformLocation = glGetUniformLocation(m_program, "viewProjectionMatrix");
        m_materialColorUniformLocation = glGetUniformLocation(m_program, "materialColor");
        m_materialDiffuseUniformLocation = glGetUniformLocation(m_program, "materialDiffuse");
        m_materialSpecularUniformLocation = glGetUniformLocation(m_program, "materialSpecular");
        m_materialShininessUniformLocation = glGetUniformLocation(m_program, "materialShininess");
        m_hasSubTextureUniformLocation = glGetUniformLocation(m_program, "hasSubTexture");
        m_isMainSphereMapUniformLocation = glGetUniformLocation(m_program, "isMainSphereMap");
        m_isSubSphereMapUniformLocation = glGetUniformLocation(m_program, "isSubSphereMap");
        m_isMainAdditiveUniformLocation = glGetUniformLocation(m_program, "isMainAdditive");
        m_isSubAdditiveUniformLocation = glGetUniformLocation(m_program, "isSubAdditive");
        m_subTextureUniformLocation = glGetUniformLocation(m_program, "subTexture");
    }

private:
    GLuint m_modelMatrixUniformLocation;
    GLuint m_viewProjectionMatrixUniformLocation;
    GLuint m_cameraPositionUniformLocation;
    GLuint m_materialColorUniformLocation;
    GLuint m_materialDiffuseUniformLocation;
    GLuint m_materialSpecularUniformLocation;
    GLuint m_materialShininessUniformLocation;
    GLuint m_hasSubTextureUniformLocation;
    GLuint m_isMainSphereMapUniformLocation;
    GLuint m_isSubSphereMapUniformLocation;
    GLuint m_isMainAdditiveUniformLocation;
    GLuint m_isSubAdditiveUniformLocation;
    GLuint m_subTextureUniformLocation;
};

class AssetRenderEngine::PrivateContext
{
public:
    typedef std::map<std::string, ITexture *> Textures;
    PrivateContext()
        : cullFaceState(true)
    {
    }
    virtual ~PrivateContext() {
        allocatedTextures.releaseAll();
    }

    Textures textures;
    PointerHash<HashPtr, ITexture> allocatedTextures;
    std::map<const struct aiMesh *, int> indices;
    std::map<const struct aiMesh *, VertexBundle *> vbo;
    std::map<const struct aiMesh *, VertexBundleLayout *> vao;
    std::map<const struct aiNode *, AssetRenderEngine::Program *> assetPrograms;
    std::map<const struct aiNode *, ZPlotProgram *> zplotPrograms;
    bool cullFaceState;
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

bool SplitTexturePath(const std::string &path, std::string &mainTexture, std::string &subTexture)
{
    std::string::size_type pos = path.find_first_of("*");
    if (pos != std::string::npos) {
        mainTexture = CanonicalizePath(path.substr(0, pos));
        subTexture  = CanonicalizePath(path.substr(pos + 1));
        return true;
    }
    else {
        mainTexture = CanonicalizePath(path);
        subTexture = "";
        return false;
    }
}

AssetRenderEngine::AssetRenderEngine(IRenderContext *renderContext, Scene *scene, asset::Model *model)
    : m_renderContextRef(renderContext),
      m_sceneRef(scene),
      m_modelRef(model),
      m_context(0)
{
    m_context = new PrivateContext();
}

AssetRenderEngine::~AssetRenderEngine()
{
    if (m_modelRef) {
        if (const aiScene *scene = m_modelRef->aiScenePtr()) {
            deleteRecurse(scene, scene->mRootNode);
        }
    }
    delete m_context;
    m_context = 0;
    m_modelRef = 0;
    m_renderContextRef = 0;
    m_sceneRef = 0;
}

void AssetRenderEngine::renderModel()
{
    if (!m_modelRef || !m_modelRef->isVisible())
        return;
    m_renderContextRef->startProfileSession(IRenderContext::kProfileRenderModelProcess, m_modelRef);
    const aiScene *a = m_modelRef->aiScenePtr();
    renderRecurse(a, a->mRootNode);
    if (!m_context->cullFaceState) {
        glEnable(GL_CULL_FACE);
        m_context->cullFaceState = true;
    }
    m_renderContextRef->stopProfileSession(IRenderContext::kProfileRenderModelProcess, m_modelRef);
}

void AssetRenderEngine::renderEdge()
{
    /* do nothing */
}

void AssetRenderEngine::renderShadow()
{
    /* do nothing */
}

void AssetRenderEngine::renderZPlot()
{
    if (!m_modelRef || !m_modelRef->isVisible())
        return;
    m_renderContextRef->startProfileSession(IRenderContext::kProfileRenderModelProcess, m_modelRef);
    const aiScene *a = m_modelRef->aiScenePtr();
    glDisable(GL_CULL_FACE);
    renderZPlotRecurse(a, a->mRootNode);
    glEnable(GL_CULL_FACE);
    m_renderContextRef->stopProfileSession(IRenderContext::kProfileRenderModelProcess, m_modelRef);
}

IModel *AssetRenderEngine::parentModelRef() const
{
    return m_modelRef && m_modelRef->parentSceneRef() ? m_modelRef : 0;
}

bool AssetRenderEngine::upload(const IString *dir)
{
    if (!m_modelRef)
        return false;
    const aiScene *scene = m_modelRef->aiScenePtr();
    if (!scene)
        return false;
    bool ret = true;
    void *userData = 0;
    m_renderContextRef->allocateUserData(m_modelRef, userData);
    m_renderContextRef->startProfileSession(IRenderContext::kProfileUploadModelProcess, m_modelRef);
    const unsigned int nmaterials = scene->mNumMaterials;
    aiString texturePath;
    std::string path, mainTexture, subTexture;
    IRenderContext::Texture texture(IRenderContext::kTexture2D);
    for (unsigned int i = 0; i < nmaterials; i++) {
        aiMaterial *material = scene->mMaterials[i];
        aiReturn found = AI_SUCCESS;
        int textureIndex = 0;
        while (found == AI_SUCCESS) {
            found = material->GetTexture(aiTextureType_DIFFUSE, textureIndex, &texturePath);
            path = texturePath.data;
            if (SplitTexturePath(path, mainTexture, subTexture)) {
                if (m_context->textures[mainTexture] == 0) {
                    IString *mainTexturePath = m_renderContextRef->toUnicode(reinterpret_cast<const uint8_t *>(mainTexture.c_str()));
                    ret = m_renderContextRef->uploadTexture(mainTexturePath, dir, texture, userData);
                    if (ret) {
                        ITexture *textureRef = texture.texturePtrRef;
                        m_context->textures[mainTexture] = m_context->allocatedTextures.insert(textureRef, textureRef);
                        VPVL2_VLOG(2, "Loaded a main texture: name=" << internal::cstr(mainTexturePath, "(null)") << " ID=" << textureRef);
                        delete mainTexturePath;
                    }
                    else {
                        delete mainTexturePath;
                        m_renderContextRef->releaseUserData(m_modelRef, userData);
                        return ret;
                    }
                }
                if (m_context->textures[subTexture] == 0) {
                    IString *subTexturePath = m_renderContextRef->toUnicode(reinterpret_cast<const uint8_t *>(subTexture.c_str()));
                    ret = m_renderContextRef->uploadTexture(subTexturePath, dir, texture, userData);
                    if (ret) {
                        ITexture *textureRef = texture.texturePtrRef;
                        m_context->textures[subTexture] = m_context->allocatedTextures.insert(textureRef, textureRef);
                        VPVL2_VLOG(2, "Loaded a sub texture: name=" << internal::cstr(subTexturePath, "(null)") << " ID=" << textureRef);
                        delete subTexturePath;
                    }
                    else {
                        delete subTexturePath;
                        m_renderContextRef->releaseUserData(m_modelRef, userData);
                        return ret;
                    }
                }
            }
            else if (m_context->textures[mainTexture] == 0) {
                IString *mainTexturePath = m_renderContextRef->toUnicode(reinterpret_cast<const uint8_t *>(mainTexture.c_str()));
                ret = m_renderContextRef->uploadTexture(mainTexturePath, dir, texture, userData);
                if (ret) {
                    ITexture *textureRef = texture.texturePtrRef;
                    m_context->textures[mainTexture] = m_context->allocatedTextures.insert(textureRef, textureRef);
                    VPVL2_VLOG(2, "Loaded a main texture: name=" << internal::cstr(mainTexturePath, "(null)") << " ID=" << textureRef);
                    delete mainTexturePath;
                }
                else {
                    delete mainTexturePath;
                    m_renderContextRef->releaseUserData(m_modelRef, userData);
                    return ret;
                }
            }
            textureIndex++;
        }
    }
    ret = uploadRecurse(scene, scene->mRootNode, dir, userData);
    m_modelRef->setVisible(ret);
    m_renderContextRef->stopProfileSession(IRenderContext::kProfileUploadModelProcess, m_modelRef);
    m_renderContextRef->releaseUserData(m_modelRef, userData);
    return ret;
}

void AssetRenderEngine::update()
{
    /* do nothing */
}

void AssetRenderEngine::setUpdateOptions(int /* options */)
{
    /* do nothing */
}

bool AssetRenderEngine::hasPreProcess() const
{
    return false;
}

bool AssetRenderEngine::hasPostProcess() const
{
    return false;
}

void AssetRenderEngine::preparePostProcess()
{
    /* do nothing */
}

void AssetRenderEngine::performPreProcess()
{
    /* do nothing */
}

void AssetRenderEngine::performPostProcess(IEffect * /* nextPostEffect */)
{
    /* do nothing */
}

IEffect *AssetRenderEngine::effectRef(IEffect::ScriptOrderType /* type */) const
{
    return 0;
}

void AssetRenderEngine::setEffect(IEffect::ScriptOrderType /* type */, IEffect * /* effect */, const IString * /* dir */)
{
    /* do nothing */
}

bool AssetRenderEngine::uploadRecurse(const aiScene *scene, const aiNode *node, const IString *dir, void *userData)
{
    bool ret = true;
    const unsigned int nmeshes = node->mNumMeshes;
    Program *assetProgram = m_context->assetPrograms[node] = new Program();
    if (!createProgram(assetProgram,
                       dir,
                       IRenderContext::kModelVertexShader,
                       IRenderContext::kModelFragmentShader,
                       userData)) {
        return ret;
    }
    ZPlotProgram *zplotProgram = m_context->zplotPrograms[node] = new ZPlotProgram();
    if (!createProgram(zplotProgram,
                       dir,
                       IRenderContext::kZPlotVertexShader,
                       IRenderContext::kZPlotFragmentShader,
                       userData)) {
        return ret;
    }
    Vertices assetVertices;
    Vertex assetVertex;
    Array<int> vertexIndices;
    for (unsigned int i = 0; i < nmeshes; i++) {
        const struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        const unsigned int nfaces = mesh->mNumFaces;
        for (unsigned int j = 0; j < nfaces; j++) {
            const struct aiFace &face = mesh->mFaces[j];
            const unsigned int nindices = face.mNumIndices;
            for (unsigned int k = 0; k < nindices; k++) {
                int vertexIndex = face.mIndices[k];
                vertexIndices.append(vertexIndex);
            }
        }
        const bool hasNormals = mesh->HasNormals();
        const bool hasTexCoords = mesh->HasTextureCoords(0);
        const aiVector3D *vertices = mesh->mVertices;
        const aiVector3D *normals = hasNormals ? mesh->mNormals : 0;
        const aiVector3D *texcoords = hasTexCoords ? mesh->mTextureCoords[0] : 0;
        const unsigned int nvertices = mesh->mNumVertices;
        for (unsigned int j = 0; j < nvertices; j++) {
            const aiVector3D &vertex = vertices[j];
            assetVertex.position.setValue(vertex.x, vertex.y, vertex.z, 1);
            if (normals) {
                const aiVector3D &normal = normals[j];
                assetVertex.normal.setValue(normal.x, normal.y, normal.z);
            }
            if (texcoords) {
                const aiVector3D &texcoord = texcoords[j];
                assetVertex.texcoord.setValue(texcoord.x, texcoord.y, texcoord.z);
            }
            assetVertices.append(assetVertex);
        }
        createVertexBundle(mesh, assetVertices, vertexIndices);
        assetVertices.clear();
        vertexIndices.clear();
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    const unsigned int nChildNodes = node->mNumChildren;
    for (unsigned int i = 0; i < nChildNodes; i++) {
        ret = uploadRecurse(scene, node->mChildren[i], dir, userData);
        if (!ret)
            return ret;
    }
    return ret;
}

void AssetRenderEngine::deleteRecurse(const aiScene *scene, const aiNode *node)
{
    const unsigned int nmeshes = node->mNumMeshes;
    for (unsigned int i = 0; i < nmeshes; i++) {
        const struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        delete m_context->vao[mesh];
        delete m_context->vbo[mesh];
    }
    delete m_context->assetPrograms[node];
    delete m_context->zplotPrograms[node];
    const unsigned int nChildNodes = node->mChildren ? node->mNumChildren : 0;
    for (unsigned int i = 0; i < nChildNodes; i++)
        deleteRecurse(scene, node->mChildren[i]);
}

void AssetRenderEngine::setAssetMaterial(const aiMaterial *material, Program *program)
{
    int textureIndex = 0;
    const ITexture *textureRef = 0;
    std::string mainTexture, subTexture;
    aiString texturePath;
    if (material->GetTexture(aiTextureType_DIFFUSE, textureIndex, &texturePath) == aiReturn_SUCCESS) {
        bool isAdditive = false;
        if (SplitTexturePath(texturePath.data, mainTexture, subTexture)) {
            textureRef = m_context->textures[subTexture];
            isAdditive = subTexture.find(".spa") != std::string::npos;
            program->setSubTexture(textureRef);
            program->setIsSubAdditive(isAdditive);
            program->setIsSubSphereMap(isAdditive || subTexture.find(".sph") != std::string::npos);
        }
        textureRef = m_context->textures[mainTexture];
        isAdditive = mainTexture.find(".spa") != std::string::npos;
        program->setIsMainAdditive(isAdditive);
        program->setIsMainSphereMap(isAdditive || mainTexture.find(".sph") != std::string::npos);
        program->setMainTexture(textureRef);
    }
    else {
        program->setMainTexture(0);
        program->setSubTexture(0);
    }
    aiColor4D ambient, diffuse, specular;
    const Vector3 &lc = m_sceneRef->light()->color();
    Color la, mc, md, ms;
    aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &ambient);
    aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse);
    la.setValue(0.7f - lc.x(), 0.7f - lc.y(), 0.7f - lc.z(), 1.0);
    mc.setValue(diffuse.r * la.x() + ambient.r, diffuse.g * la.y() + ambient.g, diffuse.b * la.z() + ambient.b, diffuse.a);
    md.setValue(diffuse.r, diffuse.g, diffuse.b, diffuse.a);
    program->setMaterialColor(mc);
    program->setMaterialDiffuse(md);
    aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &specular);
    ms.setValue(specular.r * lc.x(), specular.g * lc.y(), specular.b * lc.z(), specular.a);
    program->setMaterialSpecular(ms);
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
        program->setOpacity(opacity * m_modelRef->opacity());
    }
    else {
        program->setOpacity(m_modelRef->opacity());
    }
    GLuint textureID = 0;
    if (const IShadowMap *shadowMap = m_sceneRef->shadowMapRef()) {
        const void *textureRef = shadowMap->textureRef();
        textureID = textureRef ? *static_cast<const GLuint *>(textureRef) : 0;
    }
    if (textureID && !btFuzzyZero(opacity - 0.98f)) {
        program->setDepthTexture(textureID);
    }
    else {
        program->setDepthTexture(0);
    }
    int twoside;
    if (aiGetMaterialInteger(material, AI_MATKEY_TWOSIDED, &twoside) == aiReturn_SUCCESS && twoside && !m_context->cullFaceState) {
        glEnable(GL_CULL_FACE);
        m_context->cullFaceState = true;
    }
    else if (m_context->cullFaceState) {
        glDisable(GL_CULL_FACE);
        m_context->cullFaceState = false;
    }
}

void AssetRenderEngine::renderRecurse(const aiScene *scene, const aiNode *node)
{
    const unsigned int nmeshes = node->mNumMeshes;
    float matrix4x4[16];
    Program *program = m_context->assetPrograms[node];
    program->bind();
    m_renderContextRef->getMatrix(matrix4x4, m_modelRef,
                                  IRenderContext::kViewMatrix
                                  | IRenderContext::kProjectionMatrix
                                  | IRenderContext::kCameraMatrix);
    program->setViewProjectionMatrix(matrix4x4);
    m_renderContextRef->getMatrix(matrix4x4, m_modelRef,
                                  IRenderContext::kWorldMatrix
                                  | IRenderContext::kViewMatrix
                                  | IRenderContext::kProjectionMatrix
                                  | IRenderContext::kLightMatrix);
    program->setLightViewProjectionMatrix(matrix4x4);
    m_renderContextRef->getMatrix(matrix4x4, m_modelRef,
                                  IRenderContext::kWorldMatrix
                                  | IRenderContext::kCameraMatrix);
    program->setModelMatrix(matrix4x4);
    const ILight *light = m_sceneRef->light();
    program->setLightColor(light->color());
    program->setLightDirection(light->direction());
    program->setOpacity(m_modelRef->opacity());
    program->setCameraPosition(m_sceneRef->camera()->lookAt());
    for (unsigned int i = 0; i < nmeshes; i++) {
        const struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        setAssetMaterial(scene->mMaterials[mesh->mMaterialIndex], program);
        bindVertexBundle(mesh);
        size_t nindices = m_context->indices[mesh];
        m_renderContextRef->startProfileSession(IRenderContext::kProfileRenderModelMaterialDrawCall, mesh);
        glDrawElements(GL_TRIANGLES, nindices, GL_UNSIGNED_INT, 0);
        m_renderContextRef->stopProfileSession(IRenderContext::kProfileRenderModelMaterialDrawCall, mesh);
    }
    unbindVertexBundle();
    program->unbind();
    const unsigned int nChildNodes = node->mNumChildren;
    for (unsigned int i = 0; i < nChildNodes; i++)
        renderRecurse(scene, node->mChildren[i]);
}

void AssetRenderEngine::renderZPlotRecurse(const aiScene *scene, const aiNode *node)
{
    static const Vertex v;
    float matrix4x4[16], opacity;
    const unsigned int nmeshes = node->mNumMeshes;
    Program *program = m_context->assetPrograms[node];
    program->bind();
    m_renderContextRef->getMatrix(matrix4x4, m_modelRef,
                                  IRenderContext::kWorldMatrix
                                  | IRenderContext::kViewMatrix
                                  | IRenderContext::kProjectionMatrix
                                  | IRenderContext::kCameraMatrix);
    program->setModelViewProjectionMatrix(matrix4x4);
    for (unsigned int i = 0; i < nmeshes; i++) {
        const struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        const struct aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        bool succeeded = aiGetMaterialFloat(material, AI_MATKEY_OPACITY, &opacity) == aiReturn_SUCCESS;
        if (succeeded && btFuzzyZero(opacity - 0.98f))
            continue;
        bindVertexBundle(mesh);
        size_t nindices = m_context->indices[mesh];
        m_renderContextRef->startProfileSession(IRenderContext::kProfileRenderZPlotMaterialDrawCall, mesh);
        glDrawElements(GL_TRIANGLES, nindices, GL_UNSIGNED_INT, 0);
        m_renderContextRef->stopProfileSession(IRenderContext::kProfileRenderZPlotMaterialDrawCall, mesh);
    }
    unbindVertexBundle();
    program->unbind();
    const unsigned int nChildNodes = node->mNumChildren;
    for (unsigned int i = 0; i < nChildNodes; i++)
        renderZPlotRecurse(scene, node->mChildren[i]);
}

bool AssetRenderEngine::createProgram(BaseShaderProgram *program,
                                      const IString *dir,
                                      IRenderContext::ShaderType vertexShaderType,
                                      IRenderContext::ShaderType fragmentShaderType,
                                      void *userData)
{
    IString *vertexShaderSource = 0;
    IString *fragmentShaderSource = 0;
    vertexShaderSource = m_renderContextRef->loadShaderSource(vertexShaderType, m_modelRef, dir, userData);
    fragmentShaderSource = m_renderContextRef->loadShaderSource(fragmentShaderType, m_modelRef, dir, userData);
    program->addShaderSource(vertexShaderSource, GL_VERTEX_SHADER);
    program->addShaderSource(fragmentShaderSource, GL_FRAGMENT_SHADER);
    bool ok = program->linkProgram();
    delete vertexShaderSource;
    delete fragmentShaderSource;
    return ok;
}

void AssetRenderEngine::createVertexBundle(const aiMesh *mesh,
                                           const Vertices &vertices,
                                           const Indices &indices)
{
    m_context->vao.insert(std::make_pair(mesh, new VertexBundleLayout()));
    m_context->vbo.insert(std::make_pair(mesh, new VertexBundle()));
    VertexBundle *bundle = m_context->vbo[mesh];
    size_t isize = sizeof(indices[0]) * indices.count();
    bundle->create(VertexBundle::kIndexBuffer, 0, GL_STATIC_DRAW, &indices[0], isize);
    VPVL2_VLOG(2, "Binding asset index buffer to the vertex buffer object");
    size_t vsize = vertices.count() * sizeof(vertices[0]);
    bundle->create(VertexBundle::kVertexBuffer, 0, GL_STATIC_DRAW, &vertices[0].position, vsize);
    VPVL2_VLOG(2, "Binding asset vertex buffer to the vertex buffer object");
    VertexBundleLayout *layout = m_context->vao[mesh];
    if (layout->create() && layout->bind()) {
        VPVL2_VLOG(2, "Created an vertex array object: " << layout->name());
    }
    bundle->bind(VertexBundle::kVertexBuffer, 0);
    bindStaticVertexAttributePointers();
    bundle->bind(VertexBundle::kIndexBuffer, 0);
    glEnableVertexAttribArray(IModel::Buffer::kVertexStride);
    glEnableVertexAttribArray(IModel::Buffer::kNormalStride);
    glEnableVertexAttribArray(IModel::Buffer::kTextureCoordStride);
    VertexBundleLayout::unbindVertexArrayObject();
    m_context->indices[mesh] = indices.count();
}

void AssetRenderEngine::bindVertexBundle(const aiMesh *mesh)
{
    if (!m_context->vao[mesh]->bind()) {
        VertexBundle *bundle = m_context->vbo[mesh];
        bundle->bind(VertexBundle::kVertexBuffer, 0);
        bindStaticVertexAttributePointers();
        bundle->bind(VertexBundle::kIndexBuffer, 0);
    }
}

void AssetRenderEngine::unbindVertexBundle()
{
    if (!VertexBundleLayout::unbindVertexArrayObject()) {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}

void AssetRenderEngine::bindStaticVertexAttributePointers()
{
    static const Vertex v;
    const void *vertexPtr = 0;
    glVertexAttribPointer(IModel::Buffer::kVertexStride, 3, GL_FLOAT, GL_FALSE, sizeof(v), vertexPtr);
    const void *normalPtr = reinterpret_cast<const void *>(reinterpret_cast<const uint8_t *>(&v.normal) - reinterpret_cast<const uint8_t *>(&v.position));
    glVertexAttribPointer(IModel::Buffer::kNormalStride, 3, GL_FLOAT, GL_FALSE, sizeof(v), normalPtr);
    const void *texcoordPtr = reinterpret_cast<const void *>(reinterpret_cast<const uint8_t *>(&v.texcoord) - reinterpret_cast<const uint8_t *>(&v.position));
    glVertexAttribPointer(IModel::Buffer::kTextureCoordStride, 2, GL_FLOAT, GL_FALSE, sizeof(v), texcoordPtr);
}

} /* namespace gl2 */
} /* namespace vpvl2 */

#endif
