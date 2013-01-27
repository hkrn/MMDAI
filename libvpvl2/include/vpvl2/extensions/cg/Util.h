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

#pragma once
#ifndef VPVL2_EXTENSIONS_CG_UTIL_H_
#define VPVL2_EXTENSIONS_CG_UTIL_H_

#include "vpvl2/Common.h"
#include "vpvl2/extensions/gl/CommonMacros.h"

#include <string.h> /* strncmp */
#include <Cg/cg.h>
#include <Cg/cgGL.h>

#include <string> /* std::string */

#ifdef WIN32
#define strncasecmp _strnicmp
#endif
#define VPVL2_CG_GET_LENGTH_CONST(s) (sizeof(s) - 1)
#define VPVL2_CG_GET_SUFFIX(s, c) (s + VPVL2_CG_GET_LENGTH_CONST(c))
#define VPVL2_CG_STREQ_CONST(s, l, c) (l == VPVL2_CG_GET_LENGTH_CONST(c) && \
    0 == strncmp((s), (c), VPVL2_CG_GET_LENGTH_CONST(c)))
#define VPVL2_CG_STREQ_CASE_CONST(s, l, c) (l == VPVL2_CG_GET_LENGTH_CONST(c) && \
    0 == strncasecmp((s), (c), VPVL2_CG_GET_LENGTH_CONST(c)))
#define VPVL2_CG_STREQ_SUFFIX(s, l, c) (l >= VPVL2_CG_GET_LENGTH_CONST(c) && \
    0 == strncmp((s), (c), VPVL2_CG_GET_LENGTH_CONST(c)))

namespace vpvl2
{
namespace extensions
{
namespace cg
{

class VPVL2_API Util
{
public:
    static bool toBool(const CGannotation annotation) {
        int nvalues = 0;
        const CGbool *values = cgGetBoolAnnotationValues(annotation, &nvalues);
        return nvalues > 0 ? values[0] == CG_TRUE : false;
    }
    static int toInt(const CGannotation annotation) {
        int nvalues = 0;
        if (const int *values = cgGetIntAnnotationValues(annotation, &nvalues)) {
            return nvalues > 0 ? values[0] : 0;
        }
        else if (const float *values = cgGetFloatAnnotationValues(annotation, &nvalues)) {
            return nvalues > 0 ? int(values[0]) : 0;
        }
        return 0;
    }
    static float toFloat(const CGannotation annotation) {
        int nvalues = 0;
        if (const float *values = cgGetFloatAnnotationValues(annotation, &nvalues)) {
            return nvalues > 0 ? values[0] : 0;
        }
        else if (const int *values = cgGetIntAnnotationValues(annotation, &nvalues)) {
            return nvalues > 0 ? float(values[0]) : 0;
        }
        return 0;
    }
    static bool isPassEquals(const CGannotation annotation, const char *target) {
        if (!cgIsAnnotation(annotation))
            return true;
        const char *s = cgGetStringAnnotationValue(annotation);
        return s ? strcmp(s, target) == 0 : false;
    }
    static bool isIntegerParameter(const CGparameter parameter) {
        return cgGetParameterType(parameter) == CG_BOOL ||
                cgGetParameterType(parameter) == CG_INT ||
                cgGetParameterType(parameter) == CG_FLOAT;
    }
    static const std::string trim(const std::string &value) {
        std::string::const_iterator stringFrom = value.begin(), stringTo = value.end() - 1;
        while (isspace(*stringFrom) && (stringFrom != value.end()))
            ++stringFrom;
        while (isspace(*stringTo) && (stringTo != value.begin()))
            --stringTo;
        return (stringTo - stringFrom >= 0) ? std::string(stringFrom, ++stringTo) : "";
    }
    static const std::string trimLastSemicolon(const std::string &value) {
        std::string s = trim(value);
        if (s[s.length() - 1] == ';')
            s.erase(s.end() - 1);
        return Util::trim(s);
    }
    static void getTextureFormat(const CGparameter parameter,
                                 GLenum &internal,
                                 GLenum &format,
                                 GLenum &type)
    {
        static const char kDirect3DTextureFormatPrefix[] = "D3DFMT_";
        CGannotation formatAnnotation = cgGetNamedParameterAnnotation(parameter, "Format");
        internal = GL_RGBA8;
        format = GL_RGBA;
        type = GL_UNSIGNED_BYTE;
        const char *formatString = cgGetStringAnnotationValue(formatAnnotation);
        if (!formatString)
            return;
        const char *ptr = VPVL2_CG_STREQ_SUFFIX(formatString, VPVL2_CG_GET_LENGTH_CONST(kDirect3DTextureFormatPrefix), kDirect3DTextureFormatPrefix)
                ? VPVL2_CG_GET_SUFFIX(formatString, kDirect3DTextureFormatPrefix) : formatString;
        const size_t len = strlen(ptr);
        if (VPVL2_CG_STREQ_CONST(ptr, len, "A32B32G32R32F")) {
            internal = GL_RGBA32F;
            type = GL_FLOAT;
        }
        else if (VPVL2_CG_STREQ_CONST(ptr, len, "A16B16G16R16F")) {
            internal = GL_RGBA16F;
            type = GL_HALF_FLOAT;
        }
        else if (VPVL2_CG_STREQ_CONST(ptr, len, "X8R8G8B8")) {
            internal = GL_RGB8;
            format = GL_RGB;
            type = GL_UNSIGNED_BYTE;
        }
        else if (VPVL2_CG_STREQ_CONST(ptr, len, "G32R32F")) {
            internal = GL_RG32F;
            format = GL_RG;
            type = GL_FLOAT;
        }
        else if (VPVL2_CG_STREQ_CONST(ptr, len, "G16R16F")) {
            internal = GL_RG16F;
            format = GL_RG;
            type = GL_HALF_FLOAT;
        }
        else if (VPVL2_CG_STREQ_CONST(ptr, len, "G16R16")) {
            internal = GL_RG16;
            format = GL_RG;
            type = GL_UNSIGNED_SHORT;
        }
        else if (VPVL2_CG_STREQ_CONST(ptr, len, "R32F")) {
            internal = GL_R32F;
            format = GL_RED;
            type = GL_FLOAT;
        }
        else if (VPVL2_CG_STREQ_CONST(ptr, len, "R16F")) {
            internal = GL_R16F;
            format = GL_RED;
            type = GL_HALF_FLOAT;
        }
        else if (VPVL2_CG_STREQ_CONST(ptr, len, "A8")) {
            internal = GL_LUMINANCE8;
            format = GL_LUMINANCE;
        }
    }
    static bool getSize2(const CGparameter parameter, Vector3 &size) {
        int nvalues = 0;
        size.setValue(1, 1, 0);
        const CGannotation viewportRatioAnnotation = cgGetNamedParameterAnnotation(parameter, "ViewPortRatio");
        if (cgIsAnnotation(viewportRatioAnnotation)) {
            const float *values = cgGetFloatAnnotationValues(viewportRatioAnnotation, &nvalues);
            if (nvalues == 2) {
                size.setValue(values[0], values[1], 0);
                return false;
            }
        }
        const CGannotation dimensionsAnnotation = cgGetNamedParameterAnnotation(parameter, "Dimensions");
        if (cgIsAnnotation(dimensionsAnnotation)) {
            const int *values = cgGetIntAnnotationValues(viewportRatioAnnotation, &nvalues);
            if (nvalues == 2) {
                size.setValue(btMax(1, values[0]), btMax(1, values[1]), 0);
                return true;
            }
        }
        const CGannotation widthAnnotation = cgGetNamedParameterAnnotation(parameter, "Width");
        const CGannotation heightAnnotation = cgGetNamedParameterAnnotation(parameter, "Height");
        if (cgIsAnnotation(widthAnnotation) && cgIsAnnotation(heightAnnotation)) {
            size.setValue(btMax(1, toInt(widthAnnotation)), btMax(1, toInt(heightAnnotation)), 0);
            return true;
        }
        return false;
    }
    static bool getSize3(const CGparameter parameter, Vector3 &size) {
        int nvalues = 0;
        const CGannotation dimensionsAnnotation = cgGetNamedParameterAnnotation(parameter, "Dimensions");
        size.setValue(1, 1, 1);
        if (cgIsAnnotation(dimensionsAnnotation)) {
            const int *values = cgGetIntAnnotationValues(dimensionsAnnotation, &nvalues);
            if (nvalues == 3) {
                size.setValue(values[0], values[1], values[2]);
                return true;
            }
        }
        const CGannotation widthAnnotation = cgGetNamedParameterAnnotation(parameter, "Width");
        const CGannotation heightAnnotation = cgGetNamedParameterAnnotation(parameter, "Height");
        const CGannotation depthAnnotation = cgGetNamedParameterAnnotation(parameter, "Depth");
        if (cgIsAnnotation(widthAnnotation) && cgIsAnnotation(heightAnnotation) && cgIsAnnotation(depthAnnotation)) {
            size.setX(btMax(size_t(1), size_t(Util::toInt(widthAnnotation))));
            size.setY(btMax(size_t(1), size_t(Util::toInt(heightAnnotation))));
            size.setZ(btMax(size_t(1), size_t(Util::toInt(depthAnnotation))));
            return true;
        }
        return false;
    }
    static void setRenderColorTargets(const GLenum *targets, int ntargets) {
        glDrawBuffers(ntargets, targets);
        if (ntargets == 0)
            glDrawBuffer(GL_BACK);
    }

private:
    VPVL2_MAKE_STATIC_CLASS(Util)
};

} /* namespace cg */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
