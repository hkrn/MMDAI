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
#ifndef VPVL2_EXTENSIONS_GL_SHADERPROGRAM_H_
#define VPVL2_EXTENSIONS_GL_SHADERPROGRAM_H_

#include <vpvl2/Common.h>
#include <vpvl2/IRenderContext.h>
#include <vpvl2/IString.h>
#include <vpvl2/extensions/gl/CommonMacros.h>

namespace vpvl2
{
namespace extensions
{
namespace gl
{

class ShaderProgram
{
public:
    static const GLuint kAddressNotFound = GLuint(-1);

    ShaderProgram()
        : m_program(0),
          m_linked(false)
    {
    }
    virtual ~ShaderProgram() {
        if (m_program) {
            glDeleteProgram(m_program);
            m_program = 0;
        }
        m_linked = false;
    }

    void create() {
        if (!m_program) {
            m_program = glCreateProgram();
        }
    }
    bool addShaderSource(const IString *s, GLenum type) {
        GLuint shader = glCreateShader(type);
        const char *source = s ? reinterpret_cast<const char *>(s->toByteArray()) : "";
        glShaderSource(shader, 1, &source, 0);
        glCompileShader(shader);
        GLint compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint len;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
            if (len > 0) {
                m_message.resize(len);
                glGetShaderInfoLog(shader, len, &len, &m_message[0]);
            }
            glDeleteShader(shader);
            return false;
        }
        glAttachShader(m_program, shader);
        glDeleteShader(shader);
        return true;
    }
    bool link() {
        GLint linked;
        glLinkProgram(m_program);
        glGetProgramiv(m_program, GL_LINK_STATUS, &linked);
        if (!linked) {
            GLint len;
            glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &len);
            if (len > 0) {
                m_message.resize(len);
                glGetProgramInfoLog(m_program, len, &len, &m_message[0]);
            }
            glDeleteProgram(m_program);
            return false;
        }
        m_linked = true;
        return true;
    }
    virtual void bind() {
        glUseProgram(m_program);
    }
    virtual void unbind() {
        glUseProgram(0);
    }
    bool isLinked() const {
        return m_linked;
    }
    const char *message() const {
        return &m_message[0];
    }

protected:
    GLuint m_program;

private:
    Array<char> m_message;
    bool m_linked;

    VPVL2_DISABLE_COPY_AND_ASSIGN(ShaderProgram)
};

} /* namespace gl */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
