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

#ifndef VPVL2_GL2_PMXRENDERENGINE_H_
#define VPVL2_GL2_PMXRENDERENGINE_H_

#include "vpvl2/Common.h"
#include "vpvl2/IRenderDelegate.h"
#include "vpvl2/IRenderEngine.h"

#ifdef VPVL2_LINK_QT
#include <QtOpenGL/QtOpenGL>
#include <QtOpenGL/QGLFunctions>
#endif /* VPVL_LINK_QT */

#if defined(VPVL2_ENABLE_GLES2)
#include <GLES2/gl2.h>
#elif defined(VPVL2_BUILD_IOS)
#include <OpenGLES/ES2/gl.h>
#else
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/CGLCurrent.h>
#else
#include <GL/gl.h>
#endif /* __APPLE__ */
#endif /* VPVL_BUILD_IOS */

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

class VPVL2_API PMXRenderEngine : public vpvl2::IRenderEngine
        #ifdef VPVL2_LINK_QT
        , protected QGLFunctions
        #endif
{
public:
    class PrivateContext;

    PMXRenderEngine(IRenderDelegate *delegate,
                    const Scene *scene,
                    cl::PMXAccelerator *accelerator,
                    pmx::Model *model);
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

protected:
    void log0(void *context, IRenderDelegate::LogLevel level, const char *format ...);

    IRenderDelegate *m_delegate;

private:
    const Scene *m_scene;
    cl::PMXAccelerator *m_accelerator;
    pmx::Model *m_model;
    PrivateContext *m_context;

    VPVL2_DISABLE_COPY_AND_ASSIGN(PMXRenderEngine)
};

} /* namespace gl2 */
} /* namespace vpvl2 */

#endif
