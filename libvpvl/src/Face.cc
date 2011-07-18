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

#pragma pack(push, 1)

struct FaceVertexChunk
{
    uint32_t vertexID;
    float position[3];
};

struct FaceChunk
{
    uint8_t name[20];
    uint32_t nvertices;
    uint8_t type;
};

#pragma pack(pop)

size_t Face::totalSize(const uint8_t *data, size_t rest, size_t count, bool &ok)
{
    size_t size = 0;
    uint8_t *ptr = const_cast<uint8_t *>(data);
    for (size_t i = 0; i < count; i++) {
        size_t required = stride(ptr);
        if (required > rest) {
            ok = false;
            return 0;
        }
        rest -= required;
        size += required;
        ptr += required;
    }
    ok = true;
    return size;
}

size_t Face::stride(const uint8_t *data)
{
    const FaceChunk *ptr = reinterpret_cast<const FaceChunk *>(data);
    return sizeof(*ptr) + ptr->nvertices * sizeof(FaceVertexChunk);
}

Face::Face()
    : m_type(kOther),
      m_weight(0.0f)
{
    internal::zerofill(m_name, sizeof(m_name));
}

Face::~Face()
{
    internal::zerofill(m_name, sizeof(m_name));
    internal::clearAll(m_vertices);
    m_type = kOther;
    m_weight = 0.0f;
}

void Face::read(const uint8_t *data)
{
    FaceChunk chunk;
    internal::copyBytes(reinterpret_cast<uint8_t *>(&chunk), data, sizeof(chunk));
    copyBytesSafe(m_name, chunk.name, sizeof(m_name));
    uint32_t nvertices = chunk.nvertices;
    Type type = static_cast<Type>(chunk.type);
    m_type = type;
    uint8_t *ptr = const_cast<uint8_t *>(data);
    ptr += sizeof(chunk);
    if (nvertices > 0) {
        FaceVertexChunk vc;
        for (uint32_t i = 0; i < nvertices; i++) {
            FaceVertex *vertex = new FaceVertex();
            internal::copyBytes(reinterpret_cast<uint8_t *>(&vc), ptr, sizeof(vc));
            vertex->id = vc.vertexID;
            float *pos = vc.position;
#ifdef VPVL_COORDINATE_OPENGL
            vertex->position.setValue(pos[0], pos[1], -pos[2]);
#else
            vertex->position.setValue(pos[0], pos[1], pos[2]);
#endif
            ptr += sizeof(vc);
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
