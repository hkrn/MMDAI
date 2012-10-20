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
#include "vpvl2/pmd2/Material.h"

namespace
{

using namespace vpvl2::pmd2;

#pragma pack(push, 1)

struct MaterialUnit {
    float diffuse[3];
    float opacity;
    float shininess;
    float specular[3];
    float ambient[3];
    uint8_t toonTextureIndex;
    uint8_t edge;
    int nindices;
    uint8_t textureName[Material::kNameSize];
};

#pragma pack(pop)

}

namespace vpvl2
{
namespace pmd2
{

const int Material::kNameSize;
const Color Material::kWhiteColor(1, 1, 1, 1);

Material::Material(IEncoding *encodingRef)
    : m_encodingRef(encodingRef),
      m_mainTexture(0),
      m_sphereTexture(0),
      m_toonTextureRef(0),
      m_sphereTextureRenderMode(kNone),
      m_ambient(kZeroC),
      m_diffuse(kZeroC),
      m_specular(kZeroC),
      m_edgeColor(kZeroC),
      m_shininess(0),
      m_index(-1),
      m_indices(0),
      m_toonTextureIndex(0),
      m_enableEdge(false)
{
}

Material::~Material()
{
    delete m_mainTexture;
    m_mainTexture = 0;
    delete m_sphereTexture;
    m_sphereTexture = 0;
    m_toonTextureRef = 0;
    m_sphereTextureRenderMode = kNone;
    m_ambient.setZero();
    m_diffuse.setZero();
    m_specular.setZero();
    m_shininess = 0;
    m_index = -1;
    m_indices = 0;
    m_toonTextureIndex = 0;
    m_enableEdge = false;
}

bool Material::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    size_t size;
    if (!internal::size32(ptr, rest, size) || size * sizeof(MaterialUnit) > rest) {
        return false;
    }
    info.materialsCount = size;
    info.materialsPtr = ptr;
    internal::readBytes(size * sizeof(MaterialUnit), ptr, rest);
    return true;
}

bool Material::loadMaterials(const Array<Material *> &materials,
                             const Array<IString *> &textures,
                             int expectedIndices)
{
    const int nmaterials = materials.count();
    const int ntextures = textures.count();
    int actualIndices = 0;
    for (int i = 0; i < nmaterials; i++) {
        Material *material = materials[i];
        const int toonTextureIndex = material->m_toonTextureIndex;
        if (toonTextureIndex >= 0) {
            if (toonTextureIndex >= ntextures) {
                return false;
            }
            else {
                material->m_toonTextureRef = textures[toonTextureIndex];
            }
        }
        material->m_index = i;
        actualIndices += material->indices();
    }
    return actualIndices == expectedIndices;
}

size_t Material::estimateTotalSize(const Array<Material *> &materials, const Model::DataInfo &info)
{
    const int nmaterials = materials.count();
    size_t size = 0;
    for (int i = 0; i < nmaterials; i++) {
        Material *material = materials[i];
        size += material->estimateSize(info);
    }
    return size;
}

void Material::read(const uint8_t *data, const Model::DataInfo & /* info */, size_t &size)
{
    MaterialUnit unit;
    internal::getData(data, unit);
    const IString *separator = m_encodingRef->stringConstant(IEncoding::kAsterisk);
    const IString *sph = m_encodingRef->stringConstant(IEncoding::kSPHExtension);
    const IString *spa = m_encodingRef->stringConstant(IEncoding::kSPAExtension);
    IString *texture = m_encodingRef->toString(unit.textureName, IString::kShiftJIS, kNameSize);
    if (texture->contains(separator)) {
        Array<IString *> tokens;
        texture->split(separator, 2, tokens);
        delete texture;
        IString *mainTexture = tokens[0];
        if (mainTexture->endsWith(sph)) {
            m_sphereTexture = mainTexture;
            m_sphereTextureRenderMode = kMultTexture;
        }
        else {
            m_mainTexture = mainTexture;
        }
        if (tokens.count() == 2) {
            IString *subTexture = tokens[1];
            if (subTexture->endsWith(sph)) {
                m_sphereTexture = subTexture;
                m_sphereTextureRenderMode = kMultTexture;
            }
            else if (subTexture->endsWith(spa)) {
                m_sphereTexture = subTexture;
                m_sphereTextureRenderMode = kAddTexture;
            }
        }
    }
    else if (texture->endsWith(sph)) {
        m_sphereTexture = texture;
        m_sphereTextureRenderMode = kMultTexture;
    }
    else {
        m_mainTexture = texture;
    }
    m_ambient.setValue(unit.ambient[0], unit.ambient[1], unit.ambient[2], 1);
    m_diffuse.setValue(unit.diffuse[0], unit.diffuse[1], unit.diffuse[2], unit.opacity);
    m_specular.setValue(unit.specular[0], unit.specular[1], unit.specular[2], 1);
    m_shininess = unit.shininess;
    m_indices = unit.nindices;
    m_enableEdge = unit.edge != 0;
    int toonTextureIndex = unit.toonTextureIndex;
    m_toonTextureIndex = (toonTextureIndex == 0xff) ? 0 : toonTextureIndex + 1;
    size = sizeof(unit);
}

size_t Material::estimateSize(const Model::DataInfo & /* info */) const
{
    size_t size = 0;
    size += sizeof(MaterialUnit);
    return size;
}

void Material::write(uint8_t *data, const Model::DataInfo & /* info */) const
{
    MaterialUnit unit;
    internal::getPositionRaw(m_ambient, unit.ambient);
    internal::getPositionRaw(m_diffuse, unit.diffuse);
    unit.edge = m_enableEdge ? 1 : 0;
    unit.nindices = m_indices;
    unit.opacity = m_diffuse.w();
    unit.shininess = m_shininess;
    internal::getPositionRaw(m_specular, unit.specular);
    // TODO: texture*sphere
    uint8_t *name = m_encodingRef->toByteArray(m_mainTexture, IString::kShiftJIS);
    internal::copyBytes(unit.textureName, name, sizeof(unit.textureName));
    m_encodingRef->disposeByteArray(name);
    unit.toonTextureIndex = m_toonTextureIndex;
    internal::copyBytes(data, reinterpret_cast<const uint8_t *>(&unit), sizeof(unit));
}

bool Material::isSharedToonTextureUsed() const
{
    return m_toonTextureRef ? false : true;
}

bool Material::isCullFaceDisabled() const
{
    return !btFuzzyZero(m_diffuse.w() - 1);
}

bool Material::hasShadow() const
{
    return true;
}

bool Material::isShadowMapDrawn() const
{
    return !btFuzzyZero(m_diffuse.x() - 0.98f);
}

bool Material::isSelfShadowDrawn() const
{
    return isShadowMapDrawn();
}

bool Material::isEdgeDrawn() const
{
    return m_enableEdge;
}

void Material::setMainTexture(const IString *value)
{
    m_mainTexture = value;
}

void Material::setSphereTexture(const IString *value)
{
    m_sphereTexture = value;
}

void Material::setToonTexture(const IString *value)
{
    m_toonTextureRef = value;
}

void Material::setSphereTextureRenderMode(SphereTextureRenderMode value)
{
    m_sphereTextureRenderMode = value;
}

void Material::setAmbient(const Color &value)
{
    m_ambient = value;
}

void Material::setDiffuse(const Color &value)
{
    m_diffuse = value;
}

void Material::setSpecular(const Color &value)
{
    m_specular = value;
}

void Material::setEdgeColor(const Color &value)
{
    m_edgeColor = value;
}

void Material::setShininess(float value)
{
    m_shininess = value;
}

void Material::setIndices(int value)
{
    m_indices = value;
}

}
}
