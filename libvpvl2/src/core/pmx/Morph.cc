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

#include "vpvl2/pmx/Bone.h"
#include "vpvl2/pmx/Material.h"
#include "vpvl2/pmx/Morph.h"
#include "vpvl2/pmx/RigidBody.h"
#include "vpvl2/pmx/Vertex.h"

namespace
{

#pragma pack(push, 1)

struct MorphUnit {
    uint8_t category;
    uint8_t type;
    int size;
};

struct VertexMorph {
    float position[3];
};

struct UVMorph {
    float position[4];
};

struct BoneMorph {
    float position[3];
    float rotation[4];
};

struct MaterialMorph {
    uint8_t operation;
    float diffuse[4];
    float specular[3];
    float shininess;
    float ambient[3];
    float edgeColor[4];
    float edgeSize;
    float textureWeight[4];
    float sphereTextureWeight[4];
    float toonTextureWeight[4];
};

struct GroupMorph {
    float weight;
};

struct FlipMorph {
    float weight;
};

struct ImpulseMorph {
    uint8_t isLocal;
    float velocity[3];
    float torque[3];
};

#pragma pack(pop)

}

namespace vpvl2
{
namespace pmx
{

Morph::Morph()
    : m_name(0),
      m_englishName(0),
      m_weight(0),
      m_category(kBase),
      m_type(kUnknown),
      m_index(-1),
      m_hasParent(false)
{
}

Morph::~Morph()
{
    m_impulses.releaseAll();
    m_flips.releaseAll();
    m_vertices.releaseAll();
    m_uvs.releaseAll();
    m_bones.releaseAll();
    m_materials.releaseAll();
    m_groups.releaseAll();
    delete m_name;
    m_name = 0;
    delete m_englishName;
    m_englishName = 0;
    m_category = kBase;
    m_type = kUnknown;
    m_index = -1;
    m_hasParent = false;
}

bool Morph::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    size_t size;
    if (!internal::size32(ptr, rest, size)) {
        return false;
    }
    info.morphsPtr = ptr;
    MorphUnit morph;
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
        if (sizeof(MorphUnit) > rest) {
            return false;
        }
        internal::getData(ptr, morph);
        internal::readBytes(sizeof(MorphUnit), ptr, rest);
        int nmorphs = morph.size;
        size_t extraSize;
        switch (static_cast<Type>(morph.type)) {
        case kGroup:
            extraSize = info.morphIndexSize + sizeof(GroupMorph);
            break;
        case kVertex:
            extraSize = info.vertexIndexSize + sizeof(VertexMorph);
            break;
        case kBone:
            extraSize = info.boneIndexSize + sizeof(BoneMorph);
            break;
        case kTexCoord:
        case kUVA1:
        case kUVA2:
        case kUVA3:
        case kUVA4:
            extraSize = info.vertexIndexSize + sizeof(UVMorph);
            break;
        case kMaterial:
            extraSize = info.materialIndexSize + sizeof(MaterialMorph);
            break;
        case kFlip:
            extraSize = info.morphIndexSize + sizeof(FlipMorph);
            break;
        case kImpulse:
            extraSize = info.rigidBodyIndexSize + sizeof(ImpulseMorph);
            break;
        default:
            return false;
        }
        for (int j = 0; j < nmorphs; j++) {
            if (!internal::validateSize(ptr, extraSize, rest)) {
                return false;
            }
        }
    }
    info.morphsCount = size;
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
        switch (morph->m_type) {
        case kGroup:
            if (!loadGroups(morphs, morph)) {
                return false;
            }
            break;
        case kVertex:
            if (!loadVertices(vertices, morph)) {
                return false;
            }
            break;
        case kBone:
            if (!loadBones(bones, morph)) {
                return false;
            }
            break;
        case kTexCoord:
            if (!loadUVs(vertices, 0, morph)) {
                return false;
            }
            break;
        case kUVA1:
            if (!loadUVs(vertices, 1, morph)) {
                return false;
            }
            break;
        case kUVA2:
            if (!loadUVs(vertices, 2, morph)) {
                return false;
            }
            break;
        case kUVA3:
            if (!loadUVs(vertices, 3, morph)) {
                return false;
            }
            break;
        case kUVA4:
            if (!loadUVs(vertices, 4, morph)) {
                return false;
            }
            break;
        case kMaterial:
            if (!loadMaterials(materials, morph)) {
                return false;
            }
            break;
        case kFlip:
            /* do nothing */
            break;
        case kImpulse:
            if (!loadImpulses(rigidBodies, morph)) {
                return false;
            }
            break;
        default:
            return false;
        }
        morph->m_index = i;
    }
    return true;
}

size_t Morph::estimateTotalSize(const Array<Morph *> &morphs, const Model::DataInfo &info)
{
    const int nmorphs = morphs.count();
    size_t size = 0;
    size += sizeof(nmorphs);
    for (int i = 0; i < nmorphs; i++) {
        Morph *morph = morphs[i];
        size += morph->estimateSize(info);
    }
    return size;
}

bool Morph::loadBones(const Array<pmx::Bone *> &bones, Morph *morph)
{
    const int nMorphBones = morph->m_bones.count();
    const int nbones = bones.count();
    for (int i = 0; i < nMorphBones; i++) {
        Bone *bone = morph->m_bones[i];
        int boneIndex = bone->index;
        if (boneIndex >= 0) {
            if (boneIndex >= nbones) {
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
    for (int i = 0; i < nMorphGroups; i++) {
        Group *group = morph->m_groups[i];
        int groupIndex = group->index;
        if (groupIndex >= 0) {
            if (groupIndex >= nmorphs) {
                return false;
            }
            else {
                Morph *morph = morphs[groupIndex];
                if (morph->m_type == kGroup) {
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
                return false;
            }
            else {
                material->materials->add(materials[materialIndex]);
            }
        }
        else {
            const int nmaterials = materials.count();
            for (int j = 0; j < nmaterials; j++) {
                pmx::Material *m = materials[j];
                material->materials->add(m);
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
                return false;
            }
            else {
                vertex->vertex = vertices[vertexIndex];
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
    size_t nNameSize, rest = SIZE_MAX;
    internal::sizeText(ptr, rest, namePtr, nNameSize);
    IEncoding *encoding = info.encoding;
    internal::setStringDirect(encoding->toString(namePtr, nNameSize, info.codec), m_name);
    internal::sizeText(ptr, rest, namePtr, nNameSize);
    internal::setStringDirect(encoding->toString(namePtr, nNameSize, info.codec), m_englishName);
    MorphUnit unit;
    internal::getData(ptr, unit);
    m_category = static_cast<Category>(unit.category);
    m_type = static_cast<Type>(unit.type);
    ptr += sizeof(unit);
    switch (m_type) {
    case kGroup:
        readGroups(info, unit.size, ptr);
        break;
    case kVertex:
        readVertices(info, unit.size, ptr);
        break;
    case kBone:
        readBones(info, unit.size, ptr);
        break;
    case kTexCoord:
    case kUVA1:
    case kUVA2:
    case kUVA3:
    case kUVA4:
        readUVs(info, unit.size, m_type - kTexCoord, ptr);
        break;
    case kMaterial:
        readMaterials(info, unit.size, ptr);
        break;
    case kFlip:
        readFlips(info, unit.size, ptr);
        break;
    case kImpulse:
        readImpulses(info, unit.size, ptr);
        break;
    default:
        assert(0);
    }
    size = ptr - start;
}

void Morph::write(uint8_t *data, const Model::DataInfo &info) const
{
    internal::writeString(m_name, info.codec, data);
    internal::writeString(m_englishName, info.codec, data);
    MorphUnit mu;
    mu.category = m_category;
    mu.type = m_type;
    switch (m_type) {
    case kGroup:
        mu.size = m_groups.count();
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&mu), sizeof(mu), data);
        writeGroups(info, data);
        break;
    case kVertex:
        mu.size = m_vertices.count();
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&mu), sizeof(mu), data);
        writeVertices(info, data);
        break;
    case kBone:
        mu.size = m_bones.count();
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&mu), sizeof(mu), data);
        writeBones(info, data);
        break;
    case kTexCoord:
    case kUVA1:
    case kUVA2:
    case kUVA3:
    case kUVA4:
        mu.size = m_uvs.count();
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&mu), sizeof(mu), data);
        writeUVs(info, data);
        break;
    case kMaterial:
        mu.size = m_materials.count();
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&mu), sizeof(mu), data);
        writeMaterials(info, data);
        break;
    case kFlip:
        mu.size = m_flips.count();
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&mu), sizeof(mu), data);
        writeFlips(info, data);
        break;
    case kImpulse:
        mu.size = m_impulses.count();
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&mu), sizeof(mu), data);
        writeImpulses(info, data);
        break;
    default:
        assert(0);
    }
}

size_t Morph::estimateSize(const Model::DataInfo &info) const
{
    size_t size = 0;
    size += internal::estimateSize(m_name, info.codec);
    size += internal::estimateSize(m_englishName, info.codec);
    size += sizeof(MorphUnit);
    switch (m_type) {
    case kGroup:
        size += m_groups.count() * (sizeof(GroupMorph) + info.morphIndexSize);
        break;
    case kVertex:
        size += m_vertices.count() * (sizeof(VertexMorph) + info.vertexIndexSize);
        break;
    case kBone:
        size += m_bones.count() * (sizeof(BoneMorph) + info.boneIndexSize);
        break;
    case kTexCoord:
    case kUVA1:
    case kUVA2:
    case kUVA3:
    case kUVA4:
        size += m_uvs.count() * (sizeof(UVMorph) + info.vertexIndexSize);
        break;
    case kMaterial:
        size += m_materials.count() * (sizeof(MaterialMorph) + info.materialIndexSize);
        break;
    case kFlip:
        size += m_flips.count() * (sizeof(FlipMorph) + info.morphIndexSize);
        break;
    case kImpulse:
        size += m_impulses.count() * (sizeof(ImpulseMorph) + info.rigidBodyIndexSize);
        break;
    default:
        assert(0);
        return 0;
    }
    return size;
}

void Morph::setWeight(const IMorph::WeightPrecision &value)
{
    m_weight = value;
    int nmorphs;
    switch (m_type) {
    case kGroup:
        nmorphs = m_groups.count();
        for (int i = 0; i < nmorphs; i++) {
            Group *v = m_groups[i];
            Morph *morph = v->morph;
            if (morph) {
                morph->setWeight(v->weight * value);
            }
        }
        break;
    case kVertex:
        nmorphs = m_vertices.count();
        for (int i = 0; i < nmorphs; i++) {
            Vertex *v = m_vertices[i];
            pmx::Vertex *vertex = v->vertex;
            if (vertex) {
                vertex->mergeMorph(v, value);
            }
        }
        break;
    case kBone:
        nmorphs = m_bones.count();
        for (int i = 0; i < nmorphs; i++) {
            Bone *v = m_bones[i];
            pmx::Bone *bone = v->bone;
            if (bone) {
                bone->mergeMorph(v, value);
            }
        }
        break;
    case kTexCoord:
    case kUVA1:
    case kUVA2:
    case kUVA3:
    case kUVA4:
        nmorphs = m_uvs.count();
        for (int i = 0; i < nmorphs; i++) {
            UV *v = m_uvs[i];
            pmx::Vertex *vertex = v->vertex;
            if (vertex) {
                vertex->mergeMorph(v, value);
            }
        }
        break;
    case kMaterial:
        nmorphs = m_materials.count();
        for (int i = 0; i < nmorphs; i++) {
            Material *v = m_materials.at(i);
            const Array<pmx::Material *> *materials = v->materials;
            const int nmaterials = materials->count();
            for (int j = 0; j < nmaterials; j++) {
                pmx::Material *material = materials->at(j);
                material->mergeMorph(v, value);
            }
        }
        break;
    case kFlip:
        nmorphs = m_flips.count();
        for (int i = 0; i < nmorphs; i++) {
            Flip *v = m_flips.at(i);
            (void) v;
            // TODO
        }
        break;
    case kImpulse:
        nmorphs = m_impulses.count();
        for (int i = 0; i < nmorphs; i++) {
            Impulse *impulse = m_impulses.at(i);
            RigidBody *rigidBody = impulse->rigidBody;
            if (rigidBody) {
                rigidBody->mergeMorph(impulse, value);
            }
        }
        break;
    default:
        assert(0);
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
    m_bones.add(value);
}

void Morph::addGroupMorph(Group *value)
{
    m_groups.add(value);
}

void Morph::addMaterialMorph(Material *value)
{
    m_materials.add(value);
}

void Morph::addUVMorph(UV *value)
{
    m_uvs.add(value);
}

void Morph::addVertexMorph(Vertex *value)
{
    m_vertices.add(value);
}

void Morph::addFlip(Flip *value)
{
    m_flips.add(value);
}

void Morph::addImpulse(Impulse *value)
{
    m_impulses.add(value);
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

void Morph::readBones(const Model::DataInfo &info, int count, uint8_t *&ptr)
{
    BoneMorph morph;
    for (int i = 0; i < count; i++) {
        Morph::Bone *bone = new Morph::Bone();
        addBoneMorph(bone);
        int boneIndex = internal::readSignedIndex(ptr, info.boneIndexSize);
        internal::getData(ptr, morph);
        internal::setPosition(morph.position, bone->position);
        internal::setRotation(morph.rotation, bone->rotation);
        bone->index = boneIndex;
        ptr += sizeof(morph);
    }
}

void Morph::readGroups(const Model::DataInfo &info, int count, uint8_t *&ptr)
{
    GroupMorph morph;
    for (int i = 0; i < count; i++) {
        Morph::Group *group = new Morph::Group();
        addGroupMorph(group);
        int morphIndex = internal::readSignedIndex(ptr, info.morphIndexSize);
        internal::getData(ptr, morph);
        group->weight = morph.weight;
        group->index = morphIndex;
        ptr += sizeof(morph);
    }
}

void Morph::readMaterials(const Model::DataInfo &info, int count, uint8_t *&ptr)
{
    MaterialMorph morph;
    for (int i = 0; i < count; i++) {
        Morph::Material *material = new Morph::Material();
        addMaterialMorph(material);
        int materialIndex = internal::readSignedIndex(ptr, info.materialIndexSize);
        internal::getData(ptr, morph);
        material->materials = new Array<pmx::Material *>();
        material->ambient.setValue(morph.ambient[0], morph.ambient[1], morph.ambient[2]);
        material->diffuse.setValue(morph.diffuse[0], morph.diffuse[1], morph.diffuse[2], morph.diffuse[3]);
        material->edgeColor.setValue(morph.edgeColor[0], morph.edgeColor[1], morph.edgeColor[2], morph.edgeColor[3]);
        material->edgeSize = morph.edgeSize;
        material->index = materialIndex;
        material->operation = morph.operation;
        material->shininess = morph.shininess;
        material->specular.setValue(morph.specular[0], morph.specular[1], morph.specular[2]);
        material->sphereTextureWeight.setValue(morph.sphereTextureWeight[0], morph.sphereTextureWeight[1],
                                               morph.sphereTextureWeight[2], morph.sphereTextureWeight[3]);
        material->textureWeight.setValue(morph.textureWeight[0], morph.textureWeight[1],
                                         morph.textureWeight[2], morph.textureWeight[3]);
        material->toonTextureWeight.setValue(morph.toonTextureWeight[0], morph.toonTextureWeight[1],
                                             morph.toonTextureWeight[2], morph.toonTextureWeight[3]);
        ptr += sizeof(morph);
    }
}

void Morph::readUVs(const Model::DataInfo &info, int count, int offset, uint8_t *&ptr)
{
    UVMorph morph;
    for (int i = 0; i < count; i++) {
        Morph::UV *uv = new Morph::UV();
        addUVMorph(uv);
        int vertexIndex = internal::readUnsignedIndex(ptr, info.vertexIndexSize);
        internal::getData(ptr, morph);
        uv->position.setValue(morph.position[0], morph.position[1], morph.position[2], morph.position[3]);
        uv->index = vertexIndex;
        uv->offset = offset;
        ptr += sizeof(morph);
    }
}

void Morph::readVertices(const Model::DataInfo &info, int count, uint8_t *&ptr)
{
    VertexMorph morph;
    for (int i = 0; i < count; i++) {
        Morph::Vertex *vertex = new Morph::Vertex();
        addVertexMorph(vertex);
        int vertexIndex = internal::readUnsignedIndex(ptr, info.vertexIndexSize);
        internal::getData(ptr, morph);
        internal::setPosition(morph.position, vertex->position);
        vertex->index = vertexIndex;
        ptr += sizeof(morph);
    }
}

void Morph::readFlips(const Model::DataInfo &info, int count, uint8_t *&ptr)
{
    FlipMorph morph;
    for (int i = 0; i < count; i++) {
        Morph::Flip *flip = new Morph::Flip();
        addFlip(flip);
        int morphIndex = internal::readSignedIndex(ptr, info.morphIndexSize);
        internal::getData(ptr, morph);
        flip->weight = morph.weight;
        flip->index = morphIndex;
        ptr += sizeof(morph);
    }
}

void Morph::readImpulses(const Model::DataInfo &info, int count, uint8_t *&ptr)
{
    ImpulseMorph morph;
    for (int i = 0; i < count; i++) {
        Morph::Impulse *impulse = new Morph::Impulse();
        addImpulse(impulse);
        int rigidBodyIndex = internal::readSignedIndex(ptr, info.rigidBodyIndexSize);
        internal::getData(ptr, morph);
        internal::setPositionRaw(morph.velocity, impulse->velocity);
        internal::setPositionRaw(morph.torque, impulse->torque);
        impulse->isLocal = morph.isLocal != 0;
        impulse->index = rigidBodyIndex;
        ptr += sizeof(morph);
    }
}

void Morph::writeBones(const Model::DataInfo &info, uint8_t *&ptr) const
{
    BoneMorph morph;
    int nbones = m_bones.count(), boneIndexSize = info.boneIndexSize;
    for (int i = 0; i < nbones; i++) {
        const Morph::Bone *bone = m_bones[i];
        internal::getPosition(bone->position, morph.position);
        internal::getRotation(bone->rotation, morph.rotation);
        internal::writeSignedIndex(bone->index, boneIndexSize, ptr);
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&morph), sizeof(morph), ptr);
    }
}

void Morph::writeGroups(const Model::DataInfo &info, uint8_t *&ptr) const
{
    GroupMorph morph;
    int ngroups = m_groups.count(), morphIndexSize = info.morphIndexSize;
    for (int i = 0; i < ngroups; i++) {
        const Morph::Group *group = m_groups[i];
        morph.weight = group->weight;
        internal::writeSignedIndex(group->index, morphIndexSize, ptr);
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&morph), sizeof(morph), ptr);
    }
}

void Morph::writeMaterials(const Model::DataInfo &info, uint8_t *&ptr) const
{
    MaterialMorph morph;
    int nmaterials = m_materials.count(), materialIndexSize = info.materialIndexSize;
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
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&morph), sizeof(morph), ptr);
    }
}

void Morph::writeUVs(const Model::DataInfo &info, uint8_t *&ptr) const
{
    UVMorph morph;
    int nuvs = m_uvs.count(), vertexIndexSize = info.vertexIndexSize;
    for (int i = 0; i < nuvs; i++) {
        const Morph::UV *uv = m_uvs[i];
        const Vector4 &position = uv->position;
        morph.position[0] = position.x();
        morph.position[1] = position.y();
        morph.position[2] = position.z();
        morph.position[3] = position.w();
        internal::writeUnsignedIndex(uv->index, vertexIndexSize, ptr);
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&morph), sizeof(morph), ptr);
    }
}

void Morph::writeVertices(const Model::DataInfo &info, uint8_t *&ptr) const
{
    VertexMorph morph;
    int nvertices = m_vertices.count(), vertexIndexSize = info.vertexIndexSize;
    for (int i = 0; i < nvertices; i++) {
        const Morph::Vertex *vertex = m_vertices[i];
        internal::getPosition(vertex->position, morph.position);
        internal::writeUnsignedIndex(vertex->index, vertexIndexSize, ptr);
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&morph), sizeof(morph), ptr);
    }
}

void Morph::writeFlips(const Model::DataInfo &info, uint8_t *&ptr) const
{
    FlipMorph morph;
    int nflips = m_flips.count(), morphIndexSize = info.morphIndexSize;
    for (int i = 0; i < nflips; i++) {
        const Morph::Flip *flip = m_flips[i];
        morph.weight = flip->weight;
        internal::writeSignedIndex(flip->index, morphIndexSize, ptr);
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&morph), sizeof(morph), ptr);
    }
}

void Morph::writeImpulses(const Model::DataInfo &info, uint8_t *&ptr) const
{
    ImpulseMorph morph;
    int nimpulses = m_impulses.count(), rigidBodyIndex = info.rigidBodyIndexSize;
    for (int i = 0; i < nimpulses; i++) {
        const Morph::Impulse *impulse = m_impulses[i];
        internal::getPositionRaw(impulse->velocity, morph.velocity);
        internal::getPositionRaw(impulse->torque, morph.torque);
        morph.isLocal = impulse->isLocal ? 1 : 0;
        internal::writeSignedIndex(impulse->index, rigidBodyIndex, ptr);
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&morph), sizeof(morph), ptr);
    }
}

} /* namespace pmx */
} /* namespace vpvl2 */

