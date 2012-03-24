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

#include "vpvl2/pmx/Model.h"
#include "vpvl2/pmx/Morph.h"

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
 * Vertex class represents a morph of a Polygon Vertex Extended object.
 */

class VPVL2_API Vertex
{
public:
    enum Type {
        kBdef1,
        kBdef2,
        kBdef4,
        kSdef
    };

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
    void write(uint8_t *data) const;
    void mergeMorph(Morph::UV *morph, float weight);
    void mergeMorph(Morph::Vertex *morph, float weight);
    void performSkinning(Vector3 &position, Vector3 &normal);

    const Vector3 &origin() const { return m_origin; }
    const Vector3 &position() const { return m_position; }
    const Vector3 &normal() const { return m_normal; }
    const Vector3 &texcoord() const { return m_texcoord; }
    const Vector4 &uv(int index) const;
    Type type() const { return m_type; }
    float edge() const { return m_edge; }

private:
    Bone *m_bones[4];
    Vector4 m_originUVs[4];
    Vector4 m_positionUVs[4];
    Vector3 m_origin;
    Vector3 m_position;
    Vector3 m_normal;
    Vector3 m_texcoord;
    Vector3 m_c;
    Vector3 m_r0;
    Vector3 m_r1;
    Type m_type;
    float m_edge;
    float m_weight[4];
    int m_boneIndices[4];

    VPVL2_DISABLE_COPY_AND_ASSIGN(Vertex)
};

} /* namespace pmx */
} /* namespace vpvl2 */

#endif

