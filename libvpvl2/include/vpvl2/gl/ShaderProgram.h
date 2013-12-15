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
#ifndef VPVL2_GL_SHADERPROGRAM_H_
#define VPVL2_GL_SHADERPROGRAM_H_

#include <vpvl2/IString.h>
#include <vpvl2/gl/Global.h>

namespace vpvl2
{
namespace gl
{

class ShaderProgram
{
public:
    static const GLuint kAddressNotFound = GLuint(-1);
    static const GLenum kGL_COMPILE_STATUS = 0x8B81;
    static const GLenum kGL_LINK_STATUS = 0x8B82;
    static const GLenum kGL_INFO_LOG_LENGTH = 0x8B84;
    static const GLenum kGL_FRAGMENT_SHADER = 0x8B30;
    static const GLenum kGL_VERTEX_SHADER = 0x8B31;

    ShaderProgram(const IApplicationContext::FunctionResolver *resolver)
        : createProgarm(reinterpret_cast<PFNGLCREATEPROGRAMPROC>(resolver->resolveSymbol("glCreateProgram"))),
          createShader(reinterpret_cast<PFNGLCREATESHADERPROC>(resolver->resolveSymbol("glCreateShader"))),
          shaderSource(reinterpret_cast<PFNGLSHADERSOURCEPROC>(resolver->resolveSymbol("glShaderSource"))),
          compileShader(reinterpret_cast<PFNGLCOMPILESHADERPROC>(resolver->resolveSymbol("glCompileShader"))),
          getShaderiv(reinterpret_cast<PFNGLGETSHADERIVPROC>(resolver->resolveSymbol("glGetShaderiv"))),
          getShaderInfoLog(reinterpret_cast<PFNGLGETSHADERINFOLOGPROC>(resolver->resolveSymbol("glGetShaderInfoLog"))),
          getProgramiv(reinterpret_cast<PFNGLGETSHADERIVPROC>(resolver->resolveSymbol("glGetProgramiv"))),
          getProgramInfoLog(reinterpret_cast<PFNGLGETPROGRAMINFOLOGPROC>(resolver->resolveSymbol("glGetProgramInfoLog"))),
          attachShader(reinterpret_cast<PFNGLATTACHSHADERPROC>(resolver->resolveSymbol("glAttachShader"))),
          deleteShader(reinterpret_cast<PFNGLDELETESHADERPROC>(resolver->resolveSymbol("glDeleteShader"))),
          deleteProgram(reinterpret_cast<PFNGLDELETEPROGRAMPROC>(resolver->resolveSymbol("glDeleteProgram"))),
          linkProgram(reinterpret_cast<PFNGLLINKPROGRAMPROC>(resolver->resolveSymbol("glLinkProgram"))),
          useProgram(reinterpret_cast<PFNGLUSEPROGRAMPROC>(resolver->resolveSymbol("glUseProgram"))),
          bindAttribLocation(reinterpret_cast<PFNGLBINDATTRIBLOCATIONPROC>(resolver->resolveSymbol("glBindAttribLocation"))),
          getUniformLocation(reinterpret_cast<PFNGLGETUNIFORMLOCATIONPROC>(resolver->resolveSymbol("glGetUniformLocation"))),
          uniform1f(reinterpret_cast<PFNGLUNIFORM1FPROC>(resolver->resolveSymbol("glUniform1f"))),
          uniform1i(reinterpret_cast<PFNGLUNIFORM1IPROC>(resolver->resolveSymbol("glUniform1i"))),
          uniform2fv(reinterpret_cast<PFNGLUNIFORM2FVPROC>(resolver->resolveSymbol("glUniform2fv"))),
          uniform3fv(reinterpret_cast<PFNGLUNIFORM3FVPROC>(resolver->resolveSymbol("glUniform3fv"))),
          uniform4fv(reinterpret_cast<PFNGLUNIFORM4FVPROC>(resolver->resolveSymbol("glUniform4fv"))),
          uniformMatrix3fv(reinterpret_cast<PFNGLUNIFORMMATRIX3FVPROC>(resolver->resolveSymbol("glUniformMatrix3fv"))),
          uniformMatrix4fv(reinterpret_cast<PFNGLUNIFORMMATRIX3FVPROC>(resolver->resolveSymbol("glUniformMatrix4fv"))),
          activeTexture(reinterpret_cast<PFNGLACTIVETEXTUREPROC>(resolver->resolveSymbol("glActiveTexture"))),
          bindTexture(reinterpret_cast<PFNGLBINDTEXTUREPROC>(resolver->resolveSymbol("glBindTexture"))),
          m_program(0),
          m_linked(false)
    {
    }
    virtual ~ShaderProgram() {
        if (m_program) {
            deleteProgram(m_program);
            m_program = 0;
        }
        m_linked = false;
    }

    void create() {
        if (!m_program) {
            m_program = createProgarm();
        }
    }
    bool addShaderSource(const char *source, GLenum type) {
        GLuint shader = createShader(type);
        shaderSource(shader, 1, &source, 0);
        compileShader(shader);
        GLint compiled;
        getShaderiv(shader, kGL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint len;
            getShaderiv(shader, kGL_INFO_LOG_LENGTH, &len);
            if (len > 0) {
                m_message.resize(len);
                getShaderInfoLog(shader, len, &len, &m_message[0]);
            }
            deleteShader(shader);
            return false;
        }
        attachShader(m_program, shader);
        deleteShader(shader);
        return true;
    }
    bool addShaderSource(const IString *source, GLenum type) {
        return addShaderSource(source ? reinterpret_cast<const char *>(source->toByteArray()) : "", type);
    }
    bool link() {
        GLint linked;
        linkProgram(m_program);
        getProgramiv(m_program, kGL_LINK_STATUS, &linked);
        if (!linked) {
            GLint len;
            getProgramiv(m_program, kGL_INFO_LOG_LENGTH, &len);
            if (len > 0) {
                m_message.resize(len);
                getProgramInfoLog(m_program, len, &len, &m_message[0]);
            }
            deleteProgram(m_program);
            return false;
        }
        m_linked = true;
        return true;
    }
    virtual void bind() {
        useProgram(m_program);
    }
    virtual void unbind() {
        useProgram(0);
    }
    bool isLinked() const {
        return m_linked;
    }
    const char *message() const {
        return &m_message[0];
    }

protected:
    typedef GLuint (GLAPIENTRY * PFNGLCREATEPROGRAMPROC) (void);
    typedef GLuint (GLAPIENTRY * PFNGLCREATESHADERPROC) (GLenum type);
    typedef void (GLAPIENTRY * PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar** strings, const GLint* lengths);
    typedef void (GLAPIENTRY * PFNGLCOMPILESHADERPROC) (GLuint shader);
    typedef void (GLAPIENTRY * PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint* param);
    typedef void (GLAPIENTRY * PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
    typedef void (GLAPIENTRY * PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint* param);
    typedef void (GLAPIENTRY * PFNGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
    typedef void (GLAPIENTRY * PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
    typedef void (GLAPIENTRY * PFNGLDELETEPROGRAMPROC) (GLuint program);
    typedef void (GLAPIENTRY * PFNGLDELETESHADERPROC) (GLuint shader);
    typedef void (GLAPIENTRY * PFNGLLINKPROGRAMPROC) (GLuint program);
    typedef void (GLAPIENTRY * PFNGLUSEPROGRAMPROC) (GLuint program);
    typedef void (GLAPIENTRY * PFNGLBINDATTRIBLOCATIONPROC) (GLuint program, GLuint index, const GLchar* name);
    typedef GLint (GLAPIENTRY * PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const GLchar* name);
    typedef void (GLAPIENTRY * PFNGLUNIFORM1FPROC) (GLint location, GLfloat v0);
    typedef void (GLAPIENTRY * PFNGLUNIFORM1IPROC) (GLint location, GLint v0);
    typedef void (GLAPIENTRY * PFNGLUNIFORM2FVPROC) (GLint location, GLsizei count, const GLfloat* value);
    typedef void (GLAPIENTRY * PFNGLUNIFORM3FVPROC) (GLint location, GLsizei count, const GLfloat* value);
    typedef void (GLAPIENTRY * PFNGLUNIFORM4FVPROC) (GLint location, GLsizei count, const GLfloat* value);
    typedef void (GLAPIENTRY * PFNGLUNIFORMMATRIX3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    typedef void (GLAPIENTRY * PFNGLUNIFORMMATRIX4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    typedef void (GLAPIENTRY * PFNGLACTIVETEXTUREPROC) (GLenum texture);
    typedef void (GLAPIENTRY * PFNGLBINDTEXTUREPROC) (GLenum target, GLuint texture);
    PFNGLCREATEPROGRAMPROC createProgarm;
    PFNGLCREATESHADERPROC createShader;
    PFNGLSHADERSOURCEPROC shaderSource;
    PFNGLCOMPILESHADERPROC compileShader;
    PFNGLGETSHADERIVPROC getShaderiv;
    PFNGLGETSHADERINFOLOGPROC getShaderInfoLog;
    PFNGLGETPROGRAMIVPROC getProgramiv;
    PFNGLGETPROGRAMINFOLOGPROC getProgramInfoLog;
    PFNGLATTACHSHADERPROC attachShader;
    PFNGLDELETESHADERPROC deleteShader;
    PFNGLDELETEPROGRAMPROC deleteProgram;
    PFNGLLINKPROGRAMPROC linkProgram;
    PFNGLUSEPROGRAMPROC useProgram;
    PFNGLBINDATTRIBLOCATIONPROC bindAttribLocation;
    PFNGLGETUNIFORMLOCATIONPROC getUniformLocation;
    PFNGLUNIFORM1FPROC uniform1f;
    PFNGLUNIFORM1IPROC uniform1i;
    PFNGLUNIFORM2FVPROC uniform2fv;
    PFNGLUNIFORM3FVPROC uniform3fv;
    PFNGLUNIFORM4FVPROC uniform4fv;
    PFNGLUNIFORMMATRIX3FVPROC uniformMatrix3fv;
    PFNGLUNIFORMMATRIX4FVPROC uniformMatrix4fv;
    PFNGLACTIVETEXTUREPROC activeTexture;
    PFNGLBINDTEXTUREPROC bindTexture;

    GLuint m_program;

private:
    Array<char> m_message;
    bool m_linked;

    VPVL2_DISABLE_COPY_AND_ASSIGN(ShaderProgram)
};

} /* namespace gl */
} /* namespace vpvl2 */

#endif
