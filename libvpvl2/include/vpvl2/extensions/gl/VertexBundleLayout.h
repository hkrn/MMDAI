/**

 Copyright (c) 2010-2013  hkrn

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
#ifndef VPVL2_EXTENSIONS_GL_VERTEXBUNDLELAYOUT_H_
#define VPVL2_EXTENSIONS_GL_VERTEXBUNDLELAYOUT_H_

#include <vpvl2/extensions/gl/Global.h>

namespace vpvl2
{
namespace extensions
{
namespace gl
{

static const char *kVertexArrayObjectExtensionCandidates[] = {
    "ARB_vertex_array_object",
    "APPLE_vertex_array_object",
    0
};

class VertexBundleLayout VPVL2_DECL_FINAL {
public:
    static const GLenum kGL_VERTEX_ARRAY = 0x8074;

    VertexBundleLayout(const IApplicationContext::FunctionResolver *resolver)
        : genVertexArrays(0),
          bindVertexArray(0),
          deleteVertexArrays(0),
          m_hasExtension(resolver->query(IApplicationContext::FunctionResolver::kQueryVersion) >= makeVersion(3, 0) ||
                         hasAnyExtensions(kVertexArrayObjectExtensionCandidates, resolver)),
          m_name(0)
    {
        if (resolver->hasExtension("APPLE_vertex_array_object")) {
            genVertexArrays = reinterpret_cast<PFNGLGENVERTEXARRAYSPROC>(resolver->resolveSymbol("glGenVertexArraysAPPLE"));
            bindVertexArray = reinterpret_cast<PFNGLBINDVERTEXARRAYPROC>(resolver->resolveSymbol("glBindVertexArrayAPPLE"));
            deleteVertexArrays = reinterpret_cast<PFNGLDELETEVERTEXARRAYSPROC>(resolver->resolveSymbol("glDeleteVertexArraysAPPLE"));
        }
        else if (m_hasExtension) {
            genVertexArrays = reinterpret_cast<PFNGLGENVERTEXARRAYSPROC>(resolver->resolveSymbol("glGenVertexArrays"));
            bindVertexArray = reinterpret_cast<PFNGLBINDVERTEXARRAYPROC>(resolver->resolveSymbol("glBindVertexArray"));
            deleteVertexArrays = reinterpret_cast<PFNGLDELETEVERTEXARRAYSPROC>(resolver->resolveSymbol("glDeleteVertexArrays"));
        }
    }
    ~VertexBundleLayout() {
        release();
    }

    bool create() {
        if (m_hasExtension) {
            genVertexArrays(1, &m_name);
            return m_name != 0;
        }
        return false;
    }
    void release() {
        if (m_hasExtension) {
            deleteVertexArrays(1, &m_name);
        }
    }
    bool bind() {
        if (m_hasExtension && m_name) {
            bindVertexArray(m_name);
            return true;
        }
        return false;
    }
    bool unbind() {
        if (m_hasExtension) {
            bindVertexArray(0);
            return true;
        }
        return false;
    }
    GLuint name() const {
        return m_name;
    }

private:
    typedef void (GLAPIENTRY * PFNGLGENVERTEXARRAYSPROC) (GLsizei n, GLuint* arrays);
    typedef void (GLAPIENTRY * PFNGLBINDVERTEXARRAYPROC) (GLuint array);
    typedef void (GLAPIENTRY * PFNGLDELETEVERTEXARRAYSPROC) (GLsizei n, const GLuint* arrays);
    PFNGLGENVERTEXARRAYSPROC genVertexArrays;
    PFNGLBINDVERTEXARRAYPROC bindVertexArray;
    PFNGLDELETEVERTEXARRAYSPROC deleteVertexArrays;
    const bool m_hasExtension;
    GLuint m_name;

    VPVL2_DISABLE_COPY_AND_ASSIGN(VertexBundleLayout)
};

} /* namespace gl */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
