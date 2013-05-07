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

#include "vpvl2/pmx/Bone.h"
#include "vpvl2/pmx/Material.h"
#include "vpvl2/pmx/Morph.h"
#include "vpvl2/pmx/RigidBody.h"
#include "vpvl2/pmx/Vertex.h"

namespace
{

#pragma pack(push, 1)

struct MorphUnit {
    vpvl2::uint8_t category;
    vpvl2::uint8_t type;
    vpvl2::int32_t size;
};

struct VertexMorph {
    vpvl2::float32_t position[3];
};

struct UVMorph {
    vpvl2::float32_t position[4];
};

struct BoneMorph {
    vpvl2::float32_t position[3];
    vpvl2::float32_t rotation[4];
};

struct MaterialMorph {
    uint8_t operation;
    vpvl2::float32_t diffuse[4];
    vpvl2::float32_t specular[3];
    vpvl2::float32_t shininess;
    vpvl2::float32_t ambient[3];
    vpvl2::float32_t edgeColor[4];
    vpvl2::float32_t edgeSize;
    vpvl2::float32_t textureWeight[4];
    vpvl2::float32_t sphereTextureWeight[4];
    vpvl2::float32_t toonTextureWeight[4];
};

struct GroupMorph {
    vpvl2::float32_t weight;
};

struct FlipMorph {
    vpvl2::float32_t weight;
};

struct ImpulseMorph {
    vpvl2::uint8_t isLocal;
    vpvl2::float32_t velocity[3];
    vpvl2::float32_t torque[3];
};

#pragma pack(pop)

}

namespace vpvl2
{
namespace pmx
{

Morph::Morph(IModel *modelRef)
    : m_modelRef(modelRef),
      m_name(0),
      m_englishName(0),
      m_weight(0),
      m_internalWeight(0),
      m_category(kBase),
      m_type(kUnknownMorph),
      m_index(-1),
      m_hasParent(false)
{
}

Morph::~Morph()
{
    m_vertices.releaseAll();
    m_uvs.releaseAll();
    m_bones.releaseAll();
    m_materials.releaseAll();
    m_groups.releaseAll();
    m_flips.releaseAll();
    m_impulses.releaseAll();
    delete m_name;
    m_name = 0;
    delete m_englishName;
    m_englishName = 0;
    m_modelRef = 0;
    m_category = kBase;
    m_type = kUnknownMorph;
    m_index = -1;
    m_hasParent = false;
}

bool Morph::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    int32_t nmorphs, size;
    if (!internal::getTyped<int32_t>(ptr, rest, nmorphs)) {
        VPVL2_LOG(WARNING, "Invalid size of PMX morphs detected: size=" << nmorphs << " rest=" << rest);
        return false;
    }
    info.morphsPtr = ptr;
    MorphUnit morph;
    for (int32_t i = 0; i < nmorphs; i++) {
        uint8_t *namePtr;
        /* name in Japanese */
        if (!internal::getText(ptr, rest, namePtr, size)) {
            VPVL2_LOG(WARNING, "Invalid size of PMX morph name in Japanese detected: index=" << i << " size=" << size << " rest=" << rest);
            return false;
        }
        /* name in English */
        if (!internal::getText(ptr, rest, namePtr, size)) {
            VPVL2_LOG(WARNING, "Invalid size of PMX morph name in English detected: index=" << i << " size=" << size << " rest=" << rest);
            return false;
        }
        if (sizeof(MorphUnit) > rest) {
            VPVL2_LOG(WARNING, "Invalid size of PMX base morph unit detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
            return false;
        }
        internal::getData(ptr, morph);
        internal::drainBytes(sizeof(MorphUnit), ptr, rest);
        int nMorphsInMorph = morph.size;
        size_t extraSize;
        switch (static_cast<Type>(morph.type)) {
        case kGroupMorph:
            extraSize = info.morphIndexSize + sizeof(GroupMorph);
            break;
        case kVertexMorph:
            extraSize = info.vertexIndexSize + sizeof(VertexMorph);
            break;
        case kBoneMorph:
            extraSize = info.boneIndexSize + sizeof(BoneMorph);
            break;
        case kTexCoordMorph:
        case kUVA1Morph:
        case kUVA2Morph:
        case kUVA3Morph:
        case kUVA4Morph:
            extraSize = info.vertexIndexSize + sizeof(UVMorph);
            break;
        case kMaterialMorph:
            extraSize = info.materialIndexSize + sizeof(MaterialMorph);
            break;
        case kFlipMorph:
            extraSize = info.morphIndexSize + sizeof(FlipMorph);
            break;
        case kImpulseMorph:
            extraSize = info.rigidBodyIndexSize + sizeof(ImpulseMorph);
            break;
        default:
            return false;
        }
        for (int j = 0; j < nMorphsInMorph; j++) {
            if (!internal::validateSize(ptr, extraSize, rest)) {
                VPVL2_LOG(WARNING, "Invalid size of PMX morph chunk: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " size=" << extraSize << " rest=" << rest);
                return false;
            }
        }
    }
    info.morphsCount = nmorphs;
    return true;
}

bool Morph::loadMorphs(const Array<Morph *> &morphs,
                       const Array<pmx::Bone *> &bones,
                       const Array<pmx::Material *> &materials,
                       const Array<RigidBody *> &rigidBodies,
                       const Array<pmx::Vertex *> &vertices)
{
    const int nmorphs = morphs.count();
    for (int i = 0; i < nmorphs; i++) {
        Morph *morph = morphs[i];
        switch (morph->type()) {
        case kGroupMorph:
            if (!loadGroups(morphs, morph)) {
                return false;
            }
            break;
        case kVertexMorph:
            if (!loadVertices(vertices, morph)) {
                return false;
            }
            break;
        case kBoneMorph:
            if (!loadBones(bones, morph)) {
                return false;
            }
            break;
        case kTexCoordMorph:
            if (!loadUVs(vertices, 0, morph)) {
                return false;
            }
            break;
        case kUVA1Morph:
            if (!loadUVs(vertices, 1, morph)) {
                return false;
            }
            break;
        case kUVA2Morph:
            if (!loadUVs(vertices, 2, morph)) {
                return false;
            }
            break;
        case kUVA3Morph:
            if (!loadUVs(vertices, 3, morph)) {
                return false;
            }
            break;
        case kUVA4Morph:
            if (!loadUVs(vertices, 4, morph)) {
                return false;
            }
            break;
        case kMaterialMorph:
            if (!loadMaterials(materials, morph)) {
                return false;
            }
            break;
        case kFlipMorph:
            if (!loadFlips(morphs, morph)) {
                return false;
            }
            break;
        case kImpulseMorph:
            if (!loadImpulses(rigidBodies, morph)) {
                return false;
            }
            break;
        default:
            return false;
        }
        morph->setIndex(i);
    }
    return true;
}

void Morph::writeMorphs(const Array<Morph *> &morphs, const Model::DataInfo &info, uint8_t *&data)
{
    const int32_t nmorphs = morphs.count();
    internal::writeBytes(&nmorphs, sizeof(nmorphs), data);
    for (int32_t i = 0; i < nmorphs; i++) {
        const Morph *morph = morphs[i];
        morph->write(data, info);
    }
}

size_t Morph::estimateTotalSize(const Array<Morph *> &morphs, const Model::DataInfo &info)
{
    const int32_t nmorphs = morphs.count();
    size_t size = 0;
    size += sizeof(nmorphs);
    for (int32_t i = 0; i < nmorphs; i++) {
        Morph *morph = morphs[i];
        size += morph->estimateSize(info);
    }
    return size;
}

bool Morph::loadBones(const Array<pmx::Bone *> &bones, Morph *morph)
{
    const int nMorphBones = morph->m_bones.count();
    const int nbones = bones.count();
    for (int32_t i = 0; i < nMorphBones; i++) {
        Bone *bone = morph->m_bones[i];
        int boneIndex = bone->index;
        if (boneIndex >= 0) {
            if (boneIndex >= nbones) {
                VPVL2_LOG(WARNING, "Invalid PMX bone morph: index=" << i << " bone=" << boneIndex);
                return false;
            }
            else {
                bone->bone = bones[boneIndex];
            }
        }
    }
    return true;
}

bool Morph::loadGroups(const Array<Morph *> &morphs, Morph *morph)
{
    const int nMorphGroups = morph->m_groups.count();
    const int nmorphs = morphs.count();
    for (int32_t i = 0; i < nMorphGroups; i++) {
        Group *group = morph->m_groups[i];
        int groupIndex = group->index;
        if (groupIndex >= 0) {
            if (groupIndex >= nmorphs) {
                VPVL2_LOG(WARNING, "Invalid PMX group morph: index=" << i << " group=" << groupIndex);
                return false;
            }
            else {
                Morph *morph = morphs[groupIndex];
                if (morph->m_type == kGroupMorph) {
                    VPVL2_LOG(WARNING, "Invalid PMX group morph (cannot create chikd): index=" << i << " group=" << groupIndex);
                    return false;
                }
                else {
                    group->morph = morph;
                    morph->m_hasParent = true;
                }
            }
        }
    }
    return true;
}

bool Morph::loadMaterials(const Array<pmx::Material *> &materials, Morph *morph)
{
    const int nMorphMaterials = morph->m_materials.count();
    const int nmaterials = materials.count();
    for (int i = 0; i < nMorphMaterials; i++) {
        Material *material = morph->m_materials[i];
        int materialIndex = material->index;
        if (materialIndex >= 0) {
            if (materialIndex >= nmaterials) {
                VPVL2_LOG(WARNING, "Invalid PMX material morph: index=" << i << " material=" << materialIndex);
                return false;
            }
            else {
                material->materials->append(materials[materialIndex]);
            }
        }
        else {
            const int nmaterials = materials.count();
            for (int j = 0; j < nmaterials; j++) {
                pmx::Material *m = materials[j];
                material->materials->append(m);
            }
        }
    }
    return true;
}

bool Morph::loadUVs(const Array<pmx::Vertex *> &vertices, int offset, Morph *morph)
{
    const int nMorphUVs = morph->m_uvs.count();
    const int nvertices = vertices.count();
    for (int i = 0; i < nMorphUVs; i++) {
        UV *uv = morph->m_uvs[i];
        int vertexIndex = uv->index;
        if (vertexIndex >= 0) {
            if (vertexIndex >= nvertices) {
                VPVL2_LOG(WARNING, "Invalid PMX UV vertex morph: index=" << i << " vertex=" << vertexIndex);
                return false;
            }
            else {
                uv->vertex = vertices[vertexIndex];
                uv->offset = offset;
            }
        }
    }
    return true;
}

bool Morph::loadVertices(const Array<pmx::Vertex *> &vertices, Morph *morph)
{
    const int nMorphVertices = morph->m_vertices.count();
    const int nvertices = vertices.count();
    for (int i = 0; i < nMorphVertices; i++) {
        Vertex *vertex = morph->m_vertices[i];
        int vertexIndex = vertex->index;
        if (vertexIndex >= 0) {
            if (vertexIndex >= nvertices) {
                VPVL2_LOG(WARNING, "Invalid PMX vertex morph: index=" << i << " vertex=" << vertexIndex);
                return false;
            }
            else {
                vertex->vertex = vertices[vertexIndex];
            }
        }
    }
    return true;
}

bool Morph::loadFlips(const Array<Morph *> &morphs, Morph *morph)
{
    const int nMorphFlips = morph->m_impulses.count();
    const int nflips = morphs.count();
    for (int i = 0; i < nMorphFlips; i++) {
        Flip *flip = morph->m_flips[i];
        int flipIndex = flip->index;
        if (flipIndex >= 0) {
            if (flipIndex >= nflips) {
                VPVL2_LOG(WARNING, "Invalid flip morph: index=" << i << " morph=" << flipIndex);
                return false;
            }
            else {
                flip->morph = morphs[flipIndex];
            }
        }
    }
    return true;
}

bool Morph::loadImpulses(const Array<RigidBody *> &rigidBodies, Morph *morph)
{
    const int nMorphImpulses = morph->m_impulses.count();
    const int nbodies = rigidBodies.count();
    for (int i = 0; i < nMorphImpulses; i++) {
        Impulse *impulse = morph->m_impulses[i];
        int rigidBodyIndex = impulse->index;
        if (rigidBodyIndex >= 0) {
            if (rigidBodyIndex >= nbodies) {
                VPVL2_LOG(WARNING, "Invalid impluse morph: index=" << i << " body=" << rigidBodyIndex);
                return false;
            }
            else {
                impulse->rigidBody = rigidBodies[rigidBodyIndex];
            }
        }
    }
    return true;
}

void Morph::read(const uint8_t *data, const Model::DataInfo &info, size_t &size)
{
    uint8_t *namePtr, *ptr = const_cast<uint8_t *>(data), *start = ptr;
    size_t rest = SIZE_MAX;
    int32_t nNameSize;
    internal::getText(ptr, rest, namePtr, nNameSize);
    IEncoding *encoding = info.encoding;
    internal::setStringDirect(encoding->toString(namePtr, nNameSize, info.codec), m_name);
    VPVL2_VLOG(3, "PMXMorph: name=" << internal::cstr(m_name, "(null)"));
    internal::getText(ptr, rest, namePtr, nNameSize);
    internal::setStringDirect(encoding->toString(namePtr, nNameSize, info.codec), m_englishName);
    VPVL2_VLOG(3, "PMXMorph: englishName=" << internal::cstr(m_englishName, "(null)"));
    MorphUnit unit;
    internal::getData(ptr, unit);
    m_category = static_cast<Category>(unit.category);
    m_type = static_cast<Type>(unit.type);
    VPVL2_VLOG(3, "PMXMorph: category=" << m_category << " type=" << m_type << " size=" << unit.size);
    ptr += sizeof(unit);
    switch (m_type) {
    case kGroupMorph:
        readGroups(info, unit.size, ptr);
        break;
    case kVertexMorph:
        readVertices(info, unit.size, ptr);
        break;
    case kBoneMorph:
        readBones(info, unit.size, ptr);
        break;
    case kTexCoordMorph:
    case kUVA1Morph:
    case kUVA2Morph:
    case kUVA3Morph:
    case kUVA4Morph:
        readUVs(info, unit.size, m_type - kTexCoordMorph, ptr);
        break;
    case kMaterialMorph:
        readMaterials(info, unit.size, ptr);
        break;
    case kFlipMorph:
        readFlips(info, unit.size, ptr);
        break;
    case kImpulseMorph:
        readImpulses(info, unit.size, ptr);
        break;
    default:
        break; /* should not reach here */
    }
    size = ptr - start;
}

void Morph::write(uint8_t *&data, const Model::DataInfo &info) const
{
    internal::writeString(m_name, info.codec, data);
    internal::writeString(m_englishName, info.codec, data);
    MorphUnit mu;
    mu.category = m_category;
    mu.type = m_type;
    switch (m_type) {
    case kGroupMorph:
        mu.size = m_groups.count();
        internal::writeBytes(&mu, sizeof(mu), data);
        writeGroups(info, data);
        break;
    case kVertexMorph:
        mu.size = m_vertices.count();
        internal::writeBytes(&mu, sizeof(mu), data);
        writeVertices(info, data);
        break;
    case kBoneMorph:
        mu.size = m_bones.count();
        internal::writeBytes(&mu, sizeof(mu), data);
        writeBones(info, data);
        break;
    case kTexCoordMorph:
    case kUVA1Morph:
    case kUVA2Morph:
    case kUVA3Morph:
    case kUVA4Morph:
        mu.size = m_uvs.count();
        internal::writeBytes(&mu, sizeof(mu), data);
        writeUVs(info, data);
        break;
    case kMaterialMorph:
        mu.size = m_materials.count();
        internal::writeBytes(&mu, sizeof(mu), data);
        writeMaterials(info, data);
        break;
    case kFlipMorph:
        mu.size = m_flips.count();
        internal::writeBytes(&mu, sizeof(mu), data);
        writeFlips(info, data);
        break;
    case kImpulseMorph:
        mu.size = m_impulses.count();
        internal::writeBytes(&mu, sizeof(mu), data);
        writeImpulses(info, data);
        break;
    default:
        break; /* should not reach here */
    }
}

size_t Morph::estimateSize(const Model::DataInfo &info) const
{
    size_t size = 0;
    size += internal::estimateSize(m_name, info.codec);
    size += internal::estimateSize(m_englishName, info.codec);
    size += sizeof(MorphUnit);
    switch (m_type) {
    case kGroupMorph:
        size += m_groups.count() * (sizeof(GroupMorph) + info.morphIndexSize);
        break;
    case kVertexMorph:
        size += m_vertices.count() * (sizeof(VertexMorph) + info.vertexIndexSize);
        break;
    case kBoneMorph:
        size += m_bones.count() * (sizeof(BoneMorph) + info.boneIndexSize);
        break;
    case kTexCoordMorph:
    case kUVA1Morph:
    case kUVA2Morph:
    case kUVA3Morph:
    case kUVA4Morph:
        size += m_uvs.count() * (sizeof(UVMorph) + info.vertexIndexSize);
        break;
    case kMaterialMorph:
        size += m_materials.count() * (sizeof(MaterialMorph) + info.materialIndexSize);
        break;
    case kFlipMorph:
        size += m_flips.count() * (sizeof(FlipMorph) + info.morphIndexSize);
        break;
    case kImpulseMorph:
        size += m_impulses.count() * (sizeof(ImpulseMorph) + info.rigidBodyIndexSize);
        break;
    default:
        return 0; /* should not reach here */
    }
    return size;
}

void Morph::setWeight(const IMorph::WeightPrecision &value)
{
    m_weight = value;
}

void Morph::update()
{
    if (!btFuzzyZero(m_internalWeight)) {
        switch (m_type) {
        case kGroupMorph:
            updateGroupMorphs(m_internalWeight, false);
            break;
        case kVertexMorph:
            updateVertexMorphs(m_internalWeight);
            break;
        case kBoneMorph:
            updateBoneMorphs(m_internalWeight);
            break;
        case kTexCoordMorph:
        case kUVA1Morph:
        case kUVA2Morph:
        case kUVA3Morph:
        case kUVA4Morph:
            updateUVMorphs(m_internalWeight);
            break;
        case kMaterialMorph:
            updateMaterialMorphs(m_internalWeight);
            break;
        case kFlipMorph:
            /* do nothing */
            break;
        case kImpulseMorph:
            updateImpluseMorphs(m_internalWeight);
            break;
        default:
            break; /* should not reach here */
        }
    }
}

void Morph::syncWeight()
{
    switch (m_type) {
    case kGroupMorph:
        updateGroupMorphs(m_weight, true);
        break;
    case kFlipMorph:
        updateFlipMorphs(m_weight);
        break;
    case kVertexMorph:
    case kBoneMorph:
    case kTexCoordMorph:
    case kUVA1Morph:
    case kUVA2Morph:
    case kUVA3Morph:
    case kUVA4Morph:
    case kMaterialMorph:
    case kImpulseMorph:
    default:
        m_internalWeight = m_weight;
        break;
    }
}

void Morph::updateVertexMorphs(const WeightPrecision &value)
{
    const int nmorphs = m_vertices.count();
    for (int i = 0; i < nmorphs; i++) {
        Vertex *v = m_vertices[i];
        if (pmx::Vertex *vertex = v->vertex) {
            vertex->mergeMorph(v, value);
        }
    }
}

void Morph::updateBoneMorphs(const WeightPrecision &value)
{
    const int nmorphs = m_bones.count();
    for (int i = 0; i < nmorphs; i++) {
        Bone *v = m_bones[i];
        if (pmx::Bone *bone = v->bone) {
            bone->mergeMorph(v, value);
        }
    }
}

void Morph::updateUVMorphs(const WeightPrecision &value)
{
    const int nmorphs = m_uvs.count();
    for (int i = 0; i < nmorphs; i++) {
        UV *v = m_uvs[i];
        if (pmx::Vertex *vertex = v->vertex) {
            vertex->mergeMorph(v, value);
        }
    }
}

void Morph::updateMaterialMorphs(const WeightPrecision &value)
{
    const int nmorphs = m_materials.count();
    for (int i = 0; i < nmorphs; i++) {
        Material *v = m_materials.at(i);
        const Array<pmx::Material *> *materials = v->materials;
        const int nmaterials = materials->count();
        for (int j = 0; j < nmaterials; j++) {
            pmx::Material *material = materials->at(j);
            material->mergeMorph(v, value);
        }
    }
}

void Morph::updateGroupMorphs(const WeightPrecision &value, bool flipOnly)
{
    const int nmorphs = m_groups.count();
    for (int i = 0; i < nmorphs; i++) {
        Group *v = m_groups[i];
        if (Morph *morph = v->morph) {
            if ((morph->type() == Morph::kFlipMorph) == flipOnly) {
                morph->setInternalWeight(v->weight * value);
            }
        }
    }
}

void Morph::updateFlipMorphs(const WeightPrecision &value)
{
    const int nmorphs = m_flips.count();
    if (nmorphs > 0) {
        const WeightPrecision &weight = btClamped(value, WeightPrecision(0.0), WeightPrecision(1.0));
        int index = (nmorphs + 1) * weight - 1;
        const Flip *flip = m_flips.at(index);
        if (Morph *morph = flip->morph) {
            morph->setInternalWeight(flip->weight);
        }
    }
}

void Morph::updateImpluseMorphs(const WeightPrecision &value)
{
    const int nmorphs = m_impulses.count();
    for (int i = 0; i < nmorphs; i++) {
        Impulse *impulse = m_impulses.at(i);
        if (RigidBody *rigidBody = impulse->rigidBody) {
            rigidBody->mergeMorph(impulse, value);
        }
    }
}

void Morph::setName(const IString *value)
{
    internal::setString(value, m_name);
}

void Morph::setEnglishName(const IString *value)
{
    internal::setString(value, m_englishName);
}

void Morph::addBoneMorph(Bone *value)
{
    m_bones.append(value);
}

void Morph::addGroupMorph(Group *value)
{
    m_groups.append(value);
}

void Morph::addMaterialMorph(Material *value)
{
    m_materials.append(value);
}

void Morph::addUVMorph(UV *value)
{
    m_uvs.append(value);
}

void Morph::addVertexMorph(Vertex *value)
{
    m_vertices.append(value);
}

void Morph::addFlip(Flip *value)
{
    m_flips.append(value);
}

void Morph::addImpulse(Impulse *value)
{
    m_impulses.append(value);
}

void Morph::setCategory(Category value)
{
    m_category = value;
}

void Morph::setType(Type value)
{
    m_type = value;
}

void Morph::setIndex(int value)
{
    m_index = value;
}

void Morph::setInternalWeight(const WeightPrecision &value)
{
    m_internalWeight = value;
}

void Morph::readBones(const Model::DataInfo &info, int count, uint8_t *&ptr)
{
    BoneMorph morph;
    for (int i = 0; i < count; i++) {
        Morph::Bone *bone = m_bones.append(new Morph::Bone());
        int boneIndex = internal::readSignedIndex(ptr, info.boneIndexSize);
        VPVL2_VLOG(3, "PMXBoneMorph: index=" << i << " boneIndex=" << boneIndex);
        internal::getData(ptr, morph);
        internal::setPosition(morph.position, bone->position);
        VPVL2_VLOG(3, "PMXBoneMorph: position=" << bone->position.x() << "," << bone->position.y() << "," << bone->position.z());
        internal::setRotation(morph.rotation, bone->rotation);
        VPVL2_VLOG(3, "PMXBoneMorph: rotation=" << bone->rotation.x() << "," << bone->rotation.y() << "," << bone->rotation.z());
        bone->index = boneIndex;
        ptr += sizeof(morph);
    }
}

void Morph::readGroups(const Model::DataInfo &info, int count, uint8_t *&ptr)
{
    GroupMorph morph;
    for (int i = 0; i < count; i++) {
        Morph::Group *group = m_groups.append(new Morph::Group());
        int morphIndex = internal::readSignedIndex(ptr, info.morphIndexSize);
        internal::getData(ptr, morph);
        VPVL2_VLOG(3, "PMXGroupMorph: index=" << i << " morphIndex=" << morphIndex << " weight=" << group->weight);
        group->weight = morph.weight;
        group->index = morphIndex;
        ptr += sizeof(morph);
    }
}

void Morph::readMaterials(const Model::DataInfo &info, int count, uint8_t *&ptr)
{
    MaterialMorph morph;
    for (int i = 0; i < count; i++) {
        Morph::Material *material = m_materials.append(new Morph::Material());
        int materialIndex = internal::readSignedIndex(ptr, info.materialIndexSize);
        internal::getData(ptr, morph);
        VPVL2_VLOG(3, "PMXMaterialMorph: index=" << i << " materialIndex=" << materialIndex << " operation" << int(material->operation));
        material->materials = new Array<pmx::Material *>();
        material->ambient.setValue(morph.ambient[0], morph.ambient[1], morph.ambient[2]);
        VPVL2_VLOG(3, "PMXMaterialMorph: ambient=" << material->ambient.x() << "," << material->ambient.y() << "," << material->ambient.z());
        material->diffuse.setValue(morph.diffuse[0], morph.diffuse[1], morph.diffuse[2], morph.diffuse[3]);
        VPVL2_VLOG(3, "PMXMaterialMorph: diffuse=" << material->diffuse.x() << "," << material->diffuse.y() << "," << material->diffuse.z());
        material->edgeColor.setValue(morph.edgeColor[0], morph.edgeColor[1], morph.edgeColor[2], morph.edgeColor[3]);
        VPVL2_VLOG(3, "PMXMaterialMorph: edgeColor=" << material->edgeColor.x() << "," << material->edgeColor.y() << "," << material->edgeColor.z());
        material->edgeSize = morph.edgeSize;
        VPVL2_VLOG(3, "PMXMaterialMorph: edgeSize=" << material->edgeSize);
        material->index = materialIndex;
        material->operation = morph.operation;
        material->shininess = morph.shininess;
        VPVL2_VLOG(3, "PMXMaterialMorph: shininess=" << material->shininess);
        material->specular.setValue(morph.specular[0], morph.specular[1], morph.specular[2]);
        VPVL2_VLOG(3, "PMXMaterialMorph: specular=" << material->specular.x() << "," << material->specular.y() << "," << material->specular.z());
        material->sphereTextureWeight.setValue(morph.sphereTextureWeight[0], morph.sphereTextureWeight[1],
                morph.sphereTextureWeight[2], morph.sphereTextureWeight[3]);
        VPVL2_VLOG(3, "PMXMaterialMorph: sphereTextureWeight=" << material->sphereTextureWeight.x() << ","
                  << material->sphereTextureWeight.y() << "," << material->sphereTextureWeight.z() << "," << material->sphereTextureWeight.w());
        material->textureWeight.setValue(morph.textureWeight[0], morph.textureWeight[1],
                morph.textureWeight[2], morph.textureWeight[3]);
        VPVL2_VLOG(3, "PMXMaterialMorph: textureWeight=" << material->textureWeight.x() << ","
                  << material->textureWeight.y() << "," << material->textureWeight.z() << "," << material->textureWeight.w());
        material->toonTextureWeight.setValue(morph.toonTextureWeight[0], morph.toonTextureWeight[1],
                morph.toonTextureWeight[2], morph.toonTextureWeight[3]);
        VPVL2_VLOG(3, "PMXMaterialMorph: toonTextureWeight=" << material->sphereTextureWeight.x() << ","
                  << material->toonTextureWeight.y() << "," << material->toonTextureWeight.z() << "," << material->toonTextureWeight.w());
        ptr += sizeof(morph);
    }
}

void Morph::readUVs(const Model::DataInfo &info, int count, int offset, uint8_t *&ptr)
{
    UVMorph morph;
    for (int i = 0; i < count; i++) {
        Morph::UV *uv = m_uvs.append(new Morph::UV());
        int vertexIndex = internal::readUnsignedIndex(ptr, info.vertexIndexSize);
        VPVL2_VLOG(3, "PMXUVMorph: index=" << i << " vertexIndex=" << vertexIndex << " offset=" << offset);
        internal::getData(ptr, morph);
        uv->position.setValue(morph.position[0], morph.position[1], morph.position[2], morph.position[3]);
        VPVL2_VLOG(3, "PMXUVMorph: position=" << uv->position.x() << "," << uv->position.y() << "," << uv->position.z());
        uv->index = vertexIndex;
        uv->offset = offset;
        ptr += sizeof(morph);
    }
}

void Morph::readVertices(const Model::DataInfo &info, int count, uint8_t *&ptr)
{
    VertexMorph morph;
    for (int i = 0; i < count; i++) {
        Morph::Vertex *vertex = m_vertices.append(new Morph::Vertex());
        int vertexIndex = internal::readUnsignedIndex(ptr, info.vertexIndexSize);
        VPVL2_VLOG(3, "PMXVertexMorph: index=" << i << " vertexIndex=" << vertexIndex);
        internal::getData(ptr, morph);
        internal::setPosition(morph.position, vertex->position);
        VPVL2_VLOG(3, "PMXVertexMorph: position=" << vertex->position.x() << "," << vertex->position.y() << "," << vertex->position.z());
        vertex->index = vertexIndex;
        ptr += sizeof(morph);
    }
}

void Morph::readFlips(const Model::DataInfo &info, int count, uint8_t *&ptr)
{
    FlipMorph morph;
    for (int i = 0; i < count; i++) {
        Morph::Flip *flip = m_flips.append(new Morph::Flip());
        int morphIndex = internal::readSignedIndex(ptr, info.morphIndexSize);
        internal::getData(ptr, morph);
        VPVL2_VLOG(3, "PMXFlipMorph: index=" << i << " morphIndex=" << morphIndex << " weight=" << flip->weight);
        flip->weight = morph.weight;
        flip->index = morphIndex;
        ptr += sizeof(morph);
    }
}

void Morph::readImpulses(const Model::DataInfo &info, int count, uint8_t *&ptr)
{
    ImpulseMorph morph;
    for (int i = 0; i < count; i++) {
        Morph::Impulse *impulse = m_impulses.append(new Morph::Impulse());
        int rigidBodyIndex = internal::readSignedIndex(ptr, info.rigidBodyIndexSize);
        internal::getData(ptr, morph);
        impulse->isLocal = morph.isLocal != 0;
        impulse->index = rigidBodyIndex;
        VPVL2_VLOG(3, "PMXImpluseMorph: index=" << i << " rigidBodyIndex=" << rigidBodyIndex << " isLocal=" << impulse->isLocal);
        internal::setPositionRaw(morph.velocity, impulse->velocity);
        VPVL2_VLOG(3, "PMXImpluseMorph: velocity=" << impulse->velocity.x() << "," << impulse->velocity.y() << "," << impulse->velocity.z());
        internal::setPositionRaw(morph.torque, impulse->torque);
        VPVL2_VLOG(3, "PMXImpluseMorph: torque=" << impulse->torque.x() << "," << impulse->torque.y() << "," << impulse->torque.z());
        ptr += sizeof(morph);
    }
}

void Morph::writeBones(const Model::DataInfo &info, uint8_t *&ptr) const
{
    BoneMorph morph;
    const int nbones = m_bones.count(), boneIndexSize = info.boneIndexSize;
    for (int i = 0; i < nbones; i++) {
        const Morph::Bone *bone = m_bones[i];
        internal::getPosition(bone->position, morph.position);
        internal::getRotation(bone->rotation, morph.rotation);
        internal::writeSignedIndex(bone->index, boneIndexSize, ptr);
        internal::writeBytes(&morph, sizeof(morph), ptr);
    }
}

void Morph::writeGroups(const Model::DataInfo &info, uint8_t *&ptr) const
{
    GroupMorph morph;
    const int ngroups = m_groups.count(), morphIndexSize = info.morphIndexSize;
    for (int i = 0; i < ngroups; i++) {
        const Morph::Group *group = m_groups[i];
        morph.weight = group->weight;
        internal::writeSignedIndex(group->index, morphIndexSize, ptr);
        internal::writeBytes(&morph, sizeof(morph), ptr);
    }
}

void Morph::writeMaterials(const Model::DataInfo &info, uint8_t *&ptr) const
{
    MaterialMorph morph;
    const int nmaterials = m_materials.count(), materialIndexSize = info.materialIndexSize;
    for (int i = 0; i < nmaterials; i++) {
        const Morph::Material *material = m_materials[i];
        internal::getColor(material->ambient, morph.ambient);
        internal::getColor(material->diffuse, morph.diffuse);
        internal::getColor(material->edgeColor, morph.edgeColor);
        morph.operation = material->operation;
        morph.shininess = material->shininess;
        morph.edgeSize = material->edgeSize;
        internal::getColor(material->specular, morph.specular);
        internal::getColor(material->sphereTextureWeight, morph.sphereTextureWeight);
        internal::getColor(material->textureWeight, morph.textureWeight);
        internal::getColor(material->toonTextureWeight, morph.toonTextureWeight);
        internal::writeSignedIndex(material->index, materialIndexSize, ptr);
        internal::writeBytes(&morph, sizeof(morph), ptr);
    }
}

void Morph::writeUVs(const Model::DataInfo &info, uint8_t *&ptr) const
{
    UVMorph morph;
    const int nuvs = m_uvs.count(), vertexIndexSize = info.vertexIndexSize;
    for (int i = 0; i < nuvs; i++) {
        const Morph::UV *uv = m_uvs[i];
        const Vector4 &position = uv->position;
        morph.position[0] = position.x();
        morph.position[1] = position.y();
        morph.position[2] = position.z();
        morph.position[3] = position.w();
        internal::writeUnsignedIndex(uv->index, vertexIndexSize, ptr);
        internal::writeBytes(&morph, sizeof(morph), ptr);
    }
}

void Morph::writeVertices(const Model::DataInfo &info, uint8_t *&ptr) const
{
    VertexMorph morph;
    const int nvertices = m_vertices.count(), vertexIndexSize = info.vertexIndexSize;
    for (int i = 0; i < nvertices; i++) {
        const Morph::Vertex *vertex = m_vertices[i];
        internal::getPosition(vertex->position, morph.position);
        internal::writeUnsignedIndex(vertex->index, vertexIndexSize, ptr);
        internal::writeBytes(&morph, sizeof(morph), ptr);
    }
}

void Morph::writeFlips(const Model::DataInfo &info, uint8_t *&ptr) const
{
    FlipMorph morph;
    const int nflips = m_flips.count(), morphIndexSize = info.morphIndexSize;
    for (int i = 0; i < nflips; i++) {
        const Morph::Flip *flip = m_flips[i];
        morph.weight = flip->weight;
        internal::writeSignedIndex(flip->index, morphIndexSize, ptr);
        internal::writeBytes(&morph, sizeof(morph), ptr);
    }
}

void Morph::writeImpulses(const Model::DataInfo &info, uint8_t *&ptr) const
{
    ImpulseMorph morph;
    const int nimpulses = m_impulses.count(), rigidBodyIndex = info.rigidBodyIndexSize;
    for (int i = 0; i < nimpulses; i++) {
        const Morph::Impulse *impulse = m_impulses[i];
        internal::getPositionRaw(impulse->velocity, morph.velocity);
        internal::getPositionRaw(impulse->torque, morph.torque);
        morph.isLocal = impulse->isLocal ? 1 : 0;
        internal::writeSignedIndex(impulse->index, rigidBodyIndex, ptr);
        internal::writeBytes(&morph, sizeof(morph), ptr);
    }
}

} /* namespace pmx */
} /* namespace vpvl2 */

