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

#ifndef VPVL2_PMX_VERTEX_H_
#define VPVL2_PMX_VERTEX_H_

#include "vpvl2/IVertex.h"
#include "vpvl2/pmx/Model.h"
#include "vpvl2/pmx/Morph.h"

namespace vpvl2
{

class IBone;

namespace pmx
{

/**
 * @file
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * Vertex class represents a morph of a Polygon Vertex Extended object.
 */

class VPVL2_API Vertex : public IVertex
{
public:
    static const int kMaxBones = 4;
    static const int kMaxMorphs = 5; /* TexCoord + UVA1-4 */

    /**
     * Constructor
     */
    Vertex();
    ~Vertex();

    static bool preparse(uint8_t *&data, size_t &rest, Model::DataInfo &info);
    static bool loadVertices(const Array<Vertex *> &vertices,
                             const Array<Bone *> &bones);

    /**
     * Read and parse the buffer with id and sets it's result to the class.
     *
     * @param data The buffer to read and parse
     * @param info Model information
     * @param size Size of vertex to be output
     */
    void read(const uint8_t *data, const Model::DataInfo &info, size_t &size);
    void write(uint8_t *data, const Model::DataInfo &info) const;
    size_t estimateSize(const Model::DataInfo &info) const;
    void reset();
    void mergeMorph(const Morph::UV *morph, float weight);
    void mergeMorph(const Morph::Vertex *morph, float weight);
    void performSkinning(Vector3 &position, Vector3 &normal) const;

    const Vector3 &origin() const { return m_origin; }
    const Vector3 &delta() const { return m_morphDelta; }
    const Vector3 &normal() const { return m_normal; }
    const Vector3 &textureCoord() const { return m_texcoord; }
    const Vector4 &uv(int index) const;
    Type type() const { return m_type; }
    float edgeSize() const { return m_edgeSize; }
    float weight(int index) const;
    IBone *bone(int index) const;
    const Vector3 &sdefC() const { return m_c; }
    const Vector3 &sdefR0() const { return m_r0; }
    const Vector3 &sdefR1() const { return m_r1; }

    void setOrigin(const Vector3 &value);
    void setNormal(const Vector3 &value);
    void setTextureCoord(const Vector3 &value);
    void setUV(int index, const Vector4 &value);
    void setType(Type value);
    void setEdgeSize(float value);
    void setWeight(int index, float weight);
    void setBone(int index, IBone *value);
    void setSdefC(const Vector3 &value);
    void setSdefR0(const Vector3 &value);
    void setSdefR1(const Vector3 &value);

private:
    IBone *m_boneRefs[kMaxBones];
    Vector4 m_originUVs[kMaxMorphs];
    Vector4 m_morphUVs[kMaxMorphs];
    Vector3 m_origin;
    Vector3 m_morphDelta;
    Vector3 m_normal;
    Vector3 m_texcoord;
    Vector3 m_c;
    Vector3 m_r0;
    Vector3 m_r1;
    Type m_type;
    float m_edgeSize;
    float m_weight[kMaxBones];
    int m_boneIndices[kMaxBones];

    VPVL2_DISABLE_COPY_AND_ASSIGN(Vertex)
};

} /* namespace pmx */
} /* namespace vpvl2 */

#endif

