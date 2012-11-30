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
#ifndef VPVL2_CL_CONTEXT_H_
#define VPVL2_CL_CONTEXT_H_

#ifdef __APPLE__
#include <OpenCL/cl.h>
#include <OpenCL/cl_gl.h>
#include <OpenCL/cl_gl_ext.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#else
#ifdef VPVL2_LINK_GLEW
#include <GL/glew.h>
#ifdef _MSC_VER
#include <windows.h>
#endif /* _MSC_VER */
#endif /* VPVL2_LINK_GLEW */
#include <CL/cl.h>
#include <CL/cl_gl.h>
#include <GL/gl.h>
#ifndef _MSC_VER // workaround
#include <GL/glx.h>
#endif /* _MSC_VER */
#endif /* __APPLE__ */

#include "vpvl2/IRenderContext.h"

namespace vpvl2
{

namespace cl
{

class Context
{
public:
    Context(IRenderContext *renderContextRef);
    ~Context();

    bool isAvailable() const;
    bool initialize(cl_device_type hostDeviceType);

    IRenderContext *renderContext() const { return m_renderContextRef; }
    cl_context computeContext() const { return m_context; }
    cl_command_queue commandQueue() const { return m_queue; }
    cl_device_id hostDevice() const { return m_device; }

private:
    void log0(void *context, IRenderContext::LogLevel level, const char *format...);

    IRenderContext *m_renderContextRef;
    cl_context m_context;
    cl_command_queue m_queue;
    cl_device_id m_device;
};

} /* namespace cl */
} /* namespace vpvl2 */

#endif
