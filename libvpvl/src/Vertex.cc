/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2010  Nagoya Institute of Technology          */
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

#include "vpvl/vpvl.h"
#include "vpvl/internal/util.h"

namespace vpvl
{

Vertex::Vertex()
    : m_position(0.0f, 0.0f, 0.0f),
      m_normal(0.0f, 0.0f, 0.0f),
      m_u(0.0f),
      m_v(0.0f),
      m_bone1(0),
      m_bone2(0),
      m_weight(0.0f),
      m_edge(false)
{
}

Vertex::~Vertex()
{
    m_position.setZero();
    m_normal.setZero();
    m_u = 0.0f;
    m_v = 0.0f;
    m_bone1 = 0;
    m_bone2 = 0;
    m_weight = 0.0f;
    m_edge = false;
}

size_t Vertex::stride(const uint8_t * /* data */)
{
    return sizeof(float) * 8 + sizeof(int16_t) * 2 + sizeof(uint8_t) * 2;
}

void Vertex::read(const uint8_t *data)
{
    uint8_t *ptr = const_cast<uint8_t *>(data);
    float pos[3], normal[3];
    internal::vector3(ptr, pos);
    internal::vector3(ptr, normal);
    float u = *reinterpret_cast<float *>(ptr);
    ptr += sizeof(float);
    float v = *reinterpret_cast<float *>(ptr);
    ptr += sizeof(float);
    int16_t bone1 = *reinterpret_cast<int16_t *>(ptr);
    ptr += sizeof(int16_t);
    int16_t bone2 = *reinterpret_cast<int16_t *>(ptr);
    ptr += sizeof(int16_t);
    uint8_t weight = *reinterpret_cast<uint8_t *>(ptr);
    ptr += sizeof(uint8_t);
    uint8_t edge = *reinterpret_cast<uint8_t *>(ptr);
    ptr += sizeof(uint8_t);

#ifdef VPVL_COORDINATE_OPENGL
    m_position.setValue(pos[0], pos[1], -pos[2]);
#else
    m_position.setValue(pos[0], pos[1], pos[2]);
#endif
#ifdef VPVL_COORDINATE_OPENGL
    m_normal.setValue(normal[0], normal[1], -normal[2]);
#else
    m_normal.setValue(normal[0], normal[1], normal[2]);
#endif
    m_u = u;
    m_v = v;
    m_bone1 = bone1;
    m_bone2 = bone2;
    m_weight = weight * 0.01f;
    m_edge = edge == 0;
}

}
