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
#ifndef VPVL2_EXTENSIONS_GL_SIMPLESHADOWMAP_H_
#define VPVL2_EXTENSIONS_GL_SIMPLESHADOWMAP_H_

#include <vpvl2/IShadowMap.h>
#include <vpvl2/extensions/gl/FrameBufferObject.h>
#include <vpvl2/extensions/gl/Texture2D.h>

namespace vpvl2
{
namespace extensions
{
namespace gl
{

class SimpleShadowMap VPVL2_DECL_FINAL : public IShadowMap {
public:
    SimpleShadowMap(const IApplicationContext::FunctionResolver *resolver, vsize width, vsize height)
        : genFramebuffers(reinterpret_cast<PFNGLGENFRAMEBUFFERSPROC>(resolver->resolveSymbol("glGenFramebuffers"))),
          bindFramebuffer(reinterpret_cast<PFNGLBINDFRAMEBUFFERPROC>(resolver->resolveSymbol("glBindFramebuffer"))),
          deleteFramebuffers(reinterpret_cast<PFNGLDELETEFRAMEBUFFERSPROC>(resolver->resolveSymbol("glDeleteFramebuffers"))),
          genRenderbuffers(reinterpret_cast<PFNGLGENRENDERBUFFERSPROC>(resolver->resolveSymbol("glGenRenderbuffers"))),
          bindRenderbuffer(reinterpret_cast<PFNGLBINDRENDERBUFFERPROC>(resolver->resolveSymbol("glBindRenderbuffer"))),
          renderbufferStorage(reinterpret_cast<PFNGLRENDERBUFFERSTORAGEPROC>(resolver->resolveSymbol("glRenderbufferStorage"))),
          framebufferRenderbuffer(reinterpret_cast<PFNGLFRAMEBUFFERRENDERBUFFERPROC>(resolver->resolveSymbol("glFramebufferRenderbuffer"))),
          deleteRenderbuffers(reinterpret_cast<PFNGLDELETERENDERBUFFERSPROC>(resolver->resolveSymbol("glDeleteRenderbuffers"))),
          genTextures(reinterpret_cast<PFNGLGENTEXTURESPROC>(resolver->resolveSymbol("glGenTextures"))),
          bindTexture(reinterpret_cast<PFNGLBINDTEXTUREPROC>(resolver->resolveSymbol("glBindTexture"))),
          texParameteri(reinterpret_cast<PFNGLTEXPARAMETERIPROC>(resolver->resolveSymbol("glTexParameteri"))),
          texImage2D(reinterpret_cast<PFNGLTEXIMAGE2DPROC>(resolver->resolveSymbol("glTexImage2D"))),
          framebufferTexture2D(reinterpret_cast<PFNGLFRAMEBUFFERTEXTURE2DPROC>(resolver->resolveSymbol("glFramebufferTexture2D"))),
          deleteTextures(reinterpret_cast<PFNGLDELETETEXTURESPROC>(resolver->resolveSymbol("glDeleteTextures"))),
          m_motionRef(0),
          m_position(kZeroV3),
          m_size(Scalar(width), Scalar(height), 1),
          m_colorTexture(0),
          m_frameBuffer(0),
          m_depthBuffer(0),
          m_colorTextureRef(&m_colorTexture),
          m_distance(7.5f)
    {
    }
    ~SimpleShadowMap() {
        release();
    }

    void create() {
        release();
        genFramebuffers(1, &m_frameBuffer);
        genTextures(1, m_colorTextureRef);
        vsize width = vsize(m_size.x()), height = vsize(m_size.y());
        bindTexture(Texture2D::kGL_TEXTURE_2D, m_colorTexture);
        texImage2D(Texture2D::kGL_TEXTURE_2D, 0, kGL_RG32F, width, height, 0, kGL_RG, kGL_FLOAT, 0);
        texParameteri(Texture2D::kGL_TEXTURE_2D, BaseTexture::kGL_TEXTURE_WRAP_S, BaseTexture::kGL_CLAMP_TO_EDGE);
        texParameteri(Texture2D::kGL_TEXTURE_2D, BaseTexture::kGL_TEXTURE_WRAP_T, BaseTexture::kGL_CLAMP_TO_EDGE);
        texParameteri(Texture2D::kGL_TEXTURE_2D, BaseTexture::kGL_TEXTURE_MAG_FILTER, BaseTexture::kGL_LINEAR);
        texParameteri(Texture2D::kGL_TEXTURE_2D, BaseTexture::kGL_TEXTURE_MIN_FILTER, BaseTexture::kGL_LINEAR);
        bindTexture(Texture2D::kGL_TEXTURE_2D, 0);
        genRenderbuffers(1, &m_depthBuffer);
        bindRenderbuffer(FrameBufferObject::kGL_RENDERBUFFER, m_depthBuffer);
        renderbufferStorage(FrameBufferObject::kGL_RENDERBUFFER, FrameBufferObject::kGL_DEPTH_COMPONENT32F, width, height);
        bindRenderbuffer(FrameBufferObject::kGL_RENDERBUFFER, 0);
        bind();
        framebufferTexture2D(FrameBufferObject::kGL_FRAMEBUFFER, FrameBufferObject::kGL_COLOR_ATTACHMENT0, Texture2D::kGL_TEXTURE_2D, m_colorTexture, 0);
        framebufferRenderbuffer(FrameBufferObject::kGL_FRAMEBUFFER, FrameBufferObject::kGL_DEPTH_ATTACHMENT, FrameBufferObject::kGL_RENDERBUFFER, m_depthBuffer);
        unbind();
    }
    void bind() {
        bindFramebuffer(FrameBufferObject::kGL_FRAMEBUFFER, m_frameBuffer);
    }
    void unbind() {
        bindFramebuffer(FrameBufferObject::kGL_FRAMEBUFFER, 0);
    }
    void reset() {
        m_position.setZero();
        m_distance = 7.5;
    }

    void *textureRef() const { return m_colorTextureRef; }
    Vector3 size() const { return m_size; }
    IMotion *motion() const { return m_motionRef; }
    Vector3 position() const { return m_position; }
    Scalar distance() const { return m_distance; }
    void setMotion(IMotion *value) { m_motionRef = value; }
    void setPosition(const Vector3 &value) { m_position = value; }
    void setDistance(const Scalar &value) { m_distance = value; }

private:
    typedef void (GLAPIENTRY * PFNGLGENFRAMEBUFFERSPROC) (GLsizei n, GLuint* framebuffers);
    typedef void (GLAPIENTRY * PFNGLBINDFRAMEBUFFERPROC) (GLenum target, GLuint framebuffer);
    typedef void (GLAPIENTRY * PFNGLDELETEFRAMEBUFFERSPROC) (GLsizei n, const GLuint* framebuffers);
    typedef void (GLAPIENTRY * PFNGLGENRENDERBUFFERSPROC) (GLsizei n, GLuint* renderbuffers);
    typedef void (GLAPIENTRY * PFNGLBINDRENDERBUFFERPROC) (GLenum target, GLuint renderbuffer);
    typedef void (GLAPIENTRY * PFNGLRENDERBUFFERSTORAGEPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
    typedef void (GLAPIENTRY * PFNGLFRAMEBUFFERRENDERBUFFERPROC) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
    typedef void (GLAPIENTRY * PFNGLDELETERENDERBUFFERSPROC) (GLsizei n, const GLuint* renderbuffers);
    typedef void (GLAPIENTRY * PFNGLGENTEXTURESPROC) (GLsizei n, GLuint *textures);
    typedef void (GLAPIENTRY * PFNGLBINDTEXTUREPROC) (GLenum target, GLuint texture);
    typedef void (GLAPIENTRY * PFNGLTEXPARAMETERIPROC) (GLenum target, GLenum pname, GLint param);
    typedef void (GLAPIENTRY * PFNGLTEXIMAGE2DPROC) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
    typedef void (GLAPIENTRY * PFNGLFRAMEBUFFERTEXTURE2DPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
    typedef void (GLAPIENTRY * PFNGLDELETETEXTURESPROC) (GLsizei n, GLuint *textures);
    PFNGLGENFRAMEBUFFERSPROC genFramebuffers;
    PFNGLBINDFRAMEBUFFERPROC bindFramebuffer;
    PFNGLDELETEFRAMEBUFFERSPROC deleteFramebuffers;
    PFNGLGENRENDERBUFFERSPROC genRenderbuffers;
    PFNGLBINDRENDERBUFFERPROC bindRenderbuffer;
    PFNGLRENDERBUFFERSTORAGEPROC renderbufferStorage;
    PFNGLFRAMEBUFFERRENDERBUFFERPROC framebufferRenderbuffer;
    PFNGLDELETERENDERBUFFERSPROC deleteRenderbuffers;
    PFNGLGENTEXTURESPROC genTextures;
    PFNGLBINDTEXTUREPROC bindTexture;
    PFNGLTEXPARAMETERIPROC texParameteri;
    PFNGLTEXIMAGE2DPROC texImage2D;
    PFNGLFRAMEBUFFERTEXTURE2DPROC framebufferTexture2D;
    PFNGLDELETETEXTURESPROC deleteTextures;

    void release() {
        m_motionRef = 0;
        deleteFramebuffers(1, &m_frameBuffer);
        m_frameBuffer = 0;
        deleteTextures(1, &m_colorTexture);
        m_colorTexture = 0;
        deleteRenderbuffers(1, &m_depthBuffer);
        m_depthBuffer = 0;
    }

    IMotion *m_motionRef;
    Vector3 m_position;
    Vector3 m_size;
    GLuint m_colorTexture;
    GLuint m_frameBuffer;
    GLuint m_depthBuffer;
    GLuint *m_colorTextureRef;
    Scalar m_distance;

    VPVL2_DISABLE_COPY_AND_ASSIGN(SimpleShadowMap)
};

} /* namespace gl */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
