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
    Vertex(IModel *modelRef);
    ~Vertex();

    static bool preparse(uint8_t *&data, size_t &rest, Model::DataInfo &info);
    static bool loadVertices(const Array<Vertex *> &vertices, const Array<Bone *> &bones);
    static void writeVertices(const Array<Vertex *> &vertices, const Model::DataInfo &info, uint8_t *&data);
    static size_t estimateTotalSize(const Array<Vertex *> &vertices, const Model::DataInfo &info);

    /**
     * Read and parse the buffer with id and sets it's result to the class.
     *
     * @param data The buffer to read and parse
     * @param info Model information
     * @param size Size of vertex to be output
     */
    void read(const uint8_t *data, const Model::DataInfo &info, size_t &size);
    void write(uint8_t *&data, const Model::DataInfo &info) const;
    size_t estimateSize(const Model::DataInfo &info) const;
    void reset();
    void mergeMorph(const Morph::UV *morph, const IMorph::WeightPrecision &weight);
    void mergeMorph(const Morph::Vertex *morph, const IMorph::WeightPrecision &weight);
    void performSkinning(Vector3 &position, Vector3 &normal) const;

    IModel *parentModelRef() const;
    Vector3 origin() const;
    Vector3 delta() const;
    Vector3 normal() const;
    Vector3 textureCoord() const;
    Vector4 uv(int index) const;
    Type type() const;
    EdgeSizePrecision edgeSize() const;
    WeightPrecision weight(int index) const;
    IBone *bone(int index) const;
    IMaterial *material() const;
    int index() const;
    Vector3 sdefC() const;
    Vector3 sdefR0() const;
    Vector3 sdefR1() const;

    void setOrigin(const Vector3 &value);
    void setNormal(const Vector3 &value);
    void setTextureCoord(const Vector3 &value);
    void setUV(int index, const Vector4 &value);
    void setType(Type value);
    void setEdgeSize(const EdgeSizePrecision &value);
    void setWeight(int index, const WeightPrecision &weight);
    void setBoneRef(int index, IBone *value);
    void setMaterial(IMaterial *value);
    void setSdefC(const Vector3 &value);
    void setSdefR0(const Vector3 &value);
    void setSdefR1(const Vector3 &value);
    void setIndex(int value);

private:
    struct PrivateContext;
    PrivateContext *m_context;

    VPVL2_DISABLE_COPY_AND_ASSIGN(Vertex)
};

} /* namespace pmx */
} /* namespace vpvl2 */

#endif

