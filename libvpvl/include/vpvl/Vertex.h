/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn                                    */
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

#ifndef VPVL_VERTEX_H_
#define VPVL_VERTEX_H_

#include <LinearMath/btAlignedObjectArray.h>
#include <LinearMath/btVector3.h>
#include "vpvl/common.h"

namespace vpvl
{

class Vertex
{
public:
    Vertex();
    ~Vertex();

    static size_t stride();

    void read(const uint8_t *data);

    const btVector3 &position() const {
        return m_position;
    }
    const btVector3 &normal() const {
        return m_normal;
    }
    float u() const {
        return m_u;
    }
    float v() const {
        return m_v;
    }
    int16_t bone1() const {
        return m_bone1;
    }
    int16_t bone2() const {
        return m_bone2;
    }
    float weight() const {
        return m_weight;
    }
    bool isEdgeEnabled() const {
        return m_edge;
    }

    void setPosition(const btVector3 &value) {
        m_position = value;
    }
    void setNormal(const btVector3 &value) {
        m_normal = value;
    }
    void setU(float value) {
        m_u = value;
    }
    void setV(float value) {
        m_v = value;
    }
    void setWeight(float value) {
        m_weight = value;
    }
    void setEdgeEnable(bool value) {
        m_edge = value;
    }

private:
    btVector3 m_position;
    btVector3 m_normal;
    float m_u;
    float m_v;
    int16_t m_bone1;
    int16_t m_bone2;
    float m_weight;
    bool m_edge;
};

typedef btAlignedObjectArray<Vertex*> VertexList;
typedef btAlignedObjectArray<uint16_t> IndexList;

} /* namespace vpvl */

#endif
