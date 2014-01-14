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

#pragma once
#ifndef VPVL2_PMX_SOFTBODY_H_
#define VPVL2_PMX_SOFTBODY_H_

#include "vpvl2/ISoftBody.h"
#include "vpvl2/pmx/Model.h"

namespace vpvl2
{
namespace pmx
{

class VPVL2_API SoftBody VPVL2_DECL_FINAL : public ISoftBody
{
public:
    SoftBody(IModel *modelRef);
    ~SoftBody();

    static bool preparse(uint8 *&ptr, vsize &rest, Model::DataInfo &info);
    static bool loadSoftBodies(const Array<SoftBody *> &bodies);
    static void writeSoftBodies(const Array<SoftBody *> &bodies, const Model::DataInfo &info, uint8 *&data);
    static vsize estimateTotalSize(const Array<SoftBody *> &bodies, const Model::DataInfo &info);

    void addEventListenerRef(PropertyEventListener *value);
    void removeEventListenerRef(PropertyEventListener *value);
    void getEventListenerRefs(Array<PropertyEventListener *> &value);

    void read(const uint8 *data, const Model::DataInfo &info, vsize &size);
    void write(uint8 *&data, const Model::DataInfo &info) const;
    vsize estimateSize(const Model::DataInfo &info) const;

    const IString *name(IEncoding::LanguageType type) const;
    IModel *parentModelRef() const;
    ShapeType shapeType() const;
    uint8 collideGroup() const;
    uint16 uncollideGroupFlags() const;
    bool hasBLink() const;
    bool hasCluster() const;
    bool hasLink() const;
    int distanceBLink() const;
    int numCluster() const;
    float32 mass() const;
    float32 collisionMargin() const;
    AeroModelType aeroModelType() const;
    int index() const;

    void setName(const IString *value, IEncoding::LanguageType type);
    void setIndex(int value);

private:
    struct PrivateContext;
    PrivateContext *m_context;

    VPVL2_DISABLE_COPY_AND_ASSIGN(SoftBody)
};

} /* namespace pmx */
} /* namespace vpvl2 */

#endif

