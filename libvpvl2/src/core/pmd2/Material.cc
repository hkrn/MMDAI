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
#include "vpvl2/pmd2/Material.h"

namespace
{

using namespace vpvl2::pmd2;

#pragma pack(push, 1)

struct MaterialUnit {
    vpvl2::float32_t diffuse[3];
    vpvl2::float32_t opacity;
    vpvl2::float32_t shininess;
    vpvl2::float32_t specular[3];
    vpvl2::float32_t ambient[3];
    vpvl2::uint8_t toonTextureIndex;
    vpvl2::uint8_t edge;
    vpvl2::int32_t nindices;
    vpvl2::uint8_t textureName[Material::kNameSize];
};

#pragma pack(pop)

}

namespace vpvl2
{
namespace pmd2
{

const int Material::kNameSize;
static const Color kWhiteColor = Color(1, 1, 1, 1);

struct Material::PrivateContext {
    PrivateContext(IModel *parentModelRef, IEncoding *encodingRef)
        : parentModelRef(parentModelRef),
          encodingRef(encodingRef),
          mainTexture(0),
          sphereTexture(0),
          toonTextureRef(0),
          sphereTextureRenderMode(kNone),
          ambient(kZeroC),
          diffuse(kZeroC),
          specular(kZeroC),
          edgeColor(kZeroC),
          shininess(0),
          index(-1),
          toonTextureIndex(0),
          enableEdge(false)
    {
    }
    ~PrivateContext() {
        delete mainTexture;
        mainTexture = 0;
        delete sphereTexture;
        sphereTexture = 0;
        toonTextureRef = 0;
        sphereTextureRenderMode = kNone;
        ambient.setZero();
        diffuse.setZero();
        specular.setZero();
        shininess = 0;
        index = -1;
        toonTextureIndex = 0;
        enableEdge = false;
    }
    IModel *parentModelRef;
    IEncoding *encodingRef;
    IString *mainTexture;
    IString *sphereTexture;
    const IString *toonTextureRef;
    SphereTextureRenderMode sphereTextureRenderMode;
    Color ambient;
    Color diffuse;
    Color specular;
    Color edgeColor;
    IndexRange indexRange;
    float shininess;
    int index;
    int toonTextureIndex;
    bool enableEdge;
};

Material::Material(IModel *parentModelRef, IEncoding *encodingRef)
    : m_context(0)
{
    m_context = new PrivateContext(parentModelRef, encodingRef);
}

Material::~Material()
{
    delete m_context;
    m_context = 0;
}

bool Material::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    int32_t size;
    if (!internal::getTyped<int32_t>(ptr, rest, size) || size * sizeof(MaterialUnit) > rest) {
        return false;
    }
    info.materialsCount = size;
    info.materialsPtr = ptr;
    internal::drainBytes(size * sizeof(MaterialUnit), ptr, rest);
    return true;
}

bool Material::loadMaterials(const PointerArray<Material> &materials,
                             const PointerArray<IString> &textures,
                             int expectedIndices)
{
    const int nmaterials = materials.count();
    const int ntextures = textures.count();
    int actualIndices = 0;
    for (int i = 0; i < nmaterials; i++) {
        Material *material = materials[i];
        const int toonTextureIndex = material->m_context->toonTextureIndex;
        if (toonTextureIndex >= 0) {
            if (toonTextureIndex >= ntextures) {
                return false;
            }
            else {
                material->m_context->toonTextureRef = textures[toonTextureIndex];
            }
        }
        material->m_context->index = i;
        actualIndices += material->indexRange().count;
    }
    return actualIndices == expectedIndices;
}

void Material::writeMaterials(const Array<Material *> &materials, const Model::DataInfo &info, uint8_t *&data)
{
    const int32_t nmaterials = materials.count();
    internal::writeBytes(&nmaterials, sizeof(nmaterials), data);
    for (int i = 0; i < nmaterials; i++) {
        Material *material = materials[i];
        material->write(data, info);
    }
}

size_t Material::estimateTotalSize(const Array<Material *> &materials, const Model::DataInfo &info)
{
    const int32_t nmaterials = materials.count();
    size_t size = sizeof(nmaterials);
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
    const IString *separator = m_context->encodingRef->stringConstant(IEncoding::kAsterisk);
    const IString *sph = m_context->encodingRef->stringConstant(IEncoding::kSPHExtension);
    const IString *spa = m_context->encodingRef->stringConstant(IEncoding::kSPAExtension);
    IString *texture = m_context->encodingRef->toString(unit.textureName, IString::kShiftJIS, kNameSize);
    if (texture->contains(separator)) {
        Array<IString *> tokens;
        texture->split(separator, 2, tokens);
        delete texture;
        IString *mainTexture = tokens[0];
        if (mainTexture->endsWith(sph)) {
            m_context->sphereTexture = mainTexture;
            m_context->sphereTextureRenderMode = kMultTexture;
        }
        else {
            m_context->mainTexture = mainTexture;
        }
        if (tokens.count() == 2) {
            IString *subTexture = tokens[1];
            if (subTexture->endsWith(sph)) {
                m_context->sphereTexture = subTexture;
                m_context->sphereTextureRenderMode = kMultTexture;
            }
            else if (subTexture->endsWith(spa)) {
                m_context->sphereTexture = subTexture;
                m_context->sphereTextureRenderMode = kAddTexture;
            }
        }
    }
    else if (texture->endsWith(sph)) {
        m_context->sphereTexture = texture;
        m_context->sphereTextureRenderMode = kMultTexture;
    }
    else {
        m_context->mainTexture = texture;
    }
    m_context->ambient.setValue(unit.ambient[0], unit.ambient[1], unit.ambient[2], 1);
    m_context->diffuse.setValue(unit.diffuse[0], unit.diffuse[1], unit.diffuse[2], unit.opacity);
    m_context->specular.setValue(unit.specular[0], unit.specular[1], unit.specular[2], 1);
    m_context->shininess = unit.shininess;
    m_context->indexRange.count = unit.nindices;
    m_context->enableEdge = unit.edge != 0;
    int toonTextureIndex = unit.toonTextureIndex;
    m_context->toonTextureIndex = (toonTextureIndex == 0xff) ? 0 : toonTextureIndex + 1;
    size = sizeof(unit);
}

size_t Material::estimateSize(const Model::DataInfo & /* info */) const
{
    size_t size = 0;
    size += sizeof(MaterialUnit);
    return size;
}

void Material::write(uint8_t *&data, const Model::DataInfo & /* info */) const
{
    MaterialUnit unit;
    internal::getPositionRaw(m_context->ambient, unit.ambient);
    internal::getPositionRaw(m_context->diffuse, unit.diffuse);
    unit.edge = m_context->enableEdge ? 1 : 0;
    unit.nindices = m_context->indexRange.count;
    unit.opacity = m_context->diffuse.w();
    unit.shininess = m_context->shininess;
    internal::getPositionRaw(m_context->specular, unit.specular);
    if (m_context->mainTexture && m_context->sphereTexture) {
        const IString *separator = m_context->encodingRef->stringConstant(IEncoding::kAsterisk);
        Array<IString *> textures;
        textures.append(m_context->mainTexture);
        textures.append(m_context->sphereTexture);
        IString *s = separator->join(textures);
        uint8_t *textureNamePtr = unit.textureName;
        internal::writeStringAsByteArray(s, IString::kShiftJIS, m_context->encodingRef, sizeof(unit.textureName), textureNamePtr);
        delete s;
    }
    else if (!m_context->mainTexture && m_context->sphereTexture) {
        uint8_t *textureNamePtr = unit.textureName;
        internal::writeStringAsByteArray(m_context->sphereTexture, IString::kShiftJIS, m_context->encodingRef, sizeof(unit.textureName), textureNamePtr);
    }
    else {
        uint8_t *textureNamePtr = unit.textureName;
        internal::writeStringAsByteArray(m_context->mainTexture, IString::kShiftJIS, m_context->encodingRef, sizeof(unit.textureName), textureNamePtr);
    }
    unit.toonTextureIndex = (m_context->toonTextureIndex == 0) ? 0xff : m_context->toonTextureIndex;
    internal::writeBytes(&unit, sizeof(unit), data);
}

IModel *Material::parentModelRef() const
{
    return m_context->parentModelRef;
}

const IString *Material::name() const
{
    return 0;
}

const IString *Material::englishName() const
{
    return 0;
}

const IString *Material::userDataArea() const
{
    return 0;
}

const IString *Material::mainTexture() const
{
    return m_context->mainTexture;
}

const IString *Material::sphereTexture() const
{
    return m_context->sphereTexture;
}

const IString *Material::toonTexture() const
{
    return m_context->toonTextureRef;
}

IMaterial::SphereTextureRenderMode Material::sphereTextureRenderMode() const
{
    return m_context->sphereTextureRenderMode;
}

Color Material::ambient() const
{
    return m_context->ambient;
}

Color Material::diffuse() const
{
    return m_context->diffuse;
}

Color Material::specular() const
{
    return m_context->specular;
}

Color Material::edgeColor() const
{
    return m_context->edgeColor;
}

Color Material::mainTextureBlend() const
{
    return kWhiteColor;
}

Color Material::sphereTextureBlend() const
{
    return kWhiteColor;
}

Color Material::toonTextureBlend() const
{
    return kWhiteColor;
}

IMaterial::IndexRange Material::indexRange() const
{
    return m_context->indexRange;
}

float Material::shininess() const
{
    return m_context->shininess;
}

IVertex::EdgeSizePrecision Material::edgeSize() const
{
    return 1;
}

int Material::index() const
{
    return m_context->index;
}

int Material::textureIndex() const
{
    return -1;
}

int Material::sphereTextureIndex() const
{
    return -1;
}

int Material::toonTextureIndex() const
{
    return m_context->toonTextureIndex;
}

bool Material::isSharedToonTextureUsed() const
{
    return m_context->toonTextureRef ? false : true;
}

bool Material::isCullingDisabled() const
{
    return !btFuzzyZero(m_context->diffuse.w() - 1);
}

bool Material::hasShadow() const
{
    return true;
}

bool Material::hasShadowMap() const
{
    return !btFuzzyZero(m_context->diffuse.x() - 0.98f);
}

bool Material::isSelfShadowEnabled() const
{
    return hasShadowMap();
}

bool Material::isEdgeEnabled() const
{
    return m_context->enableEdge;
}

void Material::setMainTexture(const IString *value)
{
    internal::setString(value, m_context->mainTexture);
}

void Material::setSphereTexture(const IString *value)
{
    internal::setString(value, m_context->sphereTexture);
}

void Material::setToonTexture(const IString *value)
{
    m_context->toonTextureRef = value;
}

void Material::setSphereTextureRenderMode(SphereTextureRenderMode value)
{
    m_context->sphereTextureRenderMode = value;
}

void Material::setAmbient(const Color &value)
{
    m_context->ambient = value;
}

void Material::setDiffuse(const Color &value)
{
    m_context->diffuse = value;
}

void Material::setSpecular(const Color &value)
{
    m_context->specular = value;
}

void Material::setEdgeColor(const Color &value)
{
    m_context->edgeColor = value;
}

void Material::setIndexRange(const IndexRange &value)
{
    m_context->indexRange = value;
}

void Material::setShininess(float value)
{
    m_context->shininess = value;
}

}
}
