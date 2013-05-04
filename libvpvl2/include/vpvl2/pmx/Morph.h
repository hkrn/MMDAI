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
              weight(0),
              index(-1)
        {
        }
        Morph *morph;
        float weight;
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
        float shininess;
        float edgeSize;
        int index;
        uint8_t operation;
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
        uint32_t index;
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
        uint32_t index;
    };
    struct Flip {
        Flip()
            : morph(0),
              weight(0),
              index(-1)
        {
        }
        pmx::Morph *morph;
        float weight;
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

    static bool preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info);
    static bool loadMorphs(const Array<Morph *> &morphs,
                           const Array<pmx::Bone *> &bones,
                           const Array<pmx::Material *> &materials,
                           const Array<pmx::RigidBody *> &rigidBodies,
                           const Array<pmx::Vertex *> &vertices);
    static void writeMorphs(const Array<Morph *> &morphs, const Model::DataInfo &info, uint8_t *&data);
    static size_t estimateTotalSize(const Array<Morph *> &morphs, const Model::DataInfo &info);

    void resetTransform();
    void read(const uint8_t *data, const Model::DataInfo &info, size_t &size);
    void write(uint8_t *&data, const Model::DataInfo &info) const;
    size_t estimateSize(const Model::DataInfo &info) const;

    WeightPrecision weight() const { return m_weight; }
    void setWeight(const WeightPrecision &value);
    void updateVertexMorphs(const WeightPrecision &value);
    void updateBoneMorphs(const WeightPrecision &value);
    void updateUVMorphs(const WeightPrecision &value);
    void updateMaterialMorphs(const WeightPrecision &value);
    void updateGroupMorphs(const WeightPrecision &value);
    void updateFlipMorphs(const WeightPrecision &value);
    void updateImpluseMorphs(const WeightPrecision &value);

    void resetVertexMorphs();
    void resetUVMorphs();
    void resetImpluseMorphs();

    const IString *name() const { return m_name; }
    const IString *englishName() const { return m_englishName; }
    IModel *parentModelRef() const { return m_modelRef; }
    Category category() const { return m_category; }
    Type type() const { return m_type; }
    int index() const { return m_index; }
    bool hasParent() const { return m_hasParent; }

    void setName(const IString *value);
    void setEnglishName(const IString *value);
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

    const Array<Bone *> &bones() const { return m_bones; }
    const Array<Group *> &groups() const { return m_groups; }
    const Array<Material *> &materials() const { return m_materials; }
    const Array<UV *> &uvs() const { return m_uvs; }
    const Array<Vertex *> &vertices() const { return m_vertices; }
    const Array<Flip *> &flips() const { return m_flips; }
    const Array<Impulse *> &impulses() const { return m_impulses; }

private:
    static bool loadBones(const Array<pmx::Bone *> &bones, Morph *morph);
    static bool loadGroups(const Array<Morph *> &morphs, Morph *morph);
    static bool loadMaterials(const Array<pmx::Material *> &materials, Morph *morph);
    static bool loadUVs(const Array<pmx::Vertex *> &vertices, int offset, Morph *morph);
    static bool loadVertices(const Array<pmx::Vertex *> &vertices, Morph *morph);
    static bool loadFlips(const Array<pmx::Morph *> &morphs, Morph *morph);
    static bool loadImpulses(const Array<pmx::RigidBody *> &rigidBodies, Morph *morph);
    void readBones(const Model::DataInfo &info, int count, uint8_t *&ptr);
    void readGroups(const Model::DataInfo &info, int count, uint8_t *&ptr);
    void readMaterials(const Model::DataInfo &info, int count, uint8_t *&ptr);
    void readUVs(const Model::DataInfo &info, int count, int offset, uint8_t *&ptr);
    void readVertices(const Model::DataInfo &info, int count, uint8_t *&ptr);
    void readFlips(const Model::DataInfo &info, int count, uint8_t *&ptr);
    void readImpulses(const Model::DataInfo &info, int count, uint8_t *&ptr);
    void writeBones(const Model::DataInfo &info, uint8_t *&ptr) const;
    void writeGroups(const Model::DataInfo &info, uint8_t *&ptr) const;
    void writeMaterials(const Model::DataInfo &info, uint8_t *&ptr) const;
    void writeUVs(const Model::DataInfo &info, uint8_t *&ptr) const;
    void writeVertices(const Model::DataInfo &info, uint8_t *&ptr) const;
    void writeFlips(const Model::DataInfo &info, uint8_t *&ptr) const;
    void writeImpulses(const Model::DataInfo &info, uint8_t *&ptr) const;

    PointerArray<Vertex> m_vertices;
    PointerArray<UV> m_uvs;
    PointerArray<Bone> m_bones;
    PointerArray<Material> m_materials;
    PointerArray<Group> m_groups;
    PointerArray<Flip> m_flips;
    PointerArray<Impulse> m_impulses;
    IModel *m_modelRef;
    IString *m_name;
    IString *m_englishName;
    WeightPrecision m_weight;
    Category m_category;
    Type m_type;
    int m_index;
    bool m_hasParent;

    VPVL2_DISABLE_COPY_AND_ASSIGN(Morph)
};

} /* namespace pmx */
} /* namespace vpvl2 */

#endif

