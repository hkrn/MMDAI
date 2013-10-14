/**

 Copyright (c) 2009-2011  Nagoya Institute of Technology
                          Department of Computer Science
               2010-2013  hkrn

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

#ifndef VPVL2_GL_INTERNAL_ENGINECOMMON_H_
#define VPVL2_GL_INTERNAL_ENGINECOMMON_H_

#include "vpvl2/vpvl2.h"
#include "vpvl2/ITexture.h"
#include "vpvl2/extensions/gl/ShaderProgram.h"
#include "vpvl2/extensions/gl/Texture2D.h"

namespace vpvl2
{
namespace gl2
{

using namespace extensions::gl;

class BaseShaderProgram : public extensions::gl::ShaderProgram
{
public:
    BaseShaderProgram(const IApplicationContext::FunctionResolver *resolver)
        : ShaderProgram(resolver),
          m_modelViewProjectionUniformLocation(-1),
          m_positionAttributeLocation(-1)
    {
    }
    virtual ~BaseShaderProgram() {
        m_modelViewProjectionUniformLocation = -1;
        m_positionAttributeLocation = -1;
    }

    bool addShaderSource(const IString *s, extensions::gl::GLenum type) {
        VPVL2_CHECK(s);
        ShaderProgram::create();
        if (!ShaderProgram::addShaderSource(s, type)) {
            VPVL2_LOG(ERROR, "Compile failed: " << message());
            return false;
        }
        return true;
    }
    bool linkProgram() {
        bindAttributeLocations();
        if (!ShaderProgram::link()) {
            VPVL2_LOG(ERROR, "Link failed: " << message());
            return false;
        }
        VPVL2_VLOG(2, "Created a shader program (ID=" << m_program << ")");
        getUniformLocations();
        return true;
    }
    void setModelViewProjectionMatrix(const float value[16]) {
        uniformMatrix4fv(m_modelViewProjectionUniformLocation, 1, GL_FALSE, value);
    }

protected:
    virtual void bindAttributeLocations() {
        bindAttribLocation(m_program, IModel::Buffer::kVertexStride, "inPosition");
    }
    virtual void getUniformLocations() {
        m_modelViewProjectionUniformLocation = getUniformLocation(m_program, "modelViewProjectionMatrix");
    }

private:
    GLint m_modelViewProjectionUniformLocation;
    GLint m_positionAttributeLocation;
};

class ObjectProgram : public BaseShaderProgram
{
public:
    static const char *const kNormalAttributeName;
    static const char *const kTexCoordAttributeName;

    ObjectProgram(const IApplicationContext::FunctionResolver *resolver)
        : BaseShaderProgram(resolver),
          m_normalAttributeLocation(-1),
          m_texCoordAttributeLocation(-1),
          m_normalMatrixUniformLocation(-1),
          m_lightColorUniformLocation(-1),
          m_lightDirectionUniformLocation(-1),
          m_lightViewProjectionMatrixUniformLocation(-1),
          m_hasMainTextureUniformLocation(-1),
          m_hasDepthTextureUniformLocation(-1),
          m_mainTextureUniformLocation(-1),
          m_depthTextureUniformLocation(-1),
          m_depthTextureSizeUniformLocation(-1),
          m_enableSoftShadowUniformLocation(-1),
          m_opacityUniformLocation(-1)
    {
    }
    virtual ~ObjectProgram() {
        m_normalAttributeLocation = -1;
        m_texCoordAttributeLocation = -1;
        m_normalMatrixUniformLocation = -1;
        m_lightColorUniformLocation = -1;
        m_lightDirectionUniformLocation = -1;
        m_lightViewProjectionMatrixUniformLocation = -1;
        m_hasMainTextureUniformLocation = -1;
        m_hasDepthTextureUniformLocation = -1;
        m_mainTextureUniformLocation = -1;
        m_depthTextureUniformLocation = -1;
        m_depthTextureSizeUniformLocation = -1;
        m_enableSoftShadowUniformLocation = -1;
        m_opacityUniformLocation = -1;
    }

    void setLightColor(const Vector3 &value) {
        uniform3fv(m_lightColorUniformLocation, 1, value);
    }
    void setLightDirection(const Vector3 &value) {
        uniform3fv(m_lightDirectionUniformLocation, 1, value);
    }
    void setLightViewProjectionMatrix(const GLfloat value[16]) {
        uniformMatrix4fv(m_lightViewProjectionMatrixUniformLocation, 1, GL_FALSE, value);
    }
    void setNormalMatrix(const float value[16]) {
        float m[] = {
            value[0], value[1], value[2],
            value[4], value[5], value[6],
            value[8], value[9], value[10]
        };
        uniformMatrix3fv(m_normalMatrixUniformLocation, 1, GL_FALSE, m);
    }
    void setMainTexture(const ITexture *value) {
        if (value) {
            activeTexture(Texture2D::kGL_TEXTURE0);
            bindTexture(Texture2D::kGL_TEXTURE_2D, static_cast<GLuint>(value->data()));
            uniform1i(m_mainTextureUniformLocation, 0);
            uniform1i(m_hasMainTextureUniformLocation, 1);
        }
        else {
            uniform1i(m_hasMainTextureUniformLocation, 0);
        }
    }
    void setDepthTexture(GLuint value) {
        if (value) {
            activeTexture(Texture2D::kGL_TEXTURE0 + 3);
            bindTexture(Texture2D::kGL_TEXTURE_2D, value);
            uniform1i(m_depthTextureUniformLocation, 3);
            uniform1i(m_hasDepthTextureUniformLocation, 1);
        }
        else {
            uniform1i(m_hasDepthTextureUniformLocation, 0);
        }
    }
    void setDepthTextureSize(const Vector3 &value) {
        uniform2fv(m_depthTextureSizeUniformLocation, 1, value);
    }
    void setSoftShadowEnable(bool value) {
        uniform1f(m_enableSoftShadowUniformLocation, GLfloat(value ? 1 : 0));
    }
    void setOpacity(const Scalar &value) {
        uniform1f(m_opacityUniformLocation, value);
    }

protected:
    virtual void bindAttributeLocations() {
        BaseShaderProgram::bindAttributeLocations();
        bindAttribLocation(m_program, IModel::Buffer::kNormalStride, "inNormal");
        bindAttribLocation(m_program, IModel::Buffer::kTextureCoordStride, "inTexCoord");
    }
    virtual void getUniformLocations() {
        BaseShaderProgram::getUniformLocations();
        m_normalMatrixUniformLocation = getUniformLocation(m_program, "normalMatrix");
        m_lightColorUniformLocation = getUniformLocation(m_program, "lightColor");
        m_lightDirectionUniformLocation = getUniformLocation(m_program, "lightDirection");
        m_lightViewProjectionMatrixUniformLocation = getUniformLocation(m_program, "lightViewProjectionMatrix");
        m_hasMainTextureUniformLocation = getUniformLocation(m_program, "hasMainTexture");
        m_hasDepthTextureUniformLocation = getUniformLocation(m_program, "hasDepthTexture");
        m_mainTextureUniformLocation = getUniformLocation(m_program, "mainTexture");
        m_depthTextureUniformLocation = getUniformLocation(m_program, "depthTexture");
        m_depthTextureSizeUniformLocation = getUniformLocation(m_program, "depthTextureSize");
        m_enableSoftShadowUniformLocation = getUniformLocation(m_program, "useSoftShadow");
        m_opacityUniformLocation = getUniformLocation(m_program, "opacity");
    }

private:
    GLint m_normalAttributeLocation;
    GLint m_texCoordAttributeLocation;
    GLint m_normalMatrixUniformLocation;
    GLint m_lightColorUniformLocation;
    GLint m_lightDirectionUniformLocation;
    GLint m_lightViewProjectionMatrixUniformLocation;
    GLint m_hasMainTextureUniformLocation;
    GLint m_hasDepthTextureUniformLocation;
    GLint m_mainTextureUniformLocation;
    GLint m_depthTextureUniformLocation;
    GLint m_depthTextureSizeUniformLocation;
    GLint m_enableSoftShadowUniformLocation;
    GLint m_opacityUniformLocation;
};

class ZPlotProgram : public BaseShaderProgram
{
public:
    ZPlotProgram(const IApplicationContext::FunctionResolver *resolver)
        : BaseShaderProgram(resolver),
          m_transformUniformLocation(-1)
    {
    }
    virtual ~ZPlotProgram() {
        m_transformUniformLocation = -1;
    }

    void setTransformMatrix(const float value[16]) {
        uniformMatrix4fv(m_transformUniformLocation, 1, GL_FALSE, value);
    }

protected:
    virtual void getUniformLocations() {
        BaseShaderProgram::getUniformLocations();
        m_transformUniformLocation = getUniformLocation(m_program, "transformMatrix");
    }

private:
    GLint m_transformUniformLocation;
};

} /* namespace gl2 */
} /* namespace vpvl2 */

#endif
