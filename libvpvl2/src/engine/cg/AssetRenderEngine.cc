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
      m_context(context),
      m_model(model),
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
    delete m_context;
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
}

void AssetRenderEngine::renderModel()
{
}

void AssetRenderEngine::renderEdge()
{
}

void AssetRenderEngine::renderShadow()
{
}

void AssetRenderEngine::renderZPlot()
{
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
    AssetVertex assetVertex;
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
                assetVertex.position.setValue(v.x, v.y, v.z, 1.0f);
                assetVertices.push_back(assetVertex);
                indices.push_back(index);
                index++;
            }
        }
        AssetVBO &vbo = m_vbo[mesh];
        size_t vsize = assetVertices.size() * sizeof(assetVertices[0]);
        glGenBuffers(1, &vbo.vertices);
        glBindBuffer(GL_ARRAY_BUFFER, vbo.vertices);
        glBufferData(GL_ARRAY_BUFFER, vsize, assetVertices[0].position, GL_STATIC_DRAW);
        glGenBuffers(1, &vbo.indices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.indices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), &indices[0], GL_STATIC_DRAW);
    }
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
}

void AssetRenderEngine::renderZPlotRecurse(const aiScene *scene, const aiNode *node)
{
}

void AssetRenderEngine::setAssetMaterial(const aiMaterial *material)
{
}

} /* namespace cg */
} /* namespace vpvl2 */

#endif
