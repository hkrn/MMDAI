/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2012  hkrn                                    */
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

#include <vpvl2/vpvl2.h>
#include <vpvl2/IRenderDelegate.h>

#ifdef VPVL2_LINK_QT
#include <QtOpenGL/QtOpenGL>
#endif /* VPVL_LINK_QT */

#ifdef VPVL2_BUILD_IOS
#include <OpenGLES/ES2/gl.h>
#else
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/CGLCurrent.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif /* __APPLE__ */
#endif /* VPVL_BUILD_IOS */

namespace vpvl2
{
namespace gl2
{

const GLsizei kShadowMappingTextureWidth = 1024;
const GLsizei kShadowMappingTextureHeight = 1024;

class BaseShaderProgram
        #ifdef VPVL2_LINK_QT
        : protected QGLFunctions
        #endif
{
public:
    BaseShaderProgram(IRenderDelegate *delegate)
        : m_program(0),
          m_delegate(delegate),
          m_modelViewProjectionUniformLocation(0),
          m_positionAttributeLocation(0),
          m_message(0)
    {
#ifndef VPVL2_LINK_QT
        m_program = glCreateProgram();
#endif
    }
    virtual ~BaseShaderProgram() {
        if (m_program) {
            glDeleteProgram(m_program);
            m_program = 0;
        }
        delete[] m_message;
        m_message = 0;
        m_modelViewProjectionUniformLocation = 0;
        m_positionAttributeLocation = 0;
    }

#ifdef VPVL2_LINK_QT
    virtual void initializeContext(const QGLContext *context) {
        initializeGLFunctions(context);
        m_program = glCreateProgram();
    }
#endif

    bool addShaderSource(const IString *s, GLenum type, void *context) {
        if (!s) {
            log0(context, IRenderDelegate::kLogWarning, "Empty shader source found!");
            return false;
        }
        GLuint shader = glCreateShader(type);
        const char *source = reinterpret_cast<const char *>(s->toByteArray());
        glShaderSource(shader, 1, &source, NULL);
        glCompileShader(shader);
        GLint compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint len;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
            if (len > 0) {
                delete[] m_message;
                m_message = new char[len];
                glGetShaderInfoLog(shader, len, NULL, m_message);
                log0(context, IRenderDelegate::kLogWarning, "%s", m_message);
            }
            glDeleteShader(shader);
            return false;
        }
        glAttachShader(m_program, shader);
        glDeleteShader(shader);
        return true;
    }
    bool linkProgram(void *context) {
        GLint linked;
        glLinkProgram(m_program);
        glGetProgramiv(m_program, GL_LINK_STATUS, &linked);
        if (!linked) {
            GLint len = 0;
            glGetShaderiv(m_program, GL_INFO_LOG_LENGTH, &len);
            if (len > 0) {
                delete[] m_message;
                m_message = new char[len];
                glGetProgramInfoLog(m_program, len, NULL, m_message);
                log0(context, IRenderDelegate::kLogWarning, "Link failed: %s", m_message);
            }
            glDeleteProgram(m_program);
            return false;
        }
        GLint validated;
        glValidateProgram(m_program);
        glGetProgramiv(m_program, GL_VALIDATE_STATUS, &validated);
        if (!validated) {
            GLint len = 0;
            glGetShaderiv(m_program, GL_INFO_LOG_LENGTH, &len);
            if (len > 0) {
                delete[] m_message;
                m_message = new char[len];
                glGetProgramInfoLog(m_program, len, NULL, m_message);
                log0(context, IRenderDelegate::kLogWarning, "Validation failed: %s", m_message);
            }
            glDeleteProgram(m_program);
            return false;
        }
        log0(context, IRenderDelegate::kLogInfo, "Created a shader program (ID=%d)", m_program);
        getLocations();
        return true;
    }
    virtual void bind() {
        glUseProgram(m_program);
    }
    virtual void unbind() {
        glUseProgram(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    void setModelViewProjectionMatrix(const float value[16]) {
        glUniformMatrix4fv(m_modelViewProjectionUniformLocation, 1, GL_FALSE, value);
    }
    void setPosition(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_positionAttributeLocation);
        glVertexAttribPointer(m_positionAttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
    }

protected:
    virtual void getLocations() {
        m_modelViewProjectionUniformLocation = glGetUniformLocation(m_program, "modelViewProjectionMatrix");
        m_positionAttributeLocation = glGetAttribLocation(m_program, "inPosition");
    }
    void log0(void *context, IRenderDelegate::LogLevel level, const char *format...) {
        va_list ap;
        va_start(ap, format);
        m_delegate->log(context, level, format, ap);
        va_end(ap);
    }

    GLuint m_program;

private:
    IRenderDelegate *m_delegate;
    GLuint m_modelViewProjectionUniformLocation;
    GLuint m_positionAttributeLocation;
    char *m_message;
};

class ObjectProgram : public BaseShaderProgram
{
public:
    ObjectProgram(IRenderDelegate *delegate)
        : BaseShaderProgram(delegate),
          m_normalAttributeLocation(0),
          m_texCoordAttributeLocation(0),
          m_normalMatrixUniformLocation(0),
          m_lightColorUniformLocation(0),
          m_lightDirectionUniformLocation(0),
          m_lightViewProjectionMatrixUniformLocation(0),
          m_hasMainTextureUniformLocation(0),
          m_hasDepthTextureUniformLocation(0),
          m_mainTextureUniformLocation(0),
          m_depthTextureUniformLocation(0),
          m_depthTextureSizeUniformLocation(0),
          m_enableSoftShadowUniformLocation(0),
          m_opacityUniformLocation(0)
    {
    }
    virtual ~ObjectProgram() {
        m_normalAttributeLocation = 0;
        m_texCoordAttributeLocation = 0;
        m_normalMatrixUniformLocation = 0;
        m_lightColorUniformLocation = 0;
        m_lightDirectionUniformLocation = 0;
        m_lightViewProjectionMatrixUniformLocation = 0;
        m_hasMainTextureUniformLocation = 0;
        m_hasDepthTextureUniformLocation = 0;
        m_mainTextureUniformLocation = 0;
        m_depthTextureUniformLocation = 0;
        m_depthTextureSizeUniformLocation = 0;
        m_enableSoftShadowUniformLocation = 0;
        m_opacityUniformLocation = 0;
    }

    void setLightColor(const Vector3 &value) {
        glUniform3fv(m_lightColorUniformLocation, 1, value);
    }
    void setLightDirection(const Vector3 &value) {
        glUniform3fv(m_lightDirectionUniformLocation, 1, value);
    }
    void setLightViewProjectionMatrix(const GLfloat value[16]) {
        glUniformMatrix4fv(m_lightViewProjectionMatrixUniformLocation, 1, GL_FALSE, value);
    }
    void setNormal(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_normalAttributeLocation);
        glVertexAttribPointer(m_normalAttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setNormalMatrix(const float value[16]) {
        glUniformMatrix3fv(m_normalMatrixUniformLocation, 1, GL_FALSE, value);
    }
    void setTexCoord(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_texCoordAttributeLocation);
        glVertexAttribPointer(m_texCoordAttributeLocation, 2, GL_FLOAT, GL_FALSE, stride, ptr);
    }
    void setMainTexture(GLuint value) {
        if (value) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, value);
            glUniform1i(m_mainTextureUniformLocation, 0);
            glUniform1i(m_hasMainTextureUniformLocation, 1);
        }
        else {
            glUniform1i(m_hasMainTextureUniformLocation, 0);
        }
    }
    void setDepthTexture(GLuint value) {
        if (value) {
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, value);
            glUniform1i(m_depthTextureUniformLocation, 3);
            glUniform1i(m_hasDepthTextureUniformLocation, 1);
        }
        else {
            glUniform1i(m_hasDepthTextureUniformLocation, 0);
        }
    }
    void setDepthTextureSize(const Vector3 &value) {
        glUniform2fv(m_depthTextureSizeUniformLocation, 1, value);
    }
    void setSoftShadowEnable(bool value) {
        glUniform1f(m_enableSoftShadowUniformLocation, value ? 1 : 0);
    }
    void setOpacity(const Scalar &value) {
        glUniform1f(m_opacityUniformLocation, value);
    }

protected:
    virtual void getLocations() {
        BaseShaderProgram::getLocations();
        m_normalAttributeLocation = glGetAttribLocation(m_program, "inNormal");
        m_texCoordAttributeLocation = glGetAttribLocation(m_program, "inTexCoord");
        m_normalMatrixUniformLocation = glGetUniformLocation(m_program, "normalMatrix");
        m_lightColorUniformLocation = glGetUniformLocation(m_program, "lightColor");
        m_lightDirectionUniformLocation = glGetUniformLocation(m_program, "lightDirection");
        m_lightViewProjectionMatrixUniformLocation = glGetUniformLocation(m_program, "lightViewProjectionMatrix");
        m_hasMainTextureUniformLocation = glGetUniformLocation(m_program, "hasMainTexture");
        m_hasDepthTextureUniformLocation = glGetUniformLocation(m_program, "hasDepthTexture");
        m_mainTextureUniformLocation = glGetUniformLocation(m_program, "mainTexture");
        m_depthTextureUniformLocation = glGetUniformLocation(m_program, "depthTexture");
        m_depthTextureSizeUniformLocation = glGetUniformLocation(m_program, "depthTextureSize");
        m_enableSoftShadowUniformLocation = glGetUniformLocation(m_program, "useSoftShadow");
        m_opacityUniformLocation = glGetUniformLocation(m_program, "opacity");
    }

private:
    GLuint m_normalAttributeLocation;
    GLuint m_texCoordAttributeLocation;
    GLuint m_normalMatrixUniformLocation;
    GLuint m_lightColorUniformLocation;
    GLuint m_lightDirectionUniformLocation;
    GLuint m_lightViewProjectionMatrixUniformLocation;
    GLuint m_hasMainTextureUniformLocation;
    GLuint m_hasDepthTextureUniformLocation;
    GLuint m_mainTextureUniformLocation;
    GLuint m_depthTextureUniformLocation;
    GLuint m_depthTextureSizeUniformLocation;
    GLuint m_enableSoftShadowUniformLocation;
    GLuint m_opacityUniformLocation;
};

class ZPlotProgram : public BaseShaderProgram
{
public:
    ZPlotProgram(IRenderDelegate *delegate)
        : BaseShaderProgram(delegate),
          m_transformUniformLocation(0)
    {
    }
    virtual ~ZPlotProgram() {
        m_transformUniformLocation = 0;
    }

    void setTransformMatrix(const float value[16]) {
        glUniformMatrix4fv(m_transformUniformLocation, 1, GL_FALSE, value);
    }

protected:
    virtual void getLocations() {
        BaseShaderProgram::getLocations();
        m_transformUniformLocation = glGetUniformLocation(m_program, "transformMatrix");
    }

private:
    GLuint m_transformUniformLocation;
};

} /* namespace gl2 */
} /* namespace vpvl2 */
