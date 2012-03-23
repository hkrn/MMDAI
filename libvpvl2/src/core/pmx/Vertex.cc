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

struct VertexUnit {
    float position[3];
    float normal[3];
    float texcoord[2];
};

struct AdditinalUVUnit {
    float value[4];
};

struct Bdef2Unit {
    float weight;
};

struct Bdef4Unit {
    float weight[4];
};

struct SdefUnit {
    float c[3];
    float r0[3];
    float r1[3];
    float weight;
};

#pragma pack(pop)

}

namespace vpvl2
{
namespace pmx
{

Vertex::Vertex()
    : m_origin(kZeroV3),
      m_position(kZeroV3),
      m_normal(kZeroV3),
      m_texcoord(kZeroV3),
      m_c(kZeroV3),
      m_r0(kZeroV3),
      m_r1(kZeroV3),
      m_type(kBdef1),
      m_edge(0)
{
    for (int i = 0; i < 4; i++) {
        m_uvs[i].setZero();
        m_bones[i] = 0;
        m_weight[i] = 0;
        m_boneIndices[i] = -1;
    }
}

Vertex::~Vertex()
{
}

bool Vertex::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    size_t size;
    if (!internal::size32(ptr, rest, size)) {
        return false;
    }
    info.verticesPtr = ptr;
    size_t baseSize = sizeof(VertexUnit) + sizeof(AdditinalUVUnit) * info.additionalUVSize;
    for (size_t i = 0; i < size; i++) {
        if (!internal::validateSize(ptr, baseSize, rest)) {
            return false;
        }
        size_t type;
        /* bone type */
        if (!internal::size8(ptr, rest, type))
            return false;
        size_t boneSize;
        switch (type) {
        case 0: /* BDEF1 */
            boneSize = info.boneIndexSize;
            break;
        case 1: /* BDEF2 */
            boneSize = info.boneIndexSize * 2 + sizeof(Bdef2Unit);
            break;
        case 2: /* BDEF4 */
            boneSize = info.boneIndexSize * 4 + sizeof(Bdef4Unit);
            break;
        case 3: /* SDEF */
            boneSize = info.boneIndexSize * 2 + sizeof(SdefUnit);
            break;
        default: /* unexpected value */
            return false;
        }
        boneSize += sizeof(float); /* edge */
        if (!internal::validateSize(ptr, boneSize, rest)) {
            return false;
        }
    }
    info.verticesCount = size;
    return rest > 0;
}

bool Vertex::loadVertices(const Array<Vertex *> &vertices, const Array<Bone *> &bones)
{
    const int nvertices = vertices.count();
    const int nbones = bones.count();
    for (int i = 0; i < nvertices; i++) {
        Vertex *vertex = vertices[i];
        switch (vertex->m_type) {
        case kBdef1: {
            int boneIndex = vertex->m_boneIndices[0];
            if (boneIndex >= 0) {
                if (boneIndex >= nbones)
                    return false;
                else
                    vertex->m_bones[0] = bones[boneIndex];
            }
            break;
        }
        case kBdef2: {
            for (int j = 0; j < 2; j++) {
                int boneIndex = vertex->m_boneIndices[j];
                if (boneIndex >= 0) {
                    if (boneIndex >= nbones)
                        return false;
                    else
                        vertex->m_bones[j] = bones[boneIndex];
                }
            }
            break;
        }
        case kBdef4: {
            for (int j = 0; j < 4; j++) {
                int boneIndex = vertex->m_boneIndices[j];
                if (boneIndex >= 0) {
                    if (boneIndex >= nbones)
                        return false;
                    else
                        vertex->m_bones[j] = bones[boneIndex];
                }
            }
            break;
        }
        case kSdef: {
            int boneIndex1 = vertex->m_boneIndices[0];
            int boneIndex2 = vertex->m_boneIndices[1];
            if (boneIndex1 >= 0 && boneIndex2 >= 0) {
                if (boneIndex1 >= nbones || boneIndex2 >= nbones) {
                    return false;
                }
                Bone *bone1 = bones[boneIndex1], *bone2 = bones[boneIndex2];
                {
                    vertex->m_bones[0] = bone1;
                    vertex->m_bones[1] = bone2;
                }
            }
            break;
        }
        default:
            break;
        }
    }
    return true;
}

void Vertex::read(const uint8_t *data, const Model::DataInfo &info, size_t &size)
{
    uint8_t *ptr = const_cast<uint8_t *>(data), *start = ptr;
    const VertexUnit &vertex = *reinterpret_cast<VertexUnit *>(ptr);
    internal::setPosition(vertex.position, m_origin);
    internal::setPosition(vertex.normal, m_normal);
    m_position = m_origin;
    m_texcoord.setValue(vertex.texcoord[0], vertex.texcoord[1], 0);
    ptr += sizeof(vertex);
    int additionalUVSize = info.additionalUVSize;
    for (int i = 0; i < additionalUVSize; i++) {
        const AdditinalUVUnit &uv = *reinterpret_cast<AdditinalUVUnit *>(ptr);
        m_uvs[i].setValue(uv.value[0], uv.value[1], uv.value[2], uv.value[3]);
        ptr += sizeof(uv);
    }
    m_type = static_cast<Type>(*reinterpret_cast<uint8_t *>(ptr));
    ptr += sizeof(uint8_t);
    switch (m_type) {
    case kBdef1: { /* BDEF1 */
        m_boneIndices[0] = internal::variantIndex(ptr, info.boneIndexSize);
        break;
    }
    case kBdef2: { /* BDEF2 */
        for (int i = 0; i < 2; i++)
            m_boneIndices[i] = internal::variantIndex(ptr, info.boneIndexSize);
        const Bdef2Unit &unit = *reinterpret_cast<Bdef2Unit *>(ptr);
        m_weight[0] = unit.weight;
        ptr += sizeof(Bdef2Unit);
        break;
    }
    case kBdef4: { /* BDEF4 */
        for (int i = 0; i < 4; i++)
            m_boneIndices[i] = internal::variantIndex(ptr, info.boneIndexSize);
        const Bdef4Unit &unit = *reinterpret_cast<Bdef4Unit *>(ptr);
        for (int i = 0; i < 4; i++)
            m_weight[i] = unit.weight[i];
        ptr += sizeof(Bdef4Unit);
        break;
    }
    case kSdef: { /* SDEF */
        for (int i = 0; i < 2; i++)
            m_boneIndices[i] = internal::variantIndex(ptr, info.boneIndexSize);
        const SdefUnit &unit = *reinterpret_cast<SdefUnit *>(ptr);
        m_c.setValue(unit.c[0], unit.c[1], unit.c[2]);
        m_r0.setValue(unit.r0[0], unit.r0[1], unit.r0[2]);
        m_r1.setValue(unit.r1[0], unit.r1[1], unit.r1[2]);
        m_weight[0] = unit.weight;
        ptr += sizeof(SdefUnit);
        break;
    }
    default: /* unexpected value */
        assert(0);
        return;
    }
    m_edge = *reinterpret_cast<float *>(ptr);
    ptr += sizeof(m_edge);
    size = ptr - start;
}

void Vertex::write(uint8_t * /* data */) const
{
}

void Vertex::mergeMorph(Morph::UV *morph, float weight)
{
    m_uvs[morph->offset] += morph->position * weight;
}

void Vertex::mergeMorph(Morph::Vertex *morph, float weight)
{
    m_position = m_origin + morph->position * weight;
}

void Vertex::performSkinning(Vector3 &position, Vector3 &normal)
{
    switch (m_type) {
    case kBdef1: {
        const Transform &transform = m_bones[0]->localTransform();
        position = transform * m_position;
        normal = transform.getBasis() * m_normal;
        break;
    }
    case kBdef2:
    case kSdef: {
        const Transform &transformA = m_bones[0]->localTransform();
        const Transform &transformB = m_bones[1]->localTransform();
        const Vector3 &v1 = transformA * m_position;
        const Vector3 &n1 = transformA.getBasis() * m_normal;
        const Vector3 &v2 = transformB * m_position;
        const Vector3 &n2 = transformB.getBasis() * m_normal;
        float weight = m_weight[0];
        position.setInterpolate3(v2, v1, weight);
        normal.setInterpolate3(n2, n1, weight);
        break;
    }
    case kBdef4: {
        const Transform &transformA = m_bones[0]->localTransform();
        const Transform &transformB = m_bones[1]->localTransform();
        const Transform &transformC = m_bones[2]->localTransform();
        const Transform &transformD = m_bones[3]->localTransform();
        const Vector3 &v1 = transformA * m_position;
        const Vector3 &n1 = transformA.getBasis() * m_normal;
        const Vector3 &v2 = transformB * m_position;
        const Vector3 &n2 = transformB.getBasis() * m_normal;
        const Vector3 &v3 = transformC * m_position;
        const Vector3 &n3 = transformC.getBasis() * m_normal;
        const Vector3 &v4 = transformD * m_position;
        const Vector3 &n4 = transformD.getBasis() * m_normal;
        float w1 = m_weight[0], w2 = m_weight[1], w3 = m_weight[2], w4 = m_weight[3];
        float s  = w1 + w2 + w3 + w4, w1s = w1 / s, w2s = w2 / s, w3s = w3 / s, w4s = w4 / s;
        position = v1 * w1s + v2 * w2s + v3 * w3s + v4 * w4s;
        normal   = n1 * w1s + n2 * w2s + n3 * w3s + n4 * w4s;
        break;
    }
    }
    position.setW(1);
}

const Vector4 &Vertex::uv(int index) const
{
    return index >= 0 && index < 4 ? m_uvs[index] : kZeroV4;
}

} /* namespace pmx */
} /* namespace vpvl2 */

