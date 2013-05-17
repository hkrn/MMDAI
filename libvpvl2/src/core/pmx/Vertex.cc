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
#include "vpvl2/pmx/Vertex.h"

namespace
{

using namespace vpvl2::pmx;

#pragma pack(push, 1)

struct VertexUnit {
    vpvl2::float32_t position[3];
    vpvl2::float32_t normal[3];
    vpvl2::float32_t texcoord[2];
};

struct AdditinalUVUnit {
    vpvl2::float32_t value[Vertex::kMaxBones];
};

struct Bdef2Unit {
    vpvl2::float32_t weight;
};

struct Bdef4Unit {
    vpvl2::float32_t weight[Vertex::kMaxBones];
};

struct SdefUnit {
    vpvl2::float32_t weight;
    vpvl2::float32_t c[3];
    vpvl2::float32_t r0[3];
    vpvl2::float32_t r1[3];
};

#pragma pack(pop)

}

namespace vpvl2
{
namespace pmx
{

#ifndef _MSC_VER
const int Vertex::kMaxBones;
const int Vertex::kMaxMorphs;
#endif

struct Vertex::PrivateContext {
    PrivateContext(IModel *modelRef)
        : modelRef(modelRef),
          materialRef(0),
          origin(kZeroV3),
          morphDelta(kZeroV3),
          normal(kZeroV3),
          texcoord(kZeroV3),
          c(kZeroV3),
          r0(kZeroV3),
          r1(kZeroV3),
          type(kBdef1),
          edgeSize(0),
          index(-1)
    {
        for (int i = 0; i < kMaxBones; i++) {
            boneRefs[i] = 0;
            weight[i] = 0;
            boneIndices[i] = -1;
        }
        for (int i = 0; i < kMaxMorphs; i++) {
            originUVs[i].setZero();
            morphUVs[i].setZero();
        }
    }
    ~PrivateContext() {
        modelRef = 0;
        origin.setZero();
        morphDelta.setZero();
        normal.setZero();
        texcoord.setZero();
        c.setZero();
        r0.setZero();
        r1.setZero();
        type = kBdef1;
        edgeSize = 0;
        index = -1;
        for (int i = 0; i < kMaxBones; i++) {
            boneRefs[i] = 0;
            weight[i] = 0;
            boneIndices[i] = -1;
        }
        for (int i = 0; i < kMaxMorphs; i++) {
            originUVs[i].setZero();
            morphUVs[i].setZero();
        }
    }
    IModel *modelRef;
    IBone *boneRefs[kMaxBones];
    IMaterial *materialRef;
    Vector4 originUVs[kMaxMorphs];
    Vector4 morphUVs[kMaxMorphs];
    Vector3 origin;
    Vector3 morphDelta;
    Vector3 normal;
    Vector3 texcoord;
    Vector3 c;
    Vector3 r0;
    Vector3 r1;
    IVertex::Type type;
    IVertex::EdgeSizePrecision edgeSize;
    IVertex::WeightPrecision weight[kMaxBones];
    int boneIndices[kMaxBones];
    int index;
};

Vertex::Vertex(IModel *modelRef)
    : m_context(0)
{
    m_context = new PrivateContext(modelRef);
}

Vertex::~Vertex()
{
    delete m_context;
    m_context = 0;
}

bool Vertex::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    int32_t nvertices;
    if (!internal::getTyped<int32_t>(ptr, rest, nvertices)) {
        VPVL2_LOG(WARNING, "Invalid size of PMX vertex detected: size=" << nvertices << " rest=" << rest);
        return false;
    }
    if (!internal::checkBound(info.additionalUVSize, size_t(0), size_t(kMaxMorphs))) {
        VPVL2_LOG(WARNING, "Invalid size of PMX additional UV size detected: size=" << info.additionalUVSize << " rest=" << rest);
        return false;
    }
    info.verticesPtr = ptr;
    size_t baseSize = sizeof(VertexUnit) + sizeof(AdditinalUVUnit) * info.additionalUVSize;
    for (int i = 0; i < nvertices; i++) {
        if (!internal::validateSize(ptr, baseSize, rest)) {
            VPVL2_LOG(WARNING, "Invalid size of PMX base vertex unit detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
            return false;
        }
        uint8_t type;
        /* bone type */
        if (!internal::getTyped<uint8_t>(ptr, rest, type)) {
            VPVL2_LOG(WARNING, "Invalid size of PMX vertex type detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
            return false;
        }
        size_t boneSize = 0;
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
            VPVL2_LOG(WARNING, "Unexpected vertex type detected: index=" << i << " type=" << int(type) <<  " rest=" << rest);
            return false;
        }
        boneSize += sizeof(float); /* edge */
        if (!internal::validateSize(ptr, boneSize, rest)) {
            VPVL2_LOG(WARNING, "Invalid size of PMX vertex unit of bone detected: index=" << i << " size=" << boneSize <<  " rest=" << rest);
            return false;
        }
    }
    info.verticesCount = nvertices;
    return rest > 0;
}

bool Vertex::loadVertices(const Array<Vertex *> &vertices, const Array<Bone *> &bones)
{
    const int nvertices = vertices.count();
    const int nbones = bones.count();
    for (int i = 0; i < nvertices; i++) {
        Vertex *vertex = vertices[i];
        vertex->setIndex(i);
        switch (vertex->m_context->type) {
        case kBdef1: {
            int boneIndex = vertex->m_context->boneIndices[0];
            if (boneIndex >= 0) {
                if (boneIndex >= nbones) {
                    VPVL2_LOG(WARNING, "Invalid PMX bone (Bdef1) specified: index=" << i << " bone=" << boneIndex);
                    return false;
                }
                else {
                    vertex->m_context->boneRefs[0] = bones[boneIndex];
                }
            }
            else {
                vertex->m_context->boneRefs[0] = NullBone::sharedReference();
            }
            break;
        }
        case kBdef2:
        case kSdef:
        {
            for (int j = 0; j < 2; j++) {
                int boneIndex = vertex->m_context->boneIndices[j];
                if (boneIndex >= 0) {
                    if (boneIndex >= nbones) {
                        VPVL2_LOG(WARNING, "Invalid PMX bone (Bdef2|Sdef) specified: index=" << i << " offset=" << j << " bone=" << boneIndex);
                        return false;
                    }
                    else {
                        vertex->m_context->boneRefs[j] = bones[boneIndex];
                    }
                }
                else {
                    vertex->m_context->boneRefs[j] = NullBone::sharedReference();
                }
            }
            break;
        }
        case kBdef4:
        case kQdef:
        {
            for (int j = 0; j < 4; j++) {
                int boneIndex = vertex->m_context->boneIndices[j];
                if (boneIndex >= 0) {
                    if (boneIndex >= nbones) {
                        VPVL2_LOG(WARNING, "Invalid PMX bone (Bdef4|Qdef) specified: index=" << i << " offset=" << j << " bone=" << boneIndex);
                        return false;
                    }
                    else {
                        vertex->m_context->boneRefs[j] = bones[boneIndex];
                    }
                }
                else {
                    vertex->m_context->boneRefs[j] = NullBone::sharedReference();
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

void Vertex::writeVertices(const Array<Vertex *> &vertices, const Model::DataInfo &info, uint8_t *&data)
{
    const int32_t nveritces = vertices.count();
    internal::writeBytes(&nveritces, sizeof(nveritces), data);
    for (int32_t i = 0; i < nveritces; i++) {
        const Vertex *vertex = vertices[i];
        vertex->write(data, info);
    }
}

size_t Vertex::estimateTotalSize(const Array<Vertex *> &vertices, const Model::DataInfo &info)
{
    const int32_t nvertices = vertices.count();
    size_t size = 0;
    size += sizeof(nvertices);
    for (int32_t i = 0; i < nvertices; i++) {
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
    internal::setPosition(vertex.position, m_context->origin);
    VPVL2_VLOG(3, "PMXVertex: position=" << m_context->origin.x() << "," << m_context->origin.y() << "," << m_context->origin.z());
    internal::setPosition(vertex.normal, m_context->normal);
    VPVL2_VLOG(3, "PMXVertex: normal=" << m_context->normal.x() << "," << m_context->normal.y() << "," << m_context->normal.z());
    float32_t u = vertex.texcoord[0], v = vertex.texcoord[1];
    m_context->texcoord.setValue(u, v, 0);
    VPVL2_VLOG(3, "PMXVertex: texcoord=" << m_context->texcoord.x() << "," << m_context->texcoord.y() << "," << m_context->texcoord.z());
    ptr += sizeof(vertex);
    int additionalUVSize = info.additionalUVSize;
    AdditinalUVUnit uv;
    m_context->originUVs[0].setValue(u, v, 0, 0);
    for (int i = 0; i < additionalUVSize; i++) {
        internal::getData(ptr, uv);
        Vector4 &v = m_context->originUVs[i + 1];
        v.setValue(uv.value[0], uv.value[1], uv.value[2], uv.value[3]);
        VPVL2_VLOG(3, "PMXVertex: uv(" << i << ")=" << v.x() << "," << v.y() << "," << v.z() << "," << v.w());
        ptr += sizeof(uv);
    }
    m_context->type = static_cast<Type>(*reinterpret_cast<uint8_t *>(ptr));
    ptr += sizeof(uint8_t);
    switch (m_context->type) {
    case kBdef1: {
        m_context->boneIndices[0] = internal::readSignedIndex(ptr, info.boneIndexSize);
        VPVL2_VLOG(3, "PMXVertex: type=" << m_context->type << " bone=" << m_context->boneIndices[0]);
        break;
    }
    case kBdef2: {
        for (int i = 0; i < 2; i++) {
            m_context->boneIndices[i] = internal::readSignedIndex(ptr, info.boneIndexSize);
        }
        Bdef2Unit unit;
        internal::getData(ptr, unit);
        m_context->weight[0] = btClamped(unit.weight, 0.0f, 1.0f);
        VPVL2_VLOG(3, "PMXVertex: type=" << m_context->type << " bone=" << m_context->boneIndices[0] << "," << m_context->boneIndices[1] << " weight=" << m_context->weight[0]);
        ptr += sizeof(unit);
        break;
    }
    case kBdef4:
    case kQdef: {
        for (int i = 0; i < 4; i++) {
            m_context->boneIndices[i] = internal::readSignedIndex(ptr, info.boneIndexSize);
        }
        Bdef4Unit unit;
        internal::getData(ptr, unit);
        for (int i = 0; i < 4; i++) {
            m_context->weight[i] = btClamped(unit.weight[i], 0.0f, 1.0f);
        }
        VPVL2_VLOG(3, "PMXVertex: type=" << m_context->type << " bone=" << m_context->boneIndices[0] << "," << m_context->boneIndices[1] << "," << m_context->boneIndices[2] << "," << m_context->boneIndices[3] << " weight=" << m_context->weight[0] << "," << m_context->weight[1] << "," << m_context->weight[2] << "," << m_context->weight[3]);
        ptr += sizeof(unit);
        break;
    }
    case kSdef: {
        for (int i = 0; i < 2; i++) {
            m_context->boneIndices[i] = internal::readSignedIndex(ptr, info.boneIndexSize);
        }
        SdefUnit unit;
        internal::getData(ptr, unit);
        m_context->c.setValue(unit.c[0], unit.c[1], unit.c[2]);
        m_context->r0.setValue(unit.r0[0], unit.r0[1], unit.r0[2]);
        m_context->r1.setValue(unit.r1[0], unit.r1[1], unit.r1[2]);
        m_context->weight[0] = btClamped(unit.weight, 0.0f, 1.0f);
        VPVL2_VLOG(3, "PMXVertex: type=" << m_context->type << " bone=" << m_context->boneIndices[0] << "," << m_context->boneIndices[1] << " weight=" << m_context->weight[0]);
        VPVL2_VLOG(3, "PMXVertex: C=" << m_context->c.x() << "," << m_context->c.y() << "," << m_context->c.z());
        VPVL2_VLOG(3, "PMXVertex: R0=" << m_context->r0.x() << "," << m_context->r0.y() << "," << m_context->r0.z());
        VPVL2_VLOG(3, "PMXVertex: R1=" << m_context->r1.x() << "," << m_context->r1.y() << "," << m_context->r1.z());
        ptr += sizeof(unit);
        break;
    }
    default: /* unexpected value */
        return;
    }
    float32_t edgeSize;
    internal::getData(ptr, edgeSize);
    ptr += sizeof(edgeSize);
    m_context->edgeSize = edgeSize;
    size = ptr - start;
}

void Vertex::write(uint8_t *&data, const Model::DataInfo &info) const
{
    VertexUnit vu;
    internal::getPosition(m_context->origin, vu.position);
    internal::getPosition(m_context->normal, vu.normal);
    vu.texcoord[0] = m_context->texcoord.x();
    vu.texcoord[1] = m_context->texcoord.y();
    internal::writeBytes(&vu, sizeof(vu), data);
    int additionalUVSize = info.additionalUVSize;
    AdditinalUVUnit avu;
    for (int i = 0; i < additionalUVSize; i++) {
        const Vector4 &uv = m_context->originUVs[i + 1];
        avu.value[0] = uv.x();
        avu.value[1] = uv.y();
        avu.value[2] = uv.z();
        avu.value[3] = uv.w();
        internal::writeBytes(&avu, sizeof(avu), data);
    }
    internal::writeBytes(&m_context->type, sizeof(uint8_t), data);
    int boneIndexSize = info.boneIndexSize;
    switch (m_context->type) {
    case kBdef1: {
        internal::writeSignedIndex(m_context->boneIndices[0], boneIndexSize, data);
        break;
    }
    case kBdef2: {
        for (int i = 0; i < 2; i++) {
            internal::writeSignedIndex(m_context->boneIndices[i], boneIndexSize, data);
        }
        float weight(m_context->weight[0]);
        internal::writeBytes(&weight, sizeof(weight), data);
        break;
    }
    case kBdef4:
    case kQdef:
    {
        for (int i = 0; i < 4; i++) {
            internal::writeSignedIndex(m_context->boneIndices[i], boneIndexSize, data);
        }
        for (int i = 0; i < 4; i++) {
            float weight(m_context->weight[i]);
            internal::writeBytes(&weight, sizeof(weight), data);
        }
        break;
    }
    case kSdef: {
        for (int i = 0; i < 2; i++) {
            internal::writeSignedIndex(m_context->boneIndices[i], boneIndexSize, data);
        }
        SdefUnit unit;
        unit.c[0] = m_context->c.x();
        unit.c[1] = m_context->c.y();
        unit.c[2] = m_context->c.z();
        unit.r0[0] = m_context->r0.x();
        unit.r0[1] = m_context->r0.y();
        unit.r0[2] = m_context->r0.z();
        unit.r1[0] = m_context->r1.x();
        unit.r1[1] = m_context->r1.y();
        unit.r1[2] = m_context->r1.z();
        unit.weight = float(m_context->weight[0]);
        internal::writeBytes(&unit, sizeof(unit), data);
        break;
    }
    default: /* unexpected value */
        return;
    }
    float32_t edgeSize(m_context->edgeSize);
    internal::writeBytes(&edgeSize, sizeof(edgeSize), data);
}

size_t Vertex::estimateSize(const Model::DataInfo &info) const
{
    size_t size = 0;
    size += sizeof(VertexUnit);
    size += sizeof(AdditinalUVUnit) * info.additionalUVSize;
    size += sizeof(uint8_t);
    size += sizeof(float); /* edgeSize */
    switch (m_context->type) {
    case kBdef1:
        size += info.boneIndexSize;
        break;
    case kBdef2:
        size += info.boneIndexSize * 2 + sizeof(Bdef2Unit);
        break;
    case kBdef4:
    case kQdef:
        size += info.boneIndexSize * 4 + sizeof(Bdef4Unit);
        break;
    case kSdef:
        size += info.boneIndexSize * 2 + sizeof(SdefUnit);
        break;
    default: /* unexpected value */
        return 0;
    }
    return size;
}

void Vertex::reset()
{
    m_context->morphDelta.setZero();
    for (int i = 0; i < kMaxMorphs; i++) {
        m_context->morphUVs[i].setZero();
    }
}

void Vertex::mergeMorph(const Morph::UV *morph, const IMorph::WeightPrecision &weight)
{
    int offset = morph->offset;
    if (internal::checkBound(offset, 0, kMaxMorphs)) {
        const Vector4 &m = morph->position, &o = m_context->morphUVs[offset];
        Vector4 v(Scalar(o.x() + m.x() * weight),
                  Scalar(o.y() + m.y() * weight),
                  Scalar(o.z() + m.z() * weight),
                  Scalar(o.w() + m.w() * weight));
        m_context->morphUVs[offset] = v;
    }
}

void Vertex::mergeMorph(const Morph::Vertex *morph, const IMorph::WeightPrecision &weight)
{
    m_context->morphDelta += morph->position * Scalar(weight);
}

static inline void PrintString(const IString *value)
{
    VPVL2_VLOG(1, internal::cstr(value, "(null)"));
}

static inline void PrintPosition(const char *name, const Vector3 &position)
{
    VPVL2_VLOG(1, name << ":" << position.x() << "," << position.y() << "," << position.z());
}

void Vertex::performSkinning(Vector3 &position, Vector3 &normal) const
{
    const Vector3 &vertexPosition = m_context->origin + m_context->morphDelta;
    switch (m_context->type) {
    case kBdef1: {
        internal::transformVertex(m_context->boneRefs[0]->localTransform(), vertexPosition, m_context->normal, position, normal);
        break;
    }
    case kBdef2:
    case kSdef: {
        const WeightPrecision &weight = m_context->weight[0];
        if (btFuzzyZero(1 - weight)) {
            const Transform &transform = m_context->boneRefs[0]->localTransform();
            internal::transformVertex(transform, vertexPosition, m_context->normal, position, normal);
        }
        else if (btFuzzyZero(weight)) {
            const Transform &transform = m_context->boneRefs[1]->localTransform();
            internal::transformVertex(transform, vertexPosition, m_context->normal, position, normal);
        }
        else {
            const Transform &transformA = m_context->boneRefs[0]->localTransform();
            const Transform &transformB = m_context->boneRefs[1]->localTransform();
            internal::transformVertex(transformA, transformB, vertexPosition, m_context->normal, position, normal, weight);
        }
        break;
    }
    case kBdef4: {
        const Transform &transformA = m_context->boneRefs[0]->localTransform();
        const Transform &transformB = m_context->boneRefs[1]->localTransform();
        const Transform &transformC = m_context->boneRefs[2]->localTransform();
        const Transform &transformD = m_context->boneRefs[3]->localTransform();
        const Vector3 &v1 = transformA * vertexPosition;
        const Vector3 &n1 = transformA.getBasis() * m_context->normal;
        const Vector3 &v2 = transformB * vertexPosition;
        const Vector3 &n2 = transformB.getBasis() * m_context->normal;
        const Vector3 &v3 = transformC * vertexPosition;
        const Vector3 &n3 = transformC.getBasis() * m_context->normal;
        const Vector3 &v4 = transformD * vertexPosition;
        const Vector3 &n4 = transformD.getBasis() * m_context->normal;
        const WeightPrecision &w1 = m_context->weight[0], &w2 = m_context->weight[1], &w3 = m_context->weight[2], &w4 = m_context->weight[3];
        const WeightPrecision &s  = w1 + w2 + w3 + w4, &w1s = w1 / s, &w2s = w2 / s, &w3s = w3 / s, &w4s = w4 / s;
        position = v1 * w1s + v2 * w2s + v3 * w3s + v4 * w4s;
        normal   = n1 * w1s + n2 * w2s + n3 * w3s + n4 * w4s;
        break;
    }
    case kMaxType:
    default:
        break;
    }
}

IModel *Vertex::parentModelRef() const
{
    return m_context->modelRef;
}

Vector3 Vertex::origin() const
{
    return m_context->origin;
}

Vector3 Vertex::delta() const
{
    return m_context->morphDelta;
}

Vector3 Vertex::normal() const
{
    return m_context->normal;
}

Vector3 Vertex::textureCoord() const
{
    return m_context->texcoord;
}

IVertex::Type Vertex::type() const
{
    return m_context->type;
}

IVertex::EdgeSizePrecision Vertex::edgeSize() const
{
    return m_context->edgeSize;
}

int Vertex::index() const
{
    return m_context->index;
}

Vector3 Vertex::sdefC() const
{
    return m_context->c;
}

Vector3 Vertex::sdefR0() const
{
    return m_context->r0;
}

Vector3 Vertex::sdefR1() const
{
    return m_context->r1;
}

Vector4 Vertex::uv(int index) const
{
    return internal::checkBound(index, 0, kMaxMorphs) ? m_context->morphUVs[index] : kZeroV4;
}

IVertex::WeightPrecision Vertex::weight(int index) const
{
    return internal::checkBound(index, 0, kMaxBones) ? m_context->weight[index] : 0;
}

IBone *Vertex::bone(int index) const
{
    return internal::checkBound(index, 0, kMaxBones) ? m_context->boneRefs[index] : 0;
}

IMaterial *Vertex::material() const
{
    return m_context->materialRef;
}

void Vertex::setOrigin(const Vector3 &value)
{
    m_context->origin = value;
}

void Vertex::setNormal(const Vector3 &value)
{
    m_context->normal = value;
}

void Vertex::setTextureCoord(const Vector3 &value)
{
    m_context->texcoord = value;
}

void Vertex::setUV(int index, const Vector4 &value)
{
    if (internal::checkBound(index, 0, kMaxBones)) {
        m_context->originUVs[index + 1] = value;
    }
}

void Vertex::setType(Type value)
{
    m_context->type = value;
}

void Vertex::setEdgeSize(const EdgeSizePrecision &value)
{
    m_context->edgeSize = value;
}

void Vertex::setWeight(int index, const WeightPrecision &weight)
{
    if (internal::checkBound(index, 0, kMaxBones)) {
        m_context->weight[index] = weight;
    }
}

void Vertex::setBoneRef(int index, IBone *value)
{
    if (internal::checkBound(index, 0, kMaxBones)) {
        m_context->boneRefs[index] = value;
        m_context->boneIndices[index] = value->index();
    }
}

void Vertex::setMaterial(IMaterial *value)
{
    m_context->materialRef = value;
}

void Vertex::setSdefC(const Vector3 &value)
{
    m_context->c = value;
}

void Vertex::setSdefR0(const Vector3 &value)
{
    m_context->r0 = value;
}

void Vertex::setSdefR1(const Vector3 &value)
{
    m_context->r1 = value;
}

void Vertex::setIndex(int value)
{
    m_context->index = value;
}

} /* namespace pmx */
} /* namespace vpvl2 */

