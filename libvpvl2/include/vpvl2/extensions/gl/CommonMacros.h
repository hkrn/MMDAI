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
#ifndef VPVL2_EXTENSIONS_GL_COMMONMACROS_H_
#define VPVL2_EXTENSIONS_GL_COMMONMACROS_H_

#include <vpvl2/config.h>
#if defined(VPVL2_LINK_GLEW)
/* always uses built GLEW as static library */
#ifndef BUILD_SHARED_LIBS
#define GLEW_STATIC
#endif /* BUILD_SHARED_LIBS */
#if defined(_MSC_VER)
#include <windows.h>
#endif /* _MSC_VER */
#include "GL/glew.h"
#elif defined(__APPLE__) && !defined(VPVL2_ENABLE_OSMESA)
#if defined(VPVL2_ENABLE_GLES2)
#include <OpenGLES2/gl2.h>
#include <OpenGLES2/gl2ext.h>
#else /* VPVL2_ENABLE_GLES2 */
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#endif /* __APPLE__ */
#elif defined(VPVL2_ENABLE_GLES2)
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else /* VPVL2_ENABLE_GLES2 */
#ifdef VPVL2_ENABLE_OSMESA
#define GL_GLEXT_PROTOTYPES
#endif
#include <GL/gl.h>
#include <GL/glext.h>
#endif /* VPVL2_LINK_GLEW */

#if !defined(VPVL2_LINK_GLEW)

#ifndef GLEW_EXT_framebuffer_blit
  #ifdef GL_EXT_framebuffer_blit
    #define GLEW_EXT_framebuffer_blit 1
  #else
    #define GLEW_EXT_framebuffer_blit 0
    #define glBlitFramebufferEXT(expr) (void *) 0
  #endif
#endif /* GL_EXT_framebuffer_blit */

#ifndef GLEW_EXT_framebuffer_multisample
  #ifdef GL_EXT_framebuffer_multisample
    #define GLEW_EXT_framebuffer_multisample 1
  #else
    #define GLEW_EXT_framebuffer_multisample 0
    #define glRenderbufferStorageMultisampleEXT(expr) (void *) 0
  #endif
#endif /* GL_EXT_framebuffer_multisample */

#ifndef GLEW_ARB_debug_output
  #ifdef GL_ARB_debug_output
    #define GLEW_ARB_debug_output 1
    #define glDebugMessageCallback(callback, userParam) glDebugMessageCallbackARB(callback, userParam)
  #else
    #define GLEW_ARB_debug_output 0
    #define glDebugMessageCallback(expr) (void *) 0
  #endif
#endif /* GL_EXT_framebuffer_multisample */

#ifndef GLEW_ARB_sampler_objects
  #ifdef GL_ARB_sampler_objects
    #define GLEW_ARB_sampler_objects 1
  #else
    #define GLEW_ARB_sampler_objects 0
    #define glBindSampler(expr) (void *) 0
    #define glDeleteSamplers(expr) (void *) 0
    #define glGenSamplers(expr) (void *) 0
    #define glSamplerParameteri(expr) (void *) 0
  #endif
#endif /* GL_EXT_framebuffer_multisample */

#ifndef GLEW_ARB_texture_storage
  #ifdef GL_ARB_texture_storage
    #define GLEW_ARB_texture_storage 1
  #else
    #define GLEW_ARB_texture_storage 0
    #define glTexStorage2D(expr) (void *) 0
  #endif
#endif /* GL_EXT_framebuffer_multisample */

#if defined(GL_ARB_texture_float) && !defined(GLEW_ARB_texture_float)
#define GLEW_ARB_texture_float 1
#ifndef GL_RGBA32F
#define GL_RGBA32F GL_RGBA32F_ARB
#endif
#ifndef GL_RGB32F
#define GL_RGB32F GL_RGB32F_ARB
#endif
#ifndef GL_RGBA16F
#define GL_RGBA16F GL_RGBA16F_ARB
#endif
#ifndef GL_RGB16F
#define GL_RGB16F GL_RGB16F_ARB
#endif
#endif
#if defined(GL_ARB_depth_buffer_float) && !defined(GLEW_ARB_depth_buffer_float)
#define GLEW_ARB_depth_buffer_float 1
#endif /* GL_ARB_texture_float */

#if defined(VPVL2_ENABLE_GLES2)
#define glBlitFramebuffer(expr) (void *) 0
#define GLEW_EXT_framebuffer_blit 0
#define glGenFramebuffers(expr) (void *) 0
#define glBindFramerbuffer(expr) (void *) 0
#define glDeleteFramebuffers(expr) (void *) 0
#define glGenRenderbuffers(expr) (void *) 0
#define glBindRenderbuffer(expr) (void *) 0
#define glDeleteRenderbuffers(expr) (void *) 0
#define glRenderbufferStorageMultisample(expr) (void *) 0
#define glDebugMessageCallbackARB(expr) (void *) 0
#define GLEW_ARB_depth_buffer_float 0
#define GLEW_EXT_framebuffer_multisample 0
#define GL_DEPTH24_STENCIL8 0
#define GL_DEPTH32F_STENCIL8 0
#define GL_DEPTH_COMPONENT32F 0
#define GL_RGBA8 GL_RGBA8_OES
#define GL_BGRA GL_RGBA
#define GL_UNSIGNED_INT_8_8_8_8_REV GL_UNSIGNED_BYTE
#define GL_RGBA32F 0
#define GL_RGB32F 1
#define GL_RG32F 2
#define GL_R32F 3
#define GL_RGBA16F 4
#define GL_RGB16F 5
#define GL_RG16F 6
#define GL_R16F 7
#define GL_RG 0
#define GL_DEPTH_COMPONENT32F 0
#define glTexImage3D(expr) (void *) 0
#define GL_TEXTURE_3D 0
#define glDrawBuffers(expr) (void *) 0
#define glDrawBuffer(expr) (void *) 0
#define glReadBuffer(expr) (void *) 0
#define GL_DRAW_FRAMEBUFFER 0
#define GL_READ_FRAMEBUFFER 0
#define GL_DEBUG_SOURCE_API 1
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM 2
#define GL_DEBUG_SOURCE_SHADER_COMPILER 3
#define GL_DEBUG_SOURCE_THIRD_PARTY 4
#define GL_DEBUG_SOURCE_APPLICATION 5
#define GL_DEBUG_SOURCE_OTHER 6
#define GL_DEBUG_TYPE_ERROR 1
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR 2
#define GL_DEBUG_TYPE_PORTABILITY 3
#define GL_DEBUG_TYPE_PERFORMANCE 4
#define GL_DEBUG_TYPE_OTHER 5
#define GL_DEBUG_SEVERITY_HIGH 1
#define GL_DEBUG_SEVERITY_MEDIUM 2
#define GL_DEBUG_SEVERITY_LOW 3
#endif /* VPVL2_ENABLE_GLES2 */

#if defined(GL_ARB_vertex_array_object)
#define GLEW_ARB_vertex_array_object 1
#define GLEW_APPLE_vertex_array_object 0
#elif defined(GL_APPLE_vertex_array_object)
#define GLEW_ARB_vertex_array_object 0
#define GLEW_APPLE_vertex_array_object 1
#define glGenVertexArrays(expr) (void *) 0
#define glBindVertexArray(expr) (void *) 0
#define glDeleteVertexArrays(expr) (void *) 0
#else
#define GLEW_ARB_vertex_array_object 0
#define GLEW_APPLE_vertex_array_object 0
#define glGenVertexArrays(expr) (void *) 0
#define glBindVertexArray(expr) (void *) 0
#define glDeleteVertexArrays(expr) (void *) 0
#define glGenVertexArraysAPPLE(expr) (void *) 0
#define glBindVertexArrayAPPLE(expr) (void *) 0
#define glDeleteVertexArraysAPPLE(expr) (void *) 0
#endif /* GL_ARB_vertex_array_object */

#endif

#endif
