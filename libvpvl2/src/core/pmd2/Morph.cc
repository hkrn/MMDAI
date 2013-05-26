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
#include "vpvl2/internal/ModelHelper.h"
#include "vpvl2/pmd2/Morph.h"
#include "vpvl2/pmd2/Vertex.h"

namespace
{

using namespace vpvl2;
using namespace vpvl2::pmd2;

#pragma pack(push, 1)

struct VertexMorphUnit {
    vpvl2::int32_t vertexIndex;
    vpvl2::float32_t position[3];
};

struct MorphUnit {
    vpvl2::uint8_t name[internal::kPMDMorphNameSize];
    vpvl2::int32_t nvertices;
    vpvl2::uint8_t type;
};

#pragma pack(pop)

}

namespace vpvl2
{
namespace pmd2
{

struct InternalVertex {
    Vector3 position;
    int base;
    int index;
};

struct Morph::PrivateContext {
    PrivateContext(IModel *parentModelRef, IEncoding *encodingRef)
        : parentModelRef(parentModelRef),
          encodingRef(encodingRef),
          namePtr(0),
          englishNamePtr(0),
          category(kBase),
          weight(0),
          index(-1)
    {
    }
    ~PrivateContext() {
        delete namePtr;
        namePtr = 0;
        delete englishNamePtr;
        englishNamePtr = 0;
        category = kBase;
        weight = 0;
        index = -1;
    }
    IModel *parentModelRef;
    IEncoding *encodingRef;
    IString *namePtr;
    IString *englishNamePtr;
    Category category;
    WeightPrecision weight;
    Array<InternalVertex> vertices;
    Array<Vertex *> vertexRefs;
    int index;
};

const int Morph::kNameSize = internal::kPMDMorphNameSize;

Morph::Morph(IModel *parentModelRef, IEncoding *encodingRef)
    : m_context(0)
{
    m_context = new PrivateContext(parentModelRef, encodingRef);
}

Morph::~Morph()
{
    delete m_context;
    m_context = 0;
}

void Morph::resetTransform()
{
    const int nvertices = m_context->vertexRefs.count();
    for (int i = 0; i < nvertices; i++) {
        Vertex *vertex = m_context->vertexRefs[i];
        vertex->reset();
    }
}

bool Morph::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    uint16_t size;
    if (!internal::getTyped<uint16_t>(ptr, rest, size)) {
        return false;
    }
    info.morphsCount = size;
    info.morphsPtr = ptr;
    MorphUnit unit;
    VertexMorphUnit vunit;
    size_t unitSize = 0;
    for (size_t i = 0; i < size; i++) {
        if (sizeof(unit) > rest) {
            return false;
        }
        internal::getData(ptr, unit);
        unitSize = sizeof(unit) + unit.nvertices * sizeof(vunit);
        if (unitSize > rest) {
            return false;
        }
        internal::drainBytes(unitSize, ptr, rest);
    }
    return true;
}

bool Morph::loadMorphs(const Array<Morph *> &morphs, const Array<Vertex *> &vertices)
{
    const int nmorphs = morphs.count(), nvertices = vertices.count();
    Morph *baseMorph = 0;
    for (int i = 0; i < nmorphs; i++) {
        Morph *morph = morphs[i];
        if (morph->category() == kBase) {
            const Array<InternalVertex> &morphVertices = morph->m_context->vertices;
            const int nMorphVertices = morphVertices.count();
            Array<Vertex *> &vertexRefs = morph->m_context->vertexRefs;
            for (int j = 0; j < nMorphVertices; j++) {
                const InternalVertex &morphVertex = morphVertices[j];
                int vertexId = morphVertex.index;
                if (internal::checkBound(vertexId, 0, nvertices)) {
                    Vertex *vertex = vertices[vertexId];
                    vertex->setOrigin(morphVertex.position);
                    vertexRefs.append(vertex);
                }
            }
            baseMorph = morph;
            break;
        }
    }
    if (baseMorph) {
        for (int i = 0; i < nmorphs; i++) {
            Morph *morph = morphs[i];
            morph->m_context->index = i;
            if (morph->category() != kBase) {
                const Array<InternalVertex> &baseVertices = baseMorph->m_context->vertices;
                Array<InternalVertex> &morphVertices = morph->m_context->vertices;
                Array<Vertex *> &vertexRefs = morph->m_context->vertexRefs;
                const int nMorphVertices = morphVertices.count(), nBaseVertices = baseVertices.count();
                for (int j = 0; j < nMorphVertices; j++) {
                    InternalVertex &morphVertex = morphVertices[j];
                    int vertexIndex = morphVertex.index;
                    if (internal::checkBound(vertexIndex, 0, nBaseVertices)) {
                        int baseVertexIndex = baseVertices[vertexIndex].index;
                        morphVertex.base = baseVertexIndex;
                        vertexRefs.append(vertices[baseVertexIndex]);
                    }
                }
            }
        }
    }
    return true;
}

void Morph::writeMorphs(const Array<Morph *> &morphs, const Model::DataInfo &info, uint8_t *&data)
{
    const int nmorphs = morphs.count();
    internal::writeUnsignedIndex(nmorphs, sizeof(uint16_t), data);
    for (int i = 0; i < nmorphs; i++) {
        Morph *morph = morphs[i];
        morph->write(data, info);
    }
}

void Morph::writeEnglishNames(const Array<Morph *> &morphs, const Model::DataInfo &info, uint8_t *&data)
{
    const IEncoding *encodingRef = info.encoding;
    const int nmorphs = morphs.count();
    for (int i = 0; i < nmorphs; i++) {
        Morph *morph = morphs[i];
        internal::writeStringAsByteArray(morph->name(), IString::kShiftJIS, encodingRef, kNameSize, data);
    }
}

size_t Morph::estimateTotalSize(const Array<Morph *> &morphs, const Model::DataInfo &info)
{
    const int nmorphs = morphs.count();
    size_t size = sizeof(uint16_t);
    for (int i = 0; i < nmorphs; i++) {
        Morph *morph = morphs[i];
        size += morph->estimateSize(info);
    }
    return size;
}

void Morph::read(const uint8_t *data, size_t &size)
{
    MorphUnit unit;
    VertexMorphUnit vunit;
    InternalVertex vertex;
    internal::getData(data, unit);
    uint8_t *ptr = const_cast<uint8_t *>(data + sizeof(unit));
    int nMorphVertices = unit.nvertices;
    for (int i = 0; i < nMorphVertices; i++) {
        internal::getData(ptr, vunit);
        if (internal::checkBound(vunit.vertexIndex, 0, 65535)) {
            internal::setPosition(vunit.position, vertex.position);
            vertex.index = vunit.vertexIndex;
            m_context->vertices.append(vertex);
        }
        ptr += sizeof(vunit);
    }
    internal::setStringDirect(m_context->encodingRef->toString(unit.name, IString::kShiftJIS, kNameSize), m_context->namePtr);
    m_context->category = static_cast<Category>(unit.type);
    size = ptr - data;
}

void Morph::readEnglishName(const uint8_t *data, int index)
{
    if (data && index >= 0) {
        internal::setStringDirect(m_context->encodingRef->toString(data + kNameSize * index, IString::kShiftJIS, kNameSize), m_context->englishNamePtr);
    }
}

size_t Morph::estimateSize(const Model::DataInfo & /* info */) const
{
    size_t size = 0;
    size += sizeof(MorphUnit);
    size += m_context->vertices.count() * sizeof(VertexMorphUnit);
    return size;
}

void Morph::write(uint8_t *&data, const Model::DataInfo & /* info */) const
{
    MorphUnit unit;
    uint8_t *namePtr = unit.name;
    internal::writeStringAsByteArray(m_context->namePtr, IString::kShiftJIS, m_context->encodingRef, sizeof(unit.name), namePtr);
    unit.nvertices = m_context->vertices.count();
    unit.type = m_context->category;
    internal::writeBytes(&unit, sizeof(unit), data);
    VertexMorphUnit vunit;
    const int nvertices = m_context->vertices.count();
    for (int i = 0; i < nvertices; i++) {
        const InternalVertex &vertex = m_context->vertices[i];
        vunit.vertexIndex = vertex.index;
        internal::getPosition(vertex.position, vunit.position);
        internal::writeBytes(&vunit, sizeof(vunit), data);
    }
}

void Morph::update()
{
    const int nvertices = m_context->vertexRefs.count();
    for (int i = 0; i < nvertices; i++) {
        Vertex *vertex = m_context->vertexRefs[i];
        const InternalVertex &v = m_context->vertices[i];
        vertex->mergeMorph(v.position, m_context->weight);
    }
}

IModel *Morph::parentModelRef() const
{
    return m_context->parentModelRef;
}

const IString *Morph::name() const
{
    return m_context->namePtr;
}

const IString *Morph::englishName() const
{
    return m_context->englishNamePtr;
}

int Morph::index() const
{
    return m_context->index;
}

IMorph::Category Morph::category() const
{
    return m_context->category;
}

IMorph::Type Morph::type() const
{
    return kVertexMorph;
}

bool Morph::hasParent() const
{
    return false;
}

IMorph::WeightPrecision Morph::weight() const
{
    return m_context->weight;
}

void Morph::setWeight(const WeightPrecision &value)
{
    m_context->weight = value;
}

void Morph::setIndex(int value)
{
    m_context->index = value;
}

} /* namespace pmd2 */
} /* namespace vpvl2 */
