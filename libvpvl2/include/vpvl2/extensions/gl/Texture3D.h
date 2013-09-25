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
#ifndef VPVL2_EXTENSIONS_GL_TEXTURE3D_H_
#define VPVL2_EXTENSIONS_GL_TEXTURE3D_H_

#include <vpvl2/ITexture.h>
#include <vpvl2/extensions/gl/BaseTexture.h>

namespace vpvl2
{
namespace extensions
{
namespace gl
{

class Texture3D VPVL2_DECL_FINAL : public BaseTexture {
public:
    Texture3D(const BaseSurface::Format &format, const Vector3 &size, GLenum sampler)
        : BaseTexture(format, size, sampler)
    {
        m_format.target = GL_TEXTURE_3D;
    }
    ~Texture3D() {
    }

private:
    void generate() {
        glTexImage3D(m_format.target, 0, m_format.internal, GLsizei(m_size.x()), GLsizei(m_size.y()),
                     GLsizei(m_size.z()), 0, m_format.external, m_format.type, 0);
    }
};

} /* namespace gl */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
