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

#include "vpvl2/cl/Context.h"

namespace vpvl2
{
namespace cl
{

Context::Context(IRenderContext *renderContextRef)
    : m_renderContextRef(renderContextRef),
      m_context(0),
      m_queue(0),
      m_device(0)
{
}

Context::~Context()
{
    m_renderContextRef = 0;
    clReleaseCommandQueue(m_queue);
    m_queue = 0;
    clReleaseContext(m_context);
    m_context = 0;
}

bool Context::isAvailable() const
{
    return m_context && m_queue;
}

bool Context::initialize(cl_device_type hostDeviceType)
{
    if (isAvailable()) {
        return true;
    }
    cl_int err = 0;
    cl_uint nplatforms;
    err = clGetPlatformIDs(0, 0, &nplatforms);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(WARNING, "Failed getting number of OpenCL platforms: " << err);
        return false;
    }
    Array<cl_platform_id> platforms;
    err = clGetPlatformIDs(nplatforms, &platforms[0], 0);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(WARNING, "Failed getting OpenCL platforms: " << err);
        return false;
    }
    for (cl_uint i = 0; i < nplatforms; i++) {
        cl_char buffer[1024];
        clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, sizeof(buffer), buffer, 0);
        VPVL2_VLOG(2, "CL_PLATFORM_VENDOR: " << buffer);
        clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, sizeof(buffer), buffer, 0);
        VPVL2_VLOG(2, "CL_PLATFORM_NAME: " << buffer);
        clGetPlatformInfo(platforms[i], CL_PLATFORM_VERSION, sizeof(buffer), buffer, 0);
        VPVL2_VLOG(2, "CL_PLATFORM_VERSION: " << buffer);
    }
    cl_platform_id firstPlatform = platforms[0];
    err = clGetDeviceIDs(firstPlatform, hostDeviceType, 1, &m_device, 0);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(WARNING, "Failed getting a OpenCL device: " << err);
        return false;
    }
    {
        cl_char buffer[1024];
        cl_uint frequency, addressBits;
        cl_device_type type;
        clGetDeviceInfo(m_device, CL_DRIVER_VERSION, sizeof(buffer), buffer, 0);
        VPVL2_VLOG(2, "CL_DRIVER_VERSION: " << buffer);
        clGetDeviceInfo(m_device, CL_DEVICE_NAME, sizeof(buffer), buffer, 0);
        VPVL2_VLOG(2, "CL_DEVICE_NAME: " << buffer);
        clGetDeviceInfo(m_device, CL_DEVICE_VENDOR, sizeof(buffer), buffer, 0);
        VPVL2_VLOG(2, "CL_DEVICE_VENDOR: " << buffer);
        clGetDeviceInfo(m_device, CL_DEVICE_TYPE, sizeof(type), &type, 0);
        VPVL2_VLOG(2, "CL_DEVICE_TYPE: " << type);
        clGetDeviceInfo(m_device, CL_DEVICE_ADDRESS_BITS, sizeof(addressBits), &addressBits, 0);
        VPVL2_VLOG(2, "CL_DEVICE_ADDRESS_BITS: " << addressBits);
        clGetDeviceInfo(m_device, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(frequency), &frequency, 0);
        VPVL2_VLOG(2, "CL_DEVICE_MAX_CLOCK_FREQUENCY: " << frequency);
        clGetDeviceInfo(m_device, CL_DEVICE_EXTENSIONS, sizeof(buffer), buffer, 0);
        VPVL2_VLOG(2, "CL_DEVICE_EXTENSIONS: " << buffer);
    }
    cl_context_properties props[] = {
        CL_CONTEXT_PLATFORM,
        reinterpret_cast<cl_context_properties>(firstPlatform),
    #if defined(__APPLE__)
        CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE,
        reinterpret_cast<cl_context_properties>(CGLGetShareGroup(CGLGetCurrentContext())),
    #elif defined(_MSC_VER)
        CL_GL_CONTEXT_KHR,
        reinterpret_cast<cl_context_properties>(wglGetCurrentContext()),
        CL_WGL_HDC_KHR,
        reinterpret_cast<cl_context_properties>(wglGetCurrentDC()),
    #elif defined(GLX_USE_GL)
        CL_GL_CONTEXT_KHR,
        reinterpret_cast<cl_context_properties>(glXGetCurrentContext()),
        CL_GLX_DISPLAY_KHR,
        reinterpret_cast<cl_context_properties>(glXGetCurrentDisplay()),
    #endif
        0
    };
    clReleaseContext(m_context);
    m_context = clCreateContext(props, 1, &m_device, 0, 0, &err);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(WARNING, "Failed initialize a OpenCL context: " << err);
        return false;
    }
    clReleaseCommandQueue(m_queue);
    m_queue = clCreateCommandQueue(m_context, m_device, 0, &err);
    if (err != CL_SUCCESS) {
        VPVL2_LOG(WARNING, "Failed initialize a OpenCL command queue: " << err);
        return false;
    }
    return true;
}

} /* namespace cl */
} /* namespace vpvl2 */
