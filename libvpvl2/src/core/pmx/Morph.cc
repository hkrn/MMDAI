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

#pragma pack(pop)

}

namespace vpvl2
{
namespace pmx
{

Morph::Morph()
    : m_name(0),
      m_englishName(0),
      m_index(-1),
      m_category(0),
      m_type(0)
{
}

Morph::~Morph()
{
    delete m_name;
    m_name = 0;
    delete m_englishName;
    m_englishName = 0;
    m_index = -1;
    m_category = 0;
    m_type = 0;
}

bool Morph::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    size_t size;
    if (!internal::size32(ptr, rest, size)) {
        return false;
    }
    info.morphsPtr = ptr;
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
        MorphUnit *morph = reinterpret_cast<MorphUnit *>(ptr);
        internal::readBytes(sizeof(MorphUnit), ptr, rest);
        int nmorphs = morph->size;
        size_t extraSize;
        switch (morph->type) {
        case 0: /* group */
            extraSize = info.morphIndexSize + sizeof(GroupMorph);
            break;
        case 1: /* vertex */
            extraSize = info.vertexIndexSize + sizeof(VertexMorph);
            break;
        case 2: /* bone */
            extraSize = info.boneIndexSize + sizeof(BoneMorph);
            break;
        case 3: /* UV */
        case 4: /* UV1 */
        case 5: /* UV2 */
        case 6: /* UV3 */
        case 7: /* UV4 */
            extraSize = info.vertexIndexSize + sizeof(UVMorph);
            break;
        case 8: /* material */
            extraSize = info.materialIndexSize + sizeof(MaterialMorph);
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
                       const Array<pmx::Vertex *> &vertices)
{
    const int nmorphs = morphs.count();
    for (int i = 0; i < nmorphs; i++) {
        Morph *morph = morphs[i];
        switch (morph->m_type) {
        case 0: /* group */
            if (!loadGroups(morphs, morph))
                return false;
            break;
        case 1: /* vertex */
            if (!loadVertices(vertices, morph))
                return false;
            break;
        case 2: /* bone */
            if (!loadBones(bones, morph))
                return false;
            break;
        case 3: /* UV */
            if (!loadUVs(vertices, 0, morph))
                return false;
            break;
        case 4: /* UV1 */
            if (!loadUVs(vertices, 1, morph))
                return false;
            break;
        case 5: /* UV2 */
            if (!loadUVs(vertices, 2, morph))
                return false;
            break;
        case 6: /* UV3 */
            if (!loadUVs(vertices, 3, morph))
                return false;
            break;
        case 7: /* UV4 */
            if (!loadUVs(vertices, 4, morph))
                return false;
            break;
        case 8: /* material */
            if (!loadMaterials(materials, morph))
                return false;
            break;
        default:
            return false;
        }
        morph->m_index = i;
    }
    return true;
}

bool Morph::loadBones(const Array<pmx::Bone *> &bones, Morph *morph)
{
    const int nMorphBones = morph->m_bones.count();
    const int nbones = bones.count();
    for (int i = 0; i < nMorphBones; i++) {
        Bone &bone = morph->m_bones[i];
        int boneIndex = bone.index;
        if (boneIndex >= 0) {
            if (boneIndex >= nbones)
                return false;
            else
                bone.bone = bones[boneIndex];
        }
    }
    return true;
}

bool Morph::loadGroups(const Array<Morph *> &morphs, Morph *morph)
{
    const int nMorphGroups = morph->m_groups.count();
    const int nmorphs = morphs.count();
    for (int i = 0; i < nMorphGroups; i++) {
        Group &group = morph->m_groups[i];
        int groupIndex = group.index;
        if (groupIndex >= 0) {
            if (groupIndex >= nmorphs)
                return false;
            else
                group.morph = &morph[groupIndex];
        }
    }
    return true;
}

bool Morph::loadMaterials(const Array<pmx::Material *> &materials, Morph *morph)
{
    const int nMorphMaterials = morph->m_materials.count();
    const int nmaterials = materials.count();
    for (int i = 0; i < nMorphMaterials; i++) {
        Material &material = morph->m_materials[i];
        int materialIndex = material.index;
        if (materialIndex >= 0) {
            if (materialIndex >= nmaterials)
                return false;
            else
                material.material = materials[materialIndex];
        }
    }
    return true;
}

bool Morph::loadUVs(const Array<pmx::Vertex *> &uv, int /* offset */, Morph *morph)
{
    const int nMorphVertices = morph->m_vertices.count();
    const int nvertices = uv.count();
    for (int i = 0; i < nMorphVertices; i++) {
        Vertex &vertex = morph->m_vertices[i];
        int vertexIndex = vertex.index;
        if (vertexIndex >= 0) {
            if (vertexIndex >= nvertices)
                return false;
            else
                vertex.vertex = uv[vertexIndex];
        }
    }
    return true;
}

bool Morph::loadVertices(const Array<pmx::Vertex *> &vertices, Morph *morph)
{
    const int nMorphVertices = morph->m_vertices.count();
    const int nvertices = vertices.count();
    for (int i = 0; i < nMorphVertices; i++) {
        Vertex &vertex = morph->m_vertices[i];
        int vertexIndex = vertex.index;
        if (vertexIndex >= 0) {
            if (vertexIndex >= nvertices)
                return false;
            else
                vertex.vertex = vertices[vertexIndex];
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
    IString *string = 0;
    string = encoding->toString(namePtr, nNameSize, info.codec);
    setName(string);
    delete string;
    internal::sizeText(ptr, rest, namePtr, nNameSize);
    string = encoding->toString(namePtr, nNameSize, info.codec);
    setEnglishName(string);
    delete string;
    const MorphUnit &unit = *reinterpret_cast<const MorphUnit *>(ptr);
    m_category = unit.category;
    m_type = unit.type;
    ptr += sizeof(unit);
    switch (m_type) {
    case 0: /* group */
        readGroups(info, unit.size, ptr);
        break;
    case 1: /* vertex */
        readVertices(info, unit.size, ptr);
        break;
    case 2: /* bone */
        readBones(info, unit.size, ptr);
        break;
    case 3: /* UV0 */
    case 4: /* UV1 */
    case 5: /* UV2 */
    case 6: /* UV3 */
    case 7: /* UV4 */
        readUVs(info, unit.size, m_type - 3, ptr);
        break;
    case 8: /* material */
        readMaterials(info, unit.size, ptr);
        break;
    default:
        assert(0);
    }
    size = ptr - start;
}

void Morph::write(uint8_t *data, const Model::DataInfo &info) const
{
    internal::writeString(m_name, data);
    internal::writeString(m_englishName, data);
    MorphUnit mu;
    mu.category = m_category;
    mu.type = m_type;
    switch (m_type) {
    case 0: /* group */
        mu.size = m_groups.count();
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&mu), sizeof(mu), data);
        writeGroups(info, data);
        break;
    case 1: /* vertex */
        mu.size = m_vertices.count();
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&mu), sizeof(mu), data);
        writeVertices(info, data);
        break;
    case 2: /* bone */
        mu.size = m_bones.count();
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&mu), sizeof(mu), data);
        writeBones(info, data);
        break;
    case 3: /* UV0 */
    case 4: /* UV1 */
    case 5: /* UV2 */
    case 6: /* UV3 */
    case 7: /* UV4 */
        mu.size = m_uvs.count();
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&mu), sizeof(mu), data);
        writeUVs(info, data);
        break;
    case 8: /* material */
        mu.size = m_materials.count();
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&mu), sizeof(mu), data);
        writeMaterials(info, data);
        break;
    default:
        assert(0);
    }
}

size_t Morph::estimateSize(const Model::DataInfo &info) const
{
    size_t size = 0;
    size += internal::estimateSize(m_name);
    size += internal::estimateSize(m_englishName);
    size += sizeof(MorphUnit);
    switch (m_type) {
    case 0:
        size += m_groups.count() * (sizeof(GroupMorph) + info.morphIndexSize);
        break;
    case 1:
        size += m_vertices.count() * (sizeof(VertexMorph) + info.vertexIndexSize);
        break;
    case 2:
        size += m_bones.count() * (sizeof(BoneMorph) + info.boneIndexSize);
        break;
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        size += m_uvs.count() * (sizeof(UVMorph) + info.vertexIndexSize);
        break;
    case 8:
        size += m_materials.count() * (sizeof(MaterialMorph) + info.materialIndexSize);
        break;
    default:
        assert(0);
        return 0;
    }
    return size;
}

void Morph::performTransform(float weight)
{
    if (btFuzzyZero(weight))
        return;
    int nmorphs;
    switch (m_type) {
    case 0: /* group */
        nmorphs = m_groups.count();
        break;
    case 1: /* vertex */
        nmorphs = m_vertices.count();
        for (int i = 0; i < nmorphs; i++) {
            Vertex &v = m_vertices[i];
            v.vertex->mergeMorph(&v, weight);
        }
        break;
    case 2: /* bone */
        nmorphs = m_bones.count();
        for (int i = 0; i < nmorphs; i++) {
            Bone &v = m_bones[i];
            v.bone->mergeMorph(&v, weight);
        }
        break;
    case 3: /* UV */
    case 4: /* UV1 */
    case 5: /* UV2 */
    case 6: /* UV3 */
    case 7: /* UV4 */
        nmorphs = m_uvs.count();
        for (int i = 0; i < nmorphs; i++) {
            UV &v = m_uvs[i];
            v.vertex->mergeMorph(&v, weight);
        }
        break;
    case 8: /* material */
        nmorphs = m_materials.count();
        for (int i = 0; i < nmorphs; i++) {
            Material &v = m_materials.at(0);
            v.material->mergeMorph(&v, weight);
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

void Morph::addBoneMorph(const Bone &value)
{
    m_bones.add(value);
}

void Morph::addGroupMorph(const Group &value)
{
    m_groups.add(value);
}

void Morph::addMaterialMorph(const Material &value)
{
    m_materials.add(value);
}

void Morph::addUVMorph(const UV &value)
{
    m_uvs.add(value);
}

void Morph::addVertexMorph(const Vertex &value)
{
    m_vertices.add(value);
}

void Morph::setCategory(uint8_t value)
{
    m_category = value;
}

void Morph::setType(uint8_t value)
{
    m_type = value;
}

void Morph::setIndex(int value)
{
    m_index = value;
}

void Morph::readBones(const Model::DataInfo &info, int count, uint8_t *&ptr)
{
    Morph::Bone bone;
    for (int i = 0; i < count; i++) {
        int boneIndex = internal::readSignedIndex(ptr, info.boneIndexSize);
        const BoneMorph &morph = *reinterpret_cast<const BoneMorph *>(ptr);
        internal::setPosition(morph.position, bone.position);
        internal::setRotation(morph.rotation, bone.rotation);
        bone.bone = 0;
        bone.index = boneIndex;
        m_bones.add(bone);
        ptr += sizeof(morph);
    }
}

void Morph::readGroups(const Model::DataInfo &info, int count, uint8_t *&ptr)
{
    Morph::Group group;
    for (int i = 0; i < count; i++) {
        int morphIndex = internal::readSignedIndex(ptr, info.morphIndexSize);
        const GroupMorph &morph = *reinterpret_cast<const GroupMorph *>(ptr);
        group.morph = 0;
        group.weight = morph.weight;
        group.index = morphIndex;
        m_groups.add(group);
        ptr += sizeof(morph);
    }
}

void Morph::readMaterials(const Model::DataInfo &info, int count, uint8_t *&ptr)
{
    Morph::Material material;
    for (int i = 0; i < count; i++) {
        int materialIndex = internal::readSignedIndex(ptr, info.materialIndexSize);
        const MaterialMorph &morph = *reinterpret_cast<const MaterialMorph *>(ptr);
        material.material = 0;
        material.ambient.setValue(morph.ambient[0], morph.ambient[1], morph.ambient[2]);
        material.diffuse.setValue(morph.diffuse[0], morph.diffuse[1], morph.diffuse[2], morph.diffuse[3]);
        material.edgeColor.setValue(morph.edgeColor[0], morph.edgeColor[1], morph.edgeColor[2], morph.edgeColor[3]);
        material.edgeSize = morph.edgeSize;
        material.index = materialIndex;
        material.operation = morph.operation;
        material.shininess = morph.shininess;
        material.specular.setValue(morph.specular[0], morph.specular[1], morph.specular[2]);
        material.sphereTextureWeight.setValue(morph.sphereTextureWeight[0], morph.sphereTextureWeight[1],
                                              morph.sphereTextureWeight[2], morph.sphereTextureWeight[3]);
        material.textureWeight.setValue(morph.textureWeight[0], morph.textureWeight[1],
                                        morph.textureWeight[2], morph.textureWeight[3]);
        material.toonTextureWeight.setValue(morph.toonTextureWeight[0], morph.toonTextureWeight[1],
                                            morph.toonTextureWeight[2], morph.toonTextureWeight[3]);
        m_materials.add(material);
        ptr += sizeof(morph);
    }
}

void Morph::readUVs(const Model::DataInfo &info, int count, int offset, uint8_t *&ptr)
{
    Morph::UV uv;
    for (int i = 0; i < count; i++) {
        int vertexIndex = internal::readUnsignedIndex(ptr, info.vertexIndexSize);
        const UVMorph &morph = *reinterpret_cast<const UVMorph *>(ptr);
        uv.vertex = 0;
        uv.position.setValue(morph.position[0], morph.position[1], morph.position[2], morph.position[3]);
        uv.index = vertexIndex;
        uv.offset = offset;
        m_uvs.add(uv);
        ptr += sizeof(morph);
    }
}

void Morph::readVertices(const Model::DataInfo &info, int count, uint8_t *&ptr)
{
    Morph::Vertex vertex;
    for (int i = 0; i < count; i++) {
        int vertexIndex = internal::readUnsignedIndex(ptr, info.vertexIndexSize);
        const VertexMorph &morph = *reinterpret_cast<const VertexMorph *>(ptr);
        internal::setPosition(morph.position, vertex.position);
        vertex.vertex = 0;
        vertex.index = vertexIndex;
        m_vertices.add(vertex);
        ptr += sizeof(morph);
    }
}

void Morph::writeBones(const Model::DataInfo &info, uint8_t *&ptr) const
{
    BoneMorph morph;
    int nbones = m_bones.count(), boneIndexSize = info.boneIndexSize;
    for (int i = 0; i < nbones; i++) {
        const Morph::Bone &bone = m_bones[i];
        internal::getPosition(bone.position, morph.position);
        internal::getRotation(bone.rotation, morph.rotation);
        internal::writeSignedIndex(bone.index, boneIndexSize, ptr);
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&morph), sizeof(morph), ptr);
    }
}

void Morph::writeGroups(const Model::DataInfo &info, uint8_t *&ptr) const
{
    GroupMorph morph;
    int ngroups = m_groups.count(), morphIndexSize = info.morphIndexSize;
    for (int i = 0; i < ngroups; i++) {
        const Morph::Group &group = m_groups[i];
        morph.weight = group.weight;
        internal::writeSignedIndex(group.index, morphIndexSize, ptr);
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&morph), sizeof(morph), ptr);
    }
}

void Morph::writeMaterials(const Model::DataInfo &info, uint8_t *&ptr) const
{
    MaterialMorph morph;
    int nmaterials = m_materials.count(), materialIndexSize = info.materialIndexSize;
    for (int i = 0; i < nmaterials; i++) {
        const Morph::Material &material = m_materials[i];
        internal::getColor(material.ambient, morph.ambient);
        internal::getColor(material.diffuse, morph.diffuse);
        internal::getColor(material.edgeColor, morph.edgeColor);
        morph.operation = material.operation;
        morph.shininess = material.shininess;
        morph.edgeSize = material.edgeSize;
        internal::getColor(material.specular, morph.specular);
        internal::getColor(material.sphereTextureWeight, morph.sphereTextureWeight);
        internal::getColor(material.textureWeight, morph.textureWeight);
        internal::getColor(material.toonTextureWeight, morph.toonTextureWeight);
        internal::writeSignedIndex(material.index, materialIndexSize, ptr);
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&morph), sizeof(morph), ptr);
    }
}

void Morph::writeUVs(const Model::DataInfo &info, uint8_t *&ptr) const
{
    UVMorph morph;
    int nuvs = m_uvs.count(), vertexIndexSize = info.vertexIndexSize;
    for (int i = 0; i < nuvs; i++) {
        const Morph::UV &uv = m_uvs[i];
        morph.position[0] = uv.position.x();
        morph.position[1] = uv.position.y();
        morph.position[2] = uv.position.z();
        morph.position[3] = uv.position.w();
        internal::writeUnsignedIndex(uv.index, vertexIndexSize, ptr);
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&morph), sizeof(morph), ptr);
    }
}

void Morph::writeVertices(const Model::DataInfo &info, uint8_t *&ptr) const
{
    VertexMorph morph;
    int nvertices = m_vertices.count(), vertexIndexSize = info.vertexIndexSize;
    for (int i = 0; i < nvertices; i++) {
        const Morph::Vertex &vertex = m_vertices[i];
        internal::getPosition(vertex.position, morph.position);
        internal::writeUnsignedIndex(vertex.index, vertexIndexSize, ptr);
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&morph), sizeof(morph), ptr);
    }
}

} /* namespace pmx */
} /* namespace vpvl2 */

