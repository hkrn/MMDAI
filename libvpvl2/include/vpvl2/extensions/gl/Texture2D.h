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
#ifndef VPVL2_EXTENSIONS_GL_TEXTURE2D_H_
#define VPVL2_EXTENSIONS_GL_TEXTURE2D_H_

#include <vpvl2/ITexture.h>
#include <vpvl2/extensions/gl/BaseTexture.h>

namespace vpvl2
{
namespace extensions
{
namespace gl
{

class Texture2D VPVL2_DECL_FINAL :  public BaseTexture {
public:
    static const GLenum kGL_TEXTURE_2D = 0x0DE1;

    Texture2D(const IApplicationContext::FunctionResolver *resolver, const BaseSurface::Format &format, const Vector3 &size, GLuint sampler)
        : BaseTexture(resolver, format, size, sampler),
          texImage2D(reinterpret_cast<PFNGLTEXIMAGE2DPROC>(resolver->resolveSymbol("glTexImage2D"))),
          texSubImage2D(reinterpret_cast<PFNGLTEXSUBIMAGE2DPROC>(resolver->resolveSymbol("glTexSubImage2D")))
    {
        m_format.target = kGL_TEXTURE_2D;
    }
    ~Texture2D() {
    }

    void allocate(const void *pixels) {
        texImage2D(m_format.target, 0, m_format.internal, GLsizei(m_size.x()), GLsizei(m_size.y()), 0, m_format.external, m_format.type, pixels);
    }
    void write(const void *pixels) {
        texSubImage2D(m_format.target, 0, 0, 0, GLsizei(m_size.x()), GLsizei(m_size.y()), m_format.external, m_format.type, pixels);
    }

private:
    typedef void (GLAPIENTRY * PFNGLTEXIMAGE2DPROC) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
    typedef void (GLAPIENTRY * PFNGLTEXSUBIMAGE2DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
    PFNGLTEXIMAGE2DPROC texImage2D;
    PFNGLTEXSUBIMAGE2DPROC texSubImage2D;
};

} /* namespace gl */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
