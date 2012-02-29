/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
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

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/util.h"

namespace
{

#pragma pack(push, 1)

struct MaterialUnit {
    float diffuse[4];
    float specular[3];
    float shininess;
    float ambient[3];
    uint8_t flags;
    float edgeColor[4];
    float edgeSize;
};

#pragma pack(pop)

}

namespace vpvl2
{
namespace pmx
{

Material::Material()
    : m_name(0),
      m_englishName(0),
      m_userDataArea(0),
      m_mainTexture(0),
      m_sphereTexture(0),
      m_toonTexture(0),
      m_sphereTextureRenderMode(kNone),
      m_ambient(kZeroC),
      m_diffuse(kZeroC),
      m_specular(kZeroC),
      m_shininess(0),
      m_edgeSize(0),
      m_textureIndex(0),
      m_sphereTextureIndex(0),
      m_toonTextureIndex(0),
      m_indices(0),
      m_flags(0),
      m_useSharedToonTexture(false)
{
}

Material::~Material()
{
    delete m_name;
    m_name = 0;
    delete m_englishName;
    m_englishName = 0;
    delete m_userDataArea;
    m_userDataArea = 0;
    m_mainTexture = 0;
    m_sphereTexture = 0;
    m_toonTexture = 0;
    m_sphereTextureRenderMode = kNone;
    m_ambient.setZero();
    m_diffuse.setZero();
    m_specular.setZero();
    m_userDataArea = 0;
    m_shininess = 0;
    m_edgeSize = 0;
    m_textureIndex = 0;
    m_sphereTextureIndex = 0;
    m_toonTextureIndex = 0;
    m_indices = 0;
    m_flags = 0;
    m_useSharedToonTexture = false;
}

bool Material::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    size_t size;
    if (!internal::size32(ptr, rest, size)) {
        return false;
    }
    info.materialsPtr = ptr;
    size_t nTextureIndexSize = info.textureIndexSize * 2;
    for (size_t i = 0; i < size; i++) {
        size_t nNameSize;
        uint8_t *namePtr;
        /* name in Japanese */
        if (!internal::sizeText(ptr, rest, namePtr, nNameSize)) {
            return false;
        }
        /* name in English */
        if (!internal::sizeText(ptr, rest, namePtr, nNameSize)) {
            return false;
        }
        if (!internal::validateSize(ptr, sizeof(MaterialUnit), rest)) {
            return false;
        }
        /* normal texture + sphere map texture */
        if (!internal::validateSize(ptr, nTextureIndexSize, rest)) {
            return false;
        }
        if (sizeof(uint16_t) > rest) {
            return false;
        }
        bool isSharedToonTexture = *(ptr + sizeof(uint8_t)) == 1;
        internal::drain(sizeof(uint16_t), ptr, rest);
        /* shared toon texture index */
        if (isSharedToonTexture) {
            if (!internal::validateSize(ptr, sizeof(uint8_t), rest)) {
                return false;
            }
        }
        /* independent toon texture index */
        else {
            if (!internal::validateSize(ptr, info.textureIndexSize, rest)) {
                return false;
            }
        }
        /* free area */
        if (!internal::sizeText(ptr, rest, namePtr, nNameSize)) {
            return false;
        }
        /* number of indices */
        if (!internal::validateSize(ptr, sizeof(int), rest)) {
            return false;
        }
    }
    info.materialsCount = size;
    return true;
}

bool Material::loadMaterials(const Array<Material *> &materials, const Array<StaticString *> &textures, int expectedIndices)
{
    const int nmaterials = materials.count();
    const int ntextures = textures.count();
    int actualIndices = 0;
    for (int i = 0; i < nmaterials; i++) {
        Material *material = materials[i];
        const int textureIndex = material->m_textureIndex;
        if (textureIndex >= 0) {
            if (textureIndex >= ntextures)
                return false;
            else
                material->m_mainTexture = textures[textureIndex];
        }
        const int sphereTextureIndex = material->m_sphereTextureIndex;
        if (sphereTextureIndex >= 0) {
            if (sphereTextureIndex >= ntextures)
                return false;
            else
                material->m_sphereTexture = textures[sphereTextureIndex];
        }
        const int toonTextureIndex = material->m_toonTextureIndex;
        if (!material->m_useSharedToonTexture && toonTextureIndex >= 0) {
            if (toonTextureIndex >= ntextures)
                return false;
            else
                material->m_toonTexture = textures[toonTextureIndex];
        }
        actualIndices += material->indices();
    }
    return actualIndices == expectedIndices;
}

void Material::read(const uint8_t *data, const Model::DataInfo &info, size_t &size)
{
    uint8_t *namePtr, *ptr = const_cast<uint8_t *>(data), *start = ptr;
    size_t nNameSize, rest = SIZE_MAX;
    internal::sizeText(ptr, rest, namePtr, nNameSize);
    StaticString::Encoding encoding = info.encoding;
    m_name = new StaticString(namePtr, nNameSize, encoding);
    internal::sizeText(ptr, rest, namePtr, nNameSize);
    m_englishName = new StaticString(namePtr, nNameSize, encoding);
    const MaterialUnit &unit = *reinterpret_cast<MaterialUnit *>(ptr);
    m_ambient.setValue(unit.ambient[0], unit.ambient[1], unit.ambient[2], 0);
    m_diffuse.setValue(unit.diffuse[0], unit.diffuse[1], unit.diffuse[2], unit.diffuse[3]);
    m_specular.setValue(unit.specular[0], unit.specular[1], unit.specular[2], 0);
    m_edgeColor.setValue(unit.edgeColor[0], unit.edgeColor[1], unit.edgeColor[2], unit.edgeColor[3]);
    m_shininess = unit.shininess;
    m_edgeSize = unit.edgeSize;
    m_flags = unit.flags;
    ptr += sizeof(unit);
    m_textureIndex = internal::variantIndex(ptr, info.textureIndexSize);
    m_sphereTextureIndex = internal::variantIndex(ptr, info.textureIndexSize);
    internal::size8(ptr, rest, nNameSize);
    m_sphereTextureRenderMode = static_cast<SphereTextureRenderMode>(nNameSize);
    internal::size8(ptr, rest, nNameSize);
    m_useSharedToonTexture = nNameSize == 1;
    if (m_useSharedToonTexture) {
        internal::size8(ptr, rest, nNameSize);
        m_toonTextureIndex = nNameSize;
    }
    else {
        m_toonTextureIndex = internal::variantIndex(ptr, info.textureIndexSize);
    }
    internal::sizeText(ptr, rest, namePtr, nNameSize);
    m_userDataArea = new StaticString(namePtr, nNameSize, info.encoding);
    internal::size32(ptr, rest, nNameSize);
    m_indices = nNameSize;
    size = ptr - start;
}

void Material::write(uint8_t *data) const
{
}

void Material::mergeMorph(Morph::Material *morph, float weight)
{
    // TODO: morph->operation
    m_ambient += morph->ambient * weight;
    m_diffuse += morph->diffuse * weight;
    m_specular += morph->specular * weight;
    m_shininess += morph->shininess * weight;
    m_edgeColor += morph->edgeColor * weight;
    m_edgeSize += morph->edgeSize * weight;
}

} /* namespace pmx */
} /* namespace vpvl2 */

