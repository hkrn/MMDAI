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

#include "vpvl/vpvl.h"
#include "vpvl/internal/util.h"

namespace vpvl
{

// FIXME: boundary check
size_t Face::totalSize(const uint8_t *data, size_t n)
{
    size_t size = 0;
    uint8_t *ptr = const_cast<uint8_t *>(data);
    for (size_t i = 0; i < n; i++) {
        ptr += 20;
        uint32_t nvertices = *reinterpret_cast<uint32_t *>(ptr);
        size_t rest = sizeof(uint32_t) + sizeof(uint8_t) + nvertices * (sizeof(uint32_t) + sizeof(float) * 3);
        size += 20 + rest;
        ptr += rest;
    }
    return size;
}

size_t Face::stride(const uint8_t *data)
{
    uint8_t *ptr = const_cast<uint8_t *>(data);
    size_t base = 20;
    ptr += base;
    const int nvertices = *reinterpret_cast<int *>(ptr);
    return base + sizeof(uint32_t) + sizeof(uint8_t) + nvertices * (sizeof(uint32_t) + sizeof(float) * 3);
}

Face::Face()
    : m_type(kOther),
      m_weight(0.0f)
{
    memset(m_name, 0, sizeof(m_name));
}

Face::~Face()
{
    memset(m_name, 0, sizeof(m_name));
    m_type = kOther;
    m_weight = 0.0f;
}

void Face::read(const uint8_t *data)
{
    uint8_t *ptr = const_cast<uint8_t *>(data);
    copyBytesSafe(m_name, ptr, sizeof(m_name));
    ptr += sizeof(m_name);
    uint32_t nvertices = *reinterpret_cast<uint32_t *>(ptr);
    ptr += sizeof(uint32_t);
    FaceType type = static_cast<FaceType>(*reinterpret_cast<uint8_t *>(ptr));
    ptr += sizeof(uint8_t);
    m_type = type;
    if (nvertices > 0) {
        for (uint32_t i = 0; i < nvertices; i++) {
            FaceVertex *vertex = new FaceVertex();
            vertex->id = *reinterpret_cast<uint32_t *>(ptr);
            ptr += sizeof(uint32_t);
            float pos[3];
            internal::vector3(ptr, pos);
#ifdef VPVL_COORDINATE_OPENGL
            vertex->position.setValue(pos[0], pos[1], -pos[2]);
#else
            vertex->position.setValue(pos[0], pos[1], pos[2]);
#endif
            m_vertices.push_back(vertex);
        }
    }
}

void Face::convertIndices(const Face *base)
{
    const uint32_t nvertices = m_vertices.size();
    const uint32_t baseNVertices = base->m_vertices.size();
    if (m_type != kBase) {
        for (uint32_t i = 0; i < nvertices; i++) {
            uint32_t relID = m_vertices[i]->id;
            if (relID >= baseNVertices)
                relID -= kMaxVertexID;
            m_vertices[i]->id = base->m_vertices[relID]->id;
        }
    }
    else {
        for (uint32_t i = 0; i < nvertices; i++) {
            if (m_vertices[i]->id >= kMaxVertexID)
                m_vertices[i]->id -= kMaxVertexID;
        }
    }
}

void Face::setVertices(VertexList &vertices)
{
    const uint32_t nv = vertices.size();
    const uint32_t nfv = m_vertices.size();
    for (uint32_t i = 0; i < nfv; i++) {
        const FaceVertex *fv = m_vertices[i];
        const uint32_t id = fv->id;
        if (id < nv)
            vertices[id]->setPosition(fv->position);
    }
}

void Face::setVertices(VertexList &vertices, float rate)
{
    const uint32_t nv = vertices.size();
    const uint32_t nfv = m_vertices.size();
    for (uint32_t i = 0; i < nfv; i++) {
        const FaceVertex *fv = m_vertices[i];
        const uint32_t id = fv->id;
        if (id < nv) {
            Vertex *vertex = vertices[id];
            vertex->setPosition(vertex->position() + fv->position * rate);
        }
    }
}

} /* namespace vpvl */
