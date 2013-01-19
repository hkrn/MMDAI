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
#ifndef VPVL2_EXTENSIONS_GL_SIMPLESHADOWMAP_H_
#define VPVL2_EXTENSIONS_GL_SIMPLESHADOWMAP_H_

#include "vpvl2/Common.h"
#include "vpvl2/extensions/gl/CommonMacros.h"

namespace vpvl2
{
namespace extensions
{
namespace gl
{

class SimpleShadowMap {
public:
    SimpleShadowMap(int width, int height)
        : m_size(width, height, 0),
          m_colorTexture(0),
          m_frameBuffer(0),
          m_depthBuffer(0)
    {
    }
    ~SimpleShadowMap() {
        release();
    }

    void create() {
        release();
        glGenFramebuffers(1, &m_frameBuffer);
        glGenTextures(1, &m_colorTexture);
        size_t width = m_size.x(), height = m_size.y();
        glBindTexture(GL_TEXTURE_2D, m_colorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, width, height, 0, GL_RG, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
        glGenRenderbuffers(1, &m_depthBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, m_depthBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        bind();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthBuffer);
        unbind();
    }
    void bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    }
    void unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    const Vector3 &size() const { return m_size; }
    void setSize(const Vector3 &value) { m_size = value; }
    void *bufferRef() const { return &m_colorTexture; }

private:
    void release() {
        glDeleteFramebuffers(1, &m_frameBuffer);
        m_frameBuffer = 0;
        glDeleteTextures(1, &m_colorTexture);
        m_colorTexture = 0;
        glDeleteRenderbuffers(1, &m_depthBuffer);
        m_depthBuffer = 0;
    }

    Vector3 m_size;
    mutable GLuint m_colorTexture;
    GLuint m_frameBuffer;
    GLuint m_depthBuffer;

    VPVL2_DISABLE_COPY_AND_ASSIGN(SimpleShadowMap)
};

} /* namespace gl */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
