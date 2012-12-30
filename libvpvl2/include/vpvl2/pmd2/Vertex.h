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

#ifndef VPVL2_PMD2_VERTEX_H_
#define VPVL2_PMD2_VERTEX_H_

#include "vpvl2/Common.h"
#include "vpvl2/IMorph.h"
#include "vpvl2/IVertex.h"
#include "vpvl2/pmd2/Model.h"

namespace vpvl2
{

class IEncoding;
class IString;

namespace pmd2
{

class Bone;

class VPVL2_API Vertex : public IVertex
{
public:
    static const int kMaxBones = 2;

    Vertex(IModel *parentModelRef);
    ~Vertex();

    IModel *parentModelRef() const { return m_parentModelRef; }
    Vector3 origin() const { return m_origin; }
    Vector3 normal() const { return m_normal; }
    Vector3 textureCoord() const { return m_texcoord; }
    Vector4 uv(int /* index */) const { return kZeroV4; }
    Vector3 delta() const { return m_morphDelta; }
    Type type() const { return kBdef2; }
    float edgeSize() const { return m_edgeSize; }
    float weight(int index) const;
    IBone *bone(int index) const;
    IMaterial *material() const { return m_materialRef; }
    int index() const { return m_index; }
    void setOrigin(const Vector3 &value);
    void setNormal(const Vector3 &value);
    void setTextureCoord(const Vector3 &value);
    void setUV(int index, const Vector4 &value);
    void setType(Type value);
    void setEdgeSize(float value);
    void setWeight(int index, float weight);
    void setBone(int index, IBone *value);
    void setMaterial(IMaterial *value);

    static bool preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info);
    static bool loadVertices(const Array<Vertex *> &vertices, const Array<Bone *> &bones);
    static size_t estimateTotalSize(const Array<Vertex *> &vertices, const Model::DataInfo &info);

    void read(const uint8_t *data, const Model::DataInfo &info, size_t &size);
    size_t estimateSize(const Model::DataInfo &info) const;
    void write(uint8_t *data, const Model::DataInfo &info) const;
    void performSkinning(Vector3 &position, Vector3 &normal) const;
    void reset();
    void mergeMorph(const Vector3 &value, const IMorph::WeightPrecision &weight);

private:
    IModel *m_parentModelRef;
    Vector3 m_origin;
    Vector3 m_normal;
    Vector3 m_texcoord;
    Vector3 m_morphDelta;
    float m_edgeSize;
    float m_weight;
    IMaterial *m_materialRef;
    IBone *m_boneRefs[kMaxBones];
    int m_boneIndices[kMaxBones];
    int m_index;
};

} /* namespace pmd2 */
} /* namespace vpvl2 */

#endif
