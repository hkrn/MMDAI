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

#ifndef VPVL2_PMX_MORPH_H_
#define VPVL2_PMX_MORPH_H_

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
        pmx::Bone *bone;
        Vector3 position;
        Quaternion rotation;
        int index;
    };
    struct Group {
        Morph *morph;
        float weight;
        int index;
    };
    struct Material {
        pmx::Material *material;
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
        pmx::Vertex *vertex;
        Vector4 position;
        uint32_t index;
        int offset;
    };
    struct Vertex {
        pmx::Vertex *vertex;
        Vector3 position;
        uint32_t index;
    };


    /**
     * Constructor
     */
    Morph();
    ~Morph();

    static bool preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info);
    static bool loadMorphs(const Array<Morph *> &morphs,
                           const Array<pmx::Bone *> &bones,
                           const Array<pmx::Material *> &materials,
                           const Array<pmx::Vertex *> &vertices);

    /**
     * Read and parse the buffer with id and sets it's result to the class.
     *
     * @param data The buffer to read and parse
     */
    void read(const uint8_t *data, const Model::DataInfo &info, size_t &size);
    void write(uint8_t *data) const;

    void performTransform(float weight);
    void setWeight(float value);

    const IString *name() const { return m_name; }
    const IString *englishName() const { return m_englishName; }

private:
    static bool loadBones(const Array<pmx::Bone *> &bones, Morph *morph);
    static bool loadGroups(const Array<Morph *> &morphs, Morph *morph);
    static bool loadMaterials(const Array<pmx::Material *> &materials, Morph *morph);
    static bool loadUVs(const Array<pmx::Vertex *> &uv, int offset, Morph *morph);
    static bool loadVertices(const Array<pmx::Vertex *> &vertices, Morph *morph);
    void readBones(const Model::DataInfo &info, int count, uint8_t *&ptr);
    void readGroups(const Model::DataInfo &info, int count, uint8_t *&ptr);
    void readMaterials(const Model::DataInfo &info, int count, uint8_t *&ptr);
    void readUVs(const Model::DataInfo &info, int count, int offset, uint8_t *&ptr);
    void readVertices(const Model::DataInfo &info, int count, uint8_t *&ptr);

    Array<Vertex> m_vertices;
    Array<UV> m_uvs;
    Array<Bone> m_bones;
    Array<Material> m_materials;
    Array<Group> m_groups;
    IString *m_name;
    IString *m_englishName;
    uint8_t m_category;
    uint8_t m_type;

    VPVL2_DISABLE_COPY_AND_ASSIGN(Morph)
};

} /* namespace pmx */
} /* namespace vpvl2 */

#endif

