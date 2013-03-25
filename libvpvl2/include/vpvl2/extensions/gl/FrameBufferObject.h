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
#include <vpvl2/extensions/gl/AbstractTexture.h>

namespace vpvl2
{
namespace extensions
{
namespace gl
{

class FrameBufferObject
{
public:
    class ExternalTexture : public ITexture {
    public:
        ExternalTexture(const AbstractSurface::Format &format, const Vector3 &size, GLuint name, GLuint sampler)
            : VPVL2_ABSTRACTSURFACE_INITIALIZE_FIELDS(format, size, sampler)
        {
            m_name = name;
        }
        ~ExternalTexture() {
            VPVL2_ABSTRACTSURFACE_DESTROY_FIELDS()
        }

        /* do nothing */
        void create() {}
        void bind() {}
        void resize(const Vector3 & /* size */) {}
        void unbind() {}
        void release() {}

        VPVL2_ABSTRACTSURFACE_DEFINE_METHODS()

    private:
        void generate() {}

        VPVL2_ABSTRACTSURFACE_DEFINE_FIELDS()
    };

    class AbstractRenderBuffer {
    public:
        AbstractRenderBuffer(const AbstractSurface::Format &format, const Vector3 &size)
            : VPVL2_ABSTRACTSURFACE_INITIALIZE_FIELDS(format, size, 0)
        {
        }
        virtual ~AbstractRenderBuffer() {
            release();
            VPVL2_ABSTRACTSURFACE_DESTROY_FIELDS()
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

        VPVL2_ABSTRACTSURFACE_DEFINE_METHODS()

    protected:
        virtual void generate() = 0;

        VPVL2_ABSTRACTSURFACE_DEFINE_FIELDS()

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

    class StandardRenderBuffer : public AbstractRenderBuffer {
    public:
        StandardRenderBuffer(const AbstractSurface::Format &format, const Vector3 &size)
            : AbstractRenderBuffer(format, size)
        {
        }
        ~StandardRenderBuffer() {
        }

    private:
        void generate() {
            glRenderbufferStorage(GL_RENDERBUFFER, m_format.internal, GLsizei(m_size.x()), GLsizei(m_size.y()));
        }
    };

    class MSAARenderBuffer : public AbstractRenderBuffer {
    public:
        MSAARenderBuffer(const AbstractSurface::Format &format, const Vector3 &size, int samples)
            : AbstractRenderBuffer(format, size),
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

    static GLenum detectDepthFormat(GLenum colorFormat) {
        GLenum depthFormat = GL_DEPTH24_STENCIL8;
        if (GLEW_ARB_depth_buffer_float) {
            switch (colorFormat) {
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

    FrameBufferObject(int samples)
        : m_depthStencilBufferRef(0),
          m_renderBufferMSAARef(0),
          m_depthStencilBufferMSAA(0),
          m_fbo(0),
          m_fboMSAA(0),
          m_boundRef(0),
          m_samples(samples)
    {
    }
    ~FrameBufferObject() {
        release();
    }

    void create() {
        glGenFramebuffers(1, &m_fbo);
        if (m_samples > 0 && GLEW_EXT_framebuffer_blit && GLEW_EXT_framebuffer_multisample) {
            glGenFramebuffers(1, &m_fboMSAA);
        }
    }
    void resize(const Vector3 &size, int index) {
        const GLenum targetIndex = GL_COLOR_ATTACHMENT0 + index;
        if (ITexture *const *textureRefPtr = m_targetIndex2TextureRefs.find(targetIndex)) {
            ITexture *textureRef = *textureRefPtr;
            textureRef->resize(size);
            if (m_renderBufferMSAARef) {
                m_renderBufferMSAARef->resize(size);
                m_depthStencilBufferMSAA->resize(size);
            }
        }
    }
    void bindTexture(ITexture *textureRef, int index) {
        if (textureRef) {
            const GLenum targetIndex = GL_COLOR_ATTACHMENT0 + index;
            m_targetIndex2TextureRefs.insert(targetIndex, textureRef);
            bindFrameBuffer(m_fbo);
            glFramebufferTexture2D(GL_FRAMEBUFFER, targetIndex, GL_TEXTURE_2D,
                                   static_cast<GLuint>(textureRef->data()), 0);
            bindMSAABuffer(textureRef, targetIndex, index);
        }
    }
    void bindDepthStencilBuffer(const AbstractRenderBuffer *depthStencilBufferRef) {
        if (depthStencilBufferRef) {
            bindFrameBuffer(m_fbo);
            GLuint name = static_cast<GLuint>(depthStencilBufferRef->data());
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, name);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, name);
            m_depthStencilBufferRef = depthStencilBufferRef;
            if (m_fboMSAA && m_depthStencilBufferMSAA) {
                name = static_cast<GLuint>(m_depthStencilBufferMSAA->data());
                bindFrameBuffer(m_fboMSAA);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, name);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, name);
            }
        }
    }
    void unbindTexture(int index) {
        const GLenum targetIndex = GL_COLOR_ATTACHMENT0 + index;
        if (const ITexture *const *textureRefPtr = m_targetIndex2TextureRefs.find(targetIndex)) {
            const ITexture *textureRef = *textureRefPtr;
            bindFrameBuffer(m_fbo);
            glFramebufferTexture2D(GL_FRAMEBUFFER, targetIndex, textureRef->format(), 0, 0);
            if (m_fboMSAA) {
                bindFrameBuffer(m_fboMSAA);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, targetIndex, GL_RENDERBUFFER, 0);
            }
            m_targetIndex2TextureRefs.remove(targetIndex);
        }
    }
    void unbindDepthStencilBuffer() {
        bindFrameBuffer(m_fbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
        m_depthStencilBufferRef = 0;
        if (m_fboMSAA) {
            bindFrameBuffer(m_fboMSAA);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
        }
    }
    void unbind() {
        if (m_boundRef) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            m_boundRef = 0;
        }
    }
    void readMSAABuffer(int index) {
        if (m_fboMSAA && m_renderBufferMSAARef) {
            const GLenum targetIndex = GL_COLOR_ATTACHMENT0 + index;
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboMSAA);
            glDrawBuffers(1, &targetIndex);
            glReadBuffer(targetIndex);
            const Vector3 &size = m_renderBufferMSAARef->size();
            glBlitFramebuffer(0, 0, GLint(size.x()), GLint(size.y()), 0, 0, GLint(size.x()), GLint(size.y()),
                              GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
        }
    }
    void transferTo(FrameBufferObject *destination, const btAlignedObjectArray<GLuint> &renderColorTargets) {
        if (destination) {
            const int nRenderColorTargets = renderColorTargets.size();
            for (int i = 0; i < nRenderColorTargets; i++) {
                const GLuint targetIndex = renderColorTargets[i];
                const GLuint index = targetIndex - GL_COLOR_ATTACHMENT0;
                readMSAABuffer(index);
                if (ITexture *const *textureRefPtr = m_targetIndex2TextureRefs.find(targetIndex)) {
                    ITexture *textureRef = *textureRefPtr;
                    destination->bindTexture(textureRef, index);
                }
            }
            destination->bindDepthStencilBuffer(m_depthStencilBufferRef);
        }
    }
    void transferToWindow(const btAlignedObjectArray<GLuint> &renderColorTargets, const Vector3 &viewport) {
        const int nRenderColorTargets = renderColorTargets.size();
        for (int i = 0; i < nRenderColorTargets; i++) {
            const int target2 = renderColorTargets[i], index2 = target2 - GL_COLOR_ATTACHMENT0;
            readMSAABuffer(index2);
        }
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        const Vector3 &size = m_depthStencilBufferRef->size();
        glBlitFramebuffer(0, 0, GLint(size.x()), GLint(size.y()), 0, 0, GLint(viewport.x()), GLint(viewport.y()),
                          GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        for (int i = 0; i < nRenderColorTargets; i++) {
            const int target2 = renderColorTargets[i], index2 = target2 - GL_COLOR_ATTACHMENT0;
            unbindTexture(index2);
            unbindDepthStencilBuffer();
        }
        unbind();
    }
    GLuint name() const { return m_fbo; }
    GLuint nameForMSAA() const { return m_fboMSAA; }
    int samples() const { return m_samples; }

private:
    void bindFrameBuffer(GLuint name) {
        if (m_boundRef != name) {
            glBindFramebuffer(GL_FRAMEBUFFER, name);
            m_boundRef = name;
        }
    }
    void bindMSAABuffer(const ITexture *texture, GLenum targetIndex, int index) {
        if (m_fboMSAA) {
            AbstractRenderBuffer *renderBufferRef = 0;
            if (AbstractRenderBuffer *const *renderBufferPtr = m_targetIndex2RenderBufferMSAAs.find(index)) {
                renderBufferRef = *renderBufferPtr;
                bindFrameBuffer(m_fboMSAA);
            }
            else {
                const Vector3 &size = texture->size();
                const AbstractSurface::Format &format = *reinterpret_cast<const AbstractSurface::Format *>(texture->format());
                renderBufferRef = m_targetIndex2RenderBufferMSAAs.insert(index, new MSAARenderBuffer(format, size, m_samples));
                renderBufferRef->create();
                AbstractSurface::Format depthFormat;
                depthFormat.internal = detectDepthFormat(format.internal);
                m_depthStencilBufferMSAA = new MSAARenderBuffer(format, size, m_samples);
                m_depthStencilBufferMSAA->create();
                bindFrameBuffer(m_fboMSAA);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                                          static_cast<GLuint>(m_depthStencilBufferMSAA->data()));
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                          static_cast<GLuint>(m_depthStencilBufferMSAA->data()));
            }
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, targetIndex, GL_RENDERBUFFER,
                                      static_cast<GLuint>(renderBufferRef->data()));
            m_renderBufferMSAARef = renderBufferRef;
        }
    }
    void release() {
        delete m_depthStencilBufferMSAA;
        m_depthStencilBufferMSAA = 0;
        m_depthStencilBufferRef = 0;
        m_renderBufferMSAARef = 0;
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
        glDeleteFramebuffers(1, &m_fboMSAA);
        m_fboMSAA = 0;
        m_boundRef = 0;
    }

    PointerHash<HashInt, AbstractRenderBuffer> m_targetIndex2RenderBufferMSAAs;
    Hash<HashInt, ITexture *> m_targetIndex2TextureRefs;
    const AbstractRenderBuffer *m_depthStencilBufferRef;
    AbstractRenderBuffer *m_renderBufferMSAARef;
    AbstractRenderBuffer *m_depthStencilBufferMSAA;
    GLuint m_fbo;
    GLuint m_fboMSAA;
    GLuint m_boundRef;
    int m_samples;

    VPVL2_DISABLE_COPY_AND_ASSIGN(FrameBufferObject)
};

} /* namespace gl */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
