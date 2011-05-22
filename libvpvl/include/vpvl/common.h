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

/* convert from/to radian */
inline float vpvlMathRadian(float value)
{
    return value * static_cast<float>(M_PI / 180.0f);
}

inline float vpvlMathDegree(float value)
{
    return value * static_cast<float>(180.0f / M_PI);
}

inline size_t vpvlStringLength(const char *str)
{
    assert(str != NULL);
    return strlen(str);
}

inline char *vpvlStringClone(const char *str)
{
    assert(str != NULL);
#if defined(WIN32)
    return _strdup(str);
#else
    return strdup(str);
#endif
}

inline char *vpvlStringCopy(char *dst, const char *src, size_t max)
{
    assert(dst != NULL && src != NULL && max > 0);
    return strncpy(dst, src, max);
}

inline char *vpvlStringCopySafe(char *dst, const char *src, size_t max)
{
    assert(dst != NULL && src != NULL && max > 0);
    size_t len = max - 1;
    char *ptr = strncpy(dst, src, len);
    dst[len] = '\0';
    return ptr;
}

inline bool vpvlStringEquals(const char *s1, const char *s2)
{
    assert(s1 != NULL && s2 != NULL);
    return strcmp(s1, s2) == 0;
}

inline bool vpvlStringEqualsIn(const char *s1, const char *s2, size_t max)
{
    assert(s1 != NULL && s2 != NULL && max > 0);
    return strncmp(s1, s2, max) == 0;
}

inline char *vpvlStringGetToken(char *str, const char *delim, char **ptr)
{
    assert(delim != NULL);
#if defined(WIN32)
    (void)ptr;
    return strtok(str, delim);
#else
    return strtok_r(str, delim, ptr);
#endif
}

inline int vpvlStringFormat(char *str, size_t max, const char *format, ...)
{
    assert(str != NULL && max > 0 && format != NULL);
    va_list ap;
    va_start(ap, format);
    int len = vsnprintf(str, max, format, ap);
    va_end(ap);
    return len;
}

inline int vpvlStringFormatSafe(char *str, size_t max, const char *format, ...)
{
    assert(str != NULL && max > 0 && format != NULL);
    va_list ap;
    va_start(ap, format);
    size_t n = max - 1;
    int len = vsnprintf(str, n, format, ap);
    va_end(ap);
    str[n] = '\0';
    return len;
}

inline double vpvlStringToDouble(const char *str)
{
    assert(str != NULL);
    return atof(str);
}

inline float vpvlStringToFloat(const char *str)
{
    return (float) vpvlStringToDouble(str);
}

inline void vpvlStringGetVector3(char *ptr, float *values)
{
    for (int i = 0; i < 3; i++) {
        values[i] = *reinterpret_cast<float *>(ptr);
        ptr += sizeof(float);
    }
}

inline bool vpvlDataGetSize8(char *&ptr, size_t &rest, size_t &size)
{
    if (sizeof(uint8_t) > rest)
        return false;
    size = *reinterpret_cast<uint8_t *>(ptr);
    ptr += sizeof(uint8_t);
    rest -= sizeof(uint8_t);
    return true;
}

inline bool vpvlDataGetSize16(char *&ptr, size_t &rest, size_t &size)
{
    if (sizeof(uint16_t) > rest)
        return false;
    size = *reinterpret_cast<uint16_t *>(ptr);
    ptr += sizeof(uint16_t);
    rest -= sizeof(uint16_t);
    return true;
}

inline bool vpvlDataGetSize32(char *&ptr, size_t &rest, size_t &size)
{
    if (sizeof(uint32_t) > rest)
        return false;
    size = *reinterpret_cast<uint32_t *>(ptr);
    ptr += sizeof(uint32_t);
    rest -= sizeof(uint32_t);
    return true;
}

inline bool vpvlDataValidateSize(char *&ptr, size_t stride, size_t size, size_t &rest)
{
    size_t required = stride * size;
    if (required > rest)
        return false;
    ptr += required;
    rest -= required;
    return true;
}

#define VPVL_DISABLE_COPY_AND_ASSIGN(TypeName) \
TypeName(const TypeName &); \
         void operator=(const TypeName &);

#endif

