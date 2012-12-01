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

#if defined(VPVL2_LINK_QT)
#include <QtOpenGL/QtOpenGL>
#include <QtOpenGL/QGLFunctions>
//#define DEBUG_OUTPUT_TEXTURE
#elif defined(VPVL2_LINK_GLEW)
#include <GL/glew.h>
#endif /* VPVL_LINK_QT */

#if !defined(VPVL2_LINK_GLEW)
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#else /* __APPLE__ */
#include <GL/gl.h>
#include <GL/glext.h>
#endif /* __APPLE__ */
#endif

#ifdef __APPLE__
#define glBlitFramebufferPROC glBlitFramebuffer
#define glDrawBuffersPROC glDrawBuffers
#define glRenderbufferStorageMultisamplePROC glRenderbufferStorageMultisample
#else
PFNGLBLITFRAMEBUFFERPROC glBlitFramebufferPROC;
PFNGLDRAWBUFFERSPROC glDrawBuffersPROC;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glRenderbufferStorageMultisamplePROC;
#endif

namespace vpvl2
{
namespace extensions
{
namespace gl
{

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
        m_width(width),
        m_height(height),
        m_samples(samples)
    {
    }
    ~FrameBufferObject() {
        const int nbuffers = m_colorMSAA.count();
        for (int i = 0; i < nbuffers; i++) {
            const GLuint *buffer = m_colorMSAA.value(i);
            glDeleteBuffers(1, buffer);
        }
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
        glDeleteRenderbuffers(1, &m_depth);
        m_depth = 0;
        glDeleteFramebuffers(1, &m_fboMSAA);
        m_fboMSAA = 0;
        glDeleteRenderbuffers(1, &m_depthMSAA);
        m_depthMSAA = 0;
        m_width = 0;
        m_height = 0;
        m_samples = 0;
    }

    void create() {
#ifdef VPVL2_LINK_QT
        initializeGLFunctions();
#endif
        glGenFramebuffers(1, &m_fbo);
        glGenRenderbuffers(1, &m_depth);
        glBindRenderbuffer(GL_RENDERBUFFER, m_depth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
        if (m_samples > 0) {
            glGenFramebuffers(1, &m_fboMSAA);
            glGenRenderbuffers(1, &m_depthMSAA);
            glBindRenderbuffer(GL_RENDERBUFFER, m_depthMSAA);
            glRenderbufferStorageMultisamplePROC(GL_RENDERBUFFER, m_samples, GL_DEPTH_COMPONENT24, m_width, m_height);
        }
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
    void blit() {
        if (m_fboMSAA) {
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboMSAA);
            glBlitFramebufferPROC(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }
    }
    void bindTexture(GLuint textureID, GLenum format, int index) {
        const int target = GL_COLOR_ATTACHMENT0 + index;
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
            }
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, target, GL_RENDERBUFFER, buffer);
        }
    }
    void bindDepthStencilBuffer() {
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depth);
        if (m_fboMSAA) {
            glBindFramebuffer(GL_FRAMEBUFFER, m_fboMSAA);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthMSAA);
        }
    }
    void unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    bool hasMSAA() const {
        return m_fboMSAA != 0;
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
    Hash<btHashInt, GLuint> m_colorMSAA;
    GLuint m_fbo;
    GLuint m_depth;
    GLuint m_fboMSAA;
    GLuint m_depthMSAA;
    size_t m_width;
    size_t m_height;
    int m_samples;
};

} /* namespace gl */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
