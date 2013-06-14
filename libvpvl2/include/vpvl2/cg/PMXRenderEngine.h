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

#pragma once
#ifndef VPVL2_CG2_PMXRENDERENGINE_H_
#define VPVL2_CG2_PMXRENDERENGINE_H_

#include "vpvl2/IModel.h"
#include "vpvl2/IRenderEngine.h"
#include "vpvl2/cg/EffectEngine.h"
#include "vpvl2/extensions/gl/VertexBundle.h"
#include "vpvl2/extensions/gl/VertexBundleLayout.h"

#ifdef VPVL2_ENABLE_OPENCL
#include "vpvl2/cl/PMXAccelerator.h"
#endif

namespace vpvl2
{

namespace cl {
class PMXAccelerator;
}

class Scene;

namespace cg
{

class VPVL2_API PMXRenderEngine : public vpvl2::IRenderEngine
{
public:
    PMXRenderEngine(IRenderContext *renderContextRef,
                    Scene *scene,
                    cl::PMXAccelerator *accelerator,
                    IModel *modelRef);
    virtual ~PMXRenderEngine();

    IModel *parentModelRef() const;
    bool upload(void *userData);
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
    IEffect *effectRef(IEffect::ScriptOrderType type) const;
    void setEffect(IEffect *effectRef, IEffect::ScriptOrderType type, void *userData);

    void bindVertexBundle();
    void bindEdgeBundle();

    IRenderContext *renderContextRef() const { return m_renderContextRef; }
    Scene *sceneRef() const { return m_sceneRef; }

private:
    class PrivateEffectEngine;
    enum VertexBufferObjectType {
        kModelDynamicVertexBufferEven,
        kModelDynamicVertexBufferOdd,
        kModelStaticVertexBuffer,
        kModelIndexBuffer,
        kMaxVertexBufferObjectType
    };
    enum VertexArrayObjectType {
        kVertexArrayObjectEven,
        kVertexArrayObjectOdd,
        kEdgeVertexArrayObjectEven,
        kEdgeVertexArrayObjectOdd,
        kMaxVertexArrayObjectType
    };
    struct MaterialContext {
        MaterialContext()
            : mainTextureRef(0),
              sphereTextureRef(0),
              toonTextureColor(1, 1, 1, 1)
        {
        }
        ~MaterialContext() {
            mainTextureRef = 0;
            sphereTextureRef = 0;
        }
        ITexture *mainTextureRef;
        ITexture *sphereTextureRef;
        Color toonTextureColor;
    };

    bool uploadMaterials(void *userData);
    bool releaseUserData0(void *userData);
    void release();
    void createVertexBundle(GLuint dvbo);
    void createEdgeBundle(GLuint dvbo);
    void unbindVertexBundle();
    void bindDynamicVertexAttributePointers(IModel::Buffer::StrideType type);
    void bindStaticVertexAttributePointers();
    void getVertexBundleType(VertexArrayObjectType &vao, VertexBufferObjectType &vbo) const;
    void getEdgeBundleType(VertexArrayObjectType &vao, VertexBufferObjectType &vbo) const;
    void getDrawPrimitivesCommand(EffectEngine::DrawPrimitiveCommand &command) const;
    void updateDrawPrimitivesCommand(const IMaterial *material, EffectEngine::DrawPrimitiveCommand &command) const;

    PrivateEffectEngine *m_currentEffectEngineRef;
    cl::PMXAccelerator *m_accelerator;
#ifdef VPVL2_ENABLE_OPENCL
    cl::PMXAccelerator::VertexBufferBridgeArray m_accelerationBuffers;
#endif
    IRenderContext *m_renderContextRef;
    Scene *m_sceneRef;
    IModel *m_modelRef;
    IModel::StaticVertexBuffer *m_staticBuffer;
    IModel::DynamicVertexBuffer *m_dynamicBuffer;
    IModel::IndexBuffer *m_indexBuffer;
    extensions::gl::VertexBundle m_bundle;
    extensions::gl::VertexBundleLayout m_layouts[kMaxVertexArrayObjectType];
    Array<MaterialContext> m_materialContexts;
    PointerHash<HashPtr, ITexture> m_allocatedTextures;
    PointerHash<HashInt, PrivateEffectEngine> m_effectEngines;
    PointerArray<PrivateEffectEngine> m_oseffects;
    IEffect *m_defaultEffect;
    GLenum m_indexType;
    Vector3 m_aabbMin;
    Vector3 m_aabbMax;
    bool m_cullFaceState;
    bool m_updateEvenBuffer;
    bool m_isVertexShaderSkinning;

    VPVL2_DISABLE_COPY_AND_ASSIGN(PMXRenderEngine)
};

} /* namespace gl2 */
} /* namespace vpvl2 */

#endif
