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

#ifndef VPVL2_CG2_PMXRENDERENGINE_H_
#define VPVL2_CG2_PMXRENDERENGINE_H_

#include "vpvl2/IRenderEngine.h"
#include "vpvl2/cg/EffectEngine.h"
#include "vpvl2/pmx/Model.h"

namespace vpvl2
{

class Scene;

namespace cl {
class PMXAccelerator;
}

namespace cg
{

class VPVL2_API PMXRenderEngine : public vpvl2::IRenderEngine
        #ifdef VPVL2_LINK_QT
        , protected QGLFunctions
        #endif
{
public:
    class PrivateContext;

    PMXRenderEngine(IRenderDelegate *delegate,
                    const Scene *scene,
                    CGcontext effectContext,
                    cl::PMXAccelerator *accelerator,
                    IModel *modelRef);
    virtual ~PMXRenderEngine();

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
    void performPostProcess();
    IEffect *effect(IEffect::ScriptOrderType type) const;
    void setEffect(IEffect::ScriptOrderType type, IEffect *effect, const IString *dir);

    bool isAcceleratorAvailable() const;
    bool initializeAccelerator();

private:
    bool releaseContext0(void *context);
    void release();
    void log0(void *context, IRenderDelegate::LogLevel level, const char *format ...);

    enum VertexBufferObjectType {
        kModelDynamicVertexBuffer,
        kModelStaticVertexBuffer,
        kModelIndexBuffer,
        kVertexBufferObjectMax
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

    const Scene *m_sceneRef;
    IRenderDelegate *m_delegateRef;
    CGcontext m_contextRef;
    EffectEngine *m_currentRef;
    cl::PMXAccelerator *m_accelerator;
    IModel *m_modelRef;
    IModel::IStaticVertexBuffer *m_staticBuffer;
    IModel::IDynamicVertexBuffer *m_dynamicBuffer;
    IModel::IIndexBuffer *m_indexBuffer;
    GLuint m_vertexBufferObjects[kVertexBufferObjectMax];
    MaterialContext *m_materialContexts;
    Hash<btHashInt, EffectEngine *> m_effects;
    Array<EffectEngine *> m_oseffects;
    Array<IMaterial *> m_materials;
    GLenum m_indexType;
    bool m_cullFaceState;
    bool m_isVertexShaderSkinning;

    VPVL2_DISABLE_COPY_AND_ASSIGN(PMXRenderEngine)
};

} /* namespace gl2 */
} /* namespace vpvl2 */

#endif
