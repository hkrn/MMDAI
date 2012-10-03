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
#include "vpvl2/pmd/Bone.h"
#include "vpvl2/pmd/Vertex.h"

namespace
{

using namespace vpvl2::pmd;

#pragma pack(push, 1)

struct VertexUnit {
    float position[3];
    float normal[3];
    float uv[2];
    int16_t parentBoneID;
    int16_t childBoneID;
    uint8_t weight;
    uint8_t edge;
};

#pragma pack(pop)

}

namespace vpvl2
{
namespace pmd
{

const int Vertex::kMaxBones;

Vertex::Vertex()
    : m_origin(kZeroV3),
      m_normal(kZeroV3),
      m_texcoord(kZeroV3),
      m_delta(kZeroV3),
      m_edgeSize(0),
      m_weight(0)
{
}

Vertex::~Vertex()
{
    m_origin.setZero();
    m_normal.setZero();
    m_texcoord.setZero();
    m_edgeSize = 0;
    m_weight = 0;
}

bool Vertex::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    size_t size;
    if (!internal::size32(ptr, rest, size) || size * sizeof(VertexUnit) > rest) {
        return false;
    }
    info.verticesCount = size;
    info.verticesPtr = ptr;
    internal::readBytes(size * sizeof(VertexUnit), ptr, rest);
    return true;
}

bool Vertex::loadVertices(const Array<Vertex *> &vertices, const Array<Bone *> &bones)
{
    const int nvertices = vertices.count();
    const int nbones = bones.count();
    for (int i = 0; i < nvertices; i++) {
        Vertex *vertex = vertices[i];
        for (int j = 0; j < kMaxBones; j++) {
            int boneIndex = vertex->m_boneIndices[j];
            if (boneIndex >= 0) {
                if (boneIndex >= nbones)
                    return false;
                else
                    vertex->m_boneRefs[j] = bones[boneIndex];
            }
            else {
                vertex->m_boneRefs[j] = &kNullBone;
            }
        }
    }
    return true;
}

size_t Vertex::estimateTotalSize(const Array<Vertex *> &vertices, const Model::DataInfo &info)
{
    const int nvertices = vertices.count();
    size_t size = 0;
    for (int i = 0; i < nvertices; i++) {
        Vertex *vertex = vertices[i];
        size += vertex->estimateSize(info);
    }
    return size;
}

void Vertex::read(const uint8_t *data, const Model::DataInfo & /* info */, size_t &size)
{
    VertexUnit unit;
    internal::getData(data, unit);
    internal::setPosition(unit.position, m_origin);
    internal::setPosition(unit.normal, m_normal);
    m_texcoord.setValue(unit.uv[0], unit.uv[1], 0);
    m_boneIndices[0] = unit.parentBoneID;
    m_boneIndices[1] = unit.childBoneID;
    m_weight = unit.weight;
    m_edgeSize = unit.edge;
    size = sizeof(unit);
}

size_t Vertex::estimateSize(const Model::DataInfo & /* info */) const
{
    size_t size = 0;
    size += sizeof(VertexUnit);
    return size;
}

void Vertex::write(uint8_t *data, const Model::DataInfo & /* info */) const
{
    VertexUnit unit;
    unit.childBoneID = m_boneIndices[1];
    unit.edge = m_edgeSize;
    internal::getPosition(m_normal, unit.normal);
    unit.parentBoneID = m_boneIndices[0];
    internal::getPosition(m_origin, unit.position);
    unit.uv[0] = m_texcoord.x();
    unit.uv[1] = m_texcoord.y();
    unit.weight = m_weight;
    internal::copyBytes(data, reinterpret_cast<const uint8_t *>(&unit), sizeof(unit));
}

void Vertex::performSkinning(Vector3 &position, Vector3 &normal) const
{
    const Transform &transformA = m_boneRefs[0]->localTransform();
    const Transform &transformB = m_boneRefs[1]->localTransform();
    const Vector3 &vertexPosition = m_origin;
    const Vector3 &v1 = transformA * vertexPosition;
    const Vector3 &n1 = transformA.getBasis() * m_normal;
    const Vector3 &v2 = transformB * vertexPosition;
    const Vector3 &n2 = transformB.getBasis() * m_normal;
    position.setInterpolate3(v2, v1, m_weight);
    normal.setInterpolate3(n2, n1, m_weight);
}

float Vertex::weight(int index) const
{
    return index == 0 ? m_weight : 0;
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

void Vertex::setUV(int /* index */, const Vector4 & /* value */)
{
}

void Vertex::setType(Type /* value */)
{
}

void Vertex::setEdgeSize(float value)
{
    m_edgeSize = value;
}

void Vertex::setWeight(int index, float weight)
{
    if (index == 0)
        m_weight = weight;
}

void Vertex::setBone(int index, IBone *value)
{
    if (internal::checkBound(index, 0, kMaxBones))
        m_boneRefs[index] = value;
}

}
}
