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
#ifndef VPVL2_QT_VERTEXARRAY_H_
#define VPVL2_QT_VERTEXARRAY_H_

#include <QtOpenGL/QtOpenGL>

#include "vpvl2/qt/Common.h"
#include "vpvl2/IRenderContext.h"

#ifdef WIN32
#pragma warning(push)
#pragma warning(disable:4005)
#include <vpvl2/extensions/gl/khronos/glext.h>
#pragma warning(pop)
#endif

namespace vpvl2
{
namespace qt
{

class VertexBundle {
public:
    explicit VertexBundle(IRenderContext *renderContextRef)
        : glBindVertexArrayProcPtrRef(0),
          glDeleteVertexArraysProcPtrRef(0),
          glGenVertexArraysProcPtrRef(0),
          m_renderContextRef(renderContextRef),
          m_name(0)
    {
    }
    ~VertexBundle() {
        if (glDeleteVertexArraysProcPtrRef) {
            glDeleteVertexArraysProcPtrRef(1, &m_name);
            m_name = 0;
        }
        m_renderContextRef = 0;
    }

    void initialize() {
#ifdef __APPLE__
        static const void *kBindVertexArray[] = {
            "glBindVertexArrayAPPLE",
            0
        };
        static const void *kDeleteVertexArrays[] = {
            "glDeleteVertexArraysAPPLE",
            0
        };
        static const void *kGenVertexArrays[] = {
            "glGenVertexArraysAPPLE",
            0
        };
#else /**/
        static const void *kBindVertexArray[] = {
            "glBindVertexArray",
            0
        };
        static const void *kDeleteVertexArrays[] = {
            "glDeleteVertexArrays",
            0
        };
        static const void *kGenVertexArrays[] = {
            "glGenVertexArrays",
            0
        };
#endif /* __APPLE__ */
#ifdef WIN32
        glBindVertexArrayProcPtrRef = reinterpret_cast<PFNGLBINDVERTEXARRAYPROC>(
                    m_renderContextRef->findProcedureAddress(kBindVertexArray));
        glDeleteVertexArraysProcPtrRef = reinterpret_cast<PFNGLDELETEVERTEXARRAYSPROC>(
                    m_renderContextRef->findProcedureAddress(kDeleteVertexArrays));
        glGenVertexArraysProcPtrRef = reinterpret_cast<PFNGLGENVERTEXARRAYSPROC>(
                    m_renderContextRef->findProcedureAddress(kGenVertexArrays));
#else
        glBindVertexArrayProcPtrRef = reinterpret_cast<glBindVertexArrayProcPtr>(
                    m_renderContextRef->findProcedureAddress(kBindVertexArray));
        glDeleteVertexArraysProcPtrRef = reinterpret_cast<glDeleteVertexArraysProcPtr>(
                    m_renderContextRef->findProcedureAddress(kDeleteVertexArrays));
        glGenVertexArraysProcPtrRef = reinterpret_cast<glGenVertexArraysProcPtr>(
                    m_renderContextRef->findProcedureAddress(kGenVertexArrays));
#endif
    }

    bool bind() {
        if (glBindVertexArrayProcPtrRef) {
            glBindVertexArrayProcPtrRef(m_name);
            return true;
        }
        return false;
    }
    bool create() {
        if (glGenVertexArraysProcPtrRef) {
            glGenVertexArraysProcPtrRef(1, &m_name);
            return true;
        }
        return false;
    }
    bool release() {
        if (glBindVertexArrayProcPtrRef) {
            glBindVertexArrayProcPtrRef(0);
            return true;
        }
        return false;
    }

private:
#ifdef WIN32
    PFNGLBINDVERTEXARRAYPROC glBindVertexArrayProcPtrRef;
    PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArraysProcPtrRef;
    PFNGLGENVERTEXARRAYSPROC glGenVertexArraysProcPtrRef;
#else
    typedef void (*glBindVertexArrayProcPtr)(GLuint id);
    typedef void (*glDeleteVertexArraysProcPtr)(GLsizei n, const GLuint *ids);
    typedef void (*glGenVertexArraysProcPtr)(GLsizei n, GLuint *ids);
    glBindVertexArrayProcPtr glBindVertexArrayProcPtrRef;
    glDeleteVertexArraysProcPtr glDeleteVertexArraysProcPtrRef;
    glGenVertexArraysProcPtr glGenVertexArraysProcPtrRef;
#endif
    IRenderContext *m_renderContextRef;
    GLuint m_name;

    VPVL2_DISABLE_COPY_AND_ASSIGN(VertexBundle)
};

} /* namespace qt */
} /* namespace vpvl2 */

#endif // VPVL2_QT_VERTEXARRAY_H_
