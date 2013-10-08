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
#include "vpvl2/pmd2/Morph.h"
#include "vpvl2/pmd2/Vertex.h"

namespace
{

using namespace vpvl2;
using namespace vpvl2::pmd2;

#pragma pack(push, 1)

struct VertexMorphUnit {
    int32 vertexIndex;
    float32 position[3];
};

struct MorphUnit {
    uint8 name[internal::kPMDMorphNameSize];
    int32 nvertices;
    uint8 type;
};

#pragma pack(pop)

}

namespace vpvl2
{
namespace pmd2
{

/*
struct InternalVertex {
    Vector3 position;
    int base;
    int index;
};
*/

struct Morph::PrivateContext {
    PrivateContext(Model *parentModelRef, IEncoding *encodingRef)
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
        vertices.releaseAll();
        delete namePtr;
        namePtr = 0;
        delete englishNamePtr;
        englishNamePtr = 0;
        category = kBase;
        weight = 0;
        index = -1;
    }

    Model *parentModelRef;
    IEncoding *encodingRef;
    IString *namePtr;
    IString *englishNamePtr;
    Array<PropertyEventListener *> eventRefs;
    Category category;
    WeightPrecision weight;
    PointerArray<IMorph::Vertex> vertices;
    Array<vpvl2::pmd2::Vertex *> vertexRefs;
    int index;
};

const int Morph::kNameSize = internal::kPMDMorphNameSize;

Morph::Morph(Model *parentModelRef, IEncoding *encodingRef)
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
        pmd2::Vertex *vertex = m_context->vertexRefs[i];
        vertex->reset();
    }
}

bool Morph::preparse(uint8 *&ptr, vsize &rest, Model::DataInfo &info)
{
    uint16 size;
    if (!internal::getTyped<uint16>(ptr, rest, size)) {
        return false;
    }
    info.morphsCount = size;
    info.morphsPtr = ptr;
    MorphUnit unit;
    VertexMorphUnit vunit; (void) vunit;
    vsize unitSize = 0;
    for (vsize i = 0; i < size; i++) {
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

bool Morph::loadMorphs(const Array<Morph *> &morphs, const Array<vpvl2::pmd2::Vertex *> &vertices)
{
    const int nmorphs = morphs.count(), nvertices = vertices.count();
    Morph *baseMorph = 0;
    for (int i = 0; i < nmorphs; i++) {
        Morph *morph = morphs[i];
        if (morph->category() == kBase) {
            const Array<IMorph::Vertex *> &morphVertices = morph->m_context->vertices;
            const int nMorphVertices = morphVertices.count();
            Array<pmd2::Vertex *> &vertexRefs = morph->m_context->vertexRefs;
            for (int j = 0; j < nMorphVertices; j++) {
                const IMorph::Vertex *morphVertex = morphVertices[j];
                int vertexId = morphVertex->index;
                if (internal::checkBound(vertexId, 0, nvertices)) {
                    pmd2::Vertex *vertex = vertices[vertexId];
                    vertex->setOrigin(morphVertex->position);
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
                const Array<IMorph::Vertex *> &baseVertices = baseMorph->m_context->vertices;
                Array<IMorph::Vertex *> &morphVertices = morph->m_context->vertices;
                Array<pmd2::Vertex *> &vertexRefs = morph->m_context->vertexRefs;
                const int nMorphVertices = morphVertices.count(), nBaseVertices = baseVertices.count();
                for (int j = 0; j < nMorphVertices; j++) {
                    IMorph::Vertex *morphVertex = morphVertices[j];
                    int vertexIndex = morphVertex->index;
                    if (internal::checkBound(vertexIndex, 0, nBaseVertices)) {
                        int baseVertexIndex = baseVertices[vertexIndex]->index;
                        morphVertex->base = baseVertexIndex;
                        vertexRefs.append(vertices[baseVertexIndex]);
                    }
                }
            }
        }
    }
    return true;
}

void Morph::writeMorphs(const Array<Morph *> &morphs, const Model::DataInfo &info, uint8 *&data)
{
    const int nmorphs = morphs.count();
    internal::writeUnsignedIndex(nmorphs, sizeof(uint16), data);
    for (int i = 0; i < nmorphs; i++) {
        Morph *morph = morphs[i];
        morph->write(data, info);
    }
}

void Morph::writeEnglishNames(const Array<Morph *> &morphs, const Model::DataInfo &info, uint8 *&data)
{
    const IEncoding *encodingRef = info.encoding;
    const int nmorphs = morphs.count();
    for (int i = 0; i < nmorphs; i++) {
        Morph *morph = morphs[i];
        internal::writeStringAsByteArray(morph->name(IEncoding::kJapanese), IString::kShiftJIS, encodingRef, kNameSize, data);
    }
}

vsize Morph::estimateTotalSize(const Array<Morph *> &morphs, const Model::DataInfo &info)
{
    const int nmorphs = morphs.count();
    vsize size = sizeof(uint16);
    for (int i = 0; i < nmorphs; i++) {
        Morph *morph = morphs[i];
        size += morph->estimateSize(info);
    }
    return size;
}

void Morph::read(const uint8 *data, vsize &size)
{
    MorphUnit unit;
    VertexMorphUnit vunit;
    IMorph::Vertex *vertex = 0;
    internal::getData(data, unit);
    uint8 *ptr = const_cast<uint8 *>(data + sizeof(unit));
    int nMorphVertices = unit.nvertices;
    for (int i = 0; i < nMorphVertices; i++) {
        internal::getData(ptr, vunit);
        if (internal::checkBound(vunit.vertexIndex, 0, 65535)) {
            vertex = m_context->vertices.append(new IMorph::Vertex());
            internal::setPosition(vunit.position, vertex->position);
            vertex->index = vunit.vertexIndex;
        }
        ptr += sizeof(vunit);
    }
    internal::setStringDirect(m_context->encodingRef->toString(unit.name, IString::kShiftJIS, kNameSize), m_context->namePtr);
    m_context->category = static_cast<Category>(unit.type);
    size = ptr - data;
}

void Morph::readEnglishName(const uint8 *data, int index)
{
    if (data && index >= 0) {
        internal::setStringDirect(m_context->encodingRef->toString(data + kNameSize * index, IString::kShiftJIS, kNameSize), m_context->englishNamePtr);
    }
}

vsize Morph::estimateSize(const Model::DataInfo & /* info */) const
{
    vsize size = 0;
    size += sizeof(MorphUnit);
    size += m_context->vertices.count() * sizeof(VertexMorphUnit);
    return size;
}

void Morph::write(uint8 *&data, const Model::DataInfo & /* info */) const
{
    MorphUnit unit;
    uint8 *namePtr = unit.name;
    internal::writeStringAsByteArray(m_context->namePtr, IString::kShiftJIS, m_context->encodingRef, sizeof(unit.name), namePtr);
    unit.nvertices = m_context->vertices.count();
    unit.type = m_context->category;
    internal::writeBytes(&unit, sizeof(unit), data);
    VertexMorphUnit vunit;
    const int nvertices = m_context->vertices.count();
    for (int i = 0; i < nvertices; i++) {
        const IMorph::Vertex *vertex = m_context->vertices[i];
        vunit.vertexIndex = vertex->index;
        internal::getPosition(vertex->position, vunit.position);
        internal::writeBytes(&vunit, sizeof(vunit), data);
    }
}

void Morph::update()
{
    const int nvertices = m_context->vertexRefs.count();
    for (int i = 0; i < nvertices; i++) {
        pmd2::Vertex *vertex = static_cast<pmd2::Vertex *>(m_context->vertexRefs[i]);
        const IMorph::Vertex *v = m_context->vertices[i];
        vertex->mergeMorph(v->position, m_context->weight);
    }
}

IModel *Morph::parentModelRef() const
{
    return m_context->parentModelRef;
}

const IString *Morph::name(IEncoding::LanguageType type) const
{
    switch (type) {
    case IEncoding::kDefaultLanguage:
    case IEncoding::kJapanese:
        return m_context->namePtr;
    case IEncoding::kEnglish:
        return m_context->englishNamePtr;
    default:
        return 0;
    }
}

void Morph::setName(const IString *value, IEncoding::LanguageType type)
{
    switch (type) {
    case IEncoding::kDefaultLanguage:
    case IEncoding::kJapanese:
        if (value && !value->equals(m_context->namePtr)) {
            VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, nameWillChange(value, type, this));
            internal::setString(value, m_context->namePtr);
        }
        break;
    case IEncoding::kEnglish:
        if (value && !value->equals(m_context->englishNamePtr)) {
            VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, nameWillChange(value, type, this));
            internal::setString(value, m_context->englishNamePtr);
        }
        break;
    default:
        break;
    }
}

void Morph::addEventListenerRef(PropertyEventListener *value)
{
    if (value) {
        m_context->eventRefs.remove(value);
        m_context->eventRefs.append(value);
    }
}

void Morph::removeEventListenerRef(PropertyEventListener *value)
{
    if (value) {
        m_context->eventRefs.remove(value);
    }
}

void Morph::getEventListenerRefs(Array<PropertyEventListener *> &value)
{
    value.copy(m_context->eventRefs);
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
    if (m_context->weight != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, weightWillChange(value, this));
        m_context->weight = value;
    }
}

void Morph::setIndex(int value)
{
    m_context->index = value;
}

void Morph::markDirty()
{
    /* do nothing */
}

void Morph::addBoneMorph(Bone * /* value */)
{
}

void Morph::removeBoneMorph(Bone * /* value */)
{
}

void Morph::addGroupMorph(Group * /* value */)
{
}

void Morph::removeGroupMorph(Group * /* value */)
{
}

void Morph::addMaterialMorph(Material * /* value */)
{
}

void Morph::removeMaterialMorph(Material * /* value */)
{
}

void Morph::addUVMorph(UV * /* value */)
{
}

void Morph::removeUVMorph(UV * /* value */)
{
}

void Morph::addVertexMorph(Vertex *value)
{
    const IVertex *vertexRef = value->vertex;
    if (vertexRef && vertexRef->parentModelRef() == m_context->parentModelRef) {
        m_context->vertices.append(value);
    }
}

void Morph::removeVertexMorph(Vertex *value)
{
    m_context->vertices.remove(value);
}

void Morph::addFlipMorph(Flip * /* value */)
{
}

void Morph::removeFlipMorph(Flip * /* value */)
{
}

void Morph::addImpulseMorph(Impulse * /* value */)
{
}

void Morph::removeImpulseMorph(Impulse * /* value */)
{
}

void Morph::setType(Type /* value */)
{
}

void Morph::getBoneMorphs(Array<Bone *> &morphs) const
{
    morphs.clear();
}

void Morph::getGroupMorphs(Array<Group *> &morphs) const
{
    morphs.clear();
}

void Morph::getMaterialMorphs(Array<Material *> &morphs) const
{
    morphs.clear();
}

void Morph::getUVMorphs(Array<UV *> &morphs) const
{
    morphs.clear();
}

void Morph::getVertexMorphs(Array<Vertex *> &morphs) const
{
    morphs.copy(m_context->vertices);
}

void Morph::getFlipMorphs(Array<Flip *> &morphs) const
{
    morphs.clear();
}

void Morph::getImpulseMorphs(Array<Impulse *> &morphs) const
{
    morphs.clear();
}

} /* namespace pmd2 */
} /* namespace vpvl2 */
