/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2012  hkrn                                    */
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
#ifndef VPVL2_GL2_PMXRENDERENGINE_H_
#define VPVL2_GL2_PMXRENDERENGINE_H_

#include "vpvl2/IRenderEngine.h"
#include "vpvl2/internal/BaseRenderEngine.h"

namespace vpvl2
{

class Scene;

namespace cl {
class PMXAccelerator;
}
namespace pmx {
class Model;
}

namespace gl2
{

class BaseShaderProgram;

class VPVL2_API PMXRenderEngine : public IRenderEngine, public internal::BaseRenderEngine
        #ifdef VPVL2_LINK_QT
        , protected QGLFunctions
        #endif
{
public:
    PMXRenderEngine(IRenderContext *context,
                    Scene *scene,
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
    void performPostProcess(IEffect *nextPostEffect);
    IEffect *effect(IEffect::ScriptOrderType type) const;
    void setEffect(IEffect::ScriptOrderType type, IEffect *effect, const IString *dir);

private:
    class PrivateContext;
    void log0(void *userData, IRenderContext::LogLevel level, const char *format ...);
    bool createProgram(BaseShaderProgram *program,
                       const IString *dir,
                       IRenderContext::ShaderType vertexShaderType,
                       IRenderContext::ShaderType vertexSkinningShaderType,
                       IRenderContext::ShaderType fragmentShaderType,
                       void *userData);
    bool uploadMaterials(const IString *dir, void *userData);
    bool releaseUserData0(void *userData);
    void createVertexBundle(GLuint dvbo, GLuint svbo, GLuint ibo, bool vss);
    void createEdgeBundle(GLuint dvbo, GLuint svbo, GLuint ibo, bool vss);
    void bindVertexBundle();
    void bindEdgeBundle();
    void unbindVertexBundle();
    void bindDynamicVertexAttributePointers();
    void bindEdgeVertexAttributePointers();
    void bindStaticVertexAttributePointers();

    cl::PMXAccelerator *m_accelerator;
    IModel *m_modelRef;
    PrivateContext *m_context;
    Vector3 m_aabbMin;
    Vector3 m_aabbMax;

    VPVL2_DISABLE_COPY_AND_ASSIGN(PMXRenderEngine)
};

} /* namespace gl2 */
} /* namespace vpvl2 */

#endif
