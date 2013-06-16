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
#include "vpvl2/pmd2/Bone.h"
#include "vpvl2/pmd2/Vertex.h"

namespace
{

using namespace vpvl2::pmd2;

#pragma pack(push, 1)

struct VertexUnit {
    vpvl2::float32_t position[3];
    vpvl2::float32_t normal[3];
    vpvl2::float32_t uv[2];
    vpvl2::int16_t bones[2];
    vpvl2::uint8_t weight;
    vpvl2::uint8_t edge;
};

#pragma pack(pop)

}

namespace vpvl2
{
namespace pmd2
{

struct Vertex::PrivateContext {
    PrivateContext(Model *parentModelRef)
        : parentModelRef(parentModelRef),
          origin(kZeroV3),
          normal(kZeroV3),
          texcoord(kZeroV3),
          morphDelta(kZeroV3),
          edgeSize(0),
          weight(0),
          materialRef(Factory::sharedNullMaterialRef()),
          index(-1)
    {
        for (int i = 0; i < kMaxBones; i++) {
            boneRefs[i] = Factory::sharedNullBoneRef();
            boneIndices[i] = -1;
        }
    }
    ~PrivateContext () {
        for (int i = 0; i < kMaxBones; i++) {
            boneRefs[i] = 0;
            boneIndices[i] = -1;
        }
        origin.setZero();
        normal.setZero();
        texcoord.setZero();
        morphDelta.setZero();
        materialRef = 0;
        edgeSize = 0;
        weight = 0;
        index = -1;
    }

    Model *parentModelRef;
    Vector3 origin;
    Vector3 normal;
    Vector3 texcoord;
    Vector3 morphDelta;
    EdgeSizePrecision edgeSize;
    WeightPrecision weight;
    IMaterial *materialRef;
    IBone *boneRefs[internal::kPMDVertexMaxBoneSize];
    int boneIndices[internal::kPMDVertexMaxBoneSize];
    int index;
};

const int Vertex::kMaxBones = internal::kPMDVertexMaxBoneSize;

Vertex::Vertex(Model *parentModelRef)
    : m_context(0)
{
    m_context = new PrivateContext(parentModelRef);
}

Vertex::~Vertex()
{
    delete m_context;
    m_context = 0;
}

bool Vertex::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    int32_t size;
    if (!internal::getTyped<int32_t>(ptr, rest, size) || size * sizeof(VertexUnit) > rest) {
        return false;
    }
    info.verticesCount = size;
    info.verticesPtr = ptr;
    internal::drainBytes(size * sizeof(VertexUnit), ptr, rest);
    return true;
}

bool Vertex::loadVertices(const Array<Vertex *> &vertices, const Array<Bone *> &bones)
{
    const int nvertices = vertices.count();
    const int nbones = bones.count();
    for (int i = 0; i < nvertices; i++) {
        Vertex *vertex = vertices[i];
        vertex->m_context->index = i;
        for (int j = 0; j < kMaxBones; j++) {
            int boneIndex = vertex->m_context->boneIndices[j];
            if (boneIndex >= 0) {
                if (boneIndex >= nbones) {
                    return false;
                }
                else {
                    vertex->m_context->boneRefs[j] = bones[boneIndex];
                }
            }
            else {
                vertex->m_context->boneRefs[j] = Factory::sharedNullBoneRef();
            }
        }
    }
    return true;
}

void Vertex::writeVertices(const Array<Vertex *> &vertices, const Model::DataInfo &info, uint8_t *&data)
{
    const int32_t nvertices = vertices.count();
    internal::writeBytes(&nvertices, sizeof(nvertices), data);
    for (int32_t i = 0; i < nvertices; i++) {
        Vertex *vertex = vertices[i];
        vertex->write(data, info);
    }
}

size_t Vertex::estimateTotalSize(const Array<Vertex *> &vertices, const Model::DataInfo &info)
{
    const int32_t nvertices = vertices.count();
    size_t size = sizeof(nvertices);
    for (int32_t i = 0; i < nvertices; i++) {
        Vertex *vertex = vertices[i];
        size += vertex->estimateSize(info);
    }
    return size;
}

void Vertex::read(const uint8_t *data, const Model::DataInfo & /* info */, size_t &size)
{
    VertexUnit unit;
    internal::getData(data, unit);
    internal::setPosition(unit.position, m_context->origin);
    internal::setPosition(unit.normal, m_context->normal);
    m_context->texcoord.setValue(unit.uv[0], unit.uv[1], 0);
    m_context->boneIndices[0] = unit.bones[0];
    m_context->boneIndices[1] = unit.bones[1];
    m_context->weight = unit.weight * 0.01;
    m_context->edgeSize = unit.edge == 0 ? 1 : 0;
    size = sizeof(unit);
}

size_t Vertex::estimateSize(const Model::DataInfo & /* info */) const
{
    size_t size = 0;
    size += sizeof(VertexUnit);
    return size;
}

void Vertex::write(uint8_t *&data, const Model::DataInfo & /* info */) const
{
    VertexUnit unit;
    unit.bones[0] = m_context->boneIndices[0];
    unit.bones[1] = m_context->boneIndices[1];
    unit.edge = m_context->edgeSize > 0 ? 0 : 1;
    internal::getPosition(m_context->normal, unit.normal);
    internal::getPosition(m_context->origin, unit.position);
    unit.uv[0] = m_context->texcoord.x();
    unit.uv[1] = m_context->texcoord.y();
    unit.weight = uint8_t(m_context->weight * 100);
    internal::writeBytes(&unit, sizeof(unit), data);
}

void Vertex::performSkinning(Vector3 &position, Vector3 &normal) const
{
    const Transform &transformA = m_context->boneRefs[0]->localTransform();
    const Transform &transformB = m_context->boneRefs[1]->localTransform();
    const Vector3 &vertexPosition = m_context->origin + m_context->morphDelta;
    const Vector3 &v1 = transformA * vertexPosition;
    const Vector3 &n1 = transformA.getBasis() * m_context->normal;
    const Vector3 &v2 = transformB * vertexPosition;
    const Vector3 &n2 = transformB.getBasis() * m_context->normal;
    const Scalar &w = Scalar(m_context->weight);
    position.setInterpolate3(v2, v1, w);
    normal.setInterpolate3(n2, n1, w);
}

void Vertex::reset()
{
    m_context->morphDelta.setZero();
}

void Vertex::mergeMorph(const Vector3 &value, const IMorph::WeightPrecision &weight)
{
    const Scalar &w = Scalar(weight);
    m_context->morphDelta += value * w;
}

IModel *Vertex::parentModelRef() const
{
    return m_context->parentModelRef;
}

Vector3 Vertex::origin() const
{
    return m_context->origin;
}

Vector3 Vertex::normal() const
{
    return m_context->normal;
}

Vector3 Vertex::textureCoord() const
{
    return m_context->texcoord;
}

Vector4 Vertex::uv(int /* index */) const
{
    return kZeroV4;
}

Vector3 Vertex::delta() const
{
    return m_context->morphDelta;
}

IVertex::Type Vertex::type() const
{
    return kBdef2;
}

IVertex::EdgeSizePrecision Vertex::edgeSize() const
{
    return m_context->edgeSize;
}

IVertex::WeightPrecision Vertex::weight(int index) const
{
    return index == 0 ? m_context->weight : 0;
}

IBone *Vertex::boneRef(int index) const
{
    return internal::checkBound(index, 0, kMaxBones) ? m_context->boneRefs[index] : Factory::sharedNullBoneRef();
}

IMaterial *Vertex::materialRef() const
{
    return m_context->materialRef;
}

int Vertex::index() const
{
    return m_context->index;
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

void Vertex::setUV(int /* index */, const Vector4 & /* value */)
{
}

void Vertex::setType(Type /* value */)
{
}

void Vertex::setEdgeSize(const EdgeSizePrecision &value)
{
    m_context->edgeSize = value;
}

void Vertex::setWeight(int index, const WeightPrecision &weight)
{
    if (index == 0) {
        m_context->weight = weight;
    }
}

void Vertex::setBoneRef(int index, IBone *value)
{
    if (internal::checkBound(index, 0, kMaxBones)) {
        m_context->boneRefs[index] = value ? value : Factory::sharedNullBoneRef();
    }
}

void Vertex::setMaterialRef(IMaterial *value)
{
    m_context->materialRef = value ? value : Factory::sharedNullMaterialRef();
}

void Vertex::setIndex(int value)
{
    m_context->index = value;
}

} /* namespace pmd2 */
} /* namespace vpvl2 */
