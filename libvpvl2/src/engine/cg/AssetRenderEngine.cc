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

#include "vpvl2/cg/AssetRenderEngine.h"

#ifdef VPVL2_LINK_ASSIMP
#include "vpvl2/gl2/AssetRenderEngine.h"

#include "vpvl/Bone.h"
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

AssetRenderEngine::AssetRenderEngine(IRenderDelegate *delegate,
                                     const Scene *scene,
                                     CGcontext context,
                                     asset::Model *model)
#ifdef VPVL2_LINK_QT
    : QGLFunctions(),
      #else
    :
      #endif /* VPVL2_LINK_QT */
      m_delegate(delegate),
      m_scene(scene),
      m_model(model),
      m_context(context),
      m_effect(delegate),
      m_cullFaceState(true)
{
}

AssetRenderEngine::~AssetRenderEngine()
{
    const aiScene *scene = m_model->ptr()->getScene();
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
            if (SplitTexturePath(texture, mainTexture, subTexture)) {
                textureID = m_textures[subTexture];
                glDeleteTextures(1, &textureID);
                m_textures.erase(subTexture);
            }
            textureID = m_textures[mainTexture];
            glDeleteTextures(1, &textureID);
            m_textures.erase(mainTexture);
            textureIndex++;
        }
    }
    deleteRecurse(scene, scene->mRootNode);
    m_context = 0;
    m_model = 0;
    m_delegate = 0;
    m_scene = 0;
}

IModel *AssetRenderEngine::model() const
{
    return m_model;
}

bool AssetRenderEngine::upload(const IString *dir)
{
    bool ret = true;
#ifdef VPVL2_LINK_QT
    initializeGLFunctions(QGLContext::currentContext());
#endif /* VPVL2_LINK_QT */
    vpvl::Asset *asset = m_model->ptr();
    const aiScene *scene = asset->getScene();
    const unsigned int nmaterials = scene->mNumMaterials;
    void *context = 0;
    aiString texturePath;
    std::string path, mainTexture, subTexture;
    m_delegate->allocateContext(m_model, context);
    IString *source = m_delegate->loadShaderSource(IRenderDelegate::kModelEffectTechniques, m_model, dir, context);
    CGeffect effect = 0;
    cgSetErrorHandler(&AssetRenderEngine::handleError, this);
    if (source)
        effect = cgCreateEffect(m_context, reinterpret_cast<const char *>(source->toByteArray()), 0);
    delete source;
    if (!cgIsEffect(effect)) {
        log0(context, IRenderDelegate::kLogWarning, "CG effect compile error\n%s", cgGetLastListing(m_context));
        return false;
    }
    m_effect.attachEffect(effect, dir);
    m_effect.useToon.setValue(false);
    m_effect.parthf.setValue(false);
    m_effect.transp.setValue(false);
    m_effect.opadd.setValue(false);
    for (unsigned int i = 0; i < nmaterials; i++) {
        aiMaterial *material = scene->mMaterials[i];
        aiReturn found = AI_SUCCESS;
        GLuint textureID;
        int textureIndex = 0;
        while (found == AI_SUCCESS) {
            found = material->GetTexture(aiTextureType_DIFFUSE, textureIndex, &texturePath);
            path = texturePath.data;
            if (SplitTexturePath(path, mainTexture, subTexture)) {
                if (m_textures[mainTexture] == 0) {
                    IString *mainTexturePath = m_delegate->toUnicode(reinterpret_cast<const uint8_t *>(mainTexture.c_str()));
                    if (m_delegate->uploadTexture(context, mainTexturePath, dir, &textureID, false)) {
                        m_textures[mainTexture] = textureID;
                        log0(context, IRenderDelegate::kLogInfo, "Loaded a main texture: %s (ID=%d)", mainTexturePath->toByteArray(), textureID);
                    }
                    delete mainTexturePath;
                }
                if (m_textures[subTexture] == 0) {
                    IString *subTexturePath = m_delegate->toUnicode(reinterpret_cast<const uint8_t *>(subTexture.c_str()));
                    if (m_delegate->uploadTexture(context, subTexturePath, dir, &textureID, false)) {
                        m_textures[subTexture] = textureID;
                        log0(context, IRenderDelegate::kLogInfo, "Loaded a sub texture: %s (ID=%d)", subTexturePath->toByteArray(), textureID);
                    }
                    delete subTexturePath;
                }
            }
            else if (m_textures[mainTexture] == 0) {
                IString *mainTexturePath = m_delegate->toUnicode(reinterpret_cast<const uint8_t *>(mainTexture.c_str()));
                if (m_delegate->uploadTexture(context, mainTexturePath, dir, &textureID, false)) {
                    m_textures[mainTexture] = textureID;
                    log0(context, IRenderDelegate::kLogInfo, "Loaded a main texture: %s (ID=%d)", mainTexturePath->toByteArray(), textureID);
                }
                delete mainTexturePath;
            }
            textureIndex++;
        }
    }
    ret = uploadRecurse(scene, scene->mRootNode, context);
    m_delegate->releaseContext(m_model, context);
    return ret;
}

void AssetRenderEngine::update()
{
    if (!m_model->isVisible() || !m_effect.isAttached())
        return;
    m_effect.updateModelGeometryParameters(m_scene, m_model);
    m_effect.updateViewportParameters();
}

void AssetRenderEngine::renderModel()
{
    if (!m_model->isVisible() || !m_effect.isAttached())
        return;
    vpvl::Asset *asset = m_model->ptr();
    if (btFuzzyZero(asset->opacity()))
        return;
    m_effect.setModelMatrixParameters(m_model);
    const aiScene *a = asset->getScene();
    renderRecurse(a, a->mRootNode);
    if (!m_cullFaceState) {
        glEnable(GL_CULL_FACE);
        m_cullFaceState = true;
    }
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
    if (!m_model->isVisible() || !m_effect.isAttached())
        return;
    vpvl::Asset *asset = m_model->ptr();
    if (btFuzzyZero(asset->opacity()))
        return;
    m_effect.setModelMatrixParameters(m_model);
    const aiScene *a = asset->getScene();
    renderZPlotRecurse(a, a->mRootNode);
}

void AssetRenderEngine::log0(void *context, IRenderDelegate::LogLevel level, const char *format ...)
{
    va_list ap;
    va_start(ap, format);
    m_delegate->log(context, level, format, ap);
    va_end(ap);
}

bool AssetRenderEngine::uploadRecurse(const aiScene *scene, const aiNode *node, void *context)
{
    bool ret = true;
    const unsigned int nmeshes = node->mNumMeshes;
    int totalIndices = 0;
    AssetVertex assetVertex;
    m_effect.subsetCount.setValue(nmeshes);
    for (unsigned int i = 0; i < nmeshes; i++) {
        const struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        const aiVector3D *vertices = mesh->mVertices;
        const aiVector3D *normals = mesh->mNormals;
        const bool hasNormals = mesh->HasNormals();
        const bool hasColors = mesh->HasVertexColors(0);
        const bool hasTexCoords = mesh->HasTextureCoords(0);
        const aiColor4D *colors = hasColors ? mesh->mColors[0] : 0;
        const aiVector3D *texcoords = hasTexCoords ? mesh->mTextureCoords[0] : 0;
        AssetVertices &assetVertices = m_vertices[mesh];
        AssetIndices &indices = m_indices[mesh];
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
                assetVertex.position.setValue(v.x, v.y, v.z, index);
                assetVertices.push_back(assetVertex);
                indices.push_back(index);
                index++;
            }
        }
        AssetVBO &vbo = m_vbo[mesh];
        size_t vsize = assetVertices.size() * sizeof(assetVertices[0]);
        const int nindices = indices.size();
        glGenBuffers(1, &vbo.vertices);
        glBindBuffer(GL_ARRAY_BUFFER, vbo.vertices);
        glBufferData(GL_ARRAY_BUFFER, vsize, assetVertices[0].position, GL_STATIC_DRAW);
        glGenBuffers(1, &vbo.indices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.indices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, nindices * sizeof(indices[0]), &indices[0], GL_STATIC_DRAW);
        totalIndices += nindices;
    }
    m_effect.vertexCount.setValue(totalIndices);
    const unsigned int nChildNodes = node->mNumChildren;
    for (unsigned int i = 0; i < nChildNodes; i++) {
        ret = uploadRecurse(scene, node->mChildren[i], context);
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
        const AssetVBO &vbo = m_vbo[mesh];
        glDeleteBuffers(1, &vbo.vertices);
        glDeleteBuffers(1, &vbo.indices);
    }
    const unsigned int nChildNodes = node->mNumChildren;
    for (unsigned int i = 0; i < nChildNodes; i++)
        deleteRecurse(scene, node->mChildren[i]);
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
    if (const IBone *bone = m_model->parentBone()) {
        const Transform &boneTransform = bone->worldTransform();
        const btMatrix3x3 &boneBasis = boneTransform.getBasis();
        transform.setOrigin(boneTransform.getOrigin() + boneBasis * transform.getOrigin());
        transform.setBasis(boneBasis.scaled(scaleVector));
    }
    static const AssetVertex v;
    const uint8_t *basePtr = reinterpret_cast<const uint8_t *>(&v.position);
    const GLvoid *vertexPtr = 0;
    const GLvoid *normalPtr = reinterpret_cast<const GLvoid *>(reinterpret_cast<const uint8_t *>(&v.normal) - basePtr);
    const GLvoid *texcoordPtr = reinterpret_cast<const GLvoid *>(reinterpret_cast<const uint8_t *>(&v.texcoord) - basePtr);
    const size_t stride = sizeof(AssetVertex);
    const bool hasShadowMap = false; // light->depthTexture() ? true : false;
    const unsigned int nmeshes = node->mNumMeshes;
    for (unsigned int i = 0; i < nmeshes; i++) {
        const struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        const AssetVBO &vbo = m_vbo[mesh];
        const AssetIndices &indices = m_indices[mesh];
        bool hasTexture = false, hasSphereMap = false;
        setAssetMaterial(scene->mMaterials[mesh->mMaterialIndex], hasTexture, hasSphereMap);
        glBindBuffer(GL_ARRAY_BUFFER, vbo.vertices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.indices);
        const char *target = hasShadowMap ? "object_ss" : "object";
        CGtechnique technique = m_effect.findTechnique(target, i, nmeshes, hasTexture, hasSphereMap, false);
        if (cgIsTechnique(technique)) {
            const int nindices = indices.size();
            glVertexPointer(3, GL_FLOAT, stride, vertexPtr);
            glNormalPointer(GL_FLOAT, stride, normalPtr);
            glTexCoordPointer(2, GL_FLOAT, stride, texcoordPtr);
            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_NORMAL_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            m_effect.executeTechniquePasses(technique, nindices, GL_UNSIGNED_INT, 0);
            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_NORMAL_ARRAY);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }
    }
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
    const size_t stride = sizeof(AssetVertex);
    const unsigned int nmeshes = node->mNumMeshes;
    float opacity;
    glCullFace(GL_FRONT);
    for (unsigned int i = 0; i < nmeshes; i++) {
        const struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        const struct aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        bool succeeded = aiGetMaterialFloat(material, AI_MATKEY_OPACITY, &opacity) == aiReturn_SUCCESS;
        if (succeeded && btFuzzyZero(opacity - 0.98))
            continue;
        const AssetVBO &vbo = m_vbo[mesh];
        const AssetIndices &indices = m_indices[mesh];
        glBindBuffer(GL_ARRAY_BUFFER, vbo.vertices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.indices);
        CGtechnique technique = m_effect.findTechnique("zplot", i, nmeshes, false, false, false);
        if (cgIsTechnique(technique)) {
            const int nindices = indices.size();
            glVertexPointer(3, GL_FLOAT, stride, vertexPtr);
            glEnableClientState(GL_VERTEX_ARRAY);
            m_effect.executeTechniquePasses(technique, nindices, GL_UNSIGNED_INT, reinterpret_cast<const GLvoid *>(0));
            glDisableClientState(GL_VERTEX_ARRAY);
        }
    }
    glCullFace(GL_BACK);
    const unsigned int nChildNodes = node->mNumChildren;
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
            m_effect.materialSphereMap.setTexture(textureID);
            m_effect.spadd.setValue(isAdditive);
            m_effect.useSpheremap.setValue(true);
            hasSphereMap = true;
        }
        textureID = m_textures[mainTexture];
        if (textureID > 0) {
            m_effect.materialTexture.setTexture(textureID);
            m_effect.useTexture.setValue(true);
            hasTexture = true;
        }
    }
    else {
        m_effect.materialTexture.setTexture(0);
        m_effect.materialSphereMap.setTexture(0);
        m_effect.useTexture.setValue(false);
        m_effect.useSpheremap.setValue(false);
    }
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
    m_effect.emissive.setGeometryColor(color);
    if (aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse) == aiReturn_SUCCESS) {
        color.setValue(diffuse.r, diffuse.g, diffuse.b, diffuse.a);
    }
    else {
        color.setValue(0, 0, 0, 1);
    }
    m_effect.ambient.setGeometryColor(color);
    m_effect.diffuse.setGeometryColor(color);
    if (aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &specular) == aiReturn_SUCCESS) {
        static const float kDivide = 10.0;
        color.setValue(specular.r / kDivide, specular.g / kDivide, specular.b / kDivide, specular.a);
    }
    else {
        color.setValue(0, 0, 0, 1);
    }
    m_effect.specular.setGeometryColor(color);
    float shininess, strength;
    int ret1 = aiGetMaterialFloat(material, AI_MATKEY_SHININESS, &shininess);
    int ret2 = aiGetMaterialFloat(material, AI_MATKEY_SHININESS_STRENGTH, &strength);
    if (ret1 == aiReturn_SUCCESS && ret2 == aiReturn_SUCCESS) {
        m_effect.specularPower.setGeometryValue(shininess * strength);
    }
    else if (ret1 == aiReturn_SUCCESS) {
        m_effect.specularPower.setGeometryValue(shininess);
    }
    else {
        m_effect.specularPower.setGeometryValue(1);
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

void AssetRenderEngine::handleError(CGcontext context, CGerror error, void *data)
{
    AssetRenderEngine *engine = static_cast<AssetRenderEngine *>(data);
    Q_UNUSED(context)
    engine->log0(0, IRenderDelegate::kLogWarning, "CGerror: %s", cgGetErrorString(error));
}

} /* namespace cg */
} /* namespace vpvl2 */

#endif
