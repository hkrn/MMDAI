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

#include <vpvl2/Common.h>
#include <vpvl2/extensions/gl/CommonMacros.h>

namespace vpvl2
{
namespace extensions
{
namespace gl
{

class FrameBufferObject
{
public:
    class AbstractSurface {
    public:
        AbstractSurface(const Vector3 &size, GLenum format)
            : m_name(0),
              m_size(size),
              m_format(format)

        {
        }
        virtual ~AbstractSurface() {
        }
        Vector3 size() const { return m_size; }
        GLenum format() const { return m_format; }
        GLenum name() const { return m_name; }
    protected:
        GLuint m_name;
    private:
        const Vector3 m_size;
        const GLenum m_format;
    };
    class AbstractTexture : public AbstractSurface {
    public:
        AbstractTexture(const Vector3 &size, GLenum format, GLenum internalFormat, GLenum type, GLenum target)
            : AbstractSurface(size, format),
              m_internalFormat(internalFormat),
              m_type(type),
              m_target(target)
        {
        }
        ~AbstractTexture() {
            release();
        }
        void create() {
            glGenTextures(1, &m_name);
            glBindTexture(m_target, m_name);
            generate();
            glBindTexture(m_target, 0);
        }
        GLenum internalFormat() const { return m_internalFormat; }
        GLenum type() const { return m_type; }
        GLenum target() const { return m_target; }
    protected:
        virtual void generate() = 0;
    private:
        void release() {
            glDeleteTextures(1, &m_name);
        }
        const GLenum m_internalFormat;
        const GLenum m_type;
        const GLenum m_target;
    };
    class ExternalTexture : public AbstractTexture {
    public:
        ExternalTexture(const Vector3 &size, GLenum format, GLenum internalFormat, GLenum type, GLenum target, GLenum name)
            : AbstractTexture(size, format, internalFormat, type, target)
        {
            m_name = name;
        }
        ~ExternalTexture() {
        }
    private:
        void generate() {}
    };
    class Texture2D : public AbstractTexture {
    public:
        Texture2D(const Vector3 &size, GLenum format, GLenum internalFormat, GLenum type)
            : AbstractTexture(size, format, internalFormat, type, GL_TEXTURE_2D)
        {
        }
        ~Texture2D() {
        }
    private:
        void generate() {
            const Vector3 &s = size();
            glTexImage2D(target(), 0, internalFormat(), s.x(), s.y(), 0, format(), type(), 0);
        }
    };
    class Texture3D : public AbstractTexture {
    public:
        Texture3D(const Vector3 &size, GLenum format, GLenum internalFormat, GLenum type)
            : AbstractTexture(size, format, internalFormat, type, GL_TEXTURE_3D)
        {
        }
        ~Texture3D() {
        }
    private:
        void generate() {
            const Vector3 &s = size();
            glTexImage3D(target(), 0, internalFormat(), s.x(), s.y(), s.z(), 0, format(), type(), 0);
        }
    };
    class AbstractRenderBuffer : public AbstractSurface {
    public:
        AbstractRenderBuffer(const Vector3 &size, GLenum format)
            : AbstractSurface(size, format)
        {
        }
        ~AbstractRenderBuffer() {
            release();
        }
        void create() {
            glGenRenderbuffers(1, &m_name);
            bind();
            generate();
            unbind();
        }
        void bind() {
            glBindRenderbuffer(GL_RENDERBUFFER, m_name);
        }
        void unbind() {
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
        }
    protected:
        virtual void generate() = 0;
    private:
        void release() {
            glDeleteRenderbuffers(1, &m_name);
        }
    };
    class StandardRenderBuffer : public AbstractRenderBuffer {
    public:
        StandardRenderBuffer(const Vector3 &size, GLenum format)
            : AbstractRenderBuffer(size, format)
        {
        }
        ~StandardRenderBuffer() {
        }
    private:
        void generate() {
            const Vector3 &s = size();
            glRenderbufferStorage(GL_RENDERBUFFER, format(), s.x(), s.y());
        }
    };
    class MSAARenderBuffer : public AbstractRenderBuffer {
    public:
        MSAARenderBuffer(const Vector3 &size, GLenum format, int samples)
            : AbstractRenderBuffer(size, format),
              m_samples(samples)
        {
        }
        ~MSAARenderBuffer() {
            m_samples = 0;
        }
    private:
        void generate() {
            const Vector3 &s = size();
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_samples, format(), s.x(), s.y());
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
        : m_renderBufferRef(0),
          m_depthBufferMSAA(0),
          m_renderBufferSwap(0),
          m_depthBufferSwap(0),
          m_fbo(0),
          m_fboMSAA(0),
          m_fboSwap(0),
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
    void bindTexture(const AbstractTexture *textureRef, int index) {
        if (textureRef) {
            const GLenum targetIndex = GL_COLOR_ATTACHMENT0 + index;
            m_targetIndex2TextureRefs.insert(targetIndex, textureRef);
            bindFrameBuffer(m_fbo);
            glFramebufferTexture2D(GL_FRAMEBUFFER, targetIndex, GL_TEXTURE_2D, textureRef->name(), 0);
            bindMSAABuffer(textureRef, targetIndex, index);
        }
    }
    void bindDepthStencilBuffer(const AbstractRenderBuffer *renderBufferRef) {
        if (renderBufferRef) {
            bindFrameBuffer(m_fbo);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderBufferRef->name());
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBufferRef->name());
            if (m_fboMSAA) {
                GLenum name = m_depthBufferMSAA->name();
                bindFrameBuffer(m_fboMSAA);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, name);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, name);
            }
        }
    }
    void unbindTexture(int index) {
        const GLenum targetIndex = GL_COLOR_ATTACHMENT0 + index;
        if (const AbstractTexture *const *textureRefPtr = m_targetIndex2TextureRefs.find(targetIndex)) {
            const AbstractTexture *textureRef = *textureRefPtr;
            bindFrameBuffer(m_fbo);
            glFramebufferTexture2D(GL_FRAMEBUFFER, targetIndex, textureRef->target(), 0, 0);
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

    void bindSwapBuffer(FrameBufferObject *destination, const Vector3 &viewport) {
        static const GLuint targetIndex = GL_COLOR_ATTACHMENT0;
        const AbstractTexture *sourceTextureRef = 0;
        if (const AbstractTexture *const *sourceTextureRefPtr = m_targetIndex2TextureRefs.find(targetIndex)) {
            sourceTextureRef = *sourceTextureRefPtr;
        }
        createSwapBuffer(sourceTextureRef, viewport);
        if (destination)
            destination->createSwapBuffer(sourceTextureRef, viewport);
        bindFrameBuffer(m_fboSwap);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
    void transferSwapBuffer(FrameBufferObject *destination, const Vector3 &viewport) {
        static const GLuint targetIndex = GL_COLOR_ATTACHMENT0;
        if (m_fboSwap) {
            /*
            if (destination->m_fboMSAA) {
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destination->m_fboMSAA);
            }
            else {
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destination->m_fbo);
            }
            */
            const Vector3 &sourceSize = m_renderBufferSwap->size();
            const Vector3 &destinationSize = destination ? destination->m_renderBufferSwap->size() : viewport;
            if (destination) {
                glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboSwap);
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destination->m_fboSwap);
                glDrawBuffers(1, &targetIndex);
            }
            else {
                glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboSwap);
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
                glDrawBuffers(0, 0);
                glDrawBuffer(GL_BACK);
            }
            glReadBuffer(targetIndex);
            glBlitFramebuffer(0, 0, sourceSize.x(), sourceSize.y(), 0, 0, destinationSize.x(), destinationSize.y(),
                              GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
            //destination->transferMSAABuffer(0);
            unbind();
        }
    }
    void transferMSAABuffer(int index) {
        if (m_fboMSAA && m_renderBufferRef) {
            const GLenum targetIndex = GL_COLOR_ATTACHMENT0 + index;
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboMSAA);
            glDrawBuffers(1, &targetIndex);
            glReadBuffer(targetIndex);
            const Vector3 &size = m_renderBufferRef->size();
            glBlitFramebuffer(0, 0, size.x(), size.y(), 0, 0, size.x(), size.y(),
                              GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
        }
    }
    GLuint name() const { return m_fbo; }
    GLuint nameForMSAA() const { return m_fboMSAA; }
    GLuint nameForSwap() const { return m_fboSwap; }
    int samples() const { return m_samples; }

private:
    void bindFrameBuffer(GLuint name) {
        if (m_boundRef != name) {
            glBindFramebuffer(GL_FRAMEBUFFER, name);
            m_boundRef = name;
        }
    }
    void bindMSAABuffer(const AbstractSurface *texture, GLenum targetIndex, int index) {
        if (m_fboMSAA) {
            AbstractRenderBuffer *renderBufferRef = 0;
            if (AbstractRenderBuffer *const *renderBufferPtr = m_targetIndex2RenderBufferMSAAs.find(index)) {
                renderBufferRef = *renderBufferPtr;
                bindFrameBuffer(m_fboMSAA);
            }
            else {
                const Vector3 &size = texture->size();
                GLenum format = texture->format();
                m_targetIndex2RenderBufferMSAAs.insert(index, new MSAARenderBuffer(size, format, m_samples));
                m_depthBufferMSAA = new MSAARenderBuffer(size, detectDepthFormat(format), m_samples);
                m_depthBufferMSAA->create();
                renderBufferRef = *m_targetIndex2RenderBufferMSAAs.find(index);
                renderBufferRef->create();
                bindFrameBuffer(m_fboMSAA);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthBufferMSAA->name());
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthBufferMSAA->name());
            }
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, targetIndex, GL_RENDERBUFFER, renderBufferRef->name());
            m_renderBufferRef = renderBufferRef;
        }
    }
    void createSwapBuffer(const AbstractTexture *textureRef, const Vector3 &viewport) {
        if (!m_fboSwap) {
            const Vector3 &size = textureRef ? textureRef->size() : viewport;
            GLenum format = textureRef ? textureRef->format() : GL_RGBA8;
            m_renderBufferSwap = new MSAARenderBuffer(size, format, m_samples);
            m_renderBufferSwap->create();
            m_depthBufferSwap = new MSAARenderBuffer(size, detectDepthFormat(format), m_samples);
            m_depthBufferSwap->create();
            glGenFramebuffers(1, &m_fboSwap);
            bindFrameBuffer(m_fboSwap);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_renderBufferSwap->name());
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthBufferSwap->name());
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthBufferSwap->name());
            bindFrameBuffer(0);
        }
    }
    void release() {
        delete m_depthBufferMSAA;
        m_depthBufferMSAA = 0;
        delete m_renderBufferSwap;
        m_renderBufferSwap = 0;
        delete m_depthBufferSwap;
        m_depthBufferSwap = 0;
        m_renderBufferRef = 0;
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
        glDeleteFramebuffers(1, &m_fboMSAA);
        m_fboMSAA = 0;
        glDeleteFramebuffers(1, &m_fboSwap);
        m_fboSwap = 0;
        m_boundRef = 0;
    }

    PointerHash<HashInt, AbstractRenderBuffer> m_targetIndex2RenderBufferMSAAs;
    Hash<HashInt, const AbstractTexture *> m_targetIndex2TextureRefs;
    AbstractRenderBuffer *m_renderBufferRef;
    AbstractRenderBuffer *m_depthBufferMSAA;
    AbstractRenderBuffer *m_renderBufferSwap;
    AbstractRenderBuffer *m_depthBufferSwap;
    GLuint m_fbo;
    GLuint m_fboMSAA;
    GLuint m_fboSwap;
    GLuint m_boundRef;
    int m_samples;

    VPVL2_DISABLE_COPY_AND_ASSIGN(FrameBufferObject)
};

} /* namespace gl */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
