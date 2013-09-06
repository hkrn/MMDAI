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

#include "vpvl2/pmx/Material.h"

namespace
{

using namespace vpvl2;

#pragma pack(push, 1)

struct MaterialUnit {
    float32 diffuse[4];
    float32 specular[3];
    float32 shininess;
    float32 ambient[3];
    uint8 flags;
    float32 edgeColor[4];
    float32 edgeSize;
};

#pragma pack(pop)

}

namespace vpvl2
{
namespace pmx
{

struct RGB3 {
    Color result;
    Vector3 base;
    Vector3 mul;
    Vector3 add;
    RGB3()
        : result(kZeroC),
          base(kZeroV3),
          mul(1, 1, 1),
          add(kZeroV3)
    {
    }
    void calculate() {
        const Vector3 &mixed = base * mul + add;
        result.setValue(mixed.x(), mixed.y(), mixed.z(), 1);
    }
    void calculateMulWeight(const Vector3 &value, const Scalar &weight) {
        static const Vector3 kOne3(1.0, 1.0, 1.0);
        mul = kOne3 - (kOne3 - value) * weight;
    }
    void calculateAddWeight(const Vector3 &value, const Scalar &weight) {
        add = value * weight;
    }
    void reset() {
        mul.setValue(1, 1, 1);
        add.setZero();
        calculate();
    }
};

struct RGBA3 {
    Color result;
    Color base;
    Color mul;
    Color add;
    RGBA3()
        : result(kZeroC),
          base(kZeroC),
          mul(1, 1, 1, 1),
          add(0, 0, 0, 0)
    {
    }
    void calculate() {
        const Vector3 &mixed = base * mul + add;
        const Scalar &alpha = base.w() * mul.w() + add.w();
        result.setValue(mixed.x(), mixed.y(), mixed.z(), alpha);
    }
    void calculateMulWeight(const Vector3 &value, const Scalar &weight) {
        static const Vector3 kOne3(1.0, 1.0, 1.0);
        const Vector3 &v = kOne3 - (kOne3 - value) * weight;
        mul.setValue(v.x(), v.y(), v.z(), 1.0f - (1.0f - value.w()) * weight);
    }
    void calculateAddWeight(const Vector3 &value, const Scalar &weight) {
        const Vector3 &v = value * weight;
        add.setValue(v.x(), v.y(), v.z(), value.w() * weight);
    }
    void reset() {
        mul.setValue(1, 1, 1, 1);
        add.setValue(0, 0, 0, 0);
        calculate();
    }
};

struct Material::PrivateContext {
    PrivateContext(Model *modelRef)
        : modelRef(modelRef),
          name(0),
          englishName(0),
          userDataArea(0),
          mainTextureRef(0),
          sphereTextureRef(0),
          toonTextureRef(0),
          sphereTextureRenderMode(kNone),
          shininess(0, 1, 0),
          edgeSize(0, 1, 0),
          index(-1),
          textureIndex(0),
          sphereTextureIndex(0),
          toonTextureIndex(0),
          flags(0),
          useSharedToonTexture(false)
    {
        mainTextureBlend.base.setValue(1, 1, 1, 1);
        mainTextureBlend.calculate();
        sphereTextureBlend.base.setValue(1, 1, 1, 1);
        sphereTextureBlend.calculate();
        toonTextureBlend.base.setValue(1, 1, 1, 1);
        toonTextureBlend.calculate();
    }
    ~PrivateContext() {
        delete name;
        name = 0;
        delete englishName;
        englishName = 0;
        delete userDataArea;
        userDataArea = 0;
        modelRef = 0;
        mainTextureRef = 0;
        sphereTextureRef = 0;
        toonTextureRef = 0;
        sphereTextureRenderMode = kNone;
        userDataArea = 0;
        shininess.setZero();
        edgeSize.setZero();
        index = -1;
        textureIndex = 0;
        sphereTextureIndex = 0;
        toonTextureIndex = 0;
        flags = 0;
        useSharedToonTexture = false;
    }

    Model *modelRef;
    IString *name;
    IString *englishName;
    IString *userDataArea;
    IString *mainTextureRef;
    IString *sphereTextureRef;
    IString *toonTextureRef;
    Array<PropertyEventListener *> eventRefs;
    IMaterial::SphereTextureRenderMode sphereTextureRenderMode;
    RGB3 ambient;
    RGBA3 diffuse;
    RGB3 specular;
    RGBA3 edgeColor;
    RGBA3 mainTextureBlend;
    RGBA3 sphereTextureBlend;
    RGBA3 toonTextureBlend;
    IMaterial::IndexRange indexRange;
    Vector3 shininess;
    Vector3 edgeSize;
    int index;
    int textureIndex;
    int sphereTextureIndex;
    int toonTextureIndex;
    uint8 flags;
    bool useSharedToonTexture;
};

Material::Material(Model *modelRef)
    : m_context(0)
{
    m_context = new PrivateContext(modelRef);
}

Material::~Material()
{
    delete m_context;
    m_context = 0;
}

bool Material::preparse(uint8 *&ptr, vsize &rest, Model::DataInfo &info)
{
    int32 nmaterials, size, textureIndexSize = info.textureIndexSize;
    if (!internal::getTyped<int32>(ptr, rest, nmaterials)) {
        VPVL2_LOG(WARNING, "Invalid size of PMX materials detected: size=" << nmaterials << " rest=" << rest);
        return false;
    }
    info.materialsPtr = ptr;
    vsize nTextureIndexSize = textureIndexSize * 2;
    for (int32 i = 0; i < nmaterials; i++) {
        uint8 *namePtr;
        /* name in Japanese */
        if (!internal::getText(ptr, rest, namePtr, size)) {
            VPVL2_LOG(WARNING, "Invalid size of PMX material name in Japanese detected: index=" << i << " size=" << size << " rest=" << rest);
            return false;
        }
        /* name in English */
        if (!internal::getText(ptr, rest, namePtr, size)) {
            VPVL2_LOG(WARNING, "Invalid size of PMX material name in English detected: index=" << i << " size=" << size << " rest=" << rest);
            return false;
        }
        if (!internal::validateSize(ptr, sizeof(MaterialUnit), rest)) {
            VPVL2_LOG(WARNING, "Invalid size of PMX material unit detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
            return false;
        }
        /* main texture + sphere map texture */
        if (!internal::validateSize(ptr, nTextureIndexSize, rest)) {
            VPVL2_LOG(WARNING, "Invalid size of PMX material texture detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
            return false;
        }
        /* material flags */
        if (sizeof(uint16) > rest) {
            VPVL2_LOG(WARNING, "Invalid size of PMX material flags detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
            return false;
        }
        bool isSharedToonTexture = *(ptr + sizeof(uint8)) == 1;
        internal::drainBytes(sizeof(uint16), ptr, rest);
        /* shared toon texture index */
        if (isSharedToonTexture) {
            if (!internal::validateSize(ptr, sizeof(uint8), rest)) {
                VPVL2_LOG(WARNING, "Invalid size of PMX material shared texture index detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
                return false;
            }
        }
        /* independent toon texture index */
        else {
            if (!internal::validateSize(ptr, textureIndexSize, rest)) {
                VPVL2_LOG(WARNING, "Invalid size of PMX material texture index detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
                return false;
            }
        }
        /* free area */
        if (!internal::getText(ptr, rest, namePtr, size)) {
            VPVL2_LOG(WARNING, "Invalid size of PMX material user data detected: index=" << i << " size=" << size << " rest=" << rest);
            return false;
        }
        /* number of indices */
        if (!internal::validateSize(ptr, sizeof(int), rest)) {
            VPVL2_LOG(WARNING, "Invalid size of PMX material index detected: index=" << i << " size=" << size << " rest=" << rest);
            return false;
        }
    }
    info.materialsCount = nmaterials;
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
        const int textureIndex = material->m_context->textureIndex;
        if (textureIndex >= 0) {
            if (textureIndex >= ntextures) {
                VPVL2_LOG(WARNING, "Invalid PMX material main texture index detected: index=" << i << " textureIndex=" << textureIndex << " ntextures=" << ntextures);
                return false;
            }
            else {
                material->m_context->mainTextureRef = textures[textureIndex];
            }
        }
        const int sphereTextureIndex = material->m_context->sphereTextureIndex;
        if (sphereTextureIndex >= 0) {
            if (sphereTextureIndex >= ntextures) {
                VPVL2_LOG(WARNING, "Invalid PMX material sphere texture index detected: index=" << i << " textureIndex=" << textureIndex << " ntextures=" << ntextures);
                return false;
            }
            else {
                material->m_context->sphereTextureRef = textures[sphereTextureIndex];
            }
        }
        const int toonTextureIndex = material->m_context->toonTextureIndex;
        if (!material->m_context->useSharedToonTexture && toonTextureIndex >= 0) {
            if (toonTextureIndex >= ntextures) {
                VPVL2_LOG(WARNING, "Invalid PMX material toon texture index detected: index=" << i << " textureIndex=" << textureIndex << " ntextures=" << ntextures);
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
    for (int32 i = 0; i < nmaterials; i++) {
        const Material *material = materials[i];
        material->write(data, info);
    }
}

vsize Material::estimateTotalSize(const Array<Material *> &materials, const Model::DataInfo &info)
{
    const int32 nmaterials = materials.count();
    vsize size = 0;
    size += sizeof(nmaterials);
    for (int32 i = 0; i < nmaterials; i++) {
        Material *material = materials[i];
        size += material->estimateSize(info);
    }
    return size;
}

void Material::read(const uint8 *data, const Model::DataInfo &info, vsize &size)
{
    uint8 *namePtr, *ptr = const_cast<uint8 *>(data), *start = ptr;
    vsize rest = SIZE_MAX, textureIndexSize = info.textureIndexSize;
    int32 nNameSize;
    IEncoding *encoding = info.encoding;
    internal::getText(ptr, rest, namePtr, nNameSize);
    internal::setStringDirect(encoding->toString(namePtr, nNameSize, info.codec), m_context->name);
    VPVL2_VLOG(3, "PMXMaterial: name=" << internal::cstr(m_context->name, "(null)"));
    internal::getText(ptr, rest, namePtr, nNameSize);
    internal::setStringDirect(encoding->toString(namePtr, nNameSize, info.codec), m_context->englishName);
    VPVL2_VLOG(3, "PMXMaterial: englishName=" << internal::cstr(m_context->englishName, "(null)"));
    MaterialUnit unit;
    internal::getData(ptr, unit);
    m_context->ambient.base.setValue(unit.ambient[0], unit.ambient[1], unit.ambient[2]);
    m_context->ambient.calculate();
    VPVL2_VLOG(3, "PMXMaterial: ambient=" << m_context->ambient.result.x() << "," << m_context->ambient.result.y() << "," << m_context->ambient.result.z());
    m_context->diffuse.base.setValue(unit.diffuse[0], unit.diffuse[1], unit.diffuse[2], unit.diffuse[3]);
    m_context->diffuse.calculate();
    VPVL2_VLOG(3, "PMXMaterial: diffuse=" << m_context->diffuse.result.x() << "," << m_context->diffuse.result.y() << "," << m_context->diffuse.result.z());
    m_context->specular.base.setValue(unit.specular[0], unit.specular[1], unit.specular[2]);
    m_context->specular.calculate();
    VPVL2_VLOG(3, "PMXMaterial: specular=" << m_context->specular.result.x() << "," << m_context->specular.result.y() << "," << m_context->specular.result.z());
    m_context->edgeColor.base.setValue(unit.edgeColor[0], unit.edgeColor[1], unit.edgeColor[2], unit.edgeColor[3]);
    m_context->edgeColor.calculate();
    VPVL2_VLOG(3, "PMXMaterial: edgeColor=" << m_context->edgeColor.result.x() << "," << m_context->edgeColor.result.y() << "," << m_context->edgeColor.result.z());
    m_context->shininess.setX(unit.shininess);
    VPVL2_VLOG(3, "PMXMaterial: shininess=" << m_context->shininess.x());
    m_context->edgeSize.setX(unit.edgeSize);
    VPVL2_VLOG(3, "PMXMaterial: edgeSize=" << m_context->edgeSize.x());
    m_context->flags = unit.flags;
    ptr += sizeof(unit);
    m_context->textureIndex = internal::readSignedIndex(ptr, textureIndexSize);
    VPVL2_VLOG(3, "PMXMaterial: mainTextureIndex=" << m_context->textureIndex);
    m_context->sphereTextureIndex = internal::readSignedIndex(ptr, textureIndexSize);
    VPVL2_VLOG(3, "PMXMaterial: sphereTextureIndex=" << m_context->sphereTextureIndex);
    uint8 type;
    internal::getTyped<uint8>(ptr, rest, type);
    m_context->sphereTextureRenderMode = static_cast<SphereTextureRenderMode>(type);
    internal::getTyped<uint8>(ptr, rest, type);
    m_context->useSharedToonTexture = type == 1;
    VPVL2_VLOG(3, "PMXMaterial: useSharedToonTexture=" << m_context->useSharedToonTexture);
    if (m_context->useSharedToonTexture) {
        internal::getTyped<uint8>(ptr, rest, type);
        m_context->toonTextureIndex = internal::ModelHelper::adjustSharedToonTextureIndex(type);
        VPVL2_VLOG(3, "PMXMaterial: sharedToonTextureIndex=" << m_context->toonTextureIndex);
    }
    else {
        m_context->toonTextureIndex = internal::readSignedIndex(ptr, textureIndexSize);
        VPVL2_VLOG(3, "PMXMaterial: toonTextureIndex=" << m_context->toonTextureIndex);
    }
    internal::getText(ptr, rest, namePtr, nNameSize);
    internal::setStringDirect(encoding->toString(namePtr, nNameSize, info.codec), m_context->userDataArea);
    internal::getTyped<int>(ptr, rest, nNameSize);
    m_context->indexRange.count = nNameSize;
    VPVL2_VLOG(3, "PMXMaterial: indexCount=" << m_context->indexRange.count);
    size = ptr - start;
}

void Material::write(uint8 *&data, const Model::DataInfo &info) const
{
    internal::writeString(m_context->name, info.codec, data);
    internal::writeString(m_context->englishName, info.codec, data);
    MaterialUnit mu;
    internal::getColor(m_context->ambient.base, mu.ambient);
    internal::getColor(m_context->diffuse.base, mu.diffuse);
    internal::getColor(m_context->specular.base, mu.specular);
    internal::getColor(m_context->edgeColor.base, mu.edgeColor);
    mu.shininess = m_context->shininess.x();
    mu.edgeSize = m_context->edgeSize.x();
    mu.flags = m_context->flags;
    internal::writeBytes(&mu, sizeof(mu), data);
    vsize textureIndexSize = info.textureIndexSize;
    internal::writeSignedIndex(m_context->textureIndex, textureIndexSize, data);
    internal::writeSignedIndex(m_context->sphereTextureIndex, textureIndexSize, data);
    internal::writeBytes(&m_context->sphereTextureRenderMode, sizeof(uint8), data);
    internal::writeBytes(&m_context->useSharedToonTexture, sizeof(uint8), data);
    if (m_context->useSharedToonTexture) {
        internal::writeBytes(&m_context->toonTextureIndex, sizeof(uint8), data);
    }
    else {
        internal::writeSignedIndex(m_context->toonTextureIndex, textureIndexSize, data);
    }
    internal::writeString(m_context->userDataArea, info.codec, data);
    internal::writeBytes(&m_context->indexRange.count, sizeof(int), data);
}

vsize Material::estimateSize(const Model::DataInfo &info) const
{
    vsize size = 0, textureIndexSize = info.textureIndexSize;
    size += internal::estimateSize(m_context->name, info.codec);
    size += internal::estimateSize(m_context->englishName, info.codec);
    size += sizeof(MaterialUnit);
    size += textureIndexSize * 2;
    size += sizeof(uint16);
    size += m_context->useSharedToonTexture ? sizeof(uint8) : textureIndexSize;
    size += internal::estimateSize(m_context->userDataArea, info.codec);
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
            m_context->ambient.calculateMulWeight(morph->ambient, w);
            m_context->diffuse.calculateMulWeight(morph->diffuse, w);
            m_context->specular.calculateMulWeight(morph->specular, w);
            m_context->shininess.setY(1.0f - (1.0f - morph->shininess) * w);
            m_context->edgeColor.calculateMulWeight(morph->edgeColor, w);
            m_context->edgeSize.setY(1.0f - Scalar(1.0f - morph->edgeSize) * w);
            m_context->mainTextureBlend.calculateMulWeight(morph->textureWeight, w);
            m_context->sphereTextureBlend.calculateMulWeight(morph->sphereTextureWeight, w);
            m_context->toonTextureBlend.calculateMulWeight(morph->toonTextureWeight, w);
            break;
        }
        case 1: { // add
            m_context->ambient.calculateAddWeight(morph->ambient, w);
            m_context->diffuse.calculateAddWeight(morph->diffuse, w);
            m_context->specular.calculateAddWeight(morph->specular, w);
            m_context->shininess.setZ(morph->shininess * w);
            m_context->edgeColor.calculateAddWeight(morph->edgeColor, w);
            m_context->edgeSize.setZ(Scalar(morph->edgeSize * w));
            m_context->mainTextureBlend.calculateAddWeight(morph->textureWeight, w);
            m_context->sphereTextureBlend.calculateAddWeight(morph->sphereTextureWeight, w);
            m_context->toonTextureBlend.calculateAddWeight(morph->toonTextureWeight, w);
            break;
        }
        default:
            break;
        }
        m_context->ambient.calculate();
        m_context->diffuse.calculate();
        m_context->specular.calculate();
        m_context->edgeColor.calculate();
        m_context->mainTextureBlend.calculate();
        m_context->sphereTextureBlend.calculate();
        m_context->toonTextureBlend.calculate();
    }
}

void Material::resetMorph()
{
    m_context->ambient.reset();
    m_context->diffuse.reset();
    m_context->specular.reset();
    m_context->shininess.setValue(m_context->shininess.x(), 1, 0);
    m_context->edgeColor.reset();
    m_context->edgeSize.setValue(m_context->edgeSize.x(), 1, 0);
    m_context->mainTextureBlend.reset();
    m_context->sphereTextureBlend.reset();
    m_context->toonTextureBlend.reset();
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
    return m_context->modelRef;
}

const IString *Material::name(IEncoding::LanguageType type) const
{
    switch (type) {
    case IEncoding::kDefaultLanguage:
    case IEncoding::kJapanese:
        return m_context->name;
    case IEncoding::kEnglish:
        return m_context->englishName;
    default:
        return 0;
    }
}

const IString *Material::userDataArea() const
{
    return m_context->userDataArea;
}

const IString *Material::mainTexture() const
{
    return m_context->mainTextureRef;
}

const IString *Material::sphereTexture() const
{
    return m_context->sphereTextureRef;
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
    return m_context->ambient.result;
}

Color Material::diffuse() const
{ return m_context->diffuse.result; }

Color Material::specular() const
{
    return m_context->specular.result;
}

Color Material::edgeColor() const
{
    return m_context->edgeColor.result;
}

Color Material::mainTextureBlend() const
{
    return m_context->mainTextureBlend.result;
}

Color Material::sphereTextureBlend() const
{
    return m_context->sphereTextureBlend.result;
}

Color Material::toonTextureBlend() const
{
    return m_context->toonTextureBlend.result;
}

IMaterial::IndexRange Material::indexRange() const
{
    return m_context->indexRange;
}

float Material::shininess() const
{
    return m_context->shininess.x() * m_context->shininess.y() + m_context->shininess.z();
}

IVertex::EdgeSizePrecision Material::edgeSize() const
{
    return m_context->edgeSize.x() * m_context->edgeSize.y() + m_context->edgeSize.z();
}

int Material::index() const
{
    return m_context->index;
}

int Material::textureIndex() const
{
    return m_context->textureIndex;
}

int Material::sphereTextureIndex() const
{
    return m_context->sphereTextureIndex;
}

int Material::toonTextureIndex() const
{
    return m_context->toonTextureIndex;
}

bool Material::isSharedToonTextureUsed() const
{
    return m_context->useSharedToonTexture;
}

bool Material::isCullingDisabled() const
{
    return internal::hasFlagBits(m_context->flags, kDisableCulling);
}

bool Material::hasShadow() const
{
    return internal::hasFlagBits(m_context->flags, kHasShadow) && !isPointDrawEnabled();
}

bool Material::hasShadowMap() const
{
    return internal::hasFlagBits(m_context->flags, kHasShadowMap) && !isPointDrawEnabled();
}

bool Material::isSelfShadowEnabled() const
{
    return internal::hasFlagBits(m_context->flags, kEnableSelfShadow) && !isPointDrawEnabled();
}

bool Material::isEdgeEnabled() const
{
    return internal::hasFlagBits(m_context->flags, kEnableEdge) && !isPointDrawEnabled() && !isLineDrawEnabled();
}

bool Material::hasVertexColor() const
{
    return internal::hasFlagBits(m_context->flags, kHasVertexColor);
}

bool Material::isPointDrawEnabled() const
{
    return internal::hasFlagBits(m_context->flags, kEnablePointDraw);
}

bool Material::isLineDrawEnabled() const
{
    return internal::hasFlagBits(m_context->flags, kEnableLineDraw);
}

void Material::setName(const IString *value, IEncoding::LanguageType type)
{
    switch (type) {
    case IEncoding::kDefaultLanguage:
    case IEncoding::kJapanese:
        if (value && !value->equals(m_context->name)) {
            VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, nameWillChange(value, type, this));
            internal::setString(value, m_context->name);
        }
        break;
    case IEncoding::kEnglish:
        if (value && !value->equals(m_context->englishName)) {
            VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, nameWillChange(value, type, this));
            internal::setString(value, m_context->englishName);
        }
        break;
    default:
        break;
    }
}

void Material::setUserDataArea(const IString *value)
{
    if (value && !value->equals(m_context->userDataArea)) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, userDataAreaWillChange(value, this));
        internal::setString(value, m_context->userDataArea);
    }
}

void Material::setMainTexture(const IString *value)
{
    if (value && !value->equals(m_context->mainTextureRef)) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, mainTextureWillChange(value, this));
        internal::setString(value, m_context->mainTextureRef);
        m_context->modelRef->addTexture(value);
    }
}

void Material::setSphereTexture(const IString *value)
{
    if (value && !value->equals(m_context->sphereTextureRef)) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, sphereTextureWillChange(value, this));
        internal::setString(value, m_context->sphereTextureRef);
        m_context->modelRef->addTexture(value);
    }
}

void Material::setToonTexture(const IString *value)
{
    if (value && !value->equals(m_context->toonTextureRef)) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, toonTextureWillChange(value, this));
        internal::setString(value, m_context->toonTextureRef);
        m_context->modelRef->addTexture(value);
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
    if (m_context->ambient.base != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, ambientWillChange(value, this));
        m_context->ambient.base = value;
        m_context->ambient.base.setW(1);
        m_context->ambient.calculate();
    }
}

void Material::setDiffuse(const Color &value)
{
    if (m_context->diffuse.base != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, diffuseWillChange(value, this));
        m_context->diffuse.base = value;
        m_context->diffuse.calculate();
    }
}

void Material::setSpecular(const Color &value)
{
    if (m_context->specular.base != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, specularWillChange(value, this));
        m_context->specular.base = value;
        m_context->specular.base.setW(1);
        m_context->specular.calculate();
    }
}

void Material::setEdgeColor(const Color &value)
{
    if (m_context->edgeColor.base != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, edgeColorWillChange(value, this));
        m_context->edgeColor.base = value;
        m_context->edgeColor.calculate();
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
    if (!btFuzzyZero(m_context->shininess.x() - value)) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, shininessWillChange(value, this));
        m_context->shininess.setX(value);
    }
}

void Material::setEdgeSize(const IVertex::EdgeSizePrecision &value)
{
    if (!btFuzzyZero(m_context->edgeSize.x() - value)) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, edgeSizeWillChange(value, this));
        m_context->edgeSize.setX(Scalar(value));
    }
}

void Material::setMainTextureIndex(int value)
{
    m_context->textureIndex = value;
}

void Material::setSphereTextureIndex(int value)
{
    m_context->sphereTextureIndex = value;
}

void Material::setToonTextureIndex(int value)
{
    m_context->toonTextureIndex = value;
}

void Material::setFlags(int value)
{
    if (m_context->flags != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, flagsWillChange(value, this));
        m_context->flags = value;
    }
}

void Material::setIndex(int value)
{
    m_context->index = value;
}

} /* namespace pmx */
} /* namespace vpvl2 */
