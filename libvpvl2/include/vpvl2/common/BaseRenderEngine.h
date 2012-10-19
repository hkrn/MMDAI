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

#ifndef VPVL2_BASE_RENDERENGINE_H_
#define VPVL2_BASE_RENDERENGINE_H_

#include "vpvl2/Common.h"
#include "vpvl2/IRenderDelegate.h"

#include "vpvl2/Common.h"
#include "vpvl2/IRenderDelegate.h"

#if defined(VPVL2_LINK_QT)
#include <QtOpenGL/QtOpenGL>
#include <QtOpenGL/QGLFunctions>
#elif defined(VPVL2_LINK_GLEW)
#include <GL/glew.h>
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

namespace common
{

class BaseRenderEngine {
public:
    BaseRenderEngine(const Scene *sceneRef, IRenderDelegate *delegate)
        : m_sceneRef(sceneRef),
          m_delegateRef(delegate),
          glBindVertexArrayRef(0),
          glDeleteVertexArraysRef(0),
          glGenVertexArraysRef(0)
    {
    }
    ~BaseRenderEngine() {
        m_sceneRef = 0;
        m_delegateRef = 0;
        glBindVertexArrayRef = 0;
        glDeleteVertexArraysRef = 0;
        glGenVertexArraysRef = 0;
    }

protected:
    void initializeExtensions() {
        /* TODO: finding extension process */
#ifdef __APPLE__
        static const char *kBindVertexArray[] = {
            "glBindVertexArrayAPPLE",
            0
        };
        static const char *kDeleteVertexArrays[] = {
            "glDeleteVertexArraysAPPLE",
            0
        };
        static const char *kGenVertexArrays[] = {
            "glGenVertexArraysAPPLE",
            0
        };
#else
        static const char *kBindVertexArray[] = {
            "glBindVertexArray",
            0
        };
        static const char *kDeleteVertexArrays[] = {
            "glDeleteVertexArrays",
            0
        };
        static const char *kGenVertexArrays[] = {
            "glGenVertexArrays",
            0
        };
#endif
        glBindVertexArrayRef = reinterpret_cast<PFNGLBINDVERTEXARRAY>(
                    m_delegateRef->findProcedureAddress(reinterpret_cast<const void **>(kBindVertexArray)));
        glDeleteVertexArraysRef = reinterpret_cast<PFNGLDELETEVERTEXARRAYS>(
                    m_delegateRef->findProcedureAddress(reinterpret_cast<const void **>(kDeleteVertexArrays)));
        glGenVertexArraysRef = reinterpret_cast<PFNGLGENVERTEXARRAYS>(
                    m_delegateRef->findProcedureAddress(reinterpret_cast<const void **>(kGenVertexArrays)));
    }
    void allocateVertexArrayObjects(GLuint *vao, size_t size) {
        if (glGenVertexArraysRef) {
            glGenVertexArraysRef(size, vao);
        }
    }
    void releaseVertexArrayObjects(GLuint *vao, size_t size) {
        if (glDeleteVertexArraysRef) {
            glDeleteVertexArraysRef(size, vao);
        }
    }
    bool bindVertexArrayObject(GLuint vao) {
        if (glBindVertexArrayRef) {
            glBindVertexArrayRef(vao);
            return true;
        }
        return false;
    }
    bool unbindVertexArrayObject() {
        if (glBindVertexArrayRef) {
            glBindVertexArrayRef(0);
            return true;
        }
        return false;
    }

    const Scene *m_sceneRef;
    IRenderDelegate *m_delegateRef;

private:
    typedef void (*PFNGLBINDVERTEXARRAY)(GLuint id);
    typedef void (*PFNGLDELETEVERTEXARRAYS)(GLsizei n, const GLuint *ids);
    typedef void (*PFNGLGENVERTEXARRAYS)(GLsizei n, GLuint *ids);
    PFNGLBINDVERTEXARRAY glBindVertexArrayRef;
    PFNGLDELETEVERTEXARRAYS glDeleteVertexArraysRef;
    PFNGLGENVERTEXARRAYS glGenVertexArraysRef;

    VPVL2_DISABLE_COPY_AND_ASSIGN(BaseRenderEngine)
};

} /* namespace common */
} /* namespace vpvl2 */

#endif
