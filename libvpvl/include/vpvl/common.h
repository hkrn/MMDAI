/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn                                    */
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

#ifndef VPVL_COMMON_H_
#define VPVL_COMMON_H_

#include "vpvl/config.h"

#if defined(WIN32) && !(defined(__MINGW__) || defined(__MINGW32__))
typedef char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
#else
#include <stdint.h>
#endif

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#if defined (WIN32)
  #if defined(vpvl_EXPORTS)
    #define VPVL_EXPORT __declspec(dllexport)
  #else
    #define VPVL_EXPORT __declspec(dllimport)
  #endif /* MyLibrary_EXPORTS */
#else /* defined (_WIN32) */
 #define VPVL_EXPORT
#endif

namespace vpvl
{

VPVL_EXPORT static const float kPI = 3.14159265358979323846f;
VPVL_EXPORT bool isLibraryVersionCorrect(int version);
VPVL_EXPORT const char *libraryVersionString();

/* convert from/to radian */
VPVL_EXPORT inline float radian(float value)
{
    return value * static_cast<float>(kPI / 180.0f);
}

VPVL_EXPORT inline float degree(float value)
{
    return value * static_cast<float>(180.0f / kPI);
}

VPVL_EXPORT inline uint8_t *copyBytesSafe(uint8_t *dst, const uint8_t *src, size_t max)
{
    assert(dst != NULL && src != NULL && max > 0);
    size_t len = max - 1;
    uint8_t *ptr = static_cast<uint8_t *>(memcpy(dst, src, len));
    dst[len] = '\0';
    return ptr;
}

}

#define VPVL_DISABLE_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName &); \
    void operator=(const TypeName &);

#endif
