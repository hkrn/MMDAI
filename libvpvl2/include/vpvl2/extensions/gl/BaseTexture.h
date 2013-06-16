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
#ifndef VPVL2_EXTENSIONS_GL_BASETEXTURE_H_
#define VPVL2_EXTENSIONS_GL_BASETEXTURE_H_

#include <vpvl2/ITexture.h>
#include <vpvl2/extensions/gl/BaseSurface.h>

namespace vpvl2
{
namespace extensions
{
namespace gl
{

class BaseTexture : public ITexture {
public:
    BaseTexture(const BaseSurface::Format &format, const Vector3 &size, GLuint sampler)
        : VPVL2_BASESURFACE_INITIALIZE_FIELDS(format, size, sampler)
    {
    }
    ~BaseTexture() {
        release();
        VPVL2_BASESURFACE_DESTROY_FIELDS()
    }
    void create() {
        glGenTextures(1, &m_name);
        wrapGenerate();
    }
    void bind() {
        glBindTexture(m_format.target, m_name);
    }
    void unbind() {
        glBindTexture(m_format.target, 0);
    }
    void release() {
        glDeleteTextures(1, &m_name);
    }
    void resize(const Vector3 &value) {
        if (value != m_size) {
            m_size = value;
            wrapGenerate();
        }
    }

    VPVL2_BASESURFACE_DEFINE_METHODS()

protected:
    virtual void generate() = 0;

    VPVL2_BASESURFACE_DEFINE_FIELDS()

private:
    void wrapGenerate() {
        bind();
        generate();
        unbind();
    }
};

} /* namespace gl */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
