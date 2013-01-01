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
    class PrivateContext;

    PMXRenderEngine(IRenderContext *renderContextRef,
                    Scene *scene,
                    cl::PMXAccelerator *accelerator,
                    IModel *modelRef);
    virtual ~PMXRenderEngine();

    IModel *parentModelRef() const;
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
            : mainTextureID(0),
              sphereTextureID(0),
              toonTextureColor(1, 1, 1, 1)
        {
        }
        GLuint mainTextureID;
        GLuint sphereTextureID;
        Color toonTextureColor;
    };

    bool uploadMaterials(const IString *dir, void *userData);
    bool releaseUserData0(void *userData);
    void release();
    void createVertexBundle(GLuint dvbo, GLuint svbo, GLuint ibo);
    void createEdgeBundle(GLuint dvbo, GLuint svbo, GLuint ibo);
    void bindVertexBundle();
    void bindEdgeBundle();
    void unbindVertexBundle();
    void bindDynamicVertexAttributePointers(IModel::IBuffer::StrideType type);
    void bindStaticVertexAttributePointers();
    void getVertexBundleType(VertexArrayObjectType &vao, VertexBufferObjectType &vbo);
    void getEdgeBundleType(VertexArrayObjectType &vao, VertexBufferObjectType &vbo);
    void log0(void *userData, IRenderContext::LogLevel level, const char *format ...);

    EffectEngine *m_currentRef;
    cl::PMXAccelerator *m_accelerator;
#ifdef VPVL2_ENABLE_OPENCL
    cl::PMXAccelerator::Buffers m_accelerationBuffers;
#endif
    IRenderContext *m_renderContextRef;
    Scene *m_sceneRef;
    IModel *m_modelRef;
    IModel::IStaticVertexBuffer *m_staticBuffer;
    IModel::IDynamicVertexBuffer *m_dynamicBuffer;
    IModel::IIndexBuffer *m_indexBuffer;
    GLuint m_vertexBufferObjects[kMaxVertexBufferObjectType];
    GLuint m_vertexArrayObjects[kMaxVertexArrayObjectType];
    MaterialContext *m_materialContexts;
    Hash<btHashInt, EffectEngine *> m_effectEngines;
    Array<EffectEngine *> m_oseffects;
    Array<IMaterial *> m_materials;
    VertexBundle m_bundle;
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
