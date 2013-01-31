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
            glTexImage2D(target(), 0, internalFormat(), GLsizei(s.x()), GLsizei(s.y()), 0, format(), type(), 0);
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
            glTexImage3D(target(), 0, internalFormat(), GLsizei(s.x()), GLsizei(s.y()), GLsizei(s.z()), 0, format(), type(), 0);
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
            glRenderbufferStorage(GL_RENDERBUFFER, format(), GLsizei(s.x()), GLsizei(s.y()));
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
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_samples, format(), GLsizei(s.x()), GLsizei(s.y()));
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
    void bindTexture(const AbstractTexture *textureRef, int index) {
        if (textureRef) {
            const GLenum targetIndex = GL_COLOR_ATTACHMENT0 + index;
            m_targetIndex2TextureRefs.insert(targetIndex, textureRef);
            bindFrameBuffer(m_fbo);
            glFramebufferTexture2D(GL_FRAMEBUFFER, targetIndex, GL_TEXTURE_2D, textureRef->name(), 0);
            bindMSAABuffer(textureRef, targetIndex, index);
        }
    }
    void bindDepthStencilBuffer(const AbstractRenderBuffer *depthStencilBufferRef) {
        if (depthStencilBufferRef) {
            bindFrameBuffer(m_fbo);
            GLuint name = depthStencilBufferRef->name();
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, name);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, name);
            m_depthStencilBufferRef = depthStencilBufferRef;
            if (m_fboMSAA) {
                name = m_depthStencilBufferMSAA->name();
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
                if (const AbstractTexture *const *textureRefPtr = m_targetIndex2TextureRefs.find(targetIndex)) {
                    const AbstractTexture *textureRef = *textureRefPtr;
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
        glDrawBuffer(GL_BACK);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        const Vector3 &size = m_depthStencilBufferRef->size();
        glBlitFramebuffer(0, 0, GLint(size.x()), GLint(size.y()), 0, 0, GLint(viewport.x()), GLint(viewport.y()),
                          GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
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
                m_depthStencilBufferMSAA = new MSAARenderBuffer(size, detectDepthFormat(format), m_samples);
                m_depthStencilBufferMSAA->create();
                renderBufferRef = *m_targetIndex2RenderBufferMSAAs.find(index);
                renderBufferRef->create();
                bindFrameBuffer(m_fboMSAA);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthStencilBufferMSAA->name());
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthStencilBufferMSAA->name());
            }
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, targetIndex, GL_RENDERBUFFER, renderBufferRef->name());
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
    Hash<HashInt, const AbstractTexture *> m_targetIndex2TextureRefs;
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
