/**

 Copyright (c) 2010-2014  hkrn

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
#include "vpvl2/internal/util.h"

#include "vpvl2/pmx/SoftBody.h"

namespace
{

using namespace vpvl2;

#pragma pack(push, 1)

struct SoftBodyUnit {
    uint8 group;
    uint16 uncollidableGroup;
    uint8 flags;
    int distanceBLink;
    int numClusters;
    float32 mass;
    float32 margin;
    int aeroModel;
};

struct SoftBodyConfigUnit {
    float32 VCF;
    float32 VP;
    float32 DG;
    float32 LF;
    float32 PR;
    float32 VC;
    float32 DF;
    float32 MT;
    float32 CHR;
    float32 KHR;
    float32 SHR;
    float32 AHR;
};

struct SoftBodyClusterUnit {
    float32 SRHR_CL;
    float32 SKHR_CL;
    float32 SSHR_CL;
    float32 SR_SPLT_CL;
    float32 SK_SPLT_CL;
    float32 SS_SPLT_CL;
};

struct SoftBodyIterationUnit {
    int V_IT;
    int P_IT;
    int D_IT;
    int C_IT;
};

struct SoftBodyMaterialUnit {
    float32 LST;
    float32 AST;
    float32 VST;
};

#pragma pack(pop)

}

namespace vpvl2
{
namespace pmx
{

struct SoftBody::PrivateContext {
    PrivateContext(IModel *modelRef)
        : modelRef(modelRef),
          name(0),
          englishName(0),
          index(-1)
    {
    }
    ~PrivateContext() {
        modelRef = 0;
        internal::deleteObject(name);
        internal::deleteObject(englishName);
        index = 0;
    }

    IModel *modelRef;
    IString *name;
    IString *englishName;
    Array<PropertyEventListener *> eventRefs;
    int index;
};

SoftBody::SoftBody(IModel *modelRef)
    : m_context(new PrivateContext(modelRef))
{
}

SoftBody::~SoftBody()
{
    internal::deleteObject(m_context);
}

bool SoftBody::preparse(uint8 *&ptr, vsize &rest, Model::DataInfo &info)
{
    info.softBodiesPtr = ptr;
    if (info.version >= 2.1) {
        int nbodies = 0, size = 0;
        if (!internal::getTyped<int>(ptr, rest, nbodies)) {
            VPVL2_LOG(WARNING, "Invalid size of PMX bodies detected: size=" << nbodies << " rest=" << rest);
            return false;
        }
        for (int i = 0; i < nbodies; i++) {
            uint8 *namePtr;
            if (!internal::getText(ptr, rest, namePtr, size)) {
                VPVL2_LOG(WARNING, "Invalid size of PMX soft body name in Japanese detected: index=" << i << " size=" << size << " rest=" << rest);
                return false;
            }
            if (!internal::getText(ptr, rest, namePtr, size)) {
                VPVL2_LOG(WARNING, "Invalid size of PMX soft body name in English detected: index=" << i << " size=" << size << " rest=" << rest);
                return false;
            }
            vsize baseSize = sizeof(SoftBodyUnit) +
                    sizeof(SoftBodyConfigUnit) +
                    sizeof(SoftBodyClusterUnit) +
                    sizeof(SoftBodyIterationUnit) +
                    sizeof(SoftBodyMaterialUnit) +
                    info.materialIndexSize + 1; // shape type
            if (!internal::validateSize(ptr, baseSize, rest)) {
                VPVL2_LOG(WARNING, "Invalid size of PMX soft body unit detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
                return false;
            }
        }
        info.softBodiesCount = nbodies;
    }
    return true;
}

bool SoftBody::loadSoftBodies(const Array<SoftBody *> &bodies)
{
    const int nbodies = bodies.count();
    for (int i = 0; i < nbodies; i++) {
        SoftBody *body = bodies[i];
        body->setIndex(i);
    }
    return true;
}

void SoftBody::writeSoftBodies(const Array<SoftBody *> &bodies, const Model::DataInfo &info, uint8 *&data)
{
    if (info.version >= 2.1) {
        const int32 nbodies = bodies.count();
        internal::writeBytes(&nbodies, sizeof(nbodies), data);
        for (int32 i = 0; i < nbodies; i++) {
            const SoftBody *body = bodies[i];
            body->write(data, info);
        }
    }
}

vsize SoftBody::estimateTotalSize(const Array<SoftBody *> &bodies, const Model::DataInfo &info)
{
    vsize size = 0;
    if (info.version >= 2.1) {
        const int32 nbodies = bodies.count();
        size += sizeof(nbodies);
        for (int32 i = 0; i < nbodies; i++) {
            SoftBody *body = bodies[i];
            size += body->estimateSize(info);
        }
    }
    return size;
}

void SoftBody::read(const uint8 *data, const Model::DataInfo &info, vsize &size)
{
    uint8 *namePtr = 0, *ptr = const_cast<uint8 *>(data), *start = ptr;
    vsize rest = SIZE_MAX;
    int32 nNameSize = 0;
    IEncoding *encoding = info.encoding;
    internal::getText(ptr, rest, namePtr, nNameSize);
    internal::setStringDirect(encoding->toString(namePtr, nNameSize, info.codec), m_context->name);
    VPVL2_VLOG(3, "PMXSoftBody: name=" << internal::cstr(m_context->name, "(null)"));
    internal::getText(ptr, rest, namePtr, nNameSize);
    internal::setStringDirect(encoding->toString(namePtr, nNameSize, info.codec), m_context->englishName);
    VPVL2_VLOG(3, "PMXSoftBody: englishName=" << internal::cstr(m_context->englishName, "(null)"));
    /* TODO: implement this */
    size = ptr - start;
}

void SoftBody::write(uint8 *&data, const Model::DataInfo &info) const
{
    internal::writeString(m_context->name, info.codec, data);
    internal::writeString(m_context->englishName, info.codec, data);
    /* TODO: implement this */
}

vsize SoftBody::estimateSize(const Model::DataInfo &info) const
{
    vsize size = 0;
    size += internal::estimateSize(m_context->name, info.codec);
    size += internal::estimateSize(m_context->englishName, info.codec);
    /* TODO: implement this */
    return size;
}

void SoftBody::addEventListenerRef(PropertyEventListener *value)
{
    if (value) {
        m_context->eventRefs.remove(value);
        m_context->eventRefs.append(value);
    }
}

void SoftBody::removeEventListenerRef(PropertyEventListener *value)
{
    if (value) {
        m_context->eventRefs.remove(value);
    }
}

void SoftBody::getEventListenerRefs(Array<PropertyEventListener *> &value)
{
    value.copy(m_context->eventRefs);
}

const IString *SoftBody::name(IEncoding::LanguageType type) const
{
    switch (type) {
    case IEncoding::kDefaultLanguage:
    case IEncoding::kJapanese:
        return m_context->name;
    case IEncoding::kEnglish:
        return m_context->englishName;
    default:
        return 0;
    }
}

IModel *SoftBody::parentModelRef() const
{
    return m_context->modelRef;
}

SoftBody::ShapeType SoftBody::shapeType() const
{
    return kUnknownShape;
}

uint8 SoftBody::collideGroup() const
{
    return 0;
}

uint16 SoftBody::uncollideGroupFlags() const
{
    return 0;
}

bool SoftBody::hasBLink() const
{
    return false;
}

bool SoftBody::hasCluster() const
{
    return false;
}

bool SoftBody::hasLink() const
{
    return false;
}

int SoftBody::distanceBLink() const
{
    return 0;
}

int SoftBody::numCluster() const
{
    return 0;
}

float32 SoftBody::mass() const
{
    return 0;
}

float32 SoftBody::collisionMargin() const
{
    return 0;
}

SoftBody::AeroModelType SoftBody::aeroModelType() const
{
    return kUnknownAeroModel;
}

int SoftBody::index() const
{
    return m_context->index;
}

void SoftBody::setName(const IString *value, IEncoding::LanguageType type)
{
    switch (type) {
    case IEncoding::kDefaultLanguage:
    case IEncoding::kJapanese:
        if (value && !value->equals(m_context->name)) {
            VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, nameWillChange(value, type, this));
            internal::setString(value, m_context->name);
        }
        break;
    case IEncoding::kEnglish:
        if (value && !value->equals(m_context->englishName)) {
            VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, nameWillChange(value, type, this));
            internal::setString(value, m_context->englishName);
        }
        break;
    default:
        break;
    }
}

void SoftBody::setIndex(int value)
{
    m_context->index = value;
}

} /* namespace pmx */
} /* namespace vpvl2 */

