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

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/util.h"
#include "vpvl2/pmd/Vertex.h"

#include "vpvl/Vertex.h"

namespace vpvl2
{
namespace pmd
{

Vertex::Vertex(IModel *modelRef, vpvl::Vertex *vertexRef, Array<IBone *> *bonesRef, int index)
    : m_modelRef(modelRef),
      m_vertexRef(vertexRef),
      m_bonesRef(bonesRef),
      m_materialRef(0),
      m_texcoord(vertexRef->u(), vertexRef->v(), 0),
      m_index(index)
{
}

Vertex::~Vertex()
{
    m_vertexRef = 0;
    m_bonesRef = 0;
    m_materialRef = 0;
    m_texcoord.setZero();
    m_index = 0;
}

Vector3 Vertex::origin() const
{
    return m_vertexRef->position();
}

Vector3 Vertex::normal() const
{
    return m_vertexRef->normal();
}

Vector3 Vertex::textureCoord() const
{
    return m_texcoord;
}

float Vertex::edgeSize() const
{
    return m_vertexRef->isEdgeEnabled() ? 1.0f : 0.0f;
}

float Vertex::weight(int index) const
{
    return index == 0 ? m_vertexRef->weight() : 0;
}

IBone *Vertex::bone(int index) const
{
    switch (index) {
    case 0:
        return m_bonesRef->at(m_vertexRef->bone1());
    case 1:
        return m_bonesRef->at(m_vertexRef->bone2());
    default:
        return 0;
    }
}

IMaterial *Vertex::material() const
{
    return m_materialRef;
}

int Vertex::index() const
{
    return m_index;
}

void Vertex::performSkinning(Vector3 &position, Vector3 &normal) const
{
    const float weight = m_vertexRef->weight();
    const Vector3 &inPosition = m_vertexRef->position();
    const Vector3 &inNormal = m_vertexRef->normal();
    if (btFuzzyZero(1 - weight)) {
        const Transform &transform = bone(0)->localTransform();
        internal::transformVertex(transform, inPosition, inNormal, position, normal);
    }
    else if (btFuzzyZero(weight)) {
        const Transform &transform = bone(1)->localTransform();
        internal::transformVertex(transform, inPosition, inNormal, position, normal);
    }
    else {
        const Transform &transformA = bone(0)->localTransform();
        const Transform &transformB = bone(1)->localTransform();
        internal::transformVertex(transformA, transformB, inPosition, inNormal, position, normal, weight);
    }
}

void Vertex::reset()
{
}

void Vertex::setOrigin(const Vector3 &value)
{
    m_vertexRef->setPosition(value);
}

void Vertex::setNormal(const Vector3 &value)
{
    m_vertexRef->setNormal(value);
}

void Vertex::setTextureCoord(const Vector3 &value)
{
    m_vertexRef->setTexCoord(value.x(), value.y());
}

void Vertex::setEdgeSize(float value)
{
    m_vertexRef->setEdgeEnable(btFuzzyZero(value));
}

void Vertex::setWeight(int index, float weight)
{
    if (index == 0)
        m_vertexRef->setWeight(weight);
}

void Vertex::setMaterial(IMaterial *value)
{
    m_materialRef = value;
}

} /* namespace pmd */
} /* namespace vpvl2 */
