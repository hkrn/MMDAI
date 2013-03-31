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

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/util.h"

#include "vpvl2/pmx/Material.h"

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

Material::Material(IModel *modelRef)
    : m_modelRef(modelRef),
      m_name(0),
      m_englishName(0),
      m_userDataArea(0),
      m_mainTextureRef(0),
      m_sphereTextureRef(0),
      m_toonTextureRef(0),
      m_sphereTextureRenderMode(kNone),
      m_shininess(0, 1, 0),
      m_edgeSize(0, 1, 0),
      m_index(-1),
      m_textureIndex(0),
      m_sphereTextureIndex(0),
      m_toonTextureIndex(0),
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
    m_modelRef = 0;
    m_mainTextureRef = 0;
    m_sphereTextureRef = 0;
    m_toonTextureRef = 0;
    m_sphereTextureRenderMode = kNone;
    m_userDataArea = 0;
    m_shininess.setZero();
    m_edgeSize.setZero();
    m_index = -1;
    m_textureIndex = 0;
    m_sphereTextureIndex = 0;
    m_toonTextureIndex = 0;
    m_flags = 0;
    m_useSharedToonTexture = false;
}

bool Material::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    int nmaterials, size, textureIndexSize = info.textureIndexSize;
    if (!internal::getTyped<int>(ptr, rest, nmaterials)) {
        VPVL2_LOG(LOG(ERROR) << "Invalid size of PMX materials detected: size=" << nmaterials << " rest=" << rest);
        return false;
    }
    info.materialsPtr = ptr;
    size_t nTextureIndexSize = textureIndexSize * 2;
    for (int i = 0; i < nmaterials; i++) {
        uint8_t *namePtr;
        /* name in Japanese */
        if (!internal::getText(ptr, rest, namePtr, size)) {
            VPVL2_LOG(LOG(ERROR) << "Invalid size of PMX material name in Japanese detected: index=" << i << " size=" << size << " rest=" << rest);
            return false;
        }
        /* name in English */
        if (!internal::getText(ptr, rest, namePtr, size)) {
            VPVL2_LOG(LOG(ERROR) << "Invalid size of PMX material name in English detected: index=" << i << " size=" << size << " rest=" << rest);
            return false;
        }
        if (!internal::validateSize(ptr, sizeof(MaterialUnit), rest)) {
            VPVL2_LOG(LOG(ERROR) << "Invalid size of PMX material unit detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
            return false;
        }
        /* main texture + sphere map texture */
        if (!internal::validateSize(ptr, nTextureIndexSize, rest)) {
            VPVL2_LOG(LOG(ERROR) << "Invalid size of PMX material texture detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
            return false;
        }
        /* material flags */
        if (sizeof(uint16_t) > rest) {
            VPVL2_LOG(LOG(ERROR) << "Invalid size of PMX material flags detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
            return false;
        }
        bool isSharedToonTexture = *(ptr + sizeof(uint8_t)) == 1;
        internal::drainBytes(sizeof(uint16_t), ptr, rest);
        /* shared toon texture index */
        if (isSharedToonTexture) {
            if (!internal::validateSize(ptr, sizeof(uint8_t), rest)) {
                VPVL2_LOG(LOG(ERROR) << "Invalid size of PMX material shared texture index detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
                return false;
            }
        }
        /* independent toon texture index */
        else {
            if (!internal::validateSize(ptr, textureIndexSize, rest)) {
                VPVL2_LOG(LOG(ERROR) << "Invalid size of PMX material texture index detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
                return false;
            }
        }
        /* free area */
        if (!internal::getText(ptr, rest, namePtr, size)) {
            VPVL2_LOG(LOG(ERROR) << "Invalid size of PMX material user data detected: index=" << i << " size=" << size << " rest=" << rest);
            return false;
        }
        /* number of indices */
        if (!internal::validateSize(ptr, sizeof(int), rest)) {
            VPVL2_LOG(LOG(ERROR) << "Invalid size of PMX material index detected: index=" << i << " size=" << size << " rest=" << rest);
            return false;
        }
    }
    info.materialsCount = nmaterials;
    return true;
}

bool Material::loadMaterials(const Array<Material *> &materials, const Array<IString *> &textures, int expectedIndices)
{
    const int nmaterials = materials.count();
    const int ntextures = textures.count();
    int actualIndices = 0;
    for (int i = 0; i < nmaterials; i++) {
        Material *material = materials[i];
        const int textureIndex = material->m_textureIndex;
        if (textureIndex >= 0) {
            if (textureIndex >= ntextures) {
                VPVL2_LOG(LOG(ERROR) << "Invalid PMX material main texture index detected: index=" << i << " texture=" << textureIndex);
                return false;
            }
            else {
                material->m_mainTextureRef = textures[textureIndex];
            }
        }
        const int sphereTextureIndex = material->m_sphereTextureIndex;
        if (sphereTextureIndex >= 0) {
            if (sphereTextureIndex >= ntextures) {
                VPVL2_LOG(LOG(ERROR) << "Invalid PMX material sphere texture index detected: index=" << i << " texture=" << sphereTextureIndex);
                return false;
            }
            else {
                material->m_sphereTextureRef = textures[sphereTextureIndex];
            }
        }
        const int toonTextureIndex = material->m_toonTextureIndex;
        if (!material->m_useSharedToonTexture && toonTextureIndex >= 0) {
            if (toonTextureIndex >= ntextures) {
                VPVL2_LOG(LOG(ERROR) << "Invalid PMX material toon texture index detected: index=" << i << " texture=" << toonTextureIndex);
                return false;
            }
            else {
                material->m_toonTextureRef = textures[toonTextureIndex];
            }
        }
        material->m_index = i;
        actualIndices += material->indexRange().count;
    }
    return actualIndices == expectedIndices;
}

size_t Material::estimateTotalSize(const Array<Material *> &materials, const Model::DataInfo &info)
{
    const int nmaterials = materials.count();
    size_t size = 0;
    size += sizeof(nmaterials);
    for (int i = 0; i < nmaterials; i++) {
        Material *material = materials[i];
        size += material->estimateSize(info);
    }
    return size;
}

void Material::read(const uint8_t *data, const Model::DataInfo &info, size_t &size)
{
    uint8_t *namePtr, *ptr = const_cast<uint8_t *>(data), *start = ptr;
    size_t rest = SIZE_MAX, textureIndexSize = info.textureIndexSize;
    int nNameSize;
    IEncoding *encoding = info.encoding;
    internal::getText(ptr, rest, namePtr, nNameSize);
    internal::setStringDirect(encoding->toString(namePtr, nNameSize, info.codec), m_name);
    VPVL2_LOG(VLOG(3) << "PMXMaterial: name=" << internal::cstr(m_name));
    internal::getText(ptr, rest, namePtr, nNameSize);
    internal::setStringDirect(encoding->toString(namePtr, nNameSize, info.codec), m_englishName);
    VPVL2_LOG(VLOG(3) << "PMXMaterial: englishName=" << internal::cstr(m_englishName));
    MaterialUnit unit;
    internal::getData(ptr, unit);
    m_ambient.base.setValue(unit.ambient[0], unit.ambient[1], unit.ambient[2]);
    m_ambient.calculate();
    VPVL2_LOG(VLOG(3) << "PMXMaterial: ambient=" << m_ambient.result.x() << "," << m_ambient.result.y() << "," << m_ambient.result.z());
    m_diffuse.base.setValue(unit.diffuse[0], unit.diffuse[1], unit.diffuse[2], unit.diffuse[3]);
    m_diffuse.calculate();
    VPVL2_LOG(VLOG(3) << "PMXMaterial: diffuse=" << m_diffuse.result.x() << "," << m_diffuse.result.y() << "," << m_diffuse.result.z());
    m_specular.base.setValue(unit.specular[0], unit.specular[1], unit.specular[2]);
    m_specular.calculate();
    VPVL2_LOG(VLOG(3) << "PMXMaterial: specular=" << m_specular.result.x() << "," << m_specular.result.y() << "," << m_specular.result.z());
    m_edgeColor.base.setValue(unit.edgeColor[0], unit.edgeColor[1], unit.edgeColor[2], unit.edgeColor[3]);
    m_edgeColor.calculate();
    VPVL2_LOG(VLOG(3) << "PMXMaterial: edgeColor=" << m_edgeColor.result.x() << "," << m_edgeColor.result.y() << "," << m_edgeColor.result.z());
    m_shininess.setX(unit.shininess);
    VPVL2_LOG(VLOG(3) << "PMXMaterial: shininess=" << m_shininess.x());
    m_edgeSize.setX(unit.edgeSize);
    VPVL2_LOG(VLOG(3) << "PMXMaterial: edgeSize=" << m_edgeSize.x());
    m_mainTextureBlend.base.setValue(1, 1, 1, 1);
    m_mainTextureBlend.calculate();
    m_sphereTextureBlend.base.setValue(1, 1, 1, 1);
    m_sphereTextureBlend.calculate();
    m_toonTextureBlend.base.setValue(1, 1, 1, 1);
    m_toonTextureBlend.calculate();
    m_flags = unit.flags;
    ptr += sizeof(unit);
    m_textureIndex = internal::readSignedIndex(ptr, textureIndexSize);
    VPVL2_LOG(VLOG(3) << "PMXMaterial: mainTextureIndex=" << m_textureIndex);
    m_sphereTextureIndex = internal::readSignedIndex(ptr, textureIndexSize);
    VPVL2_LOG(VLOG(3) << "PMXMaterial: sphereTextureIndex=" << m_sphereTextureIndex);
    uint8_t type;
    internal::getTyped<uint8_t>(ptr, rest, type);
    m_sphereTextureRenderMode = static_cast<SphereTextureRenderMode>(type);
    internal::getTyped<uint8_t>(ptr, rest, type);
    m_useSharedToonTexture = type == 1;
    VPVL2_LOG(VLOG(3) << "PMXMaterial: useSharedToonTexture=" << m_useSharedToonTexture);
    if (m_useSharedToonTexture) {
        internal::getTyped<uint8_t>(ptr, rest, type);
        m_toonTextureIndex = type;
        VPVL2_LOG(VLOG(3) << "PMXMaterial: sharedToonTextureIndex=" << m_toonTextureIndex);
    }
    else {
        m_toonTextureIndex = internal::readSignedIndex(ptr, textureIndexSize);
        VPVL2_LOG(VLOG(3) << "PMXMaterial: toonTextureIndex=" << m_toonTextureIndex);
    }
    internal::getText(ptr, rest, namePtr, nNameSize);
    internal::setStringDirect(encoding->toString(namePtr, nNameSize, info.codec), m_userDataArea);
    internal::getTyped<int>(ptr, rest, nNameSize);
    m_indexRange.count = nNameSize;
    VPVL2_LOG(VLOG(3) << "PMXMaterial: indexCount=" << m_indexRange.count);
    size = ptr - start;
}

void Material::write(uint8_t *data, const Model::DataInfo &info) const
{
    internal::writeString(m_name, info.codec, data);
    internal::writeString(m_englishName, info.codec, data);
    MaterialUnit mu;
    internal::getColor(m_ambient.base, mu.ambient);
    internal::getColor(m_diffuse.base, mu.diffuse);
    internal::getColor(m_specular.base, mu.specular);
    internal::getColor(m_edgeColor.base, mu.edgeColor);
    mu.shininess = m_shininess.x();
    mu.edgeSize = m_edgeSize.x();
    mu.flags = m_flags;
    internal::writeBytes(reinterpret_cast<const uint8_t *>(&mu), sizeof(mu), data);
    size_t textureIndexSize = info.textureIndexSize;
    internal::writeSignedIndex(m_textureIndex, textureIndexSize, data);
    internal::writeSignedIndex(m_sphereTextureIndex, textureIndexSize, data);
    internal::writeBytes(reinterpret_cast<const uint8_t *>(&m_sphereTextureRenderMode), sizeof(uint8_t), data);
    internal::writeBytes(reinterpret_cast<const uint8_t *>(&m_useSharedToonTexture), sizeof(uint8_t), data);
    if (m_useSharedToonTexture) {
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&m_toonTextureIndex), sizeof(uint8_t), data);
    }
    else {
        internal::writeSignedIndex(m_toonTextureIndex, textureIndexSize, data);
    }
    internal::writeString(m_userDataArea, info.codec, data);
    internal::writeBytes(reinterpret_cast<const uint8_t *>(&m_indexRange.count), sizeof(int), data);
}

size_t Material::estimateSize(const Model::DataInfo &info) const
{
    size_t size = 0, textureIndexSize = info.textureIndexSize;
    size += internal::estimateSize(m_name, info.codec);
    size += internal::estimateSize(m_englishName, info.codec);
    size += sizeof(MaterialUnit);
    size += textureIndexSize * 2;
    size += sizeof(uint16_t);
    size += m_useSharedToonTexture ? sizeof(uint8_t) : textureIndexSize;
    size += internal::estimateSize(m_userDataArea, info.codec);
    size += sizeof(int);
    return size;
}

void Material::mergeMorph(const Morph::Material *morph, const IMorph::WeightPrecision &weight)
{
    Scalar w = Scalar(weight);
    btClamp(w, 0.0f, 1.0f);
    if (btFuzzyZero(w)) {
        resetMorph();
    }
    else {
        switch (morph->operation) {
        case 0: { // modulate
            m_ambient.calculateMulWeight(morph->ambient, w);
            m_diffuse.calculateMulWeight(morph->diffuse, w);
            m_specular.calculateMulWeight(morph->specular, w);
            m_shininess.setY(1.0f - (1.0f - morph->shininess) * w);
            m_edgeColor.calculateMulWeight(morph->edgeColor, w);
            m_edgeSize.setY(1.0f - (1.0f - morph->edgeSize) * w);
            m_mainTextureBlend.calculateMulWeight(morph->textureWeight, w);
            m_sphereTextureBlend.calculateMulWeight(morph->sphereTextureWeight, w);
            m_toonTextureBlend.calculateMulWeight(morph->toonTextureWeight, w);
            break;
        }
        case 1: { // add
            m_ambient.calculateAddWeight(morph->ambient, w);
            m_diffuse.calculateAddWeight(morph->diffuse, w);
            m_specular.calculateAddWeight(morph->specular, w);
            m_shininess.setZ(morph->shininess * w);
            m_edgeColor.calculateAddWeight(morph->edgeColor, w);
            m_edgeSize.setZ(morph->edgeSize * w);
            m_mainTextureBlend.calculateAddWeight(morph->textureWeight, w);
            m_sphereTextureBlend.calculateAddWeight(morph->sphereTextureWeight, w);
            m_toonTextureBlend.calculateAddWeight(morph->toonTextureWeight, w);
            break;
        }
        default:
            break;
        }
        m_ambient.calculate();
        m_diffuse.calculate();
        m_specular.calculate();
        m_edgeColor.calculate();
        m_mainTextureBlend.calculate();
        m_sphereTextureBlend.calculate();
        m_toonTextureBlend.calculate();
    }
}

void Material::resetMorph()
{
    m_ambient.reset();
    m_diffuse.reset();
    m_specular.reset();
    m_shininess.setValue(m_shininess.x(), 1, 0);
    m_edgeColor.reset();
    m_edgeSize.setValue(m_edgeSize.x(), 1, 0);
    m_mainTextureBlend.reset();
    m_sphereTextureBlend.reset();
    m_toonTextureBlend.reset();
}

bool Material::isCullFaceDisabled() const
{
    return internal::hasFlagBits(m_flags, 0x01);
}
bool Material::hasShadow() const
{
    return internal::hasFlagBits(m_flags, 0x02) && !isPointDraw();
}

bool Material::isShadowMapDrawn() const
{
    return internal::hasFlagBits(m_flags, 0x04) && !isPointDraw();
}

bool Material::isSelfShadowDrawn() const
{
    return internal::hasFlagBits(m_flags, 0x08) && !isPointDraw();
}

bool Material::isEdgeDrawn() const
{
    return internal::hasFlagBits(m_flags, 0x10) && !isPointDraw() && !isLineDraw();
}

bool Material::hasVertexColor() const
{
    return internal::hasFlagBits(m_flags, 0x20);
}

bool Material::isPointDraw() const
{
    return internal::hasFlagBits(m_flags, 0x40);
}

bool Material::isLineDraw() const
{
    return internal::hasFlagBits(m_flags, 0x80);
}

void Material::setName(const IString *value)
{
    internal::setString(value, m_name);
}

void Material::setEnglishName(const IString *value)
{
    internal::setString(value, m_englishName);
}

void Material::setUserDataArea(const IString *value)
{
    internal::setString(value, m_userDataArea);
}

void Material::setMainTexture(const IString *value)
{
    internal::setString(value, m_mainTextureRef);
}

void Material::setSphereTexture(const IString *value)
{
    internal::setString(value, m_sphereTextureRef);
}

void Material::setToonTexture(const IString *value)
{
    internal::setString(value, m_toonTextureRef);
}

void Material::setSphereTextureRenderMode(SphereTextureRenderMode value)
{
    m_sphereTextureRenderMode = value;
}

void Material::setAmbient(const Color &value)
{
    m_ambient.base = value;
    m_ambient.base.setW(1);
    m_ambient.calculate();
}

void Material::setDiffuse(const Color &value)
{
    m_diffuse.base = value;
    m_diffuse.calculate();
}

void Material::setSpecular(const Color &value)
{
    m_specular.base = value;
    m_specular.base.setW(1);
    m_specular.calculate();
}

void Material::setEdgeColor(const Color &value)
{
    m_edgeColor.base = value;
    m_edgeColor.calculate();
}

void Material::setIndexRange(const IndexRange &value)
{
    m_indexRange = value;
}

void Material::setShininess(float value)
{
    m_shininess.setX(value);
}

void Material::setEdgeSize(float value)
{
    m_edgeSize.setX(value);
}

void Material::setMainTextureIndex(int value)
{
    m_textureIndex = value;
}

void Material::setSphereTextureIndex(int value)
{
    m_sphereTextureIndex = value;
}

void Material::setToonTextureIndex(int value)
{
    m_toonTextureIndex = value;
}

void Material::setFlags(int value)
{
    m_flags = value;
}

} /* namespace pmx */
} /* namespace vpvl2 */

