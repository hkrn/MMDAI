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
#include "vpvl2/pmd/Morph.h"
#include "vpvl2/pmd/Vertex.h"

namespace
{

using namespace vpvl2::pmd;

#pragma pack(push, 1)

struct VertexMorphUnit {
    int vertexIndex;
    float position[3];
};

struct MorphUnit {
    uint8_t name[Morph::kNameSize];
    int nvertices;
    uint8_t type;
};

#pragma pack(pop)

}

namespace vpvl2
{
namespace pmd
{

Morph::Morph(IEncoding *encodingRef)
    : m_encodingRef(encodingRef),
      m_name(0),
      m_category(kReserved),
      m_weight(0),
      m_index(-1)
{
}

Morph::~Morph()
{
    delete m_name;
    m_name = 0;
    m_category = kReserved;
    m_weight = 0;
    m_index = -1;
}

bool Morph::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    size_t size;
    if (!internal::size16(ptr, rest, size)) {
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
        internal::readBytes(unitSize, ptr, rest);
    }
    return true;
}

bool Morph::loadMorphs(const Array<Morph *> &morphs, const Array<Vertex *> &vertices)
{
    const int nmorphs = morphs.count();
    for (int i = 0; i < nmorphs; i++) {
        // FIXME: implement this
    }
    return true;
}

void Morph::read(const uint8_t *data, const Array<Vertex *> &vertices, size_t &size)
{
    MorphUnit unit;
    VertexMorphUnit vunit;
    internal::getData(data, unit);
    uint8_t *ptr = const_cast<uint8_t *>(data + sizeof(unit));
    int nMorphVertices = unit.nvertices, nvertices = vertices.count();
    for (int i = 0; i < nMorphVertices; i++) {
        internal::getData(ptr, vunit);
        int vertexIndex = vunit.vertexIndex;
        if (internal::checkBound(vertexIndex, 0, nvertices)) {
            Vertex *vertex = vertices[vertexIndex];
            m_vertexRefs.add(vertex);
        }
        ptr += sizeof(vunit);
    }
    m_name = m_encodingRef->toString(unit.name, IString::kShiftJIS, kNameSize);
    m_category = static_cast<Category>(unit.type);
    size = ptr - data;
}

size_t Morph::estimateSize(const Model::DataInfo & /* info */) const
{
    size_t size = 0;
    size += sizeof(MorphUnit);
    return size;
}

void Morph::write(uint8_t *data, const Model::DataInfo & /* info */) const
{
    MorphUnit unit;
    uint8_t *name = m_encodingRef->toByteArray(m_name, IString::kShiftJIS);
    internal::copyBytes(unit.name, name, sizeof(unit.name));
    m_encodingRef->disposeByteArray(name);
    unit.nvertices = m_vertexRefs.count();
    unit.type = m_category;
    internal::copyBytes(data, reinterpret_cast<const uint8_t *>(&unit), sizeof(unit));
}

IMorph::Category Morph::category() const
{
    return m_category;
}

IMorph::Type Morph::type() const
{
    return kVertex;
}

bool Morph::hasParent() const
{
    return false;
}

const IMorph::WeightPrecision &Morph::weight() const
{
    return m_weight;
}

void Morph::setWeight(const WeightPrecision &value)
{
    m_weight = value;
}

void Morph::setIndex(int value)
{
    m_index = value;
}

}
}
