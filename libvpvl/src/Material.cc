/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn                                    */
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

#include "vpvl/vpvl.h"
#include "vpvl/internal/util.h"

namespace vpvl
{

size_t Material::stride()
{
    return sizeof(float) * 11 + (sizeof(uint8_t) * 2) + sizeof(uint32_t) + kNameSize;
}

Material::Material()
    : m_ambient(0.0f, 0.0f, 0.0f, 1.0f),
      m_averageColor(0.0f, 0.0f, 0.0f, 1.0f),
      m_diffuse(0.0f, 0.0f, 0.0f, 1.0f),
      m_specular(0.0f, 0.0f, 0.0f, 1.0f),
      m_opacity(0.0f),
      m_shiness(0.0f),
      m_nindices(0),
      m_toonID(0),
      m_edge(false),
      m_firstSPH(false),
      m_firstSPA(false),
      m_secondSPH(false),
      m_secondSPA(false)
{
    internal::zerofill(m_primaryTextureName, sizeof(m_primaryTextureName));
    internal::zerofill(m_secondTextureName, sizeof(m_secondTextureName));
}

Material::~Material()
{
    internal::zerofill(m_primaryTextureName, sizeof(m_primaryTextureName));
    internal::zerofill(m_secondTextureName, sizeof(m_secondTextureName));
    m_ambient.setZero();
    m_averageColor.setZero();
    m_diffuse.setZero();
    m_specular.setZero();
    m_opacity = 0.0f;
    m_shiness = 0.0f;
    m_nindices = 0;
    m_toonID = 0;
    m_edge = false;
    m_firstSPH = false;
    m_firstSPA = false;
    m_secondSPH = false;
    m_secondSPA = false;
}

void Material::read(const uint8_t *data)
{
    uint8_t *ptr = const_cast<uint8_t *>(data);
    float ambient[3], diffuse[3], specular[3];
    internal::vector3(ptr, diffuse);
    float alpha = *reinterpret_cast<float *>(ptr);
    ptr += sizeof(float);
    float shiness = *reinterpret_cast<float *>(ptr);
    ptr += sizeof(float);
    internal::vector3(ptr, specular);
    internal::vector3(ptr, ambient);
    uint8_t toonID = *reinterpret_cast<uint8_t *>(ptr);
    ptr += sizeof(uint8_t);
    uint8_t edge = *reinterpret_cast<uint8_t *>(ptr);
    ptr += sizeof(uint8_t);
    uint32_t nindices = *reinterpret_cast<uint32_t *>(ptr);
    ptr += sizeof(uint32_t);
    uint8_t name[20], *p;
    copyBytesSafe(name, ptr, sizeof(name));
    copyBytesSafe(m_rawName, ptr, sizeof(m_rawName));

    // If asterisk is included in the path, we should load two textures
    if ((p = static_cast<uint8_t *>(memchr(name, '*', sizeof(name)))) != NULL) {
        *p = 0;
        copyBytesSafe(m_primaryTextureName, name, sizeof(m_primaryTextureName));
        copyBytesSafe(m_secondTextureName, p + 1, sizeof(m_secondTextureName));
        m_firstSPH = strstr(reinterpret_cast<const char *>(m_primaryTextureName), ".sph") != NULL;
        m_firstSPA = strstr(reinterpret_cast<const char *>(m_primaryTextureName), ".spa") != NULL;
        m_secondSPH = strstr(reinterpret_cast<const char *>(m_secondTextureName), ".sph") != NULL;
        m_secondSPA = strstr(reinterpret_cast<const char *>(m_secondTextureName), ".spa") != NULL;
    }
    else {
        copyBytesSafe(m_primaryTextureName, name, sizeof(m_primaryTextureName));
        m_firstSPH = strstr(reinterpret_cast<const char *>(m_primaryTextureName), ".sph") != NULL;
        m_firstSPA = strstr(reinterpret_cast<const char *>(m_primaryTextureName), ".spa") != NULL;
    }

    m_ambient.setValue(ambient[0], ambient[1], ambient[2], 1.0f);
    m_diffuse.setValue(diffuse[0], diffuse[1], diffuse[2], 1.0f);
    m_specular.setValue(specular[0], specular[1], specular[2], 1.0f);
    btVector3 ac((m_diffuse + m_ambient) * 0.5f);
    m_averageColor.setValue(ac.x(), ac.y(), ac.z(), 1.0f);
    m_opacity = alpha;
    m_shiness = shiness;
    m_toonID = toonID == 0xff ? 0 : toonID + 1;
    m_edge = edge ? true : false;
    m_nindices = nindices;
}

}
