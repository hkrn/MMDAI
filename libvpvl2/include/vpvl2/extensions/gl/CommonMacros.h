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

#ifndef GLAPIENTRY
#define GLAPIENTRY
#endif

#ifndef GL_TRUE
#define GL_TRUE  1
#endif

#ifndef GL_FALSE
#define GL_FALSE 0
#endif

namespace vpvl2 {
namespace extensions {
namespace gl {

typedef void GLvoid;
typedef char GLchar;
typedef int GLint;
typedef int GLsizei;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLclampd;
typedef unsigned char GLboolean;
typedef unsigned int GLenum;
typedef unsigned int GLuint;
typedef unsigned int GLbitfield;
typedef ptrdiff_t GLintptr;
typedef ptrdiff_t GLsizeiptr;

static const GLenum kGL_NONE = 0;
static const GLenum kGL_POINTS = 0x0000;
static const GLenum kGL_LINES = 0x0001;
static const GLenum kGL_TRIANGLES = 0x0004;
static const GLenum kGL_QUADS = 0x0007;
static const GLenum kGL_UNSIGNED_BYTE = 0x1401;
static const GLenum kGL_UNSIGNED_SHORT = 0x1403;
static const GLenum kGL_UNSIGNED_INT = 0x1405;
static const GLenum kGL_CULL_FACE = 0x0B44;
static const GLenum kGL_FRONT = 0x0404;
static const GLenum kGL_BACK = 0x0405;
static const GLenum kGL_BLEND = 0x0BE2;
static const GLenum kGL_DEPTH_TEST = 0x0B71;
static const GLenum kGL_COLOR_BUFFER_BIT = 0x00004000;
static const GLenum kGL_DEPTH_BUFFER_BIT = 0x00000100;
static const GLenum kGL_STENCIL_BUFFER_BIT = 0x00000400;

static const GLenum kGL_RED = 0x1903;
static const GLenum kGL_LUMINANCE = 0x1909;
static const GLenum kGL_LUMINANCE8 = 0x8040;
static const GLenum kGL_RGB = 0x1907;
static const GLenum kGL_RGBA = 0x1908;
static const GLenum kGL_RGB8 = 0x8051;
static const GLenum kGL_RGBA8 = 0x8058;
static const GLenum kGL_FLOAT = 0x1406;
static const GLenum kGL_UNSIGNED_INT_8_8_8_8_REV = 0x8367;

static const GLenum kGL_RGBA32F_ARB = 0x8814;
static const GLenum kGL_RGB32F_ARB = 0x8815;
static const GLenum kGL_RGBA16F_ARB = 0x881A;
static const GLenum kGL_RGB16F_ARB = 0x881B;
static const GLenum kGL_HALF_FLOAT_ARB = 0x140B;

static const GLenum kGL_R16 = 0x822A;
static const GLenum kGL_R16F = 0x822D;
static const GLenum kGL_R32F = 0x822E;
static const GLenum kGL_RG16 = 0x822C;
static const GLenum kGL_RG16F = 0x822F;
static const GLenum kGL_RG32F = 0x8230;
static const GLenum kGL_RG = 0x8227;

static const GLenum kGL_QUERY_RESULT = 0x8866;
static const GLenum kGL_ANY_SAMPLES_PASSED = 0x8C2F;
static const GLenum kGL_SAMPLES_PASSED = 0x8914;

} /* namespace gl */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
