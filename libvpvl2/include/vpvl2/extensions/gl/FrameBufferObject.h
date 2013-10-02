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
#ifndef VPVL2_EXTENSIONS_GL_FRAMEBUFFEROBJECT_H_
#define VPVL2_EXTENSIONS_GL_FRAMEBUFFEROBJECT_H_

#include <vpvl2/IApplicationContext.h>
#include <vpvl2/ITexture.h>
#include <vpvl2/extensions/gl/BaseSurface.h>

namespace vpvl2
{
namespace extensions
{
namespace gl
{

class FrameBufferObject VPVL2_DECL_FINAL
{
public:
    class BaseRenderBuffer {
    public:
        BaseRenderBuffer(IApplicationContext::FunctionResolver *resolver, const BaseSurface::Format &format, const Vector3 &size)
            : VPVL2_BASESURFACE_INITIALIZE_FIELDS(format, size, 0),
              genRenderbuffers(reinterpret_cast<PFNGLGENRENDERBUFFERSPROC>(resolver->resolveSymbol("glGenRenderbuffers"))),
              bindRenderbuffer(reinterpret_cast<PFNGLBINDRENDERBUFFERPROC>(resolver->resolveSymbol("glBindRenderbuffer"))),
              framebufferRenderbuffer(reinterpret_cast<PFNGLFRAMEBUFFERRENDERBUFFERPROC>(resolver->resolveSymbol("glFramebufferRenderbuffer"))),
              deleteRenderbuffers(reinterpret_cast<PFNGLDELETERENDERBUFFERSPROC>(resolver->resolveSymbol("glDeleteRenderbuffers")))
        {
        }
        virtual ~BaseRenderBuffer() {
            release();
            VPVL2_BASESURFACE_DESTROY_FIELDS()
        }

        void create() {
            genRenderbuffers(1, &m_name);
            wrapGenerate();
        }
        void resize(const Vector3 &value) {
            if (value != m_size) {
                m_size = value;
                wrapGenerate();
            }
        }
        void bind() {
            bindRenderbuffer(kGL_RENDERBUFFER, m_name);
        }
        void unbind() {
            bindRenderbuffer(kGL_RENDERBUFFER, 0);
        }
        void attach(GLenum attachment) {
            framebufferRenderbuffer(kGL_FRAMEBUFFER, attachment, kGL_RENDERBUFFER, m_name);
        }
        void detach(GLenum attachment) {
            framebufferRenderbuffer(kGL_FRAMEBUFFER, attachment, kGL_RENDERBUFFER, 0);
        }

        VPVL2_BASESURFACE_DEFINE_METHODS()

        typedef void (GLAPIENTRY * PFNGLGENRENDERBUFFERSPROC) (GLsizei n, GLuint* renderbuffers);
        typedef void (GLAPIENTRY * PFNGLBINDRENDERBUFFERPROC) (GLenum target, GLuint renderbuffer);
        typedef void (GLAPIENTRY * PFNGLFRAMEBUFFERRENDERBUFFERPROC) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
        typedef void (GLAPIENTRY * PFNGLDELETERENDERBUFFERSPROC) (GLsizei n, const GLuint* renderbuffers);
        PFNGLGENRENDERBUFFERSPROC genRenderbuffers;
        PFNGLBINDRENDERBUFFERPROC bindRenderbuffer;
        PFNGLFRAMEBUFFERRENDERBUFFERPROC framebufferRenderbuffer;
        PFNGLDELETERENDERBUFFERSPROC deleteRenderbuffers;

    protected:
        virtual void generate() = 0;

        VPVL2_BASESURFACE_DEFINE_FIELDS()

        private:
            void wrapGenerate() {
            bind();
            generate();
            unbind();
        }
        void release() {
            deleteRenderbuffers(1, &m_name);
        }
    };

    class StandardRenderBuffer VPVL2_DECL_FINAL : public BaseRenderBuffer {
    public:
        StandardRenderBuffer(IApplicationContext::FunctionResolver *resolver, const BaseSurface::Format &format, const Vector3 &size)
            : BaseRenderBuffer(resolver, format, size)
        {
            renderbufferStorage = reinterpret_cast<PFNGLRENDERBUFFERSTORAGEPROC>(resolver->resolveSymbol("glRenderbufferStorage"));
        }
        ~StandardRenderBuffer() {
        }

        typedef void (GLAPIENTRY * PFNGLRENDERBUFFERSTORAGEPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
        PFNGLRENDERBUFFERSTORAGEPROC renderbufferStorage;

    private:
        void generate() {
            renderbufferStorage(kGL_RENDERBUFFER, m_format.internal, GLsizei(m_size.x()), GLsizei(m_size.y()));
        }
    };

    class MSAARenderBuffer VPVL2_DECL_FINAL : public BaseRenderBuffer {
    public:
        MSAARenderBuffer(IApplicationContext::FunctionResolver *resolver, const BaseSurface::Format &format, const Vector3 &size, int samples)
            : BaseRenderBuffer(resolver, format, size),
              m_samples(samples)
        {
            renderbufferStorageMultisample = reinterpret_cast<PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC>(resolver->resolveSymbol("glRenderbufferStorageMultisample"));
        }
        ~MSAARenderBuffer() {
            m_samples = 0;
        }

        typedef void (GLAPIENTRY * PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
        PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC renderbufferStorageMultisample;

    private:
        void generate() {
            renderbufferStorageMultisample(kGL_RENDERBUFFER, m_samples, m_format.internal, GLsizei(m_size.x()), GLsizei(m_size.y()));
        }
        int m_samples;
    };

    static const GLenum kGL_NEAREST = 0x2600;
    static const GLenum kGL_FRAMEBUFFER = 0x8D40;
    static const GLenum kGL_RENDERBUFFER = 0x8D41;
    static const GLenum kGL_READ_FRAMEBUFFER = 0x8CA8;
    static const GLenum kGL_DRAW_FRAMEBUFFER = 0x8CA9;
    static const GLenum kGL_DEPTH_COMPONENT32F = 0x8CAC;
    static const GLenum kGL_DEPTH24_STENCIL8 = 0x88F0;
    static const GLenum kGL_DEPTH32F_STENCIL8 = 0x8CAD;
    static const GLenum kGL_DEPTH_COMPONENT = 0x1902;
    static const GLenum kGL_COLOR_ATTACHMENT0 = 0x8CE0;
    static const GLenum kGL_DEPTH_ATTACHMENT = 0x8D00;
    static const GLenum kGL_STENCIL_ATTACHMENT = 0x8D20;

    static GLenum detectDepthFormat(const IApplicationContext::FunctionResolver *resolver, GLenum internalColorFormat) {
        GLenum depthFormat = kGL_DEPTH24_STENCIL8;
        if (resolver->hasExtension("ARB_texture_float")) {
            switch (internalColorFormat) {
            case kGL_RGBA32F_ARB:
            case kGL_RGB32F_ARB:
            case kGL_RGBA16F_ARB:
            case kGL_RGB16F_ARB:
                depthFormat = kGL_DEPTH32F_STENCIL8;
                break;
            default:
                break;
            }
        }
        if (resolver->hasExtension("ARB_texture_rg")) {
            switch (internalColorFormat) {
            case kGL_RG32F:
            case kGL_RG16F:
            case kGL_R32F:
            case kGL_R16F:
                depthFormat = kGL_DEPTH32F_STENCIL8;
                break;
            default:
                break;
            }
        }
        return depthFormat;
    }
    static GLenum detectDepthFormat(const IApplicationContext::FunctionResolver *resolver, const BaseSurface::Format &format) {
        return detectDepthFormat(resolver, format.internal);
    }

    FrameBufferObject(IApplicationContext::FunctionResolver *resolver, const BaseSurface::Format &format, int samples)
        : genFramebuffers(reinterpret_cast<PFNGLGENFRAMEBUFFERSPROC>(resolver->resolveSymbol("glGenFramebuffers"))),
          bindFramebuffer(reinterpret_cast<PFNGLBINDFRAMEBUFFERPROC>(resolver->resolveSymbol("glBindFramebuffer"))),
          checkFramebufferStatus(reinterpret_cast<PFNGLCHECKFRAMEBUFFERSTATUSPROC>(resolver->resolveSymbol("glCheckFramebufferStatus"))),
          framebufferTexture2D(reinterpret_cast<PFNGLFRAMEBUFFERTEXTURE2DPROC>(resolver->resolveSymbol("glFramebufferTexture2D"))),
          deleteFramebuffers(reinterpret_cast<PFNGLDELETEFRAMEBUFFERSPROC>(resolver->resolveSymbol("glDeleteFramebuffers"))),
          drawBuffers(reinterpret_cast<PFNGLDRAWBUFFERSPROC>(resolver->resolveSymbol("glDrawBuffers"))),
          blitFramebuffer(reinterpret_cast<PFNGLBLITFRAMEBUFFERPROC>(resolver->resolveSymbol("glBlitFramebuffer"))),
          readBuffer(reinterpret_cast<PFNGLREADBUFFERPROC>(resolver->resolveSymbol("glReadBuffer"))),
          m_resolver(resolver),
          m_depthStencilBufferRef(0),
          m_renderColorBufferMSAARef(0),
          m_depthStencilBufferMSAA(0),
          m_defaultRenderColorBuffer(0),
          m_defaultRenderDepthStencilBuffer(0),
          m_renderColorFormat(format),
          m_defaultFrameBuffer(0),
          m_variantFrameBuffer(0),
          m_variantFrameBufferMSAA(0),
          m_boundFrameBuffer(0),
          m_samples(samples)
    {
    }
    ~FrameBufferObject() {
        release();
    }

    void create(const Vector3 &viewport) {
        bool canUseMSAA = m_samples > 0 &&
                m_resolver->hasExtension("EXT_framebuffer_blit") &&
                m_resolver->hasExtension("EXT_framebuffer_multisample");
        if (!m_defaultFrameBuffer) {
            BaseSurface::Format depthStencilBufferFormat;
            depthStencilBufferFormat.internal = detectDepthFormat(m_resolver, m_renderColorFormat);
            genFramebuffers(1, &m_defaultFrameBuffer);
            if (canUseMSAA) {
                m_defaultRenderColorBuffer = new MSAARenderBuffer(m_resolver, m_renderColorFormat, viewport, m_samples);
                m_defaultRenderDepthStencilBuffer = new MSAARenderBuffer(m_resolver, depthStencilBufferFormat, viewport, m_samples);
            }
            else {
                m_defaultRenderColorBuffer = new StandardRenderBuffer(m_resolver, m_renderColorFormat, viewport);
                m_defaultRenderDepthStencilBuffer = new StandardRenderBuffer(m_resolver, depthStencilBufferFormat, viewport);
            }
            m_defaultRenderColorBuffer->create();
            m_defaultRenderDepthStencilBuffer->create();
            bindFrameBuffer(m_defaultFrameBuffer);
            m_defaultRenderColorBuffer->attach(kGL_COLOR_ATTACHMENT0);
            m_defaultRenderDepthStencilBuffer->attach(kGL_DEPTH_ATTACHMENT);
            m_defaultRenderDepthStencilBuffer->attach(kGL_STENCIL_ATTACHMENT);
            VPVL2_DVLOG(3, "glCheckFramebufferStatus: default=" << checkFramebufferStatus(kGL_FRAMEBUFFER));
            bindFrameBuffer(0);
        }
        if (!m_variantFrameBuffer) {
            genFramebuffers(1, &m_variantFrameBuffer);
        }
        if (!m_variantFrameBufferMSAA && canUseMSAA) {
            genFramebuffers(1, &m_variantFrameBufferMSAA);
        }
    }
    void resize(const Vector3 &size, int index) {
        const GLenum targetIndex = kGL_COLOR_ATTACHMENT0 + index;
        if (ITexture *const *textureRefPtr = m_targetIndex2TextureRefs.find(targetIndex)) {
            ITexture *textureRef = *textureRefPtr;
            textureRef->resize(size);
            m_defaultRenderColorBuffer->resize(size);
            m_defaultRenderDepthStencilBuffer->resize(size);
            if (m_depthStencilBufferRef) {
                m_depthStencilBufferRef->resize(size);
            }
            if (m_renderColorBufferMSAARef) {
                m_renderColorBufferMSAARef->resize(size);
                m_depthStencilBufferMSAA->resize(size);
            }
        }
    }
    void bindTexture(ITexture *textureRef, int index) {
        if (textureRef) {
            const BaseSurface::Format *format = reinterpret_cast<const BaseSurface::Format *>(textureRef->format());
            GLenum targetIndex = kGL_COLOR_ATTACHMENT0 + index;
            GLuint textureID = static_cast<GLuint>(textureRef->data());
            create(textureRef->size());
            bindFrameBuffer(m_variantFrameBuffer);
            framebufferTexture2D(kGL_FRAMEBUFFER, targetIndex, format->target, textureID, 0);
            VPVL2_DVLOG(3, "glCheckFramebufferStatus: variant=" << checkFramebufferStatus(kGL_FRAMEBUFFER));
            bindMSAABuffer(textureRef, targetIndex, index);
            m_targetIndex2TextureRefs.insert(targetIndex, textureRef);
        }
    }
    void bindDepthStencilBuffer(BaseRenderBuffer *depthStencilBufferRef) {
        if (depthStencilBufferRef) {
            create(depthStencilBufferRef->size());
            bindFrameBuffer(m_variantFrameBuffer);
            depthStencilBufferRef->attach(kGL_DEPTH_ATTACHMENT);
            depthStencilBufferRef->attach(kGL_STENCIL_ATTACHMENT);
            VPVL2_DVLOG(3, "glCheckFramebufferStatus: variant=" << checkFramebufferStatus(kGL_FRAMEBUFFER));
            if (m_variantFrameBufferMSAA && m_depthStencilBufferMSAA) {
                bindFrameBuffer(m_variantFrameBufferMSAA);
                m_depthStencilBufferMSAA->attach(kGL_DEPTH_ATTACHMENT);
                m_depthStencilBufferMSAA->attach(kGL_STENCIL_ATTACHMENT);
                VPVL2_DVLOG(3, "glCheckFramebufferStatus: variantMSAA=" << checkFramebufferStatus(kGL_FRAMEBUFFER));
            }
            m_depthStencilBufferRef = depthStencilBufferRef;
        }
    }
    void unbindTexture(int index) {
        const GLenum targetIndex = kGL_COLOR_ATTACHMENT0 + index;
        if (const ITexture *const *textureRefPtr = m_targetIndex2TextureRefs.find(targetIndex)) {
            const ITexture *textureRef = *textureRefPtr;
            const BaseSurface::Format *format = reinterpret_cast<const BaseSurface::Format *>(textureRef->format());
            bindFrameBuffer(m_variantFrameBuffer);
            framebufferTexture2D(kGL_FRAMEBUFFER, targetIndex, format->target, 0, 0);
            if (m_variantFrameBufferMSAA) {
                bindFrameBuffer(m_variantFrameBufferMSAA);
                m_renderColorBufferMSAARef->detach(targetIndex);
            }
            m_targetIndex2TextureRefs.remove(targetIndex);
        }
    }
    void unbindDepthStencilBuffer() {
        if (m_depthStencilBufferRef) {
            bindFrameBuffer(m_variantFrameBuffer);
            m_depthStencilBufferRef->detach(kGL_DEPTH_ATTACHMENT);
            m_depthStencilBufferRef->detach(kGL_STENCIL_ATTACHMENT);
            m_depthStencilBufferRef = 0;
            if (m_variantFrameBufferMSAA) {
                bindFrameBuffer(m_variantFrameBufferMSAA);
                m_depthStencilBufferMSAA->detach(kGL_DEPTH_ATTACHMENT);
                m_depthStencilBufferMSAA->detach(kGL_STENCIL_ATTACHMENT);
            }
        }
    }
    void unbind() {
        bindFrameBuffer(m_defaultFrameBuffer);
    }
    void readMSAABuffer(int index) {
        if (m_renderColorBufferMSAARef && m_boundFrameBuffer && m_boundFrameBuffer != m_defaultFrameBuffer) {
            const GLenum targetIndex = kGL_COLOR_ATTACHMENT0 + index;
            const Vector3 &size = m_renderColorBufferMSAARef->size();
            blit(size, size, m_variantFrameBufferMSAA, m_variantFrameBuffer, targetIndex);
        }
    }
    void transferTo(const FrameBufferObject *destination) {
        if (destination) {
            const Vector3 &size = m_renderColorBufferMSAARef ? m_renderColorBufferMSAARef->size()
                                                             : m_defaultRenderColorBuffer->size();
            const GLuint readTarget = m_defaultFrameBuffer;
            const GLuint drawTarget = destination->m_variantFrameBufferMSAA ? destination->m_variantFrameBufferMSAA
                                                                            : destination->m_variantFrameBuffer;
            blit(size, size, readTarget, drawTarget, kGL_COLOR_ATTACHMENT0);
        }
    }
    void transferToWindow(const Vector3 &viewport) {
        const Vector3 &size = m_defaultRenderColorBuffer->size();
        blit(size, viewport, m_defaultFrameBuffer, 0, kGL_COLOR_ATTACHMENT0);
        m_boundFrameBuffer = 0;
    }
    GLuint variantFrameBuffer() const { return m_variantFrameBuffer; }
    GLuint variantFrameBufferMSAA() const { return m_variantFrameBufferMSAA; }
    GLuint defaultFrameBuffer() const { return m_defaultFrameBuffer; }
    int samples() const { return m_samples; }

    typedef void (GLAPIENTRY * PFNGLGENFRAMEBUFFERSPROC) (GLsizei n, GLuint* framebuffers);
    typedef void (GLAPIENTRY * PFNGLBINDFRAMEBUFFERPROC) (GLenum target, GLuint framebuffer);
    typedef GLenum (GLAPIENTRY * PFNGLCHECKFRAMEBUFFERSTATUSPROC) (GLenum target);
    typedef void (GLAPIENTRY * PFNGLFRAMEBUFFERTEXTURE2DPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
    typedef void (GLAPIENTRY * PFNGLDELETEFRAMEBUFFERSPROC) (GLsizei n, const GLuint* framebuffers);
    typedef void (GLAPIENTRY * PFNGLDRAWBUFFERSPROC) (GLsizei n, const GLenum* bufs);
    typedef void (GLAPIENTRY * PFNGLBLITFRAMEBUFFERPROC) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
    typedef void (GLAPIENTRY * PFNGLREADBUFFERPROC) (GLenum mode);
    PFNGLGENFRAMEBUFFERSPROC genFramebuffers;
    PFNGLBINDFRAMEBUFFERPROC bindFramebuffer;
    PFNGLCHECKFRAMEBUFFERSTATUSPROC checkFramebufferStatus;
    PFNGLFRAMEBUFFERTEXTURE2DPROC framebufferTexture2D;
    PFNGLDELETEFRAMEBUFFERSPROC deleteFramebuffers;
    PFNGLDRAWBUFFERSPROC drawBuffers;
    PFNGLBLITFRAMEBUFFERPROC blitFramebuffer;
    PFNGLREADBUFFERPROC readBuffer;

private:
    void blit(const Vector3 &readSize, const Vector3 &drawSize, GLuint readTarget, GLuint drawTarget, GLenum targetIndex) {
        bindFramebuffer(kGL_DRAW_FRAMEBUFFER, drawTarget);
        bindFramebuffer(kGL_READ_FRAMEBUFFER, readTarget);
        if (drawTarget > 0) {
            drawBuffers(1, &targetIndex);
        }
        VPVL2_DVLOG(3, "glCheckFramebufferStatus: draw=" << checkFramebufferStatus(kGL_DRAW_FRAMEBUFFER) << " read=" << checkFramebufferStatus(kGL_READ_FRAMEBUFFER));
        readBuffer(targetIndex);
        blitFramebuffer(0, 0, GLint(readSize.x()), GLint(readSize.y()), 0, 0, GLint(drawSize.x()), GLint(drawSize.y()),
                        kGL_COLOR_BUFFER_BIT | kGL_DEPTH_BUFFER_BIT | kGL_STENCIL_BUFFER_BIT, kGL_NEAREST);
    }
    void bindFrameBuffer(GLuint name) {
        if (m_boundFrameBuffer != name) {
            bindFramebuffer(kGL_FRAMEBUFFER, name);
            m_boundFrameBuffer = name;
        }
    }
    void bindMSAABuffer(const ITexture *texture, GLenum targetIndex, int index) {
        if (m_variantFrameBufferMSAA) {
            BaseRenderBuffer *renderBufferRef = 0;
            if (BaseRenderBuffer *const *renderBufferPtr = m_targetIndex2RenderColorBufferMSAAs.find(index)) {
                renderBufferRef = *renderBufferPtr;
                bindFrameBuffer(m_variantFrameBufferMSAA);
            }
            else {
                const Vector3 &size = texture->size();
                const BaseSurface::Format &format = *reinterpret_cast<const BaseSurface::Format *>(texture->format());
                renderBufferRef = m_targetIndex2RenderColorBufferMSAAs.insert(index, new MSAARenderBuffer(m_resolver, format, size, m_samples));
                renderBufferRef->create();
                BaseSurface::Format depthStencilBufferFormat;
                depthStencilBufferFormat.internal = detectDepthFormat(m_resolver, format);
                m_depthStencilBufferMSAA = new MSAARenderBuffer(m_resolver, depthStencilBufferFormat, size, m_samples);
                m_depthStencilBufferMSAA->create();
                bindFrameBuffer(m_variantFrameBufferMSAA);
                m_depthStencilBufferMSAA->attach(kGL_DEPTH_ATTACHMENT);
                m_depthStencilBufferMSAA->attach(kGL_STENCIL_ATTACHMENT);
            }
            renderBufferRef->attach(targetIndex);
            VPVL2_DVLOG(3, "glCheckFramebufferStatus: variantMSAA=" << checkFramebufferStatus(kGL_FRAMEBUFFER));
            m_renderColorBufferMSAARef = renderBufferRef;
        }
    }
    void release() {
        m_targetIndex2RenderColorBufferMSAAs.releaseAll();
        delete m_defaultRenderColorBuffer;
        m_defaultRenderColorBuffer = 0;
        delete m_defaultRenderDepthStencilBuffer;
        m_defaultRenderDepthStencilBuffer = 0;
        delete m_depthStencilBufferMSAA;
        m_depthStencilBufferMSAA = 0;
        m_depthStencilBufferRef = 0;
        m_renderColorBufferMSAARef = 0;
        deleteFramebuffers(1, &m_defaultFrameBuffer);
        m_defaultFrameBuffer = 0;
        deleteFramebuffers(1, &m_variantFrameBuffer);
        m_variantFrameBuffer = 0;
        deleteFramebuffers(1, &m_variantFrameBufferMSAA);
        m_variantFrameBufferMSAA = 0;
        m_boundFrameBuffer = 0;
    }

    IApplicationContext::FunctionResolver *m_resolver;
    PointerHash<HashInt, BaseRenderBuffer> m_targetIndex2RenderColorBufferMSAAs;
    Hash<HashInt, ITexture *> m_targetIndex2TextureRefs;
    BaseRenderBuffer *m_depthStencilBufferRef;
    BaseRenderBuffer *m_renderColorBufferMSAARef;
    BaseRenderBuffer *m_depthStencilBufferMSAA;
    BaseRenderBuffer *m_defaultRenderColorBuffer;
    BaseRenderBuffer *m_defaultRenderDepthStencilBuffer;
    BaseSurface::Format m_renderColorFormat;
    GLuint m_defaultFrameBuffer;
    GLuint m_variantFrameBuffer;
    GLuint m_variantFrameBufferMSAA;
    GLuint m_boundFrameBuffer;
    int m_samples;

    VPVL2_DISABLE_COPY_AND_ASSIGN(FrameBufferObject)
};

} /* namespace gl */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
