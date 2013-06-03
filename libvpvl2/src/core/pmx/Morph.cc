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

struct Morph::PrivateContext {
    PrivateContext(IModel *modelRef)
        : modelRef(modelRef),
          name(0),
          englishName(0),
          weight(0),
          internalWeight(0),
          category(kBase),
          type(kUnknownMorph),
          index(-1),
          hasParent(false),
          dirty(false)
    {
    }
    ~PrivateContext() {
        vertices.releaseAll();
        uvs.releaseAll();
        bones.releaseAll();
        materials.releaseAll();
        groups.releaseAll();
        flips.releaseAll();
        impulses.releaseAll();
        delete name;
        name = 0;
        delete englishName;
        englishName = 0;
        modelRef = 0;
        weight = 0;
        internalWeight = 0;
        category = kBase;
        type = kUnknownMorph;
        index = -1;
        hasParent = false;
        dirty = false;
    }

    static bool loadBones(const Array<pmx::Bone *> &bones, Morph *morph) {
        const int nMorphBones = morph->m_context->bones.count();
        const int nbones = bones.count();
        for (int32_t i = 0; i < nMorphBones; i++) {
            Bone *bone = morph->m_context->bones[i];
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
    static bool loadGroups(const Array<Morph *> &morphs, Morph *morph) {
        const int nMorphGroups = morph->m_context->groups.count();
        const int nmorphs = morphs.count();
        for (int32_t i = 0; i < nMorphGroups; i++) {
            Group *group = morph->m_context->groups[i];
            int groupIndex = group->index;
            if (groupIndex >= 0) {
                if (groupIndex >= nmorphs) {
                    VPVL2_LOG(WARNING, "Invalid PMX group morph: index=" << i << " group=" << groupIndex);
                    return false;
                }
                else {
                    Morph *morph = morphs[groupIndex];
                    group->morph = morph;
                    morph->m_context->hasParent = true;
                }
            }
        }
        return true;
    }
    static bool loadMaterials(const Array<pmx::Material *> &materials, Morph *morph) {
        const int nMorphMaterials = morph->m_context->materials.count();
        const int nmaterials = materials.count();
        for (int i = 0; i < nMorphMaterials; i++) {
            Material *material = morph->m_context->materials[i];
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
    static bool loadUVs(const Array<pmx::Vertex *> &vertices, int offset, Morph *morph) {
        const int nMorphUVs = morph->m_context->uvs.count();
        const int nvertices = vertices.count();
        for (int i = 0; i < nMorphUVs; i++) {
            UV *uv = morph->m_context->uvs[i];
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
    static bool loadVertices(const Array<pmx::Vertex *> &vertices, Morph *morph) {
        const int nMorphVertices = morph->m_context->vertices.count();
        const int nvertices = vertices.count();
        for (int i = 0; i < nMorphVertices; i++) {
            Vertex *vertex = morph->m_context->vertices[i];
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
    static bool loadFlips(const Array<Morph *> &morphs, Morph *morph) {
        const int nMorphFlips = morph->m_context->impulses.count();
        const int nflips = morphs.count();
        for (int i = 0; i < nMorphFlips; i++) {
            Flip *flip = morph->m_context->flips[i];
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
    static bool loadImpulses(const Array<RigidBody *> &rigidBodies, Morph *morph) {
        const int nMorphImpulses = morph->m_context->impulses.count();
        const int nbodies = rigidBodies.count();
        for (int i = 0; i < nMorphImpulses; i++) {
            Impulse *impulse = morph->m_context->impulses[i];
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

    void readBones(const Model::DataInfo &info, int count, uint8_t *&ptr) {
        BoneMorph morph;
        for (int i = 0; i < count; i++) {
            Morph::Bone *bone = bones.append(new Morph::Bone());
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
    void readGroups(const Model::DataInfo &info, int count, uint8_t *&ptr) {
        GroupMorph morph;
        for (int i = 0; i < count; i++) {
            Morph::Group *group = groups.append(new Morph::Group());
            int morphIndex = internal::readSignedIndex(ptr, info.morphIndexSize);
            internal::getData(ptr, morph);
            VPVL2_VLOG(3, "PMXGroupMorph: index=" << i << " morphIndex=" << morphIndex << " weight=" << group->fixedWeight);
            group->fixedWeight = morph.weight;
            group->index = morphIndex;
            ptr += sizeof(morph);
        }
    }
    void readMaterials(const Model::DataInfo &info, int count, uint8_t *&ptr) {
        MaterialMorph morph;
        for (int i = 0; i < count; i++) {
            Morph::Material *material = materials.append(new Morph::Material());
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
    void readUVs(const Model::DataInfo &info, int count, int offset, uint8_t *&ptr) {
        UVMorph morph;
        for (int i = 0; i < count; i++) {
            Morph::UV *uv = uvs.append(new Morph::UV());
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
    void readVertices(const Model::DataInfo &info, int count, uint8_t *&ptr) {
        VertexMorph morph;
        for (int i = 0; i < count; i++) {
            Morph::Vertex *vertex = vertices.append(new Morph::Vertex());
            int vertexIndex = internal::readUnsignedIndex(ptr, info.vertexIndexSize);
            VPVL2_VLOG(3, "PMXVertexMorph: index=" << i << " vertexIndex=" << vertexIndex);
            internal::getData(ptr, morph);
            internal::setPosition(morph.position, vertex->position);
            VPVL2_VLOG(3, "PMXVertexMorph: position=" << vertex->position.x() << "," << vertex->position.y() << "," << vertex->position.z());
            vertex->index = vertexIndex;
            ptr += sizeof(morph);
        }
    }
    void readFlips(const Model::DataInfo &info, int count, uint8_t *&ptr) {
        FlipMorph morph;
        for (int i = 0; i < count; i++) {
            Morph::Flip *flip = flips.append(new Morph::Flip());
            int morphIndex = internal::readSignedIndex(ptr, info.morphIndexSize);
            internal::getData(ptr, morph);
            VPVL2_VLOG(3, "PMXFlipMorph: index=" << i << " morphIndex=" << morphIndex << " weight=" << flip->fixedWeight);
            flip->fixedWeight = morph.weight;
            flip->index = morphIndex;
            ptr += sizeof(morph);
        }
    }
    void readImpulses(const Model::DataInfo &info, int count, uint8_t *&ptr) {
        ImpulseMorph morph;
        for (int i = 0; i < count; i++) {
            Morph::Impulse *impulse = impulses.append(new Morph::Impulse());
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
    void writeBones(const Model::DataInfo &info, uint8_t *&ptr) const {
        BoneMorph morph;
        const int nbones = bones.count(), boneIndexSize = info.boneIndexSize;
        for (int i = 0; i < nbones; i++) {
            const Morph::Bone *bone = bones[i];
            internal::getPosition(bone->position, morph.position);
            internal::getRotation(bone->rotation, morph.rotation);
            internal::writeSignedIndex(bone->index, boneIndexSize, ptr);
            internal::writeBytes(&morph, sizeof(morph), ptr);
        }
    }
    void writeGroups(const Model::DataInfo &info, uint8_t *&ptr) const {
        GroupMorph morph;
        const int ngroups = groups.count(), morphIndexSize = info.morphIndexSize;
        for (int i = 0; i < ngroups; i++) {
            const Morph::Group *group = groups[i];
            morph.weight = float32_t(group->fixedWeight);
            internal::writeSignedIndex(group->index, morphIndexSize, ptr);
            internal::writeBytes(&morph, sizeof(morph), ptr);
        }
    }
    void writeMaterials(const Model::DataInfo &info, uint8_t *&ptr) const {
        MaterialMorph morph;
        const int nmaterials = materials.count(), materialIndexSize = info.materialIndexSize;
        for (int i = 0; i < nmaterials; i++) {
            const Morph::Material *material = materials[i];
            internal::getColor(material->ambient, morph.ambient);
            internal::getColor(material->diffuse, morph.diffuse);
            internal::getColor(material->edgeColor, morph.edgeColor);
            morph.operation = material->operation;
            morph.shininess = material->shininess;
            morph.edgeSize = float32_t(material->edgeSize);
            internal::getColor(material->specular, morph.specular);
            internal::getColor(material->sphereTextureWeight, morph.sphereTextureWeight);
            internal::getColor(material->textureWeight, morph.textureWeight);
            internal::getColor(material->toonTextureWeight, morph.toonTextureWeight);
            internal::writeSignedIndex(material->index, materialIndexSize, ptr);
            internal::writeBytes(&morph, sizeof(morph), ptr);
        }
    }
    void writeUVs(const Model::DataInfo &info, uint8_t *&ptr) const {
        UVMorph morph;
        const int nuvs = uvs.count(), vertexIndexSize = info.vertexIndexSize;
        for (int i = 0; i < nuvs; i++) {
            const Morph::UV *uv = uvs[i];
            const Vector4 &position = uv->position;
            morph.position[0] = position.x();
            morph.position[1] = position.y();
            morph.position[2] = position.z();
            morph.position[3] = position.w();
            internal::writeUnsignedIndex(uv->index, vertexIndexSize, ptr);
            internal::writeBytes(&morph, sizeof(morph), ptr);
        }
    }
    void writeVertices(const Model::DataInfo &info, uint8_t *&ptr) const {
        VertexMorph morph;
        const int nvertices = vertices.count(), vertexIndexSize = info.vertexIndexSize;
        for (int i = 0; i < nvertices; i++) {
            const Morph::Vertex *vertex = vertices[i];
            internal::getPosition(vertex->position, morph.position);
            internal::writeUnsignedIndex(vertex->index, vertexIndexSize, ptr);
            internal::writeBytes(&morph, sizeof(morph), ptr);
        }
    }
    void writeFlips(const Model::DataInfo &info, uint8_t *&ptr) const {
        FlipMorph morph;
        const int nflips = flips.count(), morphIndexSize = info.morphIndexSize;
        for (int i = 0; i < nflips; i++) {
            const Morph::Flip *flip = flips[i];
            morph.weight = float32_t(flip->fixedWeight);
            internal::writeSignedIndex(flip->index, morphIndexSize, ptr);
            internal::writeBytes(&morph, sizeof(morph), ptr);
        }
    }
    void writeImpulses(const Model::DataInfo &info, uint8_t *&ptr) const {
        ImpulseMorph morph;
        const int nimpulses = impulses.count(), rigidBodyIndex = info.rigidBodyIndexSize;
        for (int i = 0; i < nimpulses; i++) {
            const Morph::Impulse *impulse = impulses[i];
            internal::getPositionRaw(impulse->velocity, morph.velocity);
            internal::getPositionRaw(impulse->torque, morph.torque);
            morph.isLocal = impulse->isLocal ? 1 : 0;
            internal::writeSignedIndex(impulse->index, rigidBodyIndex, ptr);
            internal::writeBytes(&morph, sizeof(morph), ptr);
        }
    }

    PointerArray<Vertex> vertices;
    PointerArray<UV> uvs;
    PointerArray<Bone> bones;
    PointerArray<Material> materials;
    PointerArray<Group> groups;
    PointerArray<Flip> flips;
    PointerArray<Impulse> impulses;
    IModel *modelRef;
    IString *name;
    IString *englishName;
    IMorph::WeightPrecision weight;
    IMorph::WeightPrecision internalWeight;
    IMorph::Category category;
    IMorph::Type type;
    int index;
    bool hasParent;
    bool dirty;
};

Morph::Morph(IModel *modelRef)
    : m_context(0)
{
    m_context = new PrivateContext(modelRef);
}

Morph::~Morph()
{
    delete m_context;
    m_context = 0;
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
            if (!PrivateContext::loadGroups(morphs, morph)) {
                return false;
            }
            break;
        case kVertexMorph:
            if (!PrivateContext::loadVertices(vertices, morph)) {
                return false;
            }
            break;
        case kBoneMorph:
            if (!PrivateContext::loadBones(bones, morph)) {
                return false;
            }
            break;
        case kTexCoordMorph:
            if (!PrivateContext::loadUVs(vertices, 0, morph)) {
                return false;
            }
            break;
        case kUVA1Morph:
            if (!PrivateContext::loadUVs(vertices, 1, morph)) {
                return false;
            }
            break;
        case kUVA2Morph:
            if (!PrivateContext::loadUVs(vertices, 2, morph)) {
                return false;
            }
            break;
        case kUVA3Morph:
            if (!PrivateContext::loadUVs(vertices, 3, morph)) {
                return false;
            }
            break;
        case kUVA4Morph:
            if (!PrivateContext::loadUVs(vertices, 4, morph)) {
                return false;
            }
            break;
        case kMaterialMorph:
            if (!PrivateContext::loadMaterials(materials, morph)) {
                return false;
            }
            break;
        case kFlipMorph:
            if (!PrivateContext::loadFlips(morphs, morph)) {
                return false;
            }
            break;
        case kImpulseMorph:
            if (!PrivateContext::loadImpulses(rigidBodies, morph)) {
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

void Morph::read(const uint8_t *data, const Model::DataInfo &info, size_t &size)
{
    uint8_t *namePtr, *ptr = const_cast<uint8_t *>(data), *start = ptr;
    size_t rest = SIZE_MAX;
    int32_t nNameSize;
    internal::getText(ptr, rest, namePtr, nNameSize);
    IEncoding *encoding = info.encoding;
    internal::setStringDirect(encoding->toString(namePtr, nNameSize, info.codec), m_context->name);
    VPVL2_VLOG(3, "PMXMorph: name=" << internal::cstr(m_context->name, "(null)"));
    internal::getText(ptr, rest, namePtr, nNameSize);
    internal::setStringDirect(encoding->toString(namePtr, nNameSize, info.codec), m_context->englishName);
    VPVL2_VLOG(3, "PMXMorph: englishName=" << internal::cstr(m_context->englishName, "(null)"));
    MorphUnit unit;
    internal::getData(ptr, unit);
    m_context->category = static_cast<Category>(unit.category);
    m_context->type = static_cast<Type>(unit.type);
    VPVL2_VLOG(3, "PMXMorph: category=" << m_context->category << " type=" << m_context->type << " size=" << unit.size);
    ptr += sizeof(unit);
    switch (m_context->type) {
    case kGroupMorph:
        m_context->readGroups(info, unit.size, ptr);
        break;
    case kVertexMorph:
        m_context->readVertices(info, unit.size, ptr);
        break;
    case kBoneMorph:
        m_context->readBones(info, unit.size, ptr);
        break;
    case kTexCoordMorph:
    case kUVA1Morph:
    case kUVA2Morph:
    case kUVA3Morph:
    case kUVA4Morph:
        m_context->readUVs(info, unit.size, m_context->type - kTexCoordMorph, ptr);
        break;
    case kMaterialMorph:
        m_context->readMaterials(info, unit.size, ptr);
        break;
    case kFlipMorph:
        m_context->readFlips(info, unit.size, ptr);
        break;
    case kImpulseMorph:
        m_context->readImpulses(info, unit.size, ptr);
        break;
    default:
        break; /* should not reach here */
    }
    size = ptr - start;
}

void Morph::write(uint8_t *&data, const Model::DataInfo &info) const
{
    internal::writeString(m_context->name, info.codec, data);
    internal::writeString(m_context->englishName, info.codec, data);
    MorphUnit mu;
    mu.category = m_context->category;
    mu.type = m_context->type;
    switch (m_context->type) {
    case kGroupMorph:
        mu.size = m_context->groups.count();
        internal::writeBytes(&mu, sizeof(mu), data);
        m_context->writeGroups(info, data);
        break;
    case kVertexMorph:
        mu.size = m_context->vertices.count();
        internal::writeBytes(&mu, sizeof(mu), data);
        m_context->writeVertices(info, data);
        break;
    case kBoneMorph:
        mu.size = m_context->bones.count();
        internal::writeBytes(&mu, sizeof(mu), data);
        m_context->writeBones(info, data);
        break;
    case kTexCoordMorph:
    case kUVA1Morph:
    case kUVA2Morph:
    case kUVA3Morph:
    case kUVA4Morph:
        mu.size = m_context->uvs.count();
        internal::writeBytes(&mu, sizeof(mu), data);
        m_context->writeUVs(info, data);
        break;
    case kMaterialMorph:
        mu.size = m_context->materials.count();
        internal::writeBytes(&mu, sizeof(mu), data);
        m_context->writeMaterials(info, data);
        break;
    case kFlipMorph:
        mu.size = m_context->flips.count();
        internal::writeBytes(&mu, sizeof(mu), data);
        m_context->writeFlips(info, data);
        break;
    case kImpulseMorph:
        mu.size = m_context->impulses.count();
        internal::writeBytes(&mu, sizeof(mu), data);
        m_context->writeImpulses(info, data);
        break;
    default:
        break; /* should not reach here */
    }
}

size_t Morph::estimateSize(const Model::DataInfo &info) const
{
    size_t size = 0;
    size += internal::estimateSize(m_context->name, info.codec);
    size += internal::estimateSize(m_context->englishName, info.codec);
    size += sizeof(MorphUnit);
    switch (m_context->type) {
    case kGroupMorph:
        size += m_context->groups.count() * (sizeof(GroupMorph) + info.morphIndexSize);
        break;
    case kVertexMorph:
        size += m_context->vertices.count() * (sizeof(VertexMorph) + info.vertexIndexSize);
        break;
    case kBoneMorph:
        size += m_context->bones.count() * (sizeof(BoneMorph) + info.boneIndexSize);
        break;
    case kTexCoordMorph:
    case kUVA1Morph:
    case kUVA2Morph:
    case kUVA3Morph:
    case kUVA4Morph:
        size += m_context->uvs.count() * (sizeof(UVMorph) + info.vertexIndexSize);
        break;
    case kMaterialMorph:
        size += m_context->materials.count() * (sizeof(MaterialMorph) + info.materialIndexSize);
        break;
    case kFlipMorph:
        size += m_context->flips.count() * (sizeof(FlipMorph) + info.morphIndexSize);
        break;
    case kImpulseMorph:
        size += m_context->impulses.count() * (sizeof(ImpulseMorph) + info.rigidBodyIndexSize);
        break;
    default:
        return 0; /* should not reach here */
    }
    return size;
}

IMorph::WeightPrecision Morph::weight() const
{
    return m_context->weight;
}

void Morph::setWeight(const IMorph::WeightPrecision &value)
{
    m_context->weight = value;
    m_context->dirty = true;
}

void Morph::update()
{
    Type type = m_context->type;
    if (type == kGroupMorph) {
        /* force updating group morph to update morph children correctly even weight is not changed (not dirty) */
        updateGroupMorphs(m_context->internalWeight, false);
    }
    else if (m_context->dirty) {
        switch (type) {
        case kVertexMorph:
            updateVertexMorphs(m_context->internalWeight);
            break;
        case kBoneMorph:
            updateBoneMorphs(m_context->internalWeight);
            break;
        case kTexCoordMorph:
        case kUVA1Morph:
        case kUVA2Morph:
        case kUVA3Morph:
        case kUVA4Morph:
            updateUVMorphs(m_context->internalWeight);
            break;
        case kMaterialMorph:
            updateMaterialMorphs(m_context->internalWeight);
            break;
        case kGroupMorph:
        case kFlipMorph:
            /* do nothing */
            break;
        case kImpulseMorph:
            updateImpluseMorphs(m_context->internalWeight);
            break;
        default:
            break; /* should not reach here */
        }
        m_context->dirty = false;
    }
}

void Morph::syncWeight()
{
    if (m_context->dirty) {
        switch (m_context->type) {
        case kGroupMorph:
            updateGroupMorphs(m_context->weight, true);
            break;
        case kFlipMorph:
            updateFlipMorphs(m_context->weight);
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
            break;
        }
        setInternalWeight(m_context->weight);
    }
}

void Morph::updateVertexMorphs(const WeightPrecision &value)
{
    const int nmorphs = m_context->vertices.count();
    for (int i = 0; i < nmorphs; i++) {
        Vertex *v = m_context->vertices[i];
        if (pmx::Vertex *vertex = v->vertex) {
            vertex->mergeMorph(v, value);
        }
    }
}

void Morph::updateBoneMorphs(const WeightPrecision &value)
{
    const int nmorphs = m_context->bones.count();
    for (int i = 0; i < nmorphs; i++) {
        Bone *v = m_context->bones[i];
        if (pmx::Bone *bone = v->bone) {
            bone->mergeMorph(v, value);
        }
    }
}

void Morph::updateUVMorphs(const WeightPrecision &value)
{
    const int nmorphs = m_context->uvs.count();
    for (int i = 0; i < nmorphs; i++) {
        UV *v = m_context->uvs[i];
        if (pmx::Vertex *vertex = v->vertex) {
            vertex->mergeMorph(v, value);
        }
    }
}

void Morph::updateMaterialMorphs(const WeightPrecision &value)
{
    const int nmorphs = m_context->materials.count();
    for (int i = 0; i < nmorphs; i++) {
        Material *v = m_context->materials.at(i);
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
    const int nmorphs = m_context->groups.count();
    for (int i = 0; i < nmorphs; i++) {
        Group *v = m_context->groups[i];
        if (Morph *morph = v->morph) {
            bool isFlipMorph = morph->type() == Morph::kFlipMorph;
            if (isFlipMorph == flipOnly) {
                if (morph != this) {
                    morph->setInternalWeight(v->fixedWeight * value);
                    morph->update();
                }
            }
        }
    }
}

void Morph::updateFlipMorphs(const WeightPrecision &value)
{
    const int nmorphs = m_context->flips.count();
    if (nmorphs > 0) {
        const WeightPrecision &weight = btClamped(value, WeightPrecision(0.0), WeightPrecision(1.0));
        int index = int((nmorphs + 1) * weight) - 1;
        const Flip *flip = m_context->flips.at(index);
        if (Morph *morph = flip->morph) {
            if (morph != this) {
                morph->setInternalWeight(flip->fixedWeight);
                morph->update();
            }
        }
    }
}

void Morph::updateImpluseMorphs(const WeightPrecision &value)
{
    const int nmorphs = m_context->impulses.count();
    for (int i = 0; i < nmorphs; i++) {
        Impulse *impulse = m_context->impulses.at(i);
        if (RigidBody *rigidBody = impulse->rigidBody) {
            rigidBody->mergeMorph(impulse, value);
        }
    }
}

const IString *Morph::name() const
{
    return m_context->name;
}

const IString *Morph::englishName() const
{
    return m_context->englishName;
}

IModel *Morph::parentModelRef() const
{
    return m_context->modelRef;
}

IMorph::Category Morph::category() const
{
    return m_context->category;
}

IMorph::Type Morph::type() const
{
    return m_context->type;
}

int Morph::index() const
{
    return m_context->index;
}

bool Morph::hasParent() const
{
    return m_context->hasParent;
}

const Array<Morph::Bone *> &Morph::bones() const
{
    return m_context->bones;
}

const Array<Morph::Group *> &Morph::groups() const
{
    return m_context->groups;
}

const Array<Morph::Material *> &Morph::materials() const
{
    return m_context->materials;
}

const Array<Morph::UV *> &Morph::uvs() const
{
    return m_context->uvs;
}

const Array<Morph::Vertex *> &Morph::vertices() const
{
    return m_context->vertices;
}

const Array<Morph::Flip *> &Morph::flips() const
{
    return m_context->flips;
}

const Array<Morph::Impulse *> &Morph::impulses() const
{
    return m_context->impulses;
}

void Morph::setName(const IString *value)
{
    internal::setString(value, m_context->name);
}

void Morph::setEnglishName(const IString *value)
{
    internal::setString(value, m_context->englishName);
}

void Morph::addBoneMorph(Bone *value)
{
    m_context->bones.append(value);
}

void Morph::addGroupMorph(Group *value)
{
    m_context->groups.append(value);
}

void Morph::addMaterialMorph(Material *value)
{
    m_context->materials.append(value);
}

void Morph::addUVMorph(UV *value)
{
    m_context->uvs.append(value);
}

void Morph::addVertexMorph(Vertex *value)
{
    m_context->vertices.append(value);
}

void Morph::addFlip(Flip *value)
{
    m_context->flips.append(value);
}

void Morph::addImpulse(Impulse *value)
{
    m_context->impulses.append(value);
}

void Morph::setCategory(Category value)
{
    m_context->category = value;
}

void Morph::setType(Type value)
{
    m_context->type = value;
}

void Morph::setIndex(int value)
{
    m_context->index = value;
}

void Morph::setInternalWeight(const WeightPrecision &value)
{
    m_context->internalWeight = value;
    m_context->dirty = true;
}

} /* namespace pmx */
} /* namespace vpvl2 */

