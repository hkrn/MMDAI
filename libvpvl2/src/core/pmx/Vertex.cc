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
#include "vpvl2/pmx/Vertex.h"

namespace
{

using namespace vpvl2::pmx;

#pragma pack(push, 1)

struct VertexUnit {
    float position[3];
    float normal[3];
    float texcoord[2];
};

struct AdditinalUVUnit {
    float value[Vertex::kMaxBones];
};

struct Bdef2Unit {
    float weight;
};

struct Bdef4Unit {
    float weight[Vertex::kMaxBones];
};

struct SdefUnit {
    float weight;
    float c[3];
    float r0[3];
    float r1[3];
};

#pragma pack(pop)

}

namespace vpvl2
{
namespace pmx
{

const int Vertex::kMaxBones;
const int Vertex::kMaxMorphs;

Vertex::Vertex()
    : m_origin(kZeroV3),
      m_morphDelta(kZeroV3),
      m_normal(kZeroV3),
      m_texcoord(kZeroV3),
      m_c(kZeroV3),
      m_r0(kZeroV3),
      m_r1(kZeroV3),
      m_type(kBdef1),
      m_edgeSize(0),
      m_index(-1)
{
    for (int i = 0; i < kMaxBones; i++) {
        m_boneRefs[i] = 0;
        m_weight[i] = 0;
        m_boneIndices[i] = -1;
    }
    for (int i = 0; i < kMaxMorphs; i++) {
        m_originUVs[i].setZero();
        m_morphUVs[i].setZero();
    }
}

Vertex::~Vertex()
{
    m_origin.setZero();
    m_morphDelta.setZero();
    m_normal.setZero();
    m_texcoord.setZero();
    m_c.setZero();
    m_r0.setZero();
    m_r1.setZero();
    m_type = kBdef1;
    m_edgeSize = 0;
    m_index = -1;
    for (int i = 0; i < kMaxBones; i++) {
        m_boneRefs[i] = 0;
        m_weight[i] = 0;
        m_boneIndices[i] = -1;
    }
    for (int i = 0; i < kMaxMorphs; i++) {
        m_originUVs[i].setZero();
        m_morphUVs[i].setZero();
    }
}

bool Vertex::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    size_t size;
    if (!internal::size32(ptr, rest, size)) {
        return false;
    }
    if (!internal::checkBound(info.additionalUVSize, size_t(0), size_t(kMaxMorphs))) {
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
        case 4: /* QDEF */
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
        vertex->m_index = i;
        switch (vertex->m_type) {
        case kBdef1: {
            int boneIndex = vertex->m_boneIndices[0];
            if (boneIndex >= 0) {
                if (boneIndex >= nbones)
                    return false;
                else
                    vertex->m_boneRefs[0] = bones[boneIndex];
            }
            else {
                vertex->m_boneRefs[0] = NullBone::sharedReference();
            }
            break;
        }
        case kBdef2:
        case kSdef:
        {
            for (int j = 0; j < 2; j++) {
                int boneIndex = vertex->m_boneIndices[j];
                if (boneIndex >= 0) {
                    if (boneIndex >= nbones)
                        return false;
                    else
                        vertex->m_boneRefs[j] = bones[boneIndex];
                }
                else {
                    vertex->m_boneRefs[j] = NullBone::sharedReference();
                }
            }
            break;
        }
        case kBdef4:
        case kQdef:
        {
            for (int j = 0; j < 4; j++) {
                int boneIndex = vertex->m_boneIndices[j];
                if (boneIndex >= 0) {
                    if (boneIndex >= nbones)
                        return false;
                    else
                        vertex->m_boneRefs[j] = bones[boneIndex];
                }
                else {
                    vertex->m_boneRefs[j] = NullBone::sharedReference();
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

size_t Vertex::estimateTotalSize(const Array<Vertex *> &vertices, const Model::DataInfo &info)
{
    const int nvertices = vertices.count();
    size_t size = 0;
    size += sizeof(nvertices);
    for (int i = 0; i < nvertices; i++) {
        Vertex *vertex = vertices[i];
        size += vertex->estimateSize(info);
    }
    return size;
}

void Vertex::read(const uint8_t *data, const Model::DataInfo &info, size_t &size)
{
    uint8_t *ptr = const_cast<uint8_t *>(data), *start = ptr;
    VertexUnit vertex;
    internal::getData(ptr, vertex);
    internal::setPosition(vertex.position, m_origin);
    internal::setPosition(vertex.normal, m_normal);
    float u = vertex.texcoord[0], v = vertex.texcoord[1];
    m_texcoord.setValue(u, v, 0);
    ptr += sizeof(vertex);
    int additionalUVSize = info.additionalUVSize;
    AdditinalUVUnit uv;
    m_originUVs[0].setValue(u, v, 0, 0);
    for (int i = 0; i < additionalUVSize; i++) {
        internal::getData(ptr, uv);
        m_originUVs[i + 1].setValue(uv.value[0], uv.value[1], uv.value[2], uv.value[3]);
        ptr += sizeof(uv);
    }
    m_type = static_cast<Type>(*reinterpret_cast<uint8_t *>(ptr));
    ptr += sizeof(uint8_t);
    switch (m_type) {
    case kBdef1: {
        m_boneIndices[0] = internal::readSignedIndex(ptr, info.boneIndexSize);
        break;
    }
    case kBdef2: {
        for (int i = 0; i < 2; i++)
            m_boneIndices[i] = internal::readSignedIndex(ptr, info.boneIndexSize);
        Bdef2Unit unit;
        internal::getData(ptr, unit);
        m_weight[0] = btClamped(unit.weight, 0.0f, 1.0f);
        ptr += sizeof(unit);
        break;
    }
    case kBdef4:
    case kQdef:
    {
        for (int i = 0; i < 4; i++)
            m_boneIndices[i] = internal::readSignedIndex(ptr, info.boneIndexSize);
        Bdef4Unit unit;
        internal::getData(ptr, unit);
        for (int i = 0; i < 4; i++)
            m_weight[i] = btClamped(unit.weight[i], 0.0f, 1.0f);
        ptr += sizeof(unit);
        break;
    }
    case kSdef: {
        for (int i = 0; i < 2; i++)
            m_boneIndices[i] = internal::readSignedIndex(ptr, info.boneIndexSize);
        SdefUnit unit;
        internal::getData(ptr, unit);
        m_c.setValue(unit.c[0], unit.c[1], unit.c[2]);
        m_r0.setValue(unit.r0[0], unit.r0[1], unit.r0[2]);
        m_r1.setValue(unit.r1[0], unit.r1[1], unit.r1[2]);
        m_weight[0] = btClamped(unit.weight, 0.0f, 1.0f);
        ptr += sizeof(unit);
        break;
    }
    default: /* unexpected value */
        assert(0);
        return;
    }
    internal::getData(ptr, m_edgeSize);
    ptr += sizeof(m_edgeSize);
    size = ptr - start;
}

void Vertex::write(uint8_t *data, const Model::DataInfo &info) const
{
    VertexUnit vu;
    internal::getPosition(m_origin, vu.position);
    internal::getPosition(m_normal, vu.normal);
    vu.texcoord[0] = m_texcoord.x();
    vu.texcoord[1] = m_texcoord.y();
    internal::writeBytes(reinterpret_cast<const uint8_t *>(&vu), sizeof(vu), data);
    int additionalUVSize = info.additionalUVSize;
    AdditinalUVUnit avu;
    for (int i = 0; i < additionalUVSize; i++) {
        const Vector4 &uv = m_originUVs[i + 1];
        avu.value[0] = uv.x();
        avu.value[1] = uv.y();
        avu.value[2] = uv.z();
        avu.value[3] = uv.w();
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&avu), sizeof(avu), data);
    }
    internal::writeBytes(reinterpret_cast<const uint8_t *>(&m_type), sizeof(uint8_t), data);
    int boneIndexSize = info.boneIndexSize;
    switch (m_type) {
    case kBdef1: {
        internal::writeSignedIndex(m_boneIndices[0], boneIndexSize, data);
        break;
    }
    case kBdef2: {
        for (int i = 0; i < 2; i++)
            internal::writeSignedIndex(m_boneIndices[i], boneIndexSize, data);
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&m_weight[0]), sizeof(m_weight[0]), data);
        break;
    }
    case kBdef4:
    case kQdef:
    {
        for (int i = 0; i < 4; i++)
            internal::writeSignedIndex(m_boneIndices[i], boneIndexSize, data);
        for (int i = 0; i < 4; i++)
            internal::writeBytes(reinterpret_cast<const uint8_t *>(&m_weight[i]), sizeof(m_weight[i]), data);
        break;
    }
    case kSdef: {
        for (int i = 0; i < 2; i++)
            internal::writeSignedIndex(m_boneIndices[i], boneIndexSize, data);
        SdefUnit unit;
        unit.c[0] = m_c.x();
        unit.c[1] = m_c.y();
        unit.c[2] = m_c.z();
        unit.r0[0] = m_r0.x();
        unit.r0[1] = m_r0.y();
        unit.r0[2] = m_r0.z();
        unit.r1[0] = m_r1.x();
        unit.r1[1] = m_r1.y();
        unit.r1[2] = m_r1.z();
        unit.weight = m_weight[0];
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&unit), sizeof(unit), data);
        break;
    }
    default: /* unexpected value */
        assert(0);
        return;
    }
    internal::writeBytes(reinterpret_cast<const uint8_t *>(&m_edgeSize), sizeof(m_edgeSize), data);
}

size_t Vertex::estimateSize(const Model::DataInfo &info) const
{
    size_t size = 0;
    size += sizeof(VertexUnit);
    size += sizeof(AdditinalUVUnit) * info.additionalUVSize;
    size += sizeof(uint8_t);
    size += sizeof(m_edgeSize);
    switch (m_type) {
    case kBdef1:
        size += info.boneIndexSize;
        break;
    case kBdef2:
        size += info.boneIndexSize * 2 + sizeof(m_weight[0]);
        break;
    case kBdef4:
    case kQdef:
        size += info.boneIndexSize * 4 + sizeof(m_weight);
        break;
    case kSdef:
        size += info.boneIndexSize * 2 + sizeof(SdefUnit);
        break;
    default: /* unexpected value */
        assert(0);
        return 0;
    }
    return size;
}

void Vertex::reset()
{
    m_morphDelta.setZero();
    for (int i = 0; i < kMaxMorphs; i++)
        m_morphUVs[i].setZero();
}

void Vertex::mergeMorph(const Morph::UV *morph, const IMorph::WeightPrecision &weight)
{
    int offset = morph->offset;
    if (internal::checkBound(offset, 0, kMaxMorphs)) {
        const Vector4 &m = morph->position, &o = m_morphUVs[offset];
        Vector4 v(o.x() + m.x() * weight,
                  o.y() + m.y() * weight,
                  o.z() + m.z() * weight,
                  o.w() + m.w() * weight);
        m_morphUVs[offset] = v;
    }
}

void Vertex::mergeMorph(const Morph::Vertex *morph, const IMorph::WeightPrecision &weight)
{
    const Scalar w(weight);
    m_morphDelta += morph->position * w;
}

void Vertex::performSkinning(Vector3 &position, Vector3 &normal) const
{
    const Vector3 &vertexPosition = m_origin + m_morphDelta;
    switch (m_type) {
    case kBdef1: {
        const Transform &transform = m_boneRefs[0]->localTransform();
        position = transform * vertexPosition;
        normal = transform.getBasis() * m_normal;
        break;
    }
    case kBdef2:
    case kSdef: {
        const Transform &transformA = m_boneRefs[0]->localTransform();
        const Transform &transformB = m_boneRefs[1]->localTransform();
        const Vector3 &v1 = transformA * vertexPosition;
        const Vector3 &n1 = transformA.getBasis() * m_normal;
        const Vector3 &v2 = transformB * vertexPosition;
        const Vector3 &n2 = transformB.getBasis() * m_normal;
        float weight = m_weight[0];
        position.setInterpolate3(v2, v1, weight);
        normal.setInterpolate3(n2, n1, weight);
        break;
    }
    case kBdef4: {
        const Transform &transformA = m_boneRefs[0]->localTransform();
        const Transform &transformB = m_boneRefs[1]->localTransform();
        const Transform &transformC = m_boneRefs[2]->localTransform();
        const Transform &transformD = m_boneRefs[3]->localTransform();
        const Vector3 &v1 = transformA * vertexPosition;
        const Vector3 &n1 = transformA.getBasis() * m_normal;
        const Vector3 &v2 = transformB * vertexPosition;
        const Vector3 &n2 = transformB.getBasis() * m_normal;
        const Vector3 &v3 = transformC * vertexPosition;
        const Vector3 &n3 = transformC.getBasis() * m_normal;
        const Vector3 &v4 = transformD * vertexPosition;
        const Vector3 &n4 = transformD.getBasis() * m_normal;
        float w1 = m_weight[0], w2 = m_weight[1], w3 = m_weight[2], w4 = m_weight[3];
        float s  = w1 + w2 + w3 + w4, w1s = w1 / s, w2s = w2 / s, w3s = w3 / s, w4s = w4 / s;
        position = v1 * w1s + v2 * w2s + v3 * w3s + v4 * w4s;
        normal   = n1 * w1s + n2 * w2s + n3 * w3s + n4 * w4s;
        break;
    }
    case kMaxType:
    default:
        break;
    }
}

const Vector4 &Vertex::uv(int index) const
{
    return internal::checkBound(index, 0, kMaxMorphs) ? m_morphUVs[index] : kZeroV4;
}

float Vertex::weight(int index) const
{
    return internal::checkBound(index, 0, kMaxBones) ? m_weight[index] : 0;
}

IBone *Vertex::bone(int index) const
{
    return internal::checkBound(index, 0, kMaxBones) ? m_boneRefs[index] : 0;
}

void Vertex::setOrigin(const Vector3 &value)
{
    m_origin = value;
}

void Vertex::setNormal(const Vector3 &value)
{
    m_normal = value;
}

void Vertex::setTextureCoord(const Vector3 &value)
{
    m_texcoord = value;
}

void Vertex::setUV(int index, const Vector4 &value)
{
    if (internal::checkBound(index, 0, kMaxBones))
        m_originUVs[index + 1] = value;
}

void Vertex::setType(Type value)
{
    m_type = value;
}

void Vertex::setEdgeSize(float value)
{
    m_edgeSize = value;
}

void Vertex::setWeight(int index, float weight)
{
    if (internal::checkBound(index, 0, kMaxBones))
        m_weight[index] = weight;
}

void Vertex::setBone(int index, IBone *value)
{
    if (internal::checkBound(index, 0, kMaxBones)) {
        m_boneRefs[index] = value;
        m_boneIndices[index] = value->index();
    }
}

void Vertex::setSdefC(const Vector3 &value)
{
    m_c = value;
}

void Vertex::setSdefR0(const Vector3 &value)
{
    m_r0 = value;
}

void Vertex::setSdefR1(const Vector3 &value)
{
    m_r1 = value;
}

} /* namespace pmx */
} /* namespace vpvl2 */

