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

#pragma once
#ifndef VPVL2_GL2_ASSETRENDERENGINE_H_
#define VPVL2_GL2_ASSETRENDERENGINE_H_

#include "vpvl2/Common.h"
#ifdef VPVL2_LINK_ASSIMP

#include "vpvl2/IRenderContext.h"
#include "vpvl2/IRenderEngine.h"
#include "vpvl2/extensions/gl/VertexBundleLayout.h"

#include <assimp.h>
#include <aiScene.h>

namespace vpvl2
{

namespace asset
{
class Model;
}
namespace extensions
{
namespace gl
{
}
}

class Scene;

namespace gl2
{

class BaseShaderProgram;

/**
 * @file
 * @author Nagoya Institute of Technology Department of Computer Science
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * Bone class represents a bone of a Polygon Model Data object.
 */

class VPVL2_API AssetRenderEngine : public vpvl2::IRenderEngine
{
public:
    class Program;

    AssetRenderEngine(IRenderContext *renderContext, Scene *scene, asset::Model *parentModelRef);
    virtual ~AssetRenderEngine();

    IModel *parentModelRef() const;
    bool upload(const IString *dir);
    void update();
    void setUpdateOptions(int options);
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
    struct Vertex {
        Vertex() {}
        vpvl2::Vector4 position;
        vpvl2::Vector3 normal;
        vpvl2::Vector3 texcoord;
    };
    typedef Array<Vertex> Vertices;
    typedef Array<int> Indices;
    class PrivateContext;
    bool uploadRecurse(const aiScene *scene, const aiNode *node, const IString *dir, void *userData);
    void deleteRecurse(const aiScene *scene, const aiNode *node);
    void renderRecurse(const aiScene *scene, const aiNode *node);
    void renderZPlotRecurse(const aiScene *scene, const aiNode *node);
    void setAssetMaterial(const aiMaterial *material, Program *program);
    bool createProgram(BaseShaderProgram *program,
                       const IString *dir,
                       IRenderContext::ShaderType vertexShaderType,
                       IRenderContext::ShaderType fragmentShaderType,
                       void *userData);
    void createVertexBundle(const aiMesh *mesh,
                            const Vertices &vertices,
                            const Indices &indices);
    void bindVertexBundle(const aiMesh *mesh);
    void unbindVertexBundle();
    void bindStaticVertexAttributePointers();

    IRenderContext *m_renderContextRef;
    Scene *m_sceneRef;
    asset::Model *m_modelRef;
    PrivateContext *m_context;
    extensions::gl::VertexBundleLayout m_bundle;

    VPVL2_DISABLE_COPY_AND_ASSIGN(AssetRenderEngine)
};

} /* namespace gl2 */
} /* namespace vpvl2 */

#endif /* VPVL2_LINK_ASSIMP */
#endif

