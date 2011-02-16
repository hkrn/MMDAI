/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2010  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn (libMMDAI)                         */
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

#ifndef MMDME_COMMON_H_
#define MMDME_COMMON_H_

#include <assert.h>
#include <string.h>
#include <stdlib.h>

/* convert model coordinates from left-handed to right-handed */
#define MMDFILES_CONVERTCOORDINATESYSTEM

/* convert from/to radian */
inline float MMDME_RAD(float a)
{
  return a * (3.1415926f / 180.0f);
}

inline float MMDME_DEG(float a)
{
  return a * (180.0f / 3.1415926f);
}

inline void *MMDAIMemoryAllocate(size_t size)
{
  return malloc(size);
}

inline void MMDAIMemoryRelease(void *ptr)
{
  if (ptr != NULL)
    free(ptr);
}

inline int MMDAIStringLength(const char *str)
{
  assert(str != NULL);
  return strlen(str);
}

inline char *MMDAIStringClone(const char *str)
{
  assert(str != NULL);
  return strdup(str);
}

inline char *MMDAIStringCopy(char *dst, const char *src, int max)
{
  assert(dst != NULL && src != NULL && max != 0);
  return strncpy(dst, src, max);
}

inline bool MMDAIStringEquals(const char *s1, const char *s2)
{
  assert(s1 != NULL && s2 != NULL);
  return strcmp(s1, s2) == 0;
}

inline bool MMDAIStringEqualsIn(const char *s1, const char *s2, int max)
{
  assert(s1 != NULL && s2 != NULL && max != 0);
  return strncmp(s1, s2, max) == 0;
}

#define MMDME_DISABLE_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName &); \
  void operator=(const TypeName &);

#endif

