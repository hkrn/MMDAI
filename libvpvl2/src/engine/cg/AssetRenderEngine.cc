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
#include "vpvl2/internal/util.h"

#ifdef VPVL2_LINK_ASSIMP

#include "vpvl2/vpvl2.h"
#include "vpvl2/asset/Model.h"
#include "vpvl2/extensions/gl/VertexBundle.h"
#include "vpvl2/extensions/gl/VertexBundleLayout.h"

#if !defined(VPVL2_LINK_GLEW) && defined(GL_ARB_draw_elements_base_vertex)
#define GLEW_ARB_draw_elements_base_vertex 1
#endif

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
        mainTexture.assign(CanonicalizePath(path.substr(0, pos)));
        subTexture.assign(CanonicalizePath(path.substr(pos + 1)));
        return true;
    }
    else {
        mainTexture.assign(CanonicalizePath(path));
        subTexture.assign(std::string());
        return false;
    }
}

class AssetRenderEngine::PrivateEffectEngine : public EffectEngine {
public:
    PrivateEffectEngine(AssetRenderEngine *renderEngine)
        : EffectEngine(renderEngine->sceneRef(), renderEngine->renderContextRef()),
          m_parentRenderEngine(renderEngine)
    {
    }

    void setMesh(const aiMesh *value) {
        m_mesh = value;
    }

protected:
    void drawPrimitives(const DrawPrimitiveCommand &command) const {
        if (GLEW_ARB_draw_elements_base_vertex) {
            glDrawElementsBaseVertex(command.mode, command.count, command.type,
                                     const_cast<uint8_t *>(command.ptr) + command.offset, 0);
        }
        else {
            glDrawElements(command.mode, command.count, command.type, command.ptr + command.offset);
        }
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
      m_defaultEffect(0),
      m_nvertices(0),
      m_nmeshes(0),
      m_cullFaceState(true)
{
}

AssetRenderEngine::~AssetRenderEngine()
{
    const aiScene *scene = m_modelRef->aiScenePtr();
    if (scene) {
        deleteRecurse(scene, scene->mRootNode);
    }
    Textures::const_iterator it = m_textures.begin();
    while (it != m_textures.end()) {
        delete it->second;
        it++;
    }
    m_oseffects.releaseAll();
    delete m_defaultEffect;
    m_defaultEffect = 0;
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
    IRenderContext::Texture texture(IRenderContext::kTexture2D);
    ITexture *textureRef = 0;
    PrivateEffectEngine *engine = 0;
    if (PrivateEffectEngine *const *enginePtr = m_effectEngines.find(IEffect::kStandard)) {
        engine = *enginePtr;
        texture.mipmap |= engine->materialTexture.isMipmapEnabled() ? true : false;
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
                    if (m_renderContextRef->uploadTexture(mainTexturePath, dir, texture, userData)) {
                        m_textures[mainTexture] = textureRef = texture.texturePtrRef;
                        if (engine) {
                            engine->materialTexture.setTexture(material, textureRef);
                        }
                        VPVL2_LOG(VLOG(2) << "Loaded a main texture: name=" << internal::cstr(mainTexturePath) << " ID=" << textureRef);
                    }
                    delete mainTexturePath;
                }
                if (m_textures[subTexture] == 0) {
                    IString *subTexturePath = m_renderContextRef->toUnicode(reinterpret_cast<const uint8_t *>(subTexture.c_str()));
                    if (m_renderContextRef->uploadTexture(subTexturePath, dir, texture, userData)) {
                        m_textures[subTexture] = textureRef = texture.texturePtrRef;
                        if (engine) {
                            engine->materialSphereMap.setTexture(material, textureRef);
                        }
                        VPVL2_LOG(VLOG(2) << "Loaded a sub texture: name=" << internal::cstr(subTexturePath) << " ID=" << textureRef);
                    }
                    delete subTexturePath;
                }
            }
            else if (m_textures[mainTexture] == 0) {
                IString *mainTexturePath = m_renderContextRef->toUnicode(reinterpret_cast<const uint8_t *>(mainTexture.c_str()));
                if (m_renderContextRef->uploadTexture(mainTexturePath, dir, texture, userData)) {
                    m_textures[mainTexture] = textureRef = texture.texturePtrRef;
                    if (engine) {
                        engine->materialTexture.setTexture(material, textureRef);
                    }
                    VPVL2_LOG(VLOG(2) << "Loaded a main texture: name=" << internal::cstr(mainTexturePath) << " ID=" << textureRef);
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

void AssetRenderEngine::setUpdateOptions(int /* options */)
{
    /* do nothing */
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
    bool hasShadowMap = false;
    if (const IShadowMap *shadowMap = m_sceneRef->shadowMapRef()) {
        const void *textureRef = shadowMap->textureRef();
        const GLuint textureID = *static_cast<const GLuint *>(textureRef);
        m_currentEffectEngineRef->depthTexture.setTexture(textureID);
        m_currentEffectEngineRef->selfShadow.updateParameter(shadowMap);
        hasShadowMap = true;
    }
    m_currentEffectEngineRef->setModelMatrixParameters(m_modelRef);
    const aiScene *a = m_modelRef->aiScenePtr();
    renderRecurse(a, a->mRootNode, hasShadowMap);
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
            m_currentEffectEngineRef = new PrivateEffectEngine(this);
            m_currentEffectEngineRef->setEffect(effectRef, dir, false);
            if (m_currentEffectEngineRef->scriptOrder() == IEffect::kStandard) {
                const aiScene *scene = m_modelRef->aiScenePtr();
                const unsigned int nmaterials = scene->mNumMaterials;
                std::string texture, mainTexture, subTexture;
                aiString texturePath;
                /* copy current material textures/spheres parameters to offscreen effect */
                for (unsigned int i = 0; i < nmaterials; i++) {
                    aiMaterial *material = scene->mMaterials[i];
                    aiReturn found = AI_SUCCESS;
                    int textureIndex = 0;
                    while (found == AI_SUCCESS) {
                        found = material->GetTexture(aiTextureType_DIFFUSE, textureIndex, &texturePath);
                        if (found != AI_SUCCESS)
                            break;
                        texture = texturePath.data;
                        if (SplitTexturePath(texture, mainTexture, subTexture)) {
                            Textures::const_iterator sub = m_textures.find(subTexture);
                            if (sub != m_textures.end()) {
                                m_currentEffectEngineRef->materialSphereMap.setTexture(material, sub->second);
                            }
                        }
                        Textures::const_iterator main = m_textures.find(mainTexture);
                        if (main != m_textures.end()) {
                            m_currentEffectEngineRef->materialTexture.setTexture(material, main->second);
                        }
                        textureIndex++;
                    }
                }
                m_oseffects.append(m_currentEffectEngineRef);
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
                m_defaultEffect = m_sceneRef->createDefaultStandardEffect(m_renderContextRef);
                effectRef = static_cast<Effect *>(m_defaultEffect);
                wasEffectNull = true;
            }
            m_currentEffectEngineRef = new PrivateEffectEngine(this);
            m_currentEffectEngineRef->setEffect(effectRef, dir, wasEffectNull);
            m_effectEngines.insert(type == IEffect::kAutoDetection ? m_currentEffectEngineRef->scriptOrder() : type, m_currentEffectEngineRef);
            /* set default standard effect as secondary effect */
            if (!wasEffectNull && m_currentEffectEngineRef->scriptOrder() == IEffect::kStandard) {
                m_defaultEffect = m_sceneRef->createDefaultStandardEffect(m_renderContextRef);
                m_currentEffectEngineRef->setDefaultStandardEffectRef(m_defaultEffect);
            }
        }
    }
}

void AssetRenderEngine::bindVertexBundle(const aiMesh *mesh)
{
    m_currentEffectEngineRef->setMesh(mesh);
    if (mesh) {
        VertexBundleLayout *layout = m_vao[mesh];
        if (layout && !layout->bind()) {
            VertexBundle *bundle = m_vbo[mesh];
            bundle->bind(VertexBundle::kVertexBuffer, 0);
            bindStaticVertexAttributePointers();
            bundle->bind(VertexBundle::kIndexBuffer, 0);
        }
    }
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
        delete m_vao[mesh];
        delete m_vbo[mesh];
    }
    const unsigned int nChildNodes = node->mChildren ? node->mNumChildren : 0;
    for (unsigned int i = 0; i < nChildNodes; i++)
        deleteRecurse(scene, node->mChildren[i]);
}

void AssetRenderEngine::renderRecurse(const aiScene *scene, const aiNode *node, const bool hasShadowMap)
{
    const unsigned int nmeshes = node->mNumMeshes;
    EffectEngine::DrawPrimitiveCommand command;
    for (unsigned int i = 0; i < nmeshes; i++) {
        const struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        bool hasTexture = false, hasSphereMap = false;
        const char *target = hasShadowMap ? "object_ss" : "object";
        setAssetMaterial(scene->mMaterials[mesh->mMaterialIndex], hasTexture, hasSphereMap);
        IEffect::ITechnique *technique = m_currentEffectEngineRef->findTechnique(target, i, nmeshes, hasTexture, hasSphereMap, false);
        size_t nindices = m_indices[mesh];
        if (technique) {
            bindVertexBundle(mesh);
            command.count = nindices;
            m_renderContextRef->startProfileSession(IRenderContext::kProfileRenderModelMaterialDrawCall, mesh);
            m_currentEffectEngineRef->executeTechniquePasses(technique, command, 0);
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
    EffectEngine::DrawPrimitiveCommand command;
    for (unsigned int i = 0; i < nmeshes; i++) {
        const struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        const struct aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        bool succeeded = aiGetMaterialFloat(material, AI_MATKEY_OPACITY, &opacity) == aiReturn_SUCCESS;
        if (succeeded && btFuzzyZero(opacity - 0.98f))
            continue;
        bindVertexBundle(mesh);
        const IEffect::ITechnique *technique = m_currentEffectEngineRef->findTechnique("zplot", i, nmeshes, false, false, false);
        if (technique) {
            size_t nindices = m_indices[mesh];
            command.count = nindices;
            m_renderContextRef->startProfileSession(IRenderContext::kProfileRenderZPlotMaterialDrawCall, mesh);
            m_currentEffectEngineRef->executeTechniquePasses(technique, command, 0);
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
    ITexture *textureRef = 0;
    std::string mainTexture, subTexture;
    aiString texturePath;
    hasTexture = false;
    hasSphereMap = false;
    if (material->GetTexture(aiTextureType_DIFFUSE, textureIndex, &texturePath) == aiReturn_SUCCESS) {
        bool isAdditive = false;
        if (SplitTexturePath(texturePath.data, mainTexture, subTexture)) {
            textureRef = m_textures[subTexture];
            isAdditive = subTexture.find(".spa") != std::string::npos;
            m_currentEffectEngineRef->spadd.setValue(isAdditive);
            m_currentEffectEngineRef->useSpheremap.setValue(true);
            hasSphereMap = true;
        }
        textureRef = m_textures[mainTexture];
        if (textureRef > 0) {
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
                                           const Indices &indices)
{
    m_vao.insert(std::make_pair(mesh, new VertexBundleLayout()));
    m_vbo.insert(std::make_pair(mesh, new VertexBundle()));
    VertexBundle *bundle = m_vbo[mesh];
    size_t isize = sizeof(indices[0]) * indices.count();
    bundle->create(VertexBundle::kIndexBuffer, 0, GL_STATIC_DRAW, &indices[0], isize);
    VPVL2_LOG(VLOG(2) << "Binding asset index buffer to the vertex buffer object");
    size_t vsize = vertices.count() * sizeof(vertices[0]);
    bundle->create(VertexBundle::kVertexBuffer, 0, GL_STATIC_DRAW, &vertices[0].position, vsize);
    VPVL2_LOG(VLOG(2) << "Binding asset vertex buffer to the vertex buffer object");
    VertexBundleLayout *layout = m_vao[mesh];
    if (layout->create() && layout->bind()) {
        VPVL2_LOG(VLOG(2) << "Created an vertex array object: " << layout->name());
    }
    bundle->bind(VertexBundle::kVertexBuffer, 0);
    bindStaticVertexAttributePointers();
    bundle->bind(VertexBundle::kIndexBuffer, 0);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    VertexBundleLayout::unbindVertexArrayObject();
    m_indices[mesh] = indices.count();
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
    glVertexPointer(3, GL_FLOAT, sizeof(v), vertexPtr);
    const void *normalPtr = reinterpret_cast<const void *>(reinterpret_cast<const uint8_t *>(&v.normal) - reinterpret_cast<const uint8_t *>(&v.position));
    glNormalPointer(GL_FLOAT, sizeof(v), normalPtr);
    const void *texcoordPtr = reinterpret_cast<const void *>(reinterpret_cast<const uint8_t *>(&v.texcoord) - reinterpret_cast<const uint8_t *>(&v.position));
    glTexCoordPointer(2, GL_FLOAT, sizeof(v), texcoordPtr);
}

} /* namespace cg */
} /* namespace vpvl2 */

#endif
