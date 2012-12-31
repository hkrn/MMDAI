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
#ifndef VPVL2_EXTENSIONS_GL_VERTEXBUNDLE_H_
#define VPVL2_EXTENSIONS_GL_VERTEXBUNDLE_H_

#include "vpvl2/Common.h"
#include "vpvl2/IRenderContext.h"
#include "vpvl2/extensions/gl/CommonMacros.h"

namespace vpvl2
{
namespace extensions
{
namespace gl
{

class VertexBundle {
public:
    VertexBundle(IRenderContext *context)
        : m_renderContextRef(context)
    {
    }
    virtual ~VertexBundle() {
        m_renderContextRef = 0;
    }

    void allocateVertexArrayObjects(GLuint *vao, size_t size) {
        if (GLEW_VERSION_3_0 || GLEW_ARB_vertex_array_object) {
            glGenVertexArrays(size, vao);
        }
        else if (GLEW_APPLE_vertex_array_object) {
            glGenVertexArraysAPPLE(size, vao);
        }
    }
    void releaseVertexArrayObjects(GLuint *vao, size_t size) {
        if (GLEW_VERSION_3_0 || GLEW_ARB_vertex_array_object) {
            glDeleteVertexArrays(size, vao);
        }
        else if (GLEW_APPLE_vertex_array_object) {
            glDeleteVertexArraysAPPLE(size, vao);
        }
    }
    bool bindVertexArrayObject(GLuint vao) {
        if (GLEW_VERSION_3_0 || GLEW_ARB_vertex_array_object) {
            glBindVertexArray(vao);
            return true;
        }
        else if (GLEW_APPLE_vertex_array_object) {
            glBindVertexArrayAPPLE(vao);
            return true;
        }
        return false;
    }
    bool unbindVertexArrayObject() {
        if (GLEW_VERSION_3_0 || GLEW_ARB_vertex_array_object) {
            glBindVertexArray(0);
            return true;
        }
        else if (GLEW_APPLE_vertex_array_object) {
            glBindVertexArrayAPPLE(0);
            return true;
        }
        return false;
    }
    void *mapBuffer(GLenum target, size_t offset, size_t size) {
#ifdef GL_CHROMIUM_map_sub
        return glMapBufferSubDataCHROMIUM(target, offset, size, GL_WRITE_ONLY);
#else /* GL_CHROMIUM_map_sub */
        (void) offset;
        (void) size;
        return glMapBuffer(target, GL_WRITE_ONLY);
#endif /* GL_CHROMIUM_map_sub */
    }
    void unmapBuffer(GLenum target, void *address) {
#ifdef GL_CHROMIUM_map_sub
        (void) target;
        glUnmapBufferSubDataCHROMIUM(address);
#else /* GL_CHROMIUM_map_sub */
        (void) address;
        glUnmapBuffer(target);
#endif /* GL_CHROMIUM_map_sub */
    }

private:
    IRenderContext *m_renderContextRef;

    VPVL2_DISABLE_COPY_AND_ASSIGN(VertexBundle)
};

} /* namespace gl */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
