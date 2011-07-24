/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn                                    */
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

#ifndef VPVL_XMATERIAL_H_
#define VPVL_XMATERIAL_H_

#include <LinearMath/btAlignedObjectArray.h>
#include <LinearMath/btVector3.h>

#include "vpvl/common.h"

namespace vpvl
{

class VPVL_EXPORT XMaterial {
public:
    XMaterial()
        : m_color(0.0f, 0.0f, 0.0f, 0.0f),
          m_specular(0.0f, 0.0f, 0.0f, 0.0f),
          m_emmisive(0.0f, 0.0f, 0.0f, 0.0f),
          m_textureName(0),
          m_power(0.0f)
    {
    }
    XMaterial(const btVector4 &color,
              const btVector4 &specular,
              btVector4 &emmisive,
              char *textureName,
              float power)
        : m_color(color),
          m_specular(specular),
          m_emmisive(emmisive),
          m_textureName(0),
          m_power(power)
    {
        if (textureName) {
            size_t len = strlen(textureName);
            m_textureName = new char[len + 1];
            strcpy(m_textureName, textureName);
        }
    }
    ~XMaterial() {
        m_color.setZero();
        m_specular.setZero();
        m_emmisive.setZero();
        delete[] m_textureName;
        m_textureName = 0;
        m_power = 0;
    }

    const btVector4 &color() const { return m_color; }
    const btVector4 &specular() const { return m_specular; }
    const btVector4 &emmisive() const { return m_emmisive; }
    const char *textureName() const { return m_textureName; }
    float power() const { return m_power; }

private:
    btVector4 m_color;
    btVector4 m_specular;
    btVector4 m_emmisive;
    char *m_textureName;
    float m_power;

    VPVL_DISABLE_COPY_AND_ASSIGN(XMaterial)
};

}

#endif
