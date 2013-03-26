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
#ifndef VPVL2_EXTENSIONS_GL_BASESURFACE_H_
#define VPVL2_EXTENSIONS_GL_BASESURFACE_H_

#include <vpvl2/Common.h>
#include <vpvl2/extensions/gl/CommonMacros.h>

#define VPVL2_BASESURFACE_INITIALIZE_FIELDS(format, size, sampler) \
    m_format(format), \
    m_size(size), \
    m_name(0), \
    m_sampler(sampler)

#define VPVL2_BASESURFACE_DESTROY_FIELDS() \
    m_size.setZero(); \
    m_name = 0; \
    m_sampler = 0;

#define VPVL2_BASESURFACE_DEFINE_METHODS() \
    Vector3 size() const { return m_size; } \
    intptr_t format() const { return reinterpret_cast<intptr_t>(&m_format); } \
    intptr_t data() const { return m_name; } \
    intptr_t sampler() const { return m_sampler; }

#define VPVL2_BASESURFACE_DEFINE_FIELDS() \
    mutable BaseSurface::Format m_format; \
    Vector3 m_size; \
    GLuint m_name; \
    GLuint m_sampler;

namespace vpvl2
{
namespace extensions
{
namespace gl
{

class BaseSurface {
public:
    struct Format {
        Format()
            : external(0),
              internal(0),
              type(0),
              target(0)
        {
        }
        Format(GLenum e, GLenum i, GLenum t, GLenum g)
            : external(e),
              internal(i),
              type(t),
              target(g)
        {
        }
        GLenum external;
        GLenum internal;
        GLenum type;
        GLenum target;
    };

private:
    BaseSurface();
    ~BaseSurface() {}
};

} /* namespace gl */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
