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
#ifndef VPVL2_EXTENSIONS_GL_VERTEXBUNDLELAYOUT_H_
#define VPVL2_EXTENSIONS_GL_VERTEXBUNDLELAYOUT_H_

#include <vpvl2/Common.h>
#include <vpvl2/extensions/gl/CommonMacros.h>

namespace vpvl2
{
namespace extensions
{
namespace gl
{

class VPVL2_API VertexBundleLayout {
public:
    static bool allocateVertexArrayObjects(GLuint *vao, size_t size) {
        if (GLEW_VERSION_3_0 || GLEW_ARB_vertex_array_object) {
            glGenVertexArrays(size, vao);
            return true;
        }
        else if (GLEW_APPLE_vertex_array_object) {
            glGenVertexArraysAPPLE(size, vao);
            return true;
        }
        return false;
    }
    static bool releaseVertexArrayObjects(GLuint *vao, size_t size) {
        if (GLEW_VERSION_3_0 || GLEW_ARB_vertex_array_object) {
            glDeleteVertexArrays(size, vao);
            return true;
        }
        else if (GLEW_APPLE_vertex_array_object) {
            glDeleteVertexArraysAPPLE(size, vao);
            return true;
        }
        return false;
    }
    static bool bindVertexArrayObject(GLuint vao) {
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
    static bool unbindVertexArrayObject() {
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

    VertexBundleLayout()
        : m_name(0)
    {
    }
    ~VertexBundleLayout() {
        release();
    }

    bool create() {
        return allocateVertexArrayObjects(&m_name, 1);
    }
    bool release() {
        return releaseVertexArrayObjects(&m_name, 1);
    }
    bool bind() {
        return bindVertexArrayObject(m_name);
    }
    bool unbind() {
        return unbindVertexArrayObject();
    }

private:
    GLuint m_name;

    VPVL2_DISABLE_COPY_AND_ASSIGN(VertexBundleLayout)
};

} /* namespace gl */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
