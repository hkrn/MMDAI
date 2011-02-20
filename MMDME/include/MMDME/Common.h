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
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

/* convert model coordinates from left-handed to right-handed */
#define MMDFILES_CONVERTCOORDINATESYSTEM

enum MMDAILogLevel
{
  MMDAILogLevelDebug,
  MMDAILogLevelInfo,
  MMDAILogLevelWarning,
  MMDAILogLevelError,
};

typedef void (MMDAILoggingHandler)(const char *file, int line, enum MMDAILogLevel level, const char *format, va_list ap);

void MMDAILogSetHandler(MMDAILoggingHandler *handler);
void MMDAILogWrite(const char *file, int line, enum MMDAILogLevel level, const char *format, ...);

/* log with variable arguments */
#define MMDAILogDebug(format, ...) \
  MMDAILogWrite(__FILE__, __LINE__, (MMDAILogLevelDebug), (format), __VA_ARGS__)
#define MMDAILogInfo(format, ...) \
  MMDAILogWrite(__FILE__, __LINE__, (MMDAILogLevelInfo), (format), __VA_ARGS__)
#define MMDAILogWarn(format, ...) \
  MMDAILogWrite(__FILE__, __LINE__, (MMDAILogLevelWarning), (format), __VA_ARGS__)
#define MMDAILogError(format, ...) \
  MMDAILogWrite(__FILE__, __LINE__, (MMDAILogLevelError), (format), __VA_ARGS__)

/* log with single string */
#define MMDAILogDebugString(format) \
  MMDAILogWrite(__FILE__, __LINE__, (MMDAILogLevelDebug), (format))
#define MMDAILogInfoString(format) \
  MMDAILogWrite(__FILE__, __LINE__, (MMDAILogLevelInfo), (format))
#define MMDAILogWarnString(format) \
  MMDAILogWrite(__FILE__, __LINE__, (MMDAILogLevelWarning), (format))
#define MMDAILogErrorString(format) \
  MMDAILogWrite(__FILE__, __LINE__, (MMDAILogLevelError), (format))

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

/* disable _CRT_SECURE_NO_WARNINGS for MSVC */
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE 
#endif

inline size_t MMDAIStringLength(const char *str)
{
  assert(str != NULL);
  return strlen(str);
}

inline char *MMDAIStringClone(const char *str)
{
  assert(str != NULL);
#if defined(WIN32)
  return _strdup(str);
#else
  return strdup(str);
#endif
}

inline char *MMDAIStringCopy(char *dst, const char *src, size_t max)
{
  assert(dst != NULL && src != NULL && max != 0);
  return strncpy(dst, src, max);
}

inline bool MMDAIStringEquals(const char *s1, const char *s2)
{
  assert(s1 != NULL && s2 != NULL);
  return strcmp(s1, s2) == 0;
}

inline bool MMDAIStringEqualsIn(const char *s1, const char *s2, size_t max)
{
  assert(s1 != NULL && s2 != NULL && max != 0);
  return strncmp(s1, s2, max) == 0;
}

inline char *MMDAIStringGetToken(char *str, const char *delim, char **ptr)
{
  assert(delim != NULL);
#if defined(WIN32)
  (void)ptr;
  return strtok(str, delim);
#else
  return strtok_r(str, delim, ptr);
#endif
}

#undef _CRT_SECURE_NO_DEPRECATE

inline int MMDAIStringFormat(char *str, size_t n, const char *format, ...)
{
  assert(str != NULL && n != 0 && format != NULL);
  va_list ap;
  va_start(ap, format);
  int len = vsnprintf(str, n, format, ap);
  va_end(ap);
  return len;
}

inline double MMDAIStringToDouble(const char *str)
{
  assert(str != NULL);
  return atof(str);
}

inline float MMDAIStringToFloat(const char *str)
{
  return (float) MMDAIStringToDouble(str);
}

#define MMDME_DISABLE_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName &); \
  void operator=(const TypeName &);

#endif

