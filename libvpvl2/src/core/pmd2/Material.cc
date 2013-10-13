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

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/ModelHelper.h"
#include "vpvl2/pmd2/Material.h"

namespace
{

using namespace vpvl2;
using namespace vpvl2::pmd2;

#pragma pack(push, 1)

struct MaterialUnit {
    float32 diffuse[3];
    float32 opacity;
    float32 shininess;
    float32 specular[3];
    float32 ambient[3];
    uint8 toonTextureIndex;
    uint8 edge;
    int32 nindices;
    uint8 textureName[internal::kPMDMaterialNameSize];
};

#pragma pack(pop)

}

namespace vpvl2
{
namespace pmd2
{

static const Color kWhiteColor = Color(1, 1, 1, 1);
const int Material::kNameSize = internal::kPMDMaterialNameSize;

struct Material::PrivateContext {
    PrivateContext(Model *parentModelRef, IEncoding *encodingRef)
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
        internal::deleteObject(mainTexture);
        internal::deleteObject(sphereTexture);
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
    Model *parentModelRef;
    IEncoding *encodingRef;
    IString *mainTexture;
    IString *sphereTexture;
    const IString *toonTextureRef;
    Array<PropertyEventListener *> eventRefs;
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

Material::Material(Model *parentModelRef, IEncoding *encodingRef)
    : m_context(new PrivateContext(parentModelRef, encodingRef))
{
}

Material::~Material()
{
    internal::deleteObject(m_context);
}

bool Material::preparse(uint8 *&ptr, vsize &rest, Model::DataInfo &info)
{
    int32 size;
    if (!internal::getTyped<int32>(ptr, rest, size) || size * sizeof(MaterialUnit) > rest) {
        return false;
    }
    info.materialsCount = size;
    info.materialsPtr = ptr;
    internal::drainBytes(size * sizeof(MaterialUnit), ptr, rest);
    return true;
}

bool Material::loadMaterials(const PointerArray<Material> &materials,
                             const Array<IString *> &textures,
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
        material->setIndex(i);
        actualIndices += material->indexRange().count;
    }
    return actualIndices == expectedIndices;
}

void Material::writeMaterials(const Array<Material *> &materials, const Model::DataInfo &info, uint8 *&data)
{
    const int32 nmaterials = materials.count();
    internal::writeBytes(&nmaterials, sizeof(nmaterials), data);
    for (int i = 0; i < nmaterials; i++) {
        Material *material = materials[i];
        material->write(data, info);
    }
}

vsize Material::estimateTotalSize(const Array<Material *> &materials, const Model::DataInfo &info)
{
    const int32 nmaterials = materials.count();
    vsize size = sizeof(nmaterials);
    for (int i = 0; i < nmaterials; i++) {
        Material *material = materials[i];
        size += material->estimateSize(info);
    }
    return size;
}

void Material::read(const uint8 *data, const Model::DataInfo & /* info */, vsize &size)
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
        internal::deleteObject(texture);
        IString *mainTexture = tokens[0];
        if (mainTexture->endsWith(sph)) {
            m_context->sphereTexture = mainTexture;
            m_context->sphereTextureRenderMode = kMultTexture;
        }
        else {
            m_context->mainTexture = mainTexture;
        }
        m_context->parentModelRef->addTexture(mainTexture);
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
            m_context->parentModelRef->addTexture(subTexture);
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
    m_context->toonTextureIndex = internal::ModelHelper::adjustSharedToonTextureIndex(unit.toonTextureIndex);
    size = sizeof(unit);
}

vsize Material::estimateSize(const Model::DataInfo & /* info */) const
{
    vsize size = 0;
    size += sizeof(MaterialUnit);
    return size;
}

void Material::write(uint8 *&data, const Model::DataInfo & /* info */) const
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
        uint8 *textureNamePtr = unit.textureName;
        internal::writeStringAsByteArray(s, IString::kShiftJIS, m_context->encodingRef, sizeof(unit.textureName), textureNamePtr);
        internal::deleteObject(s);
    }
    else if (!m_context->mainTexture && m_context->sphereTexture) {
        uint8 *textureNamePtr = unit.textureName;
        internal::writeStringAsByteArray(m_context->sphereTexture, IString::kShiftJIS, m_context->encodingRef, sizeof(unit.textureName), textureNamePtr);
    }
    else {
        uint8 *textureNamePtr = unit.textureName;
        internal::writeStringAsByteArray(m_context->mainTexture, IString::kShiftJIS, m_context->encodingRef, sizeof(unit.textureName), textureNamePtr);
    }
    unit.toonTextureIndex = (m_context->toonTextureIndex == 0) ? 0xff : m_context->toonTextureIndex;
    internal::writeBytes(&unit, sizeof(unit), data);
}

void Material::addEventListenerRef(PropertyEventListener *value)
{
    if (value) {
        m_context->eventRefs.remove(value);
        m_context->eventRefs.append(value);
    }
}

void Material::removeEventListenerRef(PropertyEventListener *value)
{
    if (value) {
        m_context->eventRefs.remove(value);
    }
}

void Material::getEventListenerRefs(Array<PropertyEventListener *> &value)
{
    value.copy(m_context->eventRefs);
}

IModel *Material::parentModelRef() const
{
    return m_context->parentModelRef;
}

const IString *Material::name(IEncoding::LanguageType /* type */) const
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

int Material::mainTextureIndex() const
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
    if (!value || (value && !value->equals(m_context->mainTexture))) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, mainTextureWillChange(value, this));
        internal::setString(value, m_context->mainTexture);
    }
}

void Material::setSphereTexture(const IString *value)
{
    if (!value || (value && !value->equals(m_context->sphereTexture))) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, sphereTextureWillChange(value, this));
        internal::setString(value, m_context->sphereTexture);
    }
}

void Material::setToonTexture(const IString *value)
{
    if (!value || (value && !value->equals(m_context->toonTextureRef))) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, toonTextureWillChange(value, this));
        m_context->toonTextureRef = value;
    }
}

void Material::setSphereTextureRenderMode(SphereTextureRenderMode value)
{
    if (m_context->sphereTextureRenderMode != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, sphereTextureRenderModeWillChange(value, this));
        m_context->sphereTextureRenderMode = value;
    }
}

void Material::setAmbient(const Color &value)
{
    if (m_context->ambient != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, ambientWillChange(value, this));
        m_context->ambient = value;
    }
}

void Material::setDiffuse(const Color &value)
{
    if (m_context->diffuse != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, diffuseWillChange(value, this));
        m_context->diffuse = value;
    }
}

void Material::setSpecular(const Color &value)
{
    if (m_context->specular != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, specularWillChange(value, this));
        m_context->specular = value;
    }
}

void Material::setEdgeColor(const Color &value)
{
    if (m_context->edgeColor != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, edgeColorWillChange(value, this));
        m_context->edgeColor = value;
    }
}

void Material::setIndexRange(const IndexRange &value)
{
    if (m_context->indexRange != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, indexRangeWillChange(value, this));
        m_context->indexRange = value;
    }
}

void Material::setShininess(float32 value)
{
    if (m_context->shininess != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, shininessWillChange(value, this));
        m_context->shininess = value;
    }
}

void Material::setIndex(int value)
{
    m_context->index = value;
}

} /* namespace pmd2 */
} /* namespace vpvl2 */
