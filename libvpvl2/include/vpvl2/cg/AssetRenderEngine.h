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

#pragma once
#ifndef VPVL2_CG_ASSETRENDERENGINE_H_
#define VPVL2_CG_ASSETRENDERENGINE_H_

#include "vpvl2/Common.h"
#ifdef VPVL2_LINK_ASSIMP

#include "vpvl2/cg/EffectEngine.h"
#include "vpvl2/internal/BaseRenderEngine.h"

#include <assimp.h>
#include <aiScene.h>
#include <map>

namespace vpvl2
{

namespace asset
{
class Model;
}

class Scene;

namespace cg
{

/**
 * @file
 * @author Nagoya Institute of Technology Department of Computer Science
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * Bone class represents a bone of a Polygon Model Data object.
 */

class VPVL2_API AssetRenderEngine : public vpvl2::IRenderEngine, public vpvl2::internal::BaseRenderEngine
        #ifdef VPVL2_LINK_QT
        , protected QGLFunctions
        #endif
{
public:
    class Program;

    AssetRenderEngine(IRenderContext *renderContext, const Scene *scene, asset::Model *model);
    virtual ~AssetRenderEngine();

    IModel *model() const;
    bool upload(const IString *dir);
    void update();
    void renderModel();
    void renderEdge();
    void renderShadow();
    void renderZPlot();
    bool hasPreProcess() const;
    bool hasPostProcess() const;
    void preparePostProcess();
    void performPreProcess();
    void performPostProcess(IEffect *nextPostEffect);
    IEffect *effect(IEffect::ScriptOrderType type) const;
    void setEffect(IEffect::ScriptOrderType type, IEffect *effect, const IString *dir);

private:
    class PrivateContext;
    typedef std::map<std::string, GLuint> Textures;
    struct Vertex {
        Vertex() {}
        vpvl2::Vector4 position;
        vpvl2::Vector3 normal;
        vpvl2::Vector3 texcoord;
    };
    typedef Array<Vertex> Vertices;
    typedef Array<int> Indices;

    void log0(void *userData, IRenderContext::LogLevel level, const char *format ...);
    bool uploadRecurse(const aiScene *scene, const aiNode *node, void *userData);
    void deleteRecurse(const aiScene *scene, const aiNode *node);
    void renderRecurse(const aiScene *scene, const aiNode *node, const bool hasShadowMap);
    void renderZPlotRecurse(const aiScene *scene, const aiNode *node);
    void setAssetMaterial(const aiMaterial *material, bool &hasTexture, bool &hasSphereMap);
    void createVertexBundle(const aiMesh *mesh, const Vertices &vertices, const Indices &indices, void *userData);
    void bindVertexBundle(const aiMesh *mesh);
    void unbindVertexBundle();
    void bindStaticVertexAttributePointers();

    EffectEngine *m_currentRef;
    asset::Model *m_modelRef;
    Hash<btHashInt, EffectEngine *> m_effects;
    Array<EffectEngine *> m_oseffects;
    std::map<std::string, GLuint> m_textures;
    std::map<const struct aiMesh *, int> m_indices;
    std::map<const struct aiMesh *, GLuint> m_ibo;
    std::map<const struct aiMesh *, GLuint> m_vbo;
    std::map<const struct aiMesh *, GLuint> m_vao;
    int m_nvertices;
    int m_nmeshes;
    bool m_cullFaceState;

    VPVL2_DISABLE_COPY_AND_ASSIGN(AssetRenderEngine)
};

} /* namespace cg */
} /* namespace vpvl2 */

#endif /* VPVL2_LINK_ASSIMP */
#endif

