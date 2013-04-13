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
#ifndef VPVL2_EXTENSIONS_GL_FRAMEBUFFEROBJECT_H_
#define VPVL2_EXTENSIONS_GL_FRAMEBUFFEROBJECT_H_

#include <vpvl2/ITexture.h>
#include <vpvl2/extensions/gl/BaseSurface.h>

namespace vpvl2
{
namespace extensions
{
namespace gl
{

class FrameBufferObject
{
public:
    class BaseRenderBuffer {
    public:
        BaseRenderBuffer(const BaseSurface::Format &format, const Vector3 &size)
            : VPVL2_BASESURFACE_INITIALIZE_FIELDS(format, size, 0)
        {
        }
        virtual ~BaseRenderBuffer() {
            release();
            VPVL2_BASESURFACE_DESTROY_FIELDS()
        }

        void create() {
            glGenRenderbuffers(1, &m_name);
            wrapGenerate();
        }
        void resize(const Vector3 &value) {
            if (value != m_size) {
                m_size = value;
                wrapGenerate();
            }
        }
        void bind() {
            glBindRenderbuffer(GL_RENDERBUFFER, m_name);
        }
        void unbind() {
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
        }

        VPVL2_BASESURFACE_DEFINE_METHODS()

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
            glDeleteRenderbuffers(1, &m_name);
        }
    };

    class StandardRenderBuffer : public BaseRenderBuffer {
    public:
        StandardRenderBuffer(const BaseSurface::Format &format, const Vector3 &size)
            : BaseRenderBuffer(format, size)
        {
        }
        ~StandardRenderBuffer() {
        }

    private:
        void generate() {
            glRenderbufferStorage(GL_RENDERBUFFER, m_format.internal, GLsizei(m_size.x()), GLsizei(m_size.y()));
        }
    };

    class MSAARenderBuffer : public BaseRenderBuffer {
    public:
        MSAARenderBuffer(const BaseSurface::Format &format, const Vector3 &size, int samples)
            : BaseRenderBuffer(format, size),
              m_samples(samples)
        {
        }
        ~MSAARenderBuffer() {
            m_samples = 0;
        }

    private:
        void generate() {
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_samples, m_format.internal,
                                             GLsizei(m_size.x()), GLsizei(m_size.y()));
        }
        int m_samples;
    };

    static GLenum detectDepthFormat(GLenum internalColorFormat) {
        GLenum depthFormat = GL_DEPTH24_STENCIL8;
        if (GLEW_ARB_depth_buffer_float) {
            switch (internalColorFormat) {
            case GL_RGBA32F:
            case GL_RGB32F:
            case GL_RG32F:
            case GL_RGBA16F:
            case GL_RGB16F:
            case GL_RG16F:
            case GL_R32F:
            case GL_R16F:
                depthFormat = GL_DEPTH32F_STENCIL8;
                break;
            default:
                break;
            }
        }
        return depthFormat;
    }
    static GLenum detectDepthFormat(const BaseSurface::Format &format) {
        return detectDepthFormat(format.internal);
    }

    FrameBufferObject(const BaseSurface::Format &format, int samples)
        : m_depthStencilBufferRef(0),
          m_renderColorBufferMSAARef(0),
          m_depthStencilBufferMSAA(0),
          m_defaultRenderColorBuffer(0),
          m_defaultRenderDepthStencilBuffer(0),
          m_renderColorFormat(format),
          m_defaultFrameBuffer(0),
          m_variantFrameBuffer(0),
          m_variantFrameBufferMSAA(0),
          m_boundRef(0),
          m_samples(samples)
    {
    }
    ~FrameBufferObject() {
        release();
    }

    void create(const Vector3 &viewport) {
        if (!m_defaultFrameBuffer) {
            glGenFramebuffers(1, &m_defaultFrameBuffer);
            m_defaultRenderColorBuffer = new StandardRenderBuffer(m_renderColorFormat, viewport);
            m_defaultRenderColorBuffer->create();
            BaseSurface::Format depthStencilBufferFormat;
            depthStencilBufferFormat.internal = detectDepthFormat(m_renderColorFormat);
            m_defaultRenderDepthStencilBuffer = new StandardRenderBuffer(depthStencilBufferFormat, viewport);
            m_defaultRenderDepthStencilBuffer->create();
            bindFrameBuffer(m_defaultFrameBuffer);
            const GLuint renderColorBuffer = m_defaultRenderColorBuffer->data();
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderColorBuffer);
            const GLuint renderDepthStencilBuffer = m_defaultRenderDepthStencilBuffer->data();
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderDepthStencilBuffer);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderDepthStencilBuffer);
            bindFrameBuffer(0);
        }
        if (!m_variantFrameBuffer) {
            glGenFramebuffers(1, &m_variantFrameBuffer);
        }
        if (!m_variantFrameBufferMSAA && m_samples > 0 && GLEW_EXT_framebuffer_blit && GLEW_EXT_framebuffer_multisample) {
            glGenFramebuffers(1, &m_variantFrameBufferMSAA);
        }
    }
    void resize(const Vector3 &size, int index) {
        const GLenum targetIndex = GL_COLOR_ATTACHMENT0 + index;
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
            GLenum targetIndex = GL_COLOR_ATTACHMENT0 + index;
            GLuint textureID = static_cast<GLuint>(textureRef->data());
            create(textureRef->size());
            bindFrameBuffer(m_variantFrameBuffer);
            glFramebufferTexture2D(GL_FRAMEBUFFER, targetIndex, format->target, textureID, 0);
            bindMSAABuffer(textureRef, targetIndex, index);
            m_targetIndex2TextureRefs.insert(targetIndex, textureRef);
        }
    }
    void bindDepthStencilBuffer(BaseRenderBuffer *depthStencilBufferRef) {
        if (depthStencilBufferRef) {
            create(depthStencilBufferRef->size());
            bindFrameBuffer(m_variantFrameBuffer);
            GLuint name = static_cast<GLuint>(depthStencilBufferRef->data());
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, name);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, name);
            if (m_variantFrameBufferMSAA && m_depthStencilBufferMSAA) {
                name = static_cast<GLuint>(m_depthStencilBufferMSAA->data());
                bindFrameBuffer(m_variantFrameBufferMSAA);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, name);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, name);
            }
            m_depthStencilBufferRef = depthStencilBufferRef;
        }
    }
    void unbindTexture(int index) {
        const GLenum targetIndex = GL_COLOR_ATTACHMENT0 + index;
        if (const ITexture *const *textureRefPtr = m_targetIndex2TextureRefs.find(targetIndex)) {
            const ITexture *textureRef = *textureRefPtr;
            const BaseSurface::Format *format = reinterpret_cast<const BaseSurface::Format *>(textureRef->format());
            bindFrameBuffer(m_variantFrameBuffer);
            glFramebufferTexture2D(GL_FRAMEBUFFER, targetIndex, format->target, 0, 0);
            if (m_variantFrameBufferMSAA) {
                bindFrameBuffer(m_variantFrameBufferMSAA);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, targetIndex, GL_RENDERBUFFER, 0);
            }
            m_targetIndex2TextureRefs.remove(targetIndex);
        }
    }
    void unbindDepthStencilBuffer() {
        if (m_depthStencilBufferRef) {
            bindFrameBuffer(m_variantFrameBuffer);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
            m_depthStencilBufferRef = 0;
            if (m_variantFrameBufferMSAA) {
                bindFrameBuffer(m_variantFrameBufferMSAA);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
            }
        }
    }
    void unbind() {
        bindFrameBuffer(m_defaultFrameBuffer);
    }
    void readMSAABuffer(int index) {
        if (m_variantFrameBufferMSAA && m_renderColorBufferMSAARef) {
            const GLenum targetIndex = GL_COLOR_ATTACHMENT0 + index;
            blit(m_renderColorBufferMSAARef->size(), m_variantFrameBufferMSAA, m_variantFrameBuffer, targetIndex);
        }
    }
    void transferTo(const FrameBufferObject *destination) {
        if (destination) {
            const Vector3 &size = m_renderColorBufferMSAARef ? m_renderColorBufferMSAARef->size()
                                                             : m_defaultRenderColorBuffer->size();
            GLuint drawTarget = destination->m_variantFrameBufferMSAA ? destination->m_variantFrameBufferMSAA
                                                              : destination->m_variantFrameBuffer;
            blit(size, m_defaultFrameBuffer, drawTarget, GL_COLOR_ATTACHMENT0);
        }
    }
    void transferToWindow(const Vector3 &viewport) {
        const Vector3 &size = m_defaultRenderColorBuffer->size();
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_defaultFrameBuffer);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glBlitFramebuffer(0, 0, GLint(size.x()), GLint(size.y()), 0, 0, GLint(viewport.x()), GLint(viewport.y()),
                          GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    }
    GLuint variantFrameBuffer() const { return m_variantFrameBuffer; }
    GLuint variantFrameBufferMSAA() const { return m_variantFrameBufferMSAA; }
    GLuint defaultFrameBuffer() const { return m_defaultFrameBuffer; }
    int samples() const { return m_samples; }

private:
    void bindFrameBuffer(GLuint name) {
        if (m_boundRef != name) {
            glBindFramebuffer(GL_FRAMEBUFFER, name);
            m_boundRef = name;
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
                renderBufferRef = m_targetIndex2RenderColorBufferMSAAs.insert(index, new MSAARenderBuffer(format, size, m_samples));
                renderBufferRef->create();
                BaseSurface::Format depthStencilBufferFormat;
                depthStencilBufferFormat.internal = detectDepthFormat(format);
                m_depthStencilBufferMSAA = new MSAARenderBuffer(depthStencilBufferFormat, size, m_samples);
                m_depthStencilBufferMSAA->create();
                bindFrameBuffer(m_variantFrameBufferMSAA);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                                          static_cast<GLuint>(m_depthStencilBufferMSAA->data()));
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                          static_cast<GLuint>(m_depthStencilBufferMSAA->data()));
            }
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, targetIndex, GL_RENDERBUFFER,
                                      static_cast<GLuint>(renderBufferRef->data()));
            m_renderColorBufferMSAARef = renderBufferRef;
        }
    }
    void blit(const Vector3 &size, GLuint readTarget, GLuint drawTarget, GLenum targetIndex) {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawTarget);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, readTarget);
        glDrawBuffers(1, &targetIndex);
        glReadBuffer(targetIndex);
        glBlitFramebuffer(0, 0, GLint(size.x()), GLint(size.y()), 0, 0, GLint(size.x()), GLint(size.y()),
                          GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
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
        glDeleteFramebuffers(1, &m_defaultFrameBuffer);
        m_defaultFrameBuffer = 0;
        glDeleteFramebuffers(1, &m_variantFrameBuffer);
        m_variantFrameBuffer = 0;
        glDeleteFramebuffers(1, &m_variantFrameBufferMSAA);
        m_variantFrameBufferMSAA = 0;
        m_boundRef = 0;
    }

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
    GLuint m_boundRef;
    int m_samples;

    VPVL2_DISABLE_COPY_AND_ASSIGN(FrameBufferObject)
};

} /* namespace gl */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
