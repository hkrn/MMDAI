/**

 Copyright (c) 2009-2011  Nagoya Institute of Technology
                          Department of Computer Science
               2010-2013  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#pragma once
#ifndef VPVL2_GL2_PMXRENDERENGINE_H_
#define VPVL2_GL2_PMXRENDERENGINE_H_

#include "vpvl2/IApplicationContext.h"
#include "vpvl2/IRenderEngine.h"
#include "vpvl2/extensions/gl/Global.h"

namespace vpvl2 {

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

class VPVL2_API PMXRenderEngine : public IRenderEngine
{
public:
    PMXRenderEngine(IApplicationContext *applicationContextRef,
                    Scene *scene,
                    cl::PMXAccelerator *accelerator,
                    IModel *modelRef);
    virtual ~PMXRenderEngine();

    IModel *parentModelRef() const;
    bool upload(void *userData);
    void release();
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
    void setOverridePass(IEffect::Pass *pass);
    bool testVisible();

private:
    typedef void (GLAPIENTRY * PFNGLCULLFACEPROC) (extensions::gl::GLenum mode);
    typedef void (GLAPIENTRY * PFNGLENABLEPROC) (extensions::gl::GLenum cap);
    typedef void (GLAPIENTRY * PFNGLDISABLEPROC) (extensions::gl::GLenum cap);
    typedef void (GLAPIENTRY * PFNGLDRAWELEMENTSPROC) (extensions::gl::GLenum mode, extensions::gl::GLsizei count, extensions::gl::GLenum type, const extensions::gl::GLvoid *indices);
    typedef void (GLAPIENTRY * PFNGLGENQUERIESPROC) (extensions::gl::GLsizei n, extensions::gl::GLuint* ids);
    typedef void (GLAPIENTRY * PFNGLBEGINQUERYPROC) (extensions::gl::GLenum target, extensions::gl::GLuint id);
    typedef void (GLAPIENTRY * PFNGLENDQUERYPROC) (extensions::gl::GLenum target);
    typedef void (GLAPIENTRY * PFNGLGETQUERYOBJECTIVPROC) (extensions::gl::GLuint id, extensions::gl::GLenum pname, extensions::gl::GLint* params);
    typedef void (GLAPIENTRY * PFNGLDELETEQUERIESPROC) (extensions::gl::GLsizei n, const extensions::gl::GLuint* ids);
    typedef void (GLAPIENTRY * PFNGLENABLEVERTEXATTRIBARRAYPROC) (extensions::gl::GLuint);
    typedef void (GLAPIENTRY * PFNGLVERTEXATTRIBPOINTERPROC) (extensions::gl::GLuint index, extensions::gl::GLint size, extensions::gl::GLenum type, extensions::gl::GLboolean normalized, extensions::gl::GLsizei stride, const extensions::gl::GLvoid* pointer);
    PFNGLCULLFACEPROC cullFace;
    PFNGLENABLEPROC enable;
    PFNGLDISABLEPROC disable;
    PFNGLDRAWELEMENTSPROC drawElements;
    PFNGLGENQUERIESPROC genQueries;
    PFNGLBEGINQUERYPROC beginQuery;
    PFNGLENDQUERYPROC endQuery;
    PFNGLGETQUERYOBJECTIVPROC getQueryObjectiv;
    PFNGLDELETEQUERIESPROC deleteQueries;
    PFNGLENABLEVERTEXATTRIBARRAYPROC enableVertexAttribArray;
    PFNGLVERTEXATTRIBPOINTERPROC vertexAttribPointer;

    class PrivateContext;
    bool createProgram(BaseShaderProgram *program,
                       IApplicationContext::ShaderType vertexShaderType,
                       IApplicationContext::ShaderType vertexSkinningShaderType,
                       IApplicationContext::ShaderType fragmentShaderType,
                       void *userData);
    bool uploadMaterials(void *userData);
    void createVertexBundle(extensions::gl::GLuint dvbo);
    void createEdgeBundle(extensions::gl::GLuint dvbo);
    void bindVertexBundle();
    void bindEdgeBundle();
    void unbindVertexBundle();
    void bindDynamicVertexAttributePointers();
    void bindEdgeVertexAttributePointers();
    void bindStaticVertexAttributePointers();

    cl::PMXAccelerator *m_accelerator;
    IApplicationContext *m_applicationContextRef;
    Scene *m_sceneRef;
    IModel *m_modelRef;
    PrivateContext *m_context;

    VPVL2_DISABLE_COPY_AND_ASSIGN(PMXRenderEngine)
};

} /* namespace gl2 */
} /* namespace vpvl2 */

#endif
