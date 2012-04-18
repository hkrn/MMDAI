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

#ifdef VPVL2_ENABLE_OPENCL
#ifdef __APPLE__
#include <OpenCL/cl.h>
#include <OpenCL/cl_gl.h>
#include <OpenCL/cl_gl_ext.h>
#else
#include <CL/cl.h>
#include <CL/cl_gl.h>
#endif /* __APPLE__ */
#endif

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
    }
#endif

    virtual bool load(const char *vertexShaderSource, const char *fragmentShaderSource, void *context) {
        GLuint vertexShader = compileShader(context, vertexShaderSource, GL_VERTEX_SHADER);
        GLuint fragmentShader = compileShader(context, fragmentShaderSource, GL_FRAGMENT_SHADER);
        if (vertexShader && fragmentShader) {
            GLuint program = glCreateProgram();
            glAttachShader(program, vertexShader);
            glAttachShader(program, fragmentShader);
            glLinkProgram(program);
            glValidateProgram(program);
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            GLint linked;
            glGetProgramiv(program, GL_LINK_STATUS, &linked);
            if (!linked) {
                GLint len;
                glGetShaderiv(program, GL_INFO_LOG_LENGTH, &len);
                if (len > 0) {
                    delete[] m_message;
                    m_message = new char[len];
                    glGetProgramInfoLog(program, len, NULL, m_message);
                    log0(context, IRenderDelegate::kLogWarning, "%s", m_message);
                }
                glDeleteProgram(program);
                return false;
            }
            m_modelViewProjectionUniformLocation = glGetUniformLocation(program, "modelViewProjectionMatrix");
            m_positionAttributeLocation = glGetAttribLocation(program, "inPosition");
            m_program = program;
            log0(context, IRenderDelegate::kLogInfo, "Created a shader program (ID=%d)", m_program);
            return true;
        }
        else {
            return false;
        }
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
    void log0(void *context, IRenderDelegate::LogLevel level, const char *format...) {
        va_list ap;
        va_start(ap, format);
        m_delegate->log(context, level, format, ap);
        va_end(ap);
    }

    GLuint m_program;

private:
    GLuint compileShader(void *context, const char *source, GLenum type) {
        GLuint shader = glCreateShader(type);
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
            return 0;
        }
        return shader;
    }

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
          m_lightColorUniformLocation(0),
          m_lightPositionUniformLocation(0)
    {
    }
    ~ObjectProgram() {
        m_normalAttributeLocation = 0;
        m_lightColorUniformLocation = 0;
        m_lightPositionUniformLocation = 0;
    }

    virtual bool load(const char *vertexShaderSource, const char *fragmentShaderSource, void *context) {
        bool ret = BaseShaderProgram::load(vertexShaderSource, fragmentShaderSource, context);
        if (ret) {
            m_normalAttributeLocation = glGetAttribLocation(m_program, "inNormal");
            m_lightColorUniformLocation = glGetUniformLocation(m_program, "lightColor");
            m_lightPositionUniformLocation = glGetUniformLocation(m_program, "lightPosition");
        }
        return ret;
    }
    void setLightColor(const Vector3 &value) {
        glUniform3fv(m_lightColorUniformLocation, 1, value);
    }
    void setLightDirection(const Vector3 &value) {
        glUniform3fv(m_lightPositionUniformLocation, 1, value);
    }
    void setNormal(const GLvoid *ptr, GLsizei stride) {
        glEnableVertexAttribArray(m_normalAttributeLocation);
        glVertexAttribPointer(m_normalAttributeLocation, 4, GL_FLOAT, GL_FALSE, stride, ptr);
    }

private:
    GLuint m_normalAttributeLocation;
    GLuint m_lightColorUniformLocation;
    GLuint m_lightPositionUniformLocation;
};

class ZPlotProgram : public BaseShaderProgram
{
public:
    ZPlotProgram(IRenderDelegate *delegate)
        : BaseShaderProgram(delegate),
          m_transformUniformLocation(0)
    {
    }
    ~ZPlotProgram() {
        m_transformUniformLocation = 0;
    }

    virtual bool load(const char *vertexShaderSource, const char *fragmentShaderSource, void *context) {
        bool ret = BaseShaderProgram::load(vertexShaderSource, fragmentShaderSource, context);
        if (ret) {
            m_transformUniformLocation = glGetUniformLocation(m_program, "transformMatrix");
        }
        return ret;
    }
    void setTransformMatrix(const float value[16]) {
        glUniformMatrix4fv(m_transformUniformLocation, 1, GL_FALSE, value);
    }

private:
    GLuint m_transformUniformLocation;
};

#ifdef VPVL2_ENABLE_OPENCL
class BaseAccelerator
{
public:
    BaseAccelerator(IRenderDelegate *delegate)
        : m_delegate(delegate),
          m_context(0),
          m_queue(0),
          m_device(0),
          m_program(0)
    {
    }
    virtual ~BaseAccelerator() {
        clReleaseProgram(m_program);
        m_program = 0;
        clReleaseCommandQueue(m_queue);
        m_queue = 0;
        clReleaseContext(m_context);
        m_context = 0;
    }

    bool isAvailable() const {
        return m_context && m_queue && m_program;
    }
    bool initializeContext() {
        if (m_context && m_queue)
            return true;
        cl_int err = 0;
        cl_uint nplatforms;
        err = clGetPlatformIDs(0, 0, &nplatforms);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed getting number of OpenCL platforms: %d", err);
            return false;
        }
        cl_platform_id *platforms = new cl_platform_id[nplatforms];
        err = clGetPlatformIDs(nplatforms, platforms, 0);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed getting OpenCL platforms: %d", err);
            delete[] platforms;
            return false;
        }
        for (cl_uint i = 0; i < nplatforms; i++) {
            cl_char buffer[BUFSIZ];
            clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, sizeof(buffer), buffer, 0);
            log0(0, IRenderDelegate::kLogInfo, "CL_PLATFORM_VENDOR: %s", buffer);
            clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, sizeof(buffer), buffer, 0);
            log0(0, IRenderDelegate::kLogInfo, "CL_PLATFORM_NAME: %s", buffer);
            clGetPlatformInfo(platforms[i], CL_PLATFORM_VERSION, sizeof(buffer), buffer, 0);
            log0(0, IRenderDelegate::kLogInfo, "CL_PLATFORM_VERSION: %s", buffer);
        }
        cl_platform_id firstPlatform = platforms[0];
        err = clGetDeviceIDs(firstPlatform, CL_DEVICE_TYPE_ALL, 1, &m_device, 0);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed getting a OpenCL device: %d", err);
            delete[] platforms;
            return false;
        }
        {
            cl_char buffer[BUFSIZ];
            cl_uint frequency, addressBits;
            cl_device_type type;
            clGetDeviceInfo(m_device, CL_DRIVER_VERSION, sizeof(buffer), buffer, 0);
            log0(0, IRenderDelegate::kLogInfo, "CL_DRIVER_VERSION: %s", buffer);
            clGetDeviceInfo(m_device, CL_DEVICE_NAME, sizeof(buffer), buffer, 0);
            log0(0, IRenderDelegate::kLogInfo, "CL_DEVICE_NAME: %s", buffer);
            clGetDeviceInfo(m_device, CL_DEVICE_VENDOR, sizeof(buffer), buffer, 0);
            log0(0, IRenderDelegate::kLogInfo, "CL_DEVICE_VENDOR: %s", buffer);
            clGetDeviceInfo(m_device, CL_DEVICE_TYPE, sizeof(type), &type, 0);
            log0(0, IRenderDelegate::kLogInfo, "CL_DEVICE_TYPE: %d", type);
            clGetDeviceInfo(m_device, CL_DEVICE_ADDRESS_BITS, sizeof(addressBits), &addressBits, 0);
            log0(0, IRenderDelegate::kLogInfo, "CL_DEVICE_ADDRESS_BITS: %d", addressBits);
            clGetDeviceInfo(m_device, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(frequency), &frequency, 0);
            log0(0, IRenderDelegate::kLogInfo, "CL_DEVICE_MAX_CLOCK_FREQUENCY: %d", frequency);
            clGetDeviceInfo(m_device, CL_DEVICE_EXTENSIONS, sizeof(buffer), buffer, 0);
            log0(0, IRenderDelegate::kLogInfo, "CL_DEVICE_EXTENSIONS: %s", buffer);
        }
        cl_context_properties props[] = {
            CL_CONTEXT_PLATFORM,
            reinterpret_cast<cl_context_properties>(firstPlatform),
    #ifdef __APPLE__
            CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE,
            reinterpret_cast<cl_context_properties>(CGLGetShareGroup(CGLGetCurrentContext())),
    #endif
            0
        };
        clReleaseContext(m_context);
        m_context = clCreateContext(props, 1, &m_device, 0, 0, &err);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed initialize a OpenCL context: %d", err);
            delete[] platforms;
            return false;
        }
        clReleaseCommandQueue(m_queue);
        m_queue = clCreateCommandQueue(m_context, m_device, 0, &err);
        if (err != CL_SUCCESS) {
            log0(0, IRenderDelegate::kLogWarning, "Failed initialize a OpenCL command queue: %d", err);
            delete[] platforms;
            return false;
        }
        delete[] platforms;
        return true;
    }
    virtual bool createKernelPrograms() {
        return false;
    }

protected:
    void log0(void *context, IRenderDelegate::LogLevel level, const char *format...) {
        va_list ap;
        va_start(ap, format);
        m_delegate->log(context, level, format, ap);
        va_end(ap);
    }

    IRenderDelegate *m_delegate;
    cl_context m_context;
    cl_command_queue m_queue;
    cl_device_id m_device;
    cl_program m_program;
};
#else
class BaseAccelerator {
public:
    BaseAccelerator(IRenderDelegate * /* delegate */) {}
    ~BaseAccelerator() {}

    bool isAvailable() const { return false; }
    bool initializeContext() const { return false; }
    virtual bool createKernelPrograms() { return false; }
};

#endif

} /* namespace gl2 */
} /* namespace vpvl2 */
