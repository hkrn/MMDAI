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

#include "vpvl2/Common.h"
#include "vpvl2/extensions/gl/CommonMacros.h"

namespace vpvl2
{
namespace extensions
{
namespace gl
{

class FrameBufferObject
{
public:
    FrameBufferObject(size_t width, size_t height, int samples)
        : m_bound(0),
          m_fbo(0),
          m_depth(0),
          m_fboMSAA(0),
          m_depthMSAA(0),
          m_fboSwap(0),
          m_colorSwap(0),
          m_depthSwap(0),
          m_depthFormat(0),
          m_width(width),
          m_height(height),
          m_samples(samples)
    {
    }
    ~FrameBufferObject() {
        release();
    }

    void create() {
        glGenFramebuffers(1, &m_fbo);
        glGenRenderbuffers(1, &m_depth);
    }
    void bindTexture(GLuint textureID, GLenum colorFormat, int index) {
        const GLenum target = GL_COLOR_ATTACHMENT0 + index;
        bindFrameBuffer(GL_FRAMEBUFFER, m_fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, target, GL_TEXTURE_2D, textureID, 0);
        createDepthBuffer(colorFormat);
        bindMSAABuffer(target, colorFormat, index);
    }
    void bindDepthStencilBuffer() {
        bindFrameBuffer(GL_FRAMEBUFFER, m_fbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depth);
        if (m_fboMSAA) {
            bindFrameBuffer(GL_FRAMEBUFFER, m_fboMSAA);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthMSAA);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthMSAA);
        }
    }
    void unbindColorBuffer(int index) {
        const GLenum target = GL_COLOR_ATTACHMENT0 + index;
        bindFrameBuffer(GL_FRAMEBUFFER, m_fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, target, GL_TEXTURE_2D, 0, 0);
        if (m_fboMSAA) {
            bindFrameBuffer(GL_FRAMEBUFFER, m_fboMSAA);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, target, GL_RENDERBUFFER, 0);
        }
    }
    void unbindDepthStencilBuffer() {
        bindFrameBuffer(GL_FRAMEBUFFER, m_fbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
        if (m_fboMSAA) {
            bindFrameBuffer(GL_FRAMEBUFFER, m_fboMSAA);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
        }
    }
    void unbind() {
        unbindFrameBuffer(GL_FRAMEBUFFER);
    }
    bool hasMSAA() const {
        return m_fboMSAA != 0;
    }

    void bindSwapBuffer() {
        if (!m_fboSwap) {
            if (const GLuint *colorFormatPtr = m_colorFormats.find(0)) {
                GLuint colorFormat = *colorFormatPtr;
                glGenFramebuffers(1, &m_fboSwap);
                glGenRenderbuffers(1, &m_colorSwap);
                glGenRenderbuffers(1, &m_depthSwap);
                if (m_fboMSAA) {
                    glBindRenderbuffer(GL_RENDERBUFFER, m_colorSwap);
                    glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_samples, colorFormat, m_width, m_height);
                    glBindRenderbuffer(GL_RENDERBUFFER, m_depthSwap);
                    glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_samples, m_depthFormat, m_width, m_height);
                    glBindRenderbuffer(GL_RENDERBUFFER, 0);
                }
                else {
                    glBindRenderbuffer(GL_RENDERBUFFER, m_colorSwap);
                    glRenderbufferStorage(GL_RENDERBUFFER, colorFormat, m_width, m_height);
                    glBindRenderbuffer(GL_RENDERBUFFER, m_depthSwap);
                    glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_samples, m_depthFormat, m_width, m_height);
                    glBindRenderbuffer(GL_RENDERBUFFER, 0);
                }
            }
        }
        if (m_fboSwap) {
            bindFrameBuffer(GL_FRAMEBUFFER, m_fboSwap);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_colorSwap);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthSwap);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthSwap);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        }
    }
    void transferSwapBuffer(FrameBufferObject *destination) {
        if (m_fboSwap) {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboSwap);
            if (destination->m_fboMSAA) {
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destination->m_fboMSAA);
            }
            else {
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destination->m_fbo);
            }
            const GLuint target = GL_COLOR_ATTACHMENT0;
            glDrawBuffers(1, &target);
            glReadBuffer(target);
            glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, destination->m_width, destination->m_height,
                              GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
            destination->transferMSAABuffer(0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, 0);
            unbindFrameBuffer(GL_FRAMEBUFFER);
        }
    }
    void transferMSAABuffer(int index) {
        if (m_fboMSAA) {
            const GLenum target = GL_COLOR_ATTACHMENT0 + index;
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboMSAA);
            glDrawBuffers(1, &target);
            glReadBuffer(target);
            glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height,
                              GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
        }
    }

private:
    void bindFrameBuffer(GLenum target, GLuint index) {
        if (m_bound != index) {
            glBindFramebuffer(target, index);
            m_bound = index;
        }
    }
    void unbindFrameBuffer(GLenum target) {
        if (m_bound) {
            glBindFramebuffer(target, 0);
            m_bound = 0;
        }
    }
    void createDepthBuffer(GLenum colorFormat) {
        if (!m_depthFormat) {
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
            glBindRenderbuffer(GL_RENDERBUFFER, m_depth);
            glRenderbufferStorage(GL_RENDERBUFFER, depthFormat, m_width, m_height);
            if (m_samples > 0 && depthFormat != GL_DEPTH32F_STENCIL8) {
                glGenFramebuffers(1, &m_fboMSAA);
                glGenRenderbuffers(1, &m_depthMSAA);
                glBindRenderbuffer(GL_RENDERBUFFER, m_depthMSAA);
                glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_samples, depthFormat, m_width, m_height);
            }
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            m_depthFormat = depthFormat;
        }
    }
    void bindMSAABuffer(GLenum target, GLenum colorFormat, int index) {
        if (m_fboMSAA) {
            bindFrameBuffer(GL_FRAMEBUFFER, m_fboMSAA);
            GLuint buffer = 0;
            if (const GLuint *bufferPtr = m_colorMSAA.find(index)) {
                buffer = *bufferPtr;
            }
            else {
                glGenRenderbuffers(1, &buffer);
                glBindRenderbuffer(GL_RENDERBUFFER, buffer);
                glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_samples, colorFormat, m_width, m_height);
                glBindRenderbuffer(GL_RENDERBUFFER, 0);
                m_colorMSAA.insert(index, buffer);
                m_colorFormats.insert(index, colorFormat);
            }
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, target, GL_RENDERBUFFER, buffer);
        }
    }
    void release() {
        const int nbuffers = m_colorMSAA.count();
        for (int i = 0; i < nbuffers; i++) {
            const GLuint *buffer = m_colorMSAA.value(i);
            glDeleteBuffers(1, buffer);
        }
        m_bound = 0;
        m_colorMSAA.clear();
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
        glDeleteRenderbuffers(1, &m_depth);
        m_depth = 0;
        glDeleteFramebuffers(1, &m_fboMSAA);
        m_fboMSAA = 0;
        glDeleteRenderbuffers(1, &m_depthMSAA);
        m_depthMSAA = 0;
        glDeleteFramebuffers(1, &m_fboSwap);
        m_fboSwap = 0;
        glDeleteRenderbuffers(1, &m_colorSwap);
        m_colorSwap = 0;
        glDeleteRenderbuffers(1, &m_depthSwap);
        m_depthSwap = 0;
        m_width = 0;
        m_height = 0;
        m_samples = 0;
    }

    Hash<btHashInt, GLuint> m_colorMSAA;
    Hash<btHashInt, GLenum> m_colorFormats;
    GLuint m_bound;
    GLuint m_fbo;
    GLuint m_depth;
    GLuint m_fboMSAA;
    GLuint m_depthMSAA;
    GLuint m_fboSwap;
    GLuint m_colorSwap;
    GLuint m_depthSwap;
    GLenum m_depthFormat;
    size_t m_width;
    size_t m_height;
    int m_samples;
};

} /* namespace gl */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
