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

#pragma once
#ifndef VPVL2_PMX_MORPH_H_
#define VPVL2_PMX_MORPH_H_

#include "vpvl2/IMorph.h"
#include "vpvl2/pmx/Model.h"

namespace vpvl2
{
namespace pmx
{

/**
 * @file
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * Morph class represents a morph of a Polygon Model Extended object.
 */

class VPVL2_API Morph : public IMorph
{
public:
    struct Bone {
        Bone()
            : bone(0),
              index(-1)
        {
        }
        pmx::Bone *bone;
        Vector3 position;
        Quaternion rotation;
        int index;
    };
    struct Group {
        Group()
            : morph(0),
              fixedWeight(0),
              index(-1)
        {
        }
        Morph *morph;
        WeightPrecision fixedWeight;
        int index;
    };
    struct Material {
        Material()
            : materials(0),
              shininess(0),
              edgeSize(0),
              index(-1),
              operation(0)
        {
        }
        ~Material() {
            delete materials;
            materials = 0;
        }
        Array<pmx::Material *> *materials;
        Vector3 ambient;
        Vector4 diffuse;
        Vector3 specular;
        Vector4 edgeColor;
        Vector4 textureWeight;
        Vector4 sphereTextureWeight;
        Vector4 toonTextureWeight;
        float32 shininess;
        IVertex::EdgeSizePrecision edgeSize;
        int index;
        uint8 operation;
    };
    struct UV {
        UV()
            : vertex(0),
              index(-1),
              offset(0)
        {
        }
        pmx::Vertex *vertex;
        Vector4 position;
        uint32 index;
        int offset;
    };
    struct Vertex {
        Vertex()
            : vertex(0),
              index(-1)
        {
        }
        pmx::Vertex *vertex;
        Vector3 position;
        uint32 index;
    };
    struct Flip {
        Flip()
            : morph(0),
              fixedWeight(0),
              index(-1)
        {
        }
        pmx::Morph *morph;
        WeightPrecision fixedWeight;
        int index;
    };
    struct Impulse {
        Impulse()
            : rigidBody(0),
              velocity(kZeroV3),
              torque(kZeroV3),
              index(-1),
              isLocal(false)
        {
        }
        pmx::RigidBody *rigidBody;
        Vector3 velocity;
        Vector3 torque;
        int index;
        bool isLocal;
    };

    Morph(IModel *modelRef);
    ~Morph();

    static bool preparse(uint8 *&ptr, vsize &rest, Model::DataInfo &info);
    static bool loadMorphs(const Array<Morph *> &morphs,
                           const Array<pmx::Bone *> &bones,
                           const Array<pmx::Material *> &materials,
                           const Array<pmx::RigidBody *> &rigidBodies,
                           const Array<pmx::Vertex *> &vertices);
    static void writeMorphs(const Array<Morph *> &morphs, const Model::DataInfo &info, uint8 *&data);
    static vsize estimateTotalSize(const Array<Morph *> &morphs, const Model::DataInfo &info);

    void addEventListenerRef(PropertyEventListener *value);
    void removeEventListenerRef(PropertyEventListener *value);
    void getEventListenerRefs(Array<PropertyEventListener *> &value);

    void read(const uint8 *data, const Model::DataInfo &info, vsize &size);
    void write(uint8 *&data, const Model::DataInfo &info) const;
    vsize estimateSize(const Model::DataInfo &info) const;

    WeightPrecision weight() const;
    void setWeight(const WeightPrecision &value);
    void update();
    void markDirty();
    void syncWeight();
    void updateVertexMorphs(const WeightPrecision &value);
    void updateBoneMorphs(const WeightPrecision &value);
    void updateUVMorphs(const WeightPrecision &value);
    void updateMaterialMorphs(const WeightPrecision &value);
    void updateGroupMorphs(const WeightPrecision &value, bool flipOnly);
    void updateFlipMorphs(const WeightPrecision &value);
    void updateImpluseMorphs(const WeightPrecision &value);

    const IString *name(IEncoding::LanguageType type) const;
    void setName(const IString *value, IEncoding::LanguageType type);
    IModel *parentModelRef() const;
    Category category() const;
    Type type() const;
    int index() const;
    bool hasParent() const;
    const Array<Bone *> &bones() const;
    const Array<Group *> &groups() const;
    const Array<Material *> &materials() const;
    const Array<UV *> &uvs() const;
    const Array<Vertex *> &vertices() const;
    const Array<Flip *> &flips() const;
    const Array<Impulse *> &impulses() const;

    void addBoneMorph(Bone *value);
    void addGroupMorph(Group *value);
    void addMaterialMorph(Material *value);
    void addUVMorph(UV *value);
    void addVertexMorph(Vertex *value);
    void addFlip(Flip *value);
    void addImpulse(Impulse *value);
    void setCategory(Category value);
    void setType(Type value);
    void setIndex(int value);
    void setInternalWeight(const WeightPrecision &value);

private:
    struct PrivateContext;
    PrivateContext *m_context;

    VPVL2_DISABLE_COPY_AND_ASSIGN(Morph)
};

} /* namespace pmx */
} /* namespace vpvl2 */

#endif

