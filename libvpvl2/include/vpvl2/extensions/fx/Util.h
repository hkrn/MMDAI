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
#ifndef VPVL2_EXTENSIONS_FX_UTIL_H_
#define VPVL2_EXTENSIONS_FX_UTIL_H_

#include "vpvl2/IApplicationContext.h"
#include "vpvl2/IEffect.h"
#include "vpvl2/gl/BaseSurface.h"

#include <cstring> /* std::strncmp */
#include <string> /* std::string */

#if defined(VPVL2_OS_WINDOWS)
#define strncasecmp _strnicmp
#endif /* VPVL2_OS_WINDOWS */

#define VPVL2_FX_GET_LENGTH_CONST(s) (sizeof(s) - 1)
#define VPVL2_FX_GET_SUFFIX(s, c) (s + VPVL2_FX_GET_LENGTH_CONST(c))
#define VPVL2_FX_STREQ_CONST(s, l, c) (l == VPVL2_FX_GET_LENGTH_CONST(c) && \
    0 == std::strncmp((s), (c), VPVL2_FX_GET_LENGTH_CONST(c)))
#define VPVL2_FX_STREQ_CASE_CONST(s, l, c) (l == VPVL2_FX_GET_LENGTH_CONST(c) && \
    0 == strncasecmp((s), (c), VPVL2_FX_GET_LENGTH_CONST(c)))
#define VPVL2_FX_STREQ_SUFFIX(s, l, c) (l >= VPVL2_FX_GET_LENGTH_CONST(c) && \
    0 == std::strncmp((s), (c), VPVL2_FX_GET_LENGTH_CONST(c)))

namespace vpvl2
{
namespace extensions
{
namespace fx
{

class VPVL2_API Util VPVL2_DECL_FINAL
{
public:
    static bool isPassEquals(const IEffect::Annotation *annotation, const char *target) {
        if (annotation) {
            const char *s = annotation->stringValue();
            return std::strcmp(s, target) == 0;
        }
        return true;
    }
    static bool isIntegerParameter(const IEffect::Parameter *parameter) {
        if (parameter) {
            switch (parameter->type()) {
            case IEffect::Parameter::kBoolean:
            case IEffect::Parameter::kInteger:
            case IEffect::Parameter::kFloat:
                return true;
            default:
                return false;
            }
        }
        return false;
    }
    static std::string trim(const std::string &value) {
        std::string::const_iterator stringFrom = value.begin(), stringTo = value.end() - 1;
        while (isspace(*stringFrom) && (stringFrom != value.end())) {
            ++stringFrom;
        }
        while (isspace(*stringTo) && (stringTo != value.begin())) {
            --stringTo;
        }
        return (stringTo - stringFrom >= 0) ? std::string(stringFrom, ++stringTo) : std::string();
    }
    static std::string trimLastSemicolon(const std::string &value) {
        std::string s = trim(value);
        if (s[s.length() - 1] == ';') {
            s.erase(s.end() - 1);
        }
        return Util::trim(s);
    }
    static void getTextureFormat(const IEffect::Parameter *parameterRef, const IApplicationContext::FunctionResolver *resolver, vpvl2::gl::BaseSurface::Format &format) {
        static const char kDirect3DTextureFormatPrefix[] = "D3DFMT_";
        format.internal = vpvl2::gl::kGL_RGBA8;
        format.external = vpvl2::gl::kGL_RGBA;
        format.type = vpvl2::gl::kGL_UNSIGNED_BYTE;
        const IEffect::Annotation *formatAnnotation = parameterRef->annotationRef("Format");
        if (!formatAnnotation) {
            return;
        }
        const char *formatString = formatAnnotation->stringValue();
        const char *ptr = VPVL2_FX_STREQ_SUFFIX(formatString, VPVL2_FX_GET_LENGTH_CONST(kDirect3DTextureFormatPrefix), kDirect3DTextureFormatPrefix)
                ? VPVL2_FX_GET_SUFFIX(formatString, kDirect3DTextureFormatPrefix) : formatString;
        const vsize len = strlen(ptr);
        if (resolver->hasExtension("ARB_texture_float") && VPVL2_FX_STREQ_CONST(ptr, len, "A32B32G32R32F")) {
            format.internal = vpvl2::gl::kGL_RGBA32F;
            format.type = vpvl2::gl::kGL_FLOAT;
        }
        else if (resolver->hasExtension("ARB_texture_rg")) {
            if (VPVL2_FX_STREQ_CONST(ptr, len, "G32R32F")) {
                format.internal = vpvl2::gl::kGL_RG32F;
                format.external = vpvl2::gl::kGL_RG;
                format.type = vpvl2::gl::kGL_FLOAT;
            }
            else if (VPVL2_FX_STREQ_CONST(ptr, len, "G16R16F")) {
                format.internal = vpvl2::gl:: kGL_RG16F;
                format.external = vpvl2::gl:: kGL_RG;
                format.type = vpvl2::gl::kGL_HALF_FLOAT;
            }
            else if (VPVL2_FX_STREQ_CONST(ptr, len, "G16R16")) {
                format.internal = vpvl2::gl::kGL_RG16;
                format.external = vpvl2::gl::kGL_RG;
                format.type = vpvl2::gl::kGL_UNSIGNED_SHORT;
            }
            else if (VPVL2_FX_STREQ_CONST(ptr, len, "R32F")) {
                format.internal = vpvl2::gl::kGL_R32F;
                format.external = vpvl2::gl::kGL_RED;
                format.type = vpvl2::gl::kGL_FLOAT;
            }
        }
        else if (resolver->hasExtension("ARB_half_float_pixel")) {
            if (VPVL2_FX_STREQ_CONST(ptr, len, "A16B16G16R16F")) {
                format.internal = vpvl2::gl::kGL_RGBA16F;
                format.type = vpvl2::gl::kGL_HALF_FLOAT;
            }
            else if (VPVL2_FX_STREQ_CONST(ptr, len, "R16F")) {
                format.internal = vpvl2::gl::kGL_R16F;
                format.external = vpvl2::gl::kGL_RED;
                format.type = vpvl2::gl::kGL_HALF_FLOAT;
            }
        }
        else if (VPVL2_FX_STREQ_CONST(ptr, len, "X8R8G8B8")) {
            format.internal = vpvl2::gl::kGL_RGB8;
            format.external = vpvl2::gl::kGL_RGB;
            format.type = vpvl2::gl::kGL_UNSIGNED_BYTE;
        }
        else if (VPVL2_FX_STREQ_CONST(ptr, len, "A8")) {
            format.internal = vpvl2::gl::kGL_LUMINANCE8;
            format.external = vpvl2::gl::kGL_LUMINANCE;
        }
    }
    static bool getSize2(const IEffect::Parameter *parameterRef, Vector3 &size) {
        int nvalues = 0;
        size.setValue(1, 1, 0);
        if (const IEffect::Annotation *annotation = parameterRef->annotationRef("ViewPortRatio")) {
            const float *values = annotation->floatValues(&nvalues);
            if (nvalues == 2) {
                size.setValue(values[0], values[1], 0);
                return false;
            }
        }
        if (const IEffect::Annotation *annotation = parameterRef->annotationRef("Dimensions")) {
            const int *values = annotation->integerValues(&nvalues);
            if (nvalues == 2) {
                size.setValue(Scalar(btMax(1, values[0])), Scalar(btMax(1, values[1])), 0);
                return true;
            }
        }
        const IEffect::Annotation *widthAnnotation = parameterRef->annotationRef("Width");
        const IEffect::Annotation *heightAnnotation = parameterRef->annotationRef("Height");
        if (widthAnnotation && heightAnnotation) {
            size.setValue(Scalar(btMax(1, widthAnnotation->integerValue())), Scalar(btMax(1, heightAnnotation->integerValue())), 0);
            return true;
        }
        return false;
    }
    static bool getSize3(const IEffect::Parameter *parameterRef, Vector3 &size) {
        int nvalues = 0;
        size.setValue(1, 1, 1);
        if (const IEffect::Annotation *annotation = parameterRef->annotationRef("Dimensions")) {
            const int *values = annotation->integerValues(&nvalues);
            if (nvalues == 3) {
                size.setValue(Scalar(values[0]), Scalar(values[1]), Scalar(values[2]));
                return true;
            }
        }
        const IEffect::Annotation *widthAnnotation = parameterRef->annotationRef("Width");
        const IEffect::Annotation *heightAnnotation = parameterRef->annotationRef("Height");
        const IEffect::Annotation *depthAnnotation = parameterRef->annotationRef("Depth");
        if (widthAnnotation && heightAnnotation && depthAnnotation) {
            size.setX(Scalar(btMax(1, widthAnnotation->integerValue())));
            size.setY(Scalar(btMax(1, heightAnnotation->integerValue())));
            size.setZ(Scalar(btMax(1, depthAnnotation->integerValue())));
            return true;
        }
        return false;
    }
    static void setRenderColorTargets(const IApplicationContext::FunctionResolver *resolver, const vpvl2::gl::GLenum *targets, int ntargets) {
        typedef void (GLAPIENTRY * PFNGLDRAWBUFFERSPROC) (vpvl2::gl::GLsizei n, const vpvl2::gl::GLenum* bufs);
        if (PFNGLDRAWBUFFERSPROC drawBuffers = reinterpret_cast<PFNGLDRAWBUFFERSPROC>(resolver->resolveSymbol("glDrawBuffers"))) {
            if (ntargets == 0) {
                drawBuffers(0, 0);
            }
            else {
                drawBuffers(ntargets, targets);
            }
        }
    }

private:
    VPVL2_MAKE_STATIC_CLASS(Util)
};

} /* namespace fx */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
