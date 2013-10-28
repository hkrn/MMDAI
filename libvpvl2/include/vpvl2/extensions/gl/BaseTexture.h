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
#ifndef VPVL2_EXTENSIONS_GL_BASETEXTURE_H_
#define VPVL2_EXTENSIONS_GL_BASETEXTURE_H_

#include <vpvl2/ITexture.h>
#include <vpvl2/extensions/gl/BaseSurface.h>

namespace vpvl2
{
namespace extensions
{
namespace gl
{

class BaseTexture : public ITexture {
public:
    static const GLenum kGL_TEXTURE = 0x1702;
    static const GLenum kGL_TEXTURE0 = 0x84C0;
    static const GLenum kGL_NEAREST = 0x2600;
    static const GLenum kGL_LINEAR = 0x2601;
    static const GLenum kGL_NEAREST_MIPMAP_NEAREST = 0x2700;
    static const GLenum kGL_LINEAR_MIPMAP_NEAREST = 0x2701;
    static const GLenum kGL_NEAREST_MIPMAP_LINEAR = 0x2702;
    static const GLenum kGL_LINEAR_MIPMAP_LINEAR = 0x2703;
    static const GLenum kGL_TEXTURE_MAG_FILTER = 0x2800;
    static const GLenum kGL_TEXTURE_MIN_FILTER = 0x2801;
    static const GLenum kGL_TEXTURE_WRAP_S = 0x2802;
    static const GLenum kGL_TEXTURE_WRAP_T = 0x2803;
    static const GLenum kGL_CLAMP_TO_EDGE = 0x812F;

    BaseTexture(const IApplicationContext::FunctionResolver *resolver, const BaseSurface::Format &format, const Vector3 &size, GLuint sampler)
        : genTextures(reinterpret_cast<PFNGLGENTEXTURESPROC>(resolver->resolveSymbol("glGenTextures"))),
          bindTexture(reinterpret_cast<PFNGLBINDTEXTUREPROC>(resolver->resolveSymbol("glBindTexture"))),
          deleteTextures(reinterpret_cast<PFNGLDELETETEXTURESPROC>(resolver->resolveSymbol("glDeleteTextures"))),
          VPVL2_BASESURFACE_INITIALIZE_FIELDS(format, size, sampler)
    {
    }
    ~BaseTexture() {
        release();
        VPVL2_BASESURFACE_DESTROY_FIELDS()
    }
    void create() {
        genTextures(1, &m_name);
        wrapGenerate();
    }
    void bind() {
        bindTexture(m_format.target, m_name);
    }
    void unbind() {
        bindTexture(m_format.target, 0);
    }
    void release() {
        deleteTextures(1, &m_name);
    }
    void resize(const Vector3 &value) {
        if (value != m_size) {
            m_size = value;
            wrapGenerate();
        }
    }

    VPVL2_BASESURFACE_DEFINE_METHODS()

protected:
    virtual void generate() = 0;

    typedef void (GLAPIENTRY * PFNGLGENTEXTURESPROC) (GLsizei n, GLuint *textures);
    typedef void (GLAPIENTRY * PFNGLBINDTEXTUREPROC) (GLenum target, GLuint texture);
    typedef void (GLAPIENTRY * PFNGLDELETETEXTURESPROC) (GLsizei n, GLuint *textures);
    PFNGLGENTEXTURESPROC genTextures;
    PFNGLBINDTEXTUREPROC bindTexture;
    PFNGLDELETETEXTURESPROC deleteTextures;

    VPVL2_BASESURFACE_DEFINE_FIELDS()

private:
    void wrapGenerate() {
        if (!m_size.isZero()) {
            bind();
            generate();
            unbind();
        }
    }
};

} /* namespace gl */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
