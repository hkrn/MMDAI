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

#ifndef VPVL2_PMD_MORPH_H_
#define VPVL2_PMD_MORPH_H_

#include "vpvl2/Common.h"
#include "vpvl2/IVertex.h"

namespace vpvl2
{

class IEncoding;
class IString;

namespace pmd
{

class VPVL2_API Vertex : public IVertex
{
public:
    Vertex();
    ~Vertex();

    const Vector3 &origin() const { return m_origin; }
    const Vector3 &normal() const { return m_normal; }
    const Vector3 &textureCoord() const { return m_texcoord; }
    const Vector4 &uv(int /* index */) const { return kZeroV4; }
    Type type() const { return kBdef2; }
    float edgeSize() const { return m_edgeSize; }
    float weight(int /* index */) const { return m_weight; }
    IBone *bone(int /* index */) const { return m_boneRef; }
    void setOrigin(const Vector3 &value);
    void setNormal(const Vector3 &value);
    void setTextureCoord(const Vector3 &value);
    void setUV(int index, const Vector4 &value);
    void setType(Type value);
    void setEdgeSize(float value);
    void setWeight(int index, float weight);
    void setBone(int index, IBone *value);

    Vector3 m_origin;
    Vector3 m_normal;
    Vector3 m_texcoord;
    float m_edgeSize;
    float m_weight;
    IBone *m_boneRef;
};

}
}

#endif
