/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
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

#ifdef VPVL2_LINK_QT
#include <QtOpenGL/QtOpenGL>
#include <QtOpenGL/QGLFunctions>
//#define DEBUG_OUTPUT_TEXTURE
#elif defined(VPVL2_LINK_GLEW)
#include <GL/glew.h>
#endif /* VPVL_LINK_QT */

#ifndef VPVL2_LINK_GLEW
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#else /* __APPLE__ */
#include <GL/gl.h>
#ifndef _MSC_VER
#include <GL/glext.h>
#else
#pragma warning(push)
#pragma warning(disable:4005)
#include <vpvl2/extensions/gl/khronos/glext.h>
#pragma warning(pop)
#endif /* _MSC_VER */
#endif /* __APPLE__ */
#endif /* VPVL2_LINK_GLEW */

namespace vpvl2
{
namespace extensions
{
namespace gl
{

#ifdef __APPLE__
#define glBlitFramebufferPROC glBlitFramebuffer
#define glDrawBuffersPROC glDrawBuffers
#define glRenderbufferStorageMultisamplePROC glRenderbufferStorageMultisample
#else
static PFNGLBLITFRAMEBUFFERPROC glBlitFramebufferPROC;
static PFNGLDRAWBUFFERSPROC glDrawBuffersPROC;
static PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glRenderbufferStorageMultisamplePROC;
#endif

class FrameBufferObject
        #ifdef VPVL2_LINK_QT
        : protected QGLFunctions
        #endif
{
public:
    FrameBufferObject(size_t width, size_t height, int samples) :
    #ifdef VPVL2_LINK_QT
        QGLFunctions(),
    #endif
        m_fbo(0),
        m_depth(0),
        m_fboMSAA(0),
        m_depthMSAA(0),
        m_fboSwap(0),
        m_colorSwap(0),
        m_depthSwap(0),
        m_width(width),
        m_height(height),
        m_samples(samples)
    {
    }
    ~FrameBufferObject() {
        release();
    }

    void create(bool enableAA) {
#ifdef VPVL2_LINK_QT
        const QGLContext *context = QGLContext::currentContext();
        initializeGLFunctions(context);
#ifndef __APPLE__
        glBlitFramebufferPROC = reinterpret_cast<PFNGLBLITFRAMEBUFFERPROC>(
                    context->getProcAddress("glBlitFramebuffer"));
        if (!glBlitFramebufferPROC) {
            glBlitFramebufferPROC = reinterpret_cast<PFNGLBLITFRAMEBUFFERPROC>(
                        context->getProcAddress("glBlitFramebufferEXT"));
        }
        glDrawBuffersPROC = reinterpret_cast<PFNGLDRAWBUFFERSPROC>(
                    context->getProcAddress("glDrawBuffers"));
        if (!glDrawBuffersPROC) {
            glDrawBuffersPROC = reinterpret_cast<PFNGLDRAWBUFFERSPROC>(
                        context->getProcAddress("glDrawBuffersARB"));
        }
        glRenderbufferStorageMultisamplePROC = reinterpret_cast<PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC>(
                    context->getProcAddress("glRenderbufferStorageMultisample"));
        if (!glRenderbufferStorageMultisamplePROC) {
            glRenderbufferStorageMultisamplePROC = reinterpret_cast<PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC>(
                        context->getProcAddress("glRenderbufferStorageMultisampleEXT"));
        }
#endif /* __APPLE__ */
#else
        glBlitFramebufferPROC = glBlitFramebuffer;
        glDrawBuffersPROC = glDrawBuffers;
        glRenderbufferStorageMultisamplePROC = glRenderbufferStorageMultisample;
#endif /* VPVL2_LINK_QT */
        glGenFramebuffers(1, &m_fbo);
        glGenRenderbuffers(1, &m_depth);
        glBindRenderbuffer(GL_RENDERBUFFER, m_depth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
#ifdef __APPLE__
        if (enableAA && m_samples > 0) {
#else
        if (m_samples > 0 && glBlitFramebufferPROC && glDrawBuffersPROC && glRenderbufferStorageMultisamplePROC) {
#endif /* __APPLE__ */
            glGenFramebuffers(1, &m_fboMSAA);
            glGenRenderbuffers(1, &m_depthMSAA);
            glBindRenderbuffer(GL_RENDERBUFFER, m_depthMSAA);
            glRenderbufferStorageMultisamplePROC(GL_RENDERBUFFER, m_samples, GL_DEPTH24_STENCIL8, m_width, m_height);
        }
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
    void bindTexture(GLuint textureID, GLenum format, int index) {
        const GLenum target = GL_COLOR_ATTACHMENT0 + index;
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, target, GL_TEXTURE_2D, textureID, 0);
        if (m_fboMSAA) {
            glBindFramebuffer(GL_FRAMEBUFFER, m_fboMSAA);
            GLuint buffer = 0;
            if (const GLuint *bufferPtr = m_colorMSAA.find(index)) {
                buffer = *bufferPtr;
            }
            else {
                glGenRenderbuffers(1, &buffer);
                glBindRenderbuffer(GL_RENDERBUFFER, buffer);
                glRenderbufferStorageMultisamplePROC(GL_RENDERBUFFER, m_samples, format, m_width, m_height);
                glBindRenderbuffer(GL_RENDERBUFFER, 0);
                m_colorMSAA.insert(index, buffer);
                m_colorFormats.insert(index, format);
            }
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, target, GL_RENDERBUFFER, buffer);
        }
        m_boundTextureTargets.insert(index, index);
    }
    void bindDepthStencilBuffer() {
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depth);
        if (m_fboMSAA) {
            glBindFramebuffer(GL_FRAMEBUFFER, m_fboMSAA);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthMSAA);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthMSAA);
        }
    }
    void unbindColorBuffer(int index) {
        const GLenum target = GL_COLOR_ATTACHMENT0 + index;
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, target, GL_TEXTURE_2D, 0, 0);
        if (m_fboMSAA) {
            glBindFramebuffer(GL_FRAMEBUFFER, m_fboMSAA);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, target, GL_RENDERBUFFER, 0);
        }
        m_boundTextureTargets.remove(index);
    }
    void unbindDepthStencilBuffer() {
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
        if (m_fboMSAA) {
            glBindFramebuffer(GL_FRAMEBUFFER, m_fboMSAA);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
        }
    }
    void unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void resize(size_t width, size_t height) {
        if (m_width != width || m_height != height) {
            bool enableAA = m_fboMSAA ? true : false;
            int samples = m_samples;
            release();
            m_width = width;
            m_height = height;
            m_samples = samples;
            create(enableAA);
        }
    }
    bool hasMSAA() const {
        return m_fboMSAA != 0;
    }

    void bindSwapBuffer() {
        if (!m_fboSwap) {
            GLuint format = *m_colorFormats.find(0);
            glGenFramebuffers(1, &m_fboSwap);
            glGenRenderbuffers(1, &m_colorSwap);
            glGenRenderbuffers(1, &m_depthSwap);
            if (m_fboMSAA) {
                glBindRenderbuffer(GL_RENDERBUFFER, m_colorSwap);
                glRenderbufferStorageMultisamplePROC(GL_RENDERBUFFER, m_samples, format, m_width, m_height);
                glBindRenderbuffer(GL_RENDERBUFFER, m_depthSwap);
                glRenderbufferStorageMultisamplePROC(GL_RENDERBUFFER, m_samples, GL_DEPTH24_STENCIL8, m_width, m_height);
                glBindRenderbuffer(GL_RENDERBUFFER, 0);
            }
            else {
                glBindRenderbuffer(GL_RENDERBUFFER, m_colorSwap);
                glRenderbufferStorage(GL_RENDERBUFFER, format, m_width, m_height);
                glBindRenderbuffer(GL_RENDERBUFFER, m_depthSwap);
                glRenderbufferStorageMultisamplePROC(GL_RENDERBUFFER, m_samples, GL_DEPTH24_STENCIL8, m_width, m_height);
                glBindRenderbuffer(GL_RENDERBUFFER, 0);
            }
        }
        glBindFramebuffer(GL_FRAMEBUFFER, m_fboSwap);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_colorSwap);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthSwap);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthSwap);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
    void transferSwapBuffer(FrameBufferObject *destination) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboSwap);
        if (destination->m_fboMSAA) {
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destination->m_fboMSAA);
        }
        else {
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destination->m_fbo);
        }
        const GLuint target = GL_COLOR_ATTACHMENT0;
        glDrawBuffersPROC(1, &target);
        glReadBuffer(target);
        glBlitFramebufferPROC(0, 0, m_width, m_height, 0, 0, destination->m_width, destination->m_height,
                              GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
        destination->transferMSAABuffer(0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void transferMSAABuffer(int index) {
        if (m_fboMSAA) {
            const GLenum target = GL_COLOR_ATTACHMENT0 + index;
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboMSAA);
            glDrawBuffersPROC(1, &target);
            glReadBuffer(target);
            glBlitFramebufferPROC(0, 0, m_width, m_height, 0, 0, m_width, m_height,
                                  GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
        }
    }

#if defined(VPVL2_LINK_QT)
    void getImage(QImage &output) {
        QImage image(m_width, m_height, QImage::Format_ARGB32_Premultiplied);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glReadPixels(0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        output = image.rgbSwapped().mirrored();
    }
#endif

private:
    void release() {
#if defined(VPVL2_LINK_QT)
        initializeGLFunctions(QGLContext::currentContext());
#endif
        const int nbuffers = m_colorMSAA.count();
        for (int i = 0; i < nbuffers; i++) {
            const GLuint *buffer = m_colorMSAA.value(i);
            glDeleteBuffers(1, buffer);
        }
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
    Hash<btHashInt, int> m_boundTextureTargets;
    GLuint m_fbo;
    GLuint m_depth;
    GLuint m_fboMSAA;
    GLuint m_depthMSAA;
    GLuint m_fboSwap;
    GLuint m_colorSwap;
    GLuint m_depthSwap;
    size_t m_width;
    size_t m_height;
    int m_samples;
};

} /* namespace gl */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
