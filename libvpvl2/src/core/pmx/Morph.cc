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
      m_englishName(0)
{
}

Morph::~Morph()
{
    delete m_name;
    m_name = 0;
    delete m_englishName;
    m_englishName = 0;
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
        internal::drain(sizeof(MorphUnit), ptr, rest);
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

bool Morph::loadUVs(const Array<pmx::Vertex *> &uv, int offset, Morph *morph)
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
    StaticString::Encoding encoding = info.encoding;
    m_name = new StaticString(namePtr, nNameSize, encoding);
    internal::sizeText(ptr, rest, namePtr, nNameSize);
    m_englishName = new StaticString(namePtr, nNameSize, encoding);
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
    case 3: /* UV */
        readUVs(info, unit.size, 0, ptr);
        break;
    case 4: /* UV1 */
        readUVs(info, unit.size, 1, ptr);
        break;
    case 5: /* UV2 */
        readUVs(info, unit.size, 2, ptr);
        break;
    case 6: /* UV3 */
        readUVs(info, unit.size, 3, ptr);
        break;
    case 7: /* UV4 */
        readUVs(info, unit.size, 4, ptr);
        break;
    case 8: /* material */
        readMaterials(info, unit.size, ptr);
        break;
    default:
        assert(0);
    }
    size = ptr - start;
}

void Morph::write(uint8_t *data) const
{
}

void Morph::performTransform(float weight)
{
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

void Morph::readBones(const Model::DataInfo &info, int count, uint8_t *&ptr)
{
    for (int i = 0; i < count; i++) {
        int boneIndex = internal::variantIndex(ptr, info.boneIndexSize);
        const BoneMorph &morph = *reinterpret_cast<const BoneMorph *>(ptr);
        Morph::Bone bone;
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
    for (int i = 0; i < count; i++) {
        int morphIndex = internal::variantIndex(ptr, info.morphIndexSize);
        const GroupMorph &morph = *reinterpret_cast<const GroupMorph *>(ptr);
        Morph::Group group;
        group.morph = 0;
        group.weight = morph.weight;
        group.index = morphIndex;
        m_groups.add(group);
        ptr += sizeof(morph);
    }
}

void Morph::readMaterials(const Model::DataInfo &info, int count, uint8_t *&ptr)
{
    for (int i = 0; i < count; i++) {
        int materialIndex = internal::variantIndex(ptr, info.materialIndexSize);
        const MaterialMorph &morph = *reinterpret_cast<const MaterialMorph *>(ptr);
        Morph::Material material;
        material.material = 0;
        material.index = materialIndex;
        m_materials.add(material);
        ptr += sizeof(morph);
    }
}

void Morph::readUVs(const Model::DataInfo &info, int count, int offset, uint8_t *&ptr)
{
    for (int i = 0; i < count; i++) {
        int vertexIndex = internal::variantIndexUnsigned(ptr, info.vertexIndexSize);
        const UVMorph &morph = *reinterpret_cast<const UVMorph *>(ptr);
        Morph::UV uv;
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
    for (int i = 0; i < count; i++) {
        int vertexIndex = internal::variantIndexUnsigned(ptr, info.vertexIndexSize);
        const VertexMorph &morph = *reinterpret_cast<const VertexMorph *>(ptr);
        Morph::Vertex vertex;
        internal::setPosition(morph.position, vertex.position);
        vertex.vertex = 0;
        vertex.index = vertexIndex;
        m_vertices.add(vertex);
        ptr += sizeof(morph);
    }
}

} /* namespace pmx */
} /* namespace vpvl2 */

