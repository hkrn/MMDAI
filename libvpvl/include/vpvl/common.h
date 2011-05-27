#ifndef VPVL_COMMON_H_
#define VPVL_COMMON_H_

#define VPVL_COORDINATE_OPENGL

#if defined(WIN32)
#define _USE_MATH_DEFINES
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

enum vpvlLogLevel
{
    vpvlLogLevelDebug,
    vpvlLogLevelInfo,
    vpvlLogLevelWarning,
    vpvlLogLevelError,
};

typedef void (vpvlLoggingHandler)(const char *file,
                                  int line,
                                  const enum vpvlLogLevel level,
                                  const char *format,
                                  va_list ap);
typedef void (vpvlLoggingSJISHandler)(const char *file,
                                      int line,
                                      const enum vpvlLogLevel level,
                                      const char *format,
                                      va_list ap);

void vpvlLogSetHandler(vpvlLoggingHandler *handler);
void vpvlLogSetHandlerSJIS(vpvlLoggingSJISHandler *handler);
void vpvlLogWrite(const char *file,
                  int line,
                  const enum vpvlLogLevel level,
                  const char *format, ...);
void vpvlLogWriteSJIS(const char *file,
                       int line,
                       const enum vpvlLogLevel level,
                       const char *format, ...);

/* log with variable arguments */
#define vpvlLogDebug(format, ...) \
vpvlLogWrite(__FILE__, __LINE__, (vpvlLogLevelDebug), (format), __VA_ARGS__)
#define vpvlLogInfo(format, ...) \
        vpvlLogWrite(__FILE__, __LINE__, (vpvlLogLevelInfo), (format), __VA_ARGS__)
#define vpvlLogWarn(format, ...) \
        vpvlLogWrite(__FILE__, __LINE__, (vpvlLogLevelWarning), (format), __VA_ARGS__)
#define vpvlLogError(format, ...) \
        vpvlLogWrite(__FILE__, __LINE__, (vpvlLogLevelError), (format), __VA_ARGS__)

        /* log with single string */
#define vpvlLogDebugString(format) \
        vpvlLogWrite(__FILE__, __LINE__, (vpvlLogLevelDebug), (format))
#define vpvlLogInfoString(format) \
        vpvlLogWrite(__FILE__, __LINE__, (vpvlLogLevelInfo), (format))
#define vpvlLogWarnString(format) \
        vpvlLogWrite(__FILE__, __LINE__, (vpvlLogLevelWarning), (format))
#define vpvlLogErrorString(format) \
        vpvlLogWrite(__FILE__, __LINE__, (vpvlLogLevelError), (format))

        /* log with variable arguments for SJIS */
#define vpvlLogDebugSJIS(format, ...) \
        vpvlLogWriteSJIS(__FILE__, __LINE__, (vpvlLogLevelDebug), (format), __VA_ARGS__)
#define vpvlLogInfoSJIS(format, ...) \
        vpvlLogWriteSJIS(__FILE__, __LINE__, (vpvlLogLevelInfo), (format), __VA_ARGS__)
#define vpvlLogWarnSJIS(format, ...) \
        vpvlLogWriteSJIS(__FILE__, __LINE__, (vpvlLogLevelWarning), (format), __VA_ARGS__)
#define vpvlLogErrorSJIS(format, ...) \
        vpvlLogWriteSJIS(__FILE__, __LINE__, (vpvlLogLevelError), (format), __VA_ARGS__)

        /* log with single string for SJIS */
#define vpvlLogDebugStringSJIS(format) \
        vpvlLogWriteSJIS(__FILE__, __LINE__, (vpvlLogLevelDebug), (format))
#define vpvlLogInfoStringSJIS(format) \
        vpvlLogWriteSJIS(__FILE__, __LINE__, (vpvlLogLevelInfo), (format))
#define vpvlLogWarnStringSJIS(format) \
        vpvlLogWriteSJIS(__FILE__, __LINE__, (vpvlLogLevelWarning), (format))
#define vpvlLogErrorStringSJIS(format) \
        vpvlLogWriteSJIS(__FILE__, __LINE__, (vpvlLogLevelError), (format))

namespace vpvl
{

/* convert from/to radian */
inline float radian(float value)
{
    return value * static_cast<float>(M_PI / 180.0f);
}

inline float degree(float value)
{
    return value * static_cast<float>(180.0f / M_PI);
}

inline size_t stringLength(const char *str)
{
    assert(str != NULL);
    return strlen(str);
}

inline char *stringClone(const char *str)
{
    assert(str != NULL);
#if defined(WIN32)
    return _strdup(str);
#else
    return strdup(str);
#endif
}

inline char *stringCopy(char *dst, const char *src, size_t max)
{
    assert(dst != NULL && src != NULL && max > 0);
    return strncpy(dst, src, max);
}

inline char *stringCopySafe(char *dst, const char *src, size_t max)
{
    assert(dst != NULL && src != NULL && max > 0);
    size_t len = max - 1;
    char *ptr = strncpy(dst, src, len);
    dst[len] = '\0';
    return ptr;
}

inline bool stringEquals(const char *s1, const char *s2)
{
    assert(s1 != NULL && s2 != NULL);
    return strcmp(s1, s2) == 0;
}

inline bool stringEqualsIn(const char *s1, const char *s2, size_t max)
{
    assert(s1 != NULL && s2 != NULL && max > 0);
    return strncmp(s1, s2, max) == 0;
}

inline double stringToDouble(const char *str)
{
    assert(str != NULL);
    return atof(str);
}

inline float stringToFloat(const char *str)
{
    return (float) stringToDouble(str);
}

}

#endif
