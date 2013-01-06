/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
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

#include "vpvl2/cg/AssetRenderEngine.h"

#ifdef VPVL2_LINK_ASSIMP

#include "vpvl2/vpvl2.h"
#include "vpvl2/asset/Model.h"

namespace vpvl2
{
namespace cg
{

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

class AssetRenderEngine::PrivateEffectEngine : public EffectEngine {
public:
    PrivateEffectEngine(AssetRenderEngine *renderEngine,
                        Effect *effectRef,
                        const IString *dir,
                        bool isDefaultStandardEffect)
        : EffectEngine(renderEngine->sceneRef(),
                       effectRef,
                       renderEngine->renderContextRef(),
                       dir,
                       isDefaultStandardEffect),
          m_parentRenderEngine(renderEngine)
    {
    }

    void setMesh(const aiMesh *value) {
        m_mesh = value;
    }

protected:
    void drawPrimitives(const GLenum mode, const GLsizei count, const GLenum type, const GLvoid *ptr) const {
        glDrawElements(mode, count, type, ptr);
    }
    void rebindVertexBundle() {
        m_parentRenderEngine->bindVertexBundle(m_mesh);
    }

private:
    AssetRenderEngine *m_parentRenderEngine;
    const aiMesh *m_mesh;

    VPVL2_DISABLE_COPY_AND_ASSIGN(PrivateEffectEngine)
};

AssetRenderEngine::AssetRenderEngine(IRenderContext *renderContext, Scene *scene, asset::Model *model)
    : m_currentEffectEngineRef(0),
      m_renderContextRef(renderContext),
      m_sceneRef(scene),
      m_modelRef(model),
      m_bundle(renderContext),
      m_nvertices(0),
      m_nmeshes(0),
      m_cullFaceState(true)
{
}

AssetRenderEngine::~AssetRenderEngine()
{
    const aiScene *scene = m_modelRef->aiScenePtr();
    if (scene) {
        const unsigned int nmaterials = scene->mNumMaterials;
        std::string texture, mainTexture, subTexture;
        aiString texturePath;
        for (unsigned int i = 0; i < nmaterials; i++) {
            aiMaterial *material = scene->mMaterials[i];
            aiReturn found = AI_SUCCESS;
            GLuint textureID;
            int textureIndex = 0;
            while (found == AI_SUCCESS) {
                found = material->GetTexture(aiTextureType_DIFFUSE, textureIndex, &texturePath);
                if (found != AI_SUCCESS)
                    break;
                texture = texturePath.data;
                if (SplitTexturePath(texture, mainTexture, subTexture)) {
                    Textures::const_iterator sub = m_textures.find(subTexture);
                    if (sub != m_textures.end()) {
                        textureID = sub->second;
                        glDeleteTextures(1, &textureID);
                        m_textures.erase(subTexture);
                    }
                }
                Textures::const_iterator main = m_textures.find(mainTexture);
                if (main != m_textures.end()) {
                    textureID = main->second;
                    glDeleteTextures(1, &textureID);
                    m_textures.erase(mainTexture);
                }
                textureIndex++;
            }
        }
        deleteRecurse(scene, scene->mRootNode);
    }
    m_effectEngines.releaseAll();
    m_oseffects.releaseAll();
    m_currentEffectEngineRef = 0;
    m_modelRef = 0;
    m_renderContextRef = 0;
    m_sceneRef = 0;
    m_nvertices = 0;
    m_nmeshes = 0;
    m_cullFaceState = false;
}

IModel *AssetRenderEngine::parentModelRef() const
{
    return m_modelRef && m_modelRef->parentSceneRef() ? m_modelRef : 0;
}

bool AssetRenderEngine::upload(const IString *dir)
{
    bool ret = true;
    const aiScene *scene = m_modelRef->aiScenePtr();
    if (!scene)
        return true;
    void *userData = 0;
    m_renderContextRef->allocateUserData(m_modelRef, userData);
    m_renderContextRef->startProfileSession(IRenderContext::kProfileUploadModelProcess, m_modelRef);
    const unsigned int nmaterials = scene->mNumMaterials;
    aiString texturePath;
    std::string path, mainTexture, subTexture;
    IRenderContext::Texture texture;
    GLuint textureID;
    int flags = IRenderContext::kTexture2D;
    PrivateEffectEngine *engine = 0;
    if (PrivateEffectEngine *const *enginePtr = m_effectEngines.find(IEffect::kStandard)) {
        engine = *enginePtr;
        flags |= engine->materialTexture.isMipmapEnabled() ? IRenderContext::kGenerateTextureMipmap : 0;
    }
    for (unsigned int i = 0; i < nmaterials; i++) {
        aiMaterial *material = scene->mMaterials[i];
        aiReturn found = AI_SUCCESS;
        int textureIndex = 0;
        while (found == AI_SUCCESS) {
            found = material->GetTexture(aiTextureType_DIFFUSE, textureIndex, &texturePath);
            path = texturePath.data;
            if (SplitTexturePath(path, mainTexture, subTexture)) {
                if (m_textures[mainTexture] == 0) {
                    IString *mainTexturePath = m_renderContextRef->toUnicode(reinterpret_cast<const uint8_t *>(mainTexture.c_str()));
                    if (m_renderContextRef->uploadTexture(mainTexturePath, dir, flags, texture, userData)) {
                        m_textures[mainTexture] = textureID = static_cast<GLuint>(texture.object);
                        if (engine)
                            engine->materialTexture.setTexture(material, textureID);
                        log0(userData, IRenderContext::kLogInfo, "Loaded a main texture: %s (ID=%d)", mainTexturePath->toByteArray(), textureID);
                    }
                    delete mainTexturePath;
                }
                if (m_textures[subTexture] == 0) {
                    IString *subTexturePath = m_renderContextRef->toUnicode(reinterpret_cast<const uint8_t *>(subTexture.c_str()));
                    if (m_renderContextRef->uploadTexture(subTexturePath, dir, IRenderContext::kTexture2D, texture, userData)) {
                        m_textures[subTexture] = textureID = static_cast<GLuint>(texture.object);
                        if (engine)
                            engine->materialSphereMap.setTexture(material, textureID);
                        log0(userData, IRenderContext::kLogInfo, "Loaded a sub texture: %s (ID=%d)", subTexturePath->toByteArray(), textureID);
                    }
                    delete subTexturePath;
                }
            }
            else if (m_textures[mainTexture] == 0) {
                IString *mainTexturePath = m_renderContextRef->toUnicode(reinterpret_cast<const uint8_t *>(mainTexture.c_str()));
                if (m_renderContextRef->uploadTexture(mainTexturePath, dir, IRenderContext::kTexture2D, texture, userData)) {
                    m_textures[mainTexture] = textureID = static_cast<const GLuint>(texture.object);
                    if (engine)
                        engine->materialTexture.setTexture(material, textureID);
                    log0(userData, IRenderContext::kLogInfo, "Loaded a main texture: %s (ID=%d)", mainTexturePath->toByteArray(), textureID);
                }
                delete mainTexturePath;
            }
            textureIndex++;
        }
    }
    ret = uploadRecurse(scene, scene->mRootNode, userData);
    m_modelRef->setVisible(ret);
    m_renderContextRef->stopProfileSession(IRenderContext::kProfileUploadModelProcess, m_modelRef);
    m_renderContextRef->releaseUserData(m_modelRef, userData);
    return ret;
}

void AssetRenderEngine::update()
{
    if (m_currentEffectEngineRef) {
        m_currentEffectEngineRef->useToon.setValue(false);
        m_currentEffectEngineRef->parthf.setValue(false);
        m_currentEffectEngineRef->transp.setValue(false);
        m_currentEffectEngineRef->opadd.setValue(false);
        m_currentEffectEngineRef->subsetCount.setValue(m_nmeshes);
        m_currentEffectEngineRef->vertexCount.setValue(m_nvertices);
        m_currentEffectEngineRef->updateModelGeometryParameters(m_sceneRef, m_modelRef);
        m_currentEffectEngineRef->updateSceneParameters();
    }
}

void AssetRenderEngine::renderModel()
{
    if (!m_modelRef)
        return;
    if (!m_modelRef->aiScenePtr() && m_currentEffectEngineRef && m_currentEffectEngineRef->isStandardEffect()) {
        m_currentEffectEngineRef->executeProcess(0, 0, IEffect::kStandard);
        return;
    }
    if (!m_modelRef->isVisible() || !m_currentEffectEngineRef || !m_currentEffectEngineRef->isStandardEffect())
        return;
    if (btFuzzyZero(m_modelRef->opacity()))
        return;
    m_renderContextRef->startProfileSession(IRenderContext::kProfileRenderModelProcess, m_modelRef);
    const ILight *light = m_sceneRef->light();
    const GLuint *depthTexturePtr = static_cast<const GLuint *>(light->depthTexture());
    if (depthTexturePtr && light->hasFloatTexture()) {
        const GLuint depthTexture = *depthTexturePtr;
        m_currentEffectEngineRef->depthTexture.setTexture(depthTexturePtr, depthTexture);
    }
    m_currentEffectEngineRef->setModelMatrixParameters(m_modelRef);
    const aiScene *a = m_modelRef->aiScenePtr();
    renderRecurse(a, a->mRootNode, depthTexturePtr ? true : false);
    if (!m_cullFaceState) {
        glEnable(GL_CULL_FACE);
        m_cullFaceState = true;
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
    if (!m_modelRef || !m_modelRef->isVisible() || !m_currentEffectEngineRef || m_currentEffectEngineRef->scriptOrder() != IEffect::kStandard)
        return;
    if (btFuzzyZero(m_modelRef->opacity()))
        return;
    m_renderContextRef->startProfileSession(IRenderContext::kProfileRenderZPlotProcess, m_modelRef);
    m_currentEffectEngineRef->setModelMatrixParameters(m_modelRef);
    const aiScene *a = m_modelRef->aiScenePtr();
    glDisable(GL_CULL_FACE);
    renderZPlotRecurse(a, a->mRootNode);
    glEnable(GL_CULL_FACE);
    m_renderContextRef->stopProfileSession(IRenderContext::kProfileRenderZPlotProcess, m_modelRef);
}

bool AssetRenderEngine::hasPreProcess() const
{
    return m_currentEffectEngineRef ? m_currentEffectEngineRef->hasTechniques(IEffect::kPreProcess) : false;
}

bool AssetRenderEngine::hasPostProcess() const
{
    return m_currentEffectEngineRef ? m_currentEffectEngineRef->hasTechniques(IEffect::kPostProcess) : false;
}

void AssetRenderEngine::preparePostProcess()
{
    if (m_currentEffectEngineRef)
        m_currentEffectEngineRef->executeScriptExternal();
}

void AssetRenderEngine::performPreProcess()
{
    if (m_currentEffectEngineRef)
        m_currentEffectEngineRef->executeProcess(m_modelRef, 0, IEffect::kPreProcess);
}

void AssetRenderEngine::performPostProcess(IEffect *nextPostEffect)
{
    if (m_currentEffectEngineRef)
        m_currentEffectEngineRef->executeProcess(m_modelRef, nextPostEffect, IEffect::kPostProcess);
}

IEffect *AssetRenderEngine::effect(IEffect::ScriptOrderType type) const
{
    const PrivateEffectEngine *const *ee = m_effectEngines.find(type);
    return ee ? (*ee)->effect() : 0;
}

void AssetRenderEngine::setEffect(IEffect::ScriptOrderType type, IEffect *effect, const IString *dir)
{
    Effect *effectRef = static_cast<Effect *>(effect);
    if (type == IEffect::kStandardOffscreen) {
        const int neffects = m_oseffects.count();
        bool found = false;
        PrivateEffectEngine *ee = 0;
        for (int i = 0; i < neffects; i++) {
            ee = m_oseffects[i];
            if (ee->effect() == effectRef) {
                found = true;
                break;
            }
        }
        if (found) {
            m_currentEffectEngineRef = ee;
        }
        else if (effectRef) {
            PrivateEffectEngine *previous = m_currentEffectEngineRef;
            m_currentEffectEngineRef = new PrivateEffectEngine(this, effectRef, dir, false);
            if (m_currentEffectEngineRef->scriptOrder() == IEffect::kStandard) {
                m_oseffects.add(m_currentEffectEngineRef);
            }
            else {
                delete m_currentEffectEngineRef;
                m_currentEffectEngineRef = previous;
            }
        }
    }
    else {
        IEffect::ScriptOrderType findType = (type == IEffect::kAutoDetection && effectRef) ? effectRef->scriptOrderType() : type;
        if (PrivateEffectEngine *const *ee = m_effectEngines.find(findType)) {
            m_currentEffectEngineRef = *ee;
        }
        else {
            /* set default standard effect if effect is null */
            bool wasEffectNull = false;
            if (!effectRef) {
                effectRef = static_cast<Effect *>(m_sceneRef->createDefaultStandardEffectRef(m_renderContextRef));
                wasEffectNull = true;
            }
            m_currentEffectEngineRef = new PrivateEffectEngine(this, effectRef, dir, wasEffectNull);
            m_effectEngines.insert(type == IEffect::kAutoDetection ? m_currentEffectEngineRef->scriptOrder() : type, m_currentEffectEngineRef);
            /* set default standard effect as secondary effect */
            if (!wasEffectNull && m_currentEffectEngineRef->scriptOrder() == IEffect::kStandard) {
                m_currentEffectEngineRef->setDefaultStandardEffectRef(m_sceneRef->createDefaultStandardEffectRef(m_renderContextRef));
            }
        }
    }
}

void AssetRenderEngine::bindVertexBundle(const aiMesh *mesh)
{
    m_currentEffectEngineRef->setMesh(mesh);
    if (!m_bundle.bindVertexArrayObject(m_vao[mesh])) {
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo[mesh]);
        bindStaticVertexAttributePointers();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo[mesh]);
    }
}

void AssetRenderEngine::log0(void *userData, IRenderContext::LogLevel level, const char *format ...)
{
    va_list ap;
    va_start(ap, format);
    m_renderContextRef->log(userData, level, format, ap);
    va_end(ap);
}

bool AssetRenderEngine::uploadRecurse(const aiScene *scene, const aiNode *node, void *userData)
{
    const unsigned int nmeshes = node->mNumMeshes;
    bool ret = true;
    m_nmeshes = nmeshes;
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
                vertexIndices.add(vertexIndex);
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
            assetVertices.add(assetVertex);
        }
        createVertexBundle(mesh, assetVertices, vertexIndices, userData);
        assetVertices.clear();
        vertexIndices.clear();
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    const unsigned int nChildNodes = node->mChildren ? node->mNumChildren : 0;
    for (unsigned int i = 0; i < nChildNodes; i++) {
        ret = uploadRecurse(scene, node->mChildren[i], userData);
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
        m_bundle.releaseVertexArrayObjects(&m_vao[mesh], 1);
        glDeleteBuffers(1, &m_ibo[mesh]);
        glDeleteBuffers(1, &m_vbo[mesh]);
    }
    const unsigned int nChildNodes = node->mChildren ? node->mNumChildren : 0;
    for (unsigned int i = 0; i < nChildNodes; i++)
        deleteRecurse(scene, node->mChildren[i]);
}

void AssetRenderEngine::renderRecurse(const aiScene *scene, const aiNode *node, const bool hasShadowMap)
{
    const unsigned int nmeshes = node->mNumMeshes;
    for (unsigned int i = 0; i < nmeshes; i++) {
        const struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        bool hasTexture = false, hasSphereMap = false;
        const char *target = hasShadowMap ? "object_ss" : "object";
        setAssetMaterial(scene->mMaterials[mesh->mMaterialIndex], hasTexture, hasSphereMap);
        CGtechnique technique = m_currentEffectEngineRef->findTechnique(target, i, nmeshes, hasTexture, hasSphereMap, false);
        size_t nindices = m_indices[mesh];
        if (cgIsTechnique(technique)) {
            bindVertexBundle(mesh);
            m_renderContextRef->startProfileSession(IRenderContext::kProfileRenderModelMaterialDrawCall, mesh);
            m_currentEffectEngineRef->executeTechniquePasses(technique, 0, GL_TRIANGLES, nindices, GL_UNSIGNED_INT, 0);
            m_renderContextRef->stopProfileSession(IRenderContext::kProfileRenderModelMaterialDrawCall, mesh);
        }
    }
    unbindVertexBundle();
    const unsigned int nChildNodes = node->mChildren ? node->mNumChildren : 0;
    for (unsigned int i = 0; i < nChildNodes; i++)
        renderRecurse(scene, node->mChildren[i], hasShadowMap);
}

void AssetRenderEngine::renderZPlotRecurse(const aiScene *scene, const aiNode *node)
{
    const unsigned int nmeshes = node->mNumMeshes;
    float opacity;
    for (unsigned int i = 0; i < nmeshes; i++) {
        const struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        const struct aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        bool succeeded = aiGetMaterialFloat(material, AI_MATKEY_OPACITY, &opacity) == aiReturn_SUCCESS;
        if (succeeded && btFuzzyZero(opacity - 0.98f))
            continue;
        bindVertexBundle(mesh);
        CGtechnique technique = m_currentEffectEngineRef->findTechnique("zplot", i, nmeshes, false, false, false);
        size_t nindices = m_indices[mesh];
        if (cgIsTechnique(technique)) {
            m_renderContextRef->startProfileSession(IRenderContext::kProfileRenderZPlotMaterialDrawCall, mesh);
            m_currentEffectEngineRef->executeTechniquePasses(technique, 0, GL_TRIANGLES, nindices, GL_UNSIGNED_INT, 0);
            m_renderContextRef->stopProfileSession(IRenderContext::kProfileRenderZPlotMaterialDrawCall, mesh);
        }
    }
    unbindVertexBundle();
    const unsigned int nChildNodes = node->mChildren ? node->mNumChildren : 0;
    for (unsigned int i = 0; i < nChildNodes; i++)
        renderZPlotRecurse(scene, node->mChildren[i]);
}

void AssetRenderEngine::setAssetMaterial(const aiMaterial *material, bool &hasTexture, bool &hasSphereMap)
{
    int textureIndex = 0;
    GLuint textureID;
    std::string mainTexture, subTexture;
    aiString texturePath;
    hasTexture = false;
    hasSphereMap = false;
    if (material->GetTexture(aiTextureType_DIFFUSE, textureIndex, &texturePath) == aiReturn_SUCCESS) {
        bool isAdditive = false;
        if (SplitTexturePath(texturePath.data, mainTexture, subTexture)) {
            textureID = m_textures[subTexture];
            isAdditive = subTexture.find(".spa") != std::string::npos;
            m_currentEffectEngineRef->spadd.setValue(isAdditive);
            m_currentEffectEngineRef->useSpheremap.setValue(true);
            hasSphereMap = true;
        }
        textureID = m_textures[mainTexture];
        if (textureID > 0) {
            m_currentEffectEngineRef->useTexture.setValue(true);
            hasTexture = true;
        }
    }
    else {
        m_currentEffectEngineRef->useTexture.setValue(false);
        m_currentEffectEngineRef->useSpheremap.setValue(false);
    }
    m_currentEffectEngineRef->materialTexture.updateParameter(material);
    m_currentEffectEngineRef->materialSphereMap.updateParameter(material);
    // * ambient = diffuse
    // * specular / 10
    // * emissive
    aiColor4D ambient, diffuse, specular;
    Color color;
    if (aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &ambient) == aiReturn_SUCCESS) {
        color.setValue(ambient.r, ambient.g, ambient.b, ambient.a);
    }
    else {
        color.setValue(0, 0, 0, 1);
    }
    m_currentEffectEngineRef->emissive.setGeometryColor(color);
    if (aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse) == aiReturn_SUCCESS) {
        color.setValue(diffuse.r, diffuse.g, diffuse.b, diffuse.a * m_modelRef->opacity());
    }
    else {
        color.setValue(0, 0, 0, m_modelRef->opacity());
    }
    m_currentEffectEngineRef->ambient.setGeometryColor(color);
    m_currentEffectEngineRef->diffuse.setGeometryColor(color);
    if (aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &specular) == aiReturn_SUCCESS) {
        static const float kDivide = 10.0;
        color.setValue(specular.r / kDivide, specular.g / kDivide, specular.b / kDivide, specular.a);
    }
    else {
        color.setValue(0, 0, 0, 1);
    }
    m_currentEffectEngineRef->specular.setGeometryColor(color);
    float shininess, strength;
    int ret1 = aiGetMaterialFloat(material, AI_MATKEY_SHININESS, &shininess);
    int ret2 = aiGetMaterialFloat(material, AI_MATKEY_SHININESS_STRENGTH, &strength);
    if (ret1 == aiReturn_SUCCESS && ret2 == aiReturn_SUCCESS) {
        m_currentEffectEngineRef->specularPower.setGeometryValue(shininess * strength);
    }
    else if (ret1 == aiReturn_SUCCESS) {
        m_currentEffectEngineRef->specularPower.setGeometryValue(shininess);
    }
    else {
        m_currentEffectEngineRef->specularPower.setGeometryValue(1);
    }
    int twoside;
    if (aiGetMaterialInteger(material, AI_MATKEY_TWOSIDED, &twoside) == aiReturn_SUCCESS && twoside && !m_cullFaceState) {
        glEnable(GL_CULL_FACE);
        m_cullFaceState = true;
    }
    else if (m_cullFaceState) {
        glDisable(GL_CULL_FACE);
        m_cullFaceState = false;
    }
}

void AssetRenderEngine::createVertexBundle(const aiMesh *mesh,
                                           const Vertices &vertices,
                                           const Indices &indices,
                                           void *userData)
{
    GLuint &ibo = m_ibo[mesh];
    size_t isize = sizeof(indices[0]) * indices.count();
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, isize, &indices[0], GL_STATIC_DRAW);
    log0(userData, IRenderContext::kLogInfo,
         "Binding asset index buffer to the vertex buffer object (ID=%d)", ibo);
    GLuint &vbo = m_vbo[mesh];
    size_t vsize = vertices.count() * sizeof(vertices[0]);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vsize, &vertices[0].position, GL_STATIC_DRAW);
    log0(userData, IRenderContext::kLogInfo,
         "Binding asset vertex buffer to the vertex buffer object (ID=%d)", vbo);
    GLuint &vao = m_vao[mesh];
    m_bundle.allocateVertexArrayObjects(&vao, 1);
    if (m_bundle.bindVertexArrayObject(vao)) {
        log0(userData, IRenderContext::kLogInfo, "Created an vertex array object (ID=%d)", vao);
    }
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    bindStaticVertexAttributePointers();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    m_bundle.unbindVertexArrayObject();
    m_indices[mesh] = indices.count();
}

void AssetRenderEngine::unbindVertexBundle()
{
    if (!m_bundle.unbindVertexArrayObject()) {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}

void AssetRenderEngine::bindStaticVertexAttributePointers()
{
    static const Vertex v;
    const void *vertexPtr = 0;
    glVertexPointer(3, GL_FLOAT, sizeof(v), vertexPtr);
    const void *normalPtr = reinterpret_cast<const void *>(reinterpret_cast<const uint8_t *>(&v.normal) - reinterpret_cast<const uint8_t *>(&v.position));
    glNormalPointer(GL_FLOAT, sizeof(v), normalPtr);
    const void *texcoordPtr = reinterpret_cast<const void *>(reinterpret_cast<const uint8_t *>(&v.texcoord) - reinterpret_cast<const uint8_t *>(&v.position));
    glTexCoordPointer(2, GL_FLOAT, sizeof(v), texcoordPtr);
}

} /* namespace cg */
} /* namespace vpvl2 */

#endif
