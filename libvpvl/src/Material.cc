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

#pragma pack(push, 1)

struct MaterialChunk
{
    float diffuse[3];
    float alpha;
    float shiness;
    float specular[3];
    float ambient[3];
    uint8_t toonID;
    uint8_t edge;
    int nindices;
    uint8_t textureName[Material::kNameSize];
};

#pragma pack(pop)

size_t Material::stride()
{
    return sizeof(MaterialChunk);
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
      m_mainSphereModulate(false),
      m_mainSphereAdd(false),
      m_subSphereModulate(false),
      m_subSphereAdd(false)
{
    internal::zerofill(m_mainTextureName, sizeof(m_mainTextureName));
    internal::zerofill(m_subTextureName, sizeof(m_subTextureName));
}

Material::~Material()
{
    internal::zerofill(m_mainTextureName, sizeof(m_mainTextureName));
    internal::zerofill(m_subTextureName, sizeof(m_subTextureName));
    m_ambient.setZero();
    m_averageColor.setZero();
    m_diffuse.setZero();
    m_specular.setZero();
    m_opacity = 0.0f;
    m_shiness = 0.0f;
    m_nindices = 0;
    m_toonID = 0;
    m_edge = false;
    m_mainSphereModulate = false;
    m_mainSphereAdd = false;
    m_subSphereModulate = false;
    m_subSphereAdd = false;
}

void Material::read(const uint8_t *data)
{
    MaterialChunk chunk;
    internal::copyBytes(reinterpret_cast<uint8_t *>(&chunk), data, sizeof(chunk));
    float *diffuse = chunk.diffuse;
    float alpha = chunk.alpha;
    float shiness = chunk.shiness;
    float *specular = chunk.specular;
    float *ambient = chunk.ambient;
    uint8_t toonID = chunk.toonID;
    uint8_t edge = chunk.edge;
    int nindices = chunk.nindices;
    uint8_t name[20], *p;
    copyBytesSafe(name, chunk.textureName, sizeof(name));
    copyBytesSafe(m_rawName, chunk.textureName, sizeof(m_rawName));

    // If asterisk is included in the path, we should load two textures
    if ((p = static_cast<uint8_t *>(memchr(name, '*', sizeof(name)))) != NULL) {
        *p = 0;
        copyBytesSafe(m_mainTextureName, name, sizeof(m_mainTextureName));
        copyBytesSafe(m_subTextureName, p + 1, sizeof(m_subTextureName));
        m_mainSphereModulate = strstr(reinterpret_cast<const char *>(m_mainTextureName), ".sph") != NULL;
        m_mainSphereAdd = strstr(reinterpret_cast<const char *>(m_mainTextureName), ".spa") != NULL;
        m_subSphereModulate = strstr(reinterpret_cast<const char *>(m_subTextureName), ".sph") != NULL;
        m_subSphereAdd = strstr(reinterpret_cast<const char *>(m_subTextureName), ".spa") != NULL;
    }
    else {
        copyBytesSafe(m_mainTextureName, name, sizeof(m_mainTextureName));
        m_mainSphereModulate = strstr(reinterpret_cast<const char *>(m_mainTextureName), ".sph") != NULL;
        m_mainSphereAdd = strstr(reinterpret_cast<const char *>(m_mainTextureName), ".spa") != NULL;
    }

    m_ambient.setValue(ambient[0], ambient[1], ambient[2], 1.0f);
    m_diffuse.setValue(diffuse[0], diffuse[1], diffuse[2], 1.0f);
    m_specular.setValue(specular[0], specular[1], specular[2], 1.0f);
    Vector3 ac((m_diffuse + m_ambient) * 0.5f);
    m_averageColor.setValue(ac.x(), ac.y(), ac.z(), 1.0f);
    m_opacity = alpha;
    m_shiness = shiness;
    m_toonID = toonID == 0xff ? 0 : toonID + 1;
    m_edge = edge ? true : false;
    m_nindices = nindices;
}

void Material::write(uint8_t *data) const
{
    MaterialChunk chunk;
    copyBytesSafe(chunk.textureName, m_rawName, sizeof(chunk.textureName));
    chunk.diffuse[0] = m_diffuse.x();
    chunk.diffuse[1] = m_diffuse.y();
    chunk.diffuse[2] = m_diffuse.z();
    chunk.alpha = m_opacity;
    chunk.shiness = m_shiness;
    chunk.specular[0] = m_specular.x();
    chunk.specular[1] = m_specular.y();
    chunk.specular[2] = m_specular.z();
    chunk.ambient[0] = m_ambient.x();
    chunk.ambient[1] = m_ambient.y();
    chunk.ambient[2] = m_ambient.z();
    chunk.toonID = m_toonID == 0 ? 0xff : m_toonID - 1;
    chunk.edge = m_edge;
    chunk.nindices = m_nindices;
    internal::copyBytes(data, reinterpret_cast<const uint8_t *>(&chunk), sizeof(chunk));
}

}
