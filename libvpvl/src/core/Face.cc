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

struct FaceVertex
{
    int id;
    int rawID;
    Vector3 position;
};

struct FaceVertexChunk
{
    int vertexID;
    float position[3];
};

struct FaceChunk
{
    uint8_t name[Face::kNameSize];
    int nvertices;
    uint8_t type;
};

#pragma pack(pop)

size_t Face::totalSize(const uint8_t *data, size_t rest, size_t count, bool &ok)
{
    size_t size = 0;
    uint8_t *ptr = const_cast<uint8_t *>(data);
    for (size_t i = 0; i < count; i++) {
        if (sizeof(FaceVertexChunk) > rest) {
            ok = false;
            return 0;
        }
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
    m_vertices.releaseAll();
    m_type = kOther;
    m_weight = 0.0f;
}

void Face::read(const uint8_t *data)
{
    FaceChunk chunk;
    internal::copyBytes(reinterpret_cast<uint8_t *>(&chunk), data, sizeof(chunk));
    setName(chunk.name);
    const int nvertices = chunk.nvertices;
    Type type = static_cast<Type>(chunk.type);
    m_type = type;
    uint8_t *ptr = const_cast<uint8_t *>(data);
    ptr += sizeof(chunk);
    if (nvertices > 0) {
        FaceVertexChunk vc;
        for (int i = 0; i < nvertices; i++) {
            FaceVertex *vertex = new FaceVertex();
            internal::copyBytes(reinterpret_cast<uint8_t *>(&vc), ptr, sizeof(vc));
            vertex->id = vc.vertexID;
            vertex->rawID = vc.vertexID;
#ifdef VPVL_BUILD_IOS
            float pos[3];
            memcpy(pos, &vc.position, sizeof(pos));
#else
            float *pos = vc.position;
#endif
#ifdef VPVL_COORDINATE_OPENGL
            vertex->position.setValue(pos[0], pos[1], -pos[2]);
#else
            vertex->position.setValue(pos[0], pos[1], pos[2]);
#endif
            ptr += sizeof(vc);
            m_vertices.add(vertex);
        }
    }
}

size_t Face::estimateSize() const
{
    return sizeof(FaceChunk) + m_vertices.count() * sizeof(FaceVertexChunk);
}

void Face::write(uint8_t *data) const
{
    FaceChunk chunk;
    int nvertices = m_vertices.count();
    internal::copyBytes(chunk.name, m_name, sizeof(chunk.name));
    chunk.nvertices = nvertices;
    chunk.type = m_type;
    uint8_t *ptr = data;
    internal::copyBytes(ptr, reinterpret_cast<const uint8_t *>(&chunk), sizeof(chunk));
    ptr += sizeof(chunk);
    if (nvertices > 0) {
        FaceVertexChunk vc;
        for (int i = 0; i < nvertices; i++) {
            FaceVertex *vertex = m_vertices[i];
            const Vector3 &p = vertex->position;
            vc.vertexID = vertex->rawID;
            vc.position[0] = p.x();
            vc.position[1] = p.y();
#ifdef VPVL_COORDINATE_OPENGL
            vc.position[2] = -p.z();
#else
            vc.position[2] = p.z();
#endif
            internal::copyBytes(ptr, reinterpret_cast<const uint8_t *>(&vc), sizeof(vc));
            ptr += sizeof(vc);
        }
    }
}

void Face::convertIndices(const Face *base)
{
    const int nvertices = m_vertices.count();
    const int nBaseVertices = base->m_vertices.count();
    if (m_type != kBase) {
        for (int i = 0; i < nvertices; i++) {
            int relID = m_vertices[i]->id;
            if (relID < 0 || relID >= nBaseVertices)
                continue;
            m_vertices[i]->id = base->m_vertices[relID]->id;
        }
    }
    else {
        int max = kMaxVertexID;
        for (int i = 0; i < nvertices; i++)
            btSetMin(m_vertices[i]->id, max);
    }
}

void Face::setBaseVertices(VertexList &vertices)
{
    const int nv = vertices.count();
    const int nfv = m_vertices.count();
    for (int i = 0; i < nfv; i++) {
        const FaceVertex *fv = m_vertices[i];
        int id = fv->id;
        if (id < nv)
            vertices[id]->setPosition(fv->position);
    }
}

void Face::setVertices(VertexList &vertices, float rate)
{
    const int nv = vertices.count();
    const int nfv = m_vertices.count();
    for (int i = 0; i < nfv; i++) {
        const FaceVertex *fv = m_vertices[i];
        int id = fv->id;
        if (id < nv) {
            Vertex *vertex = vertices[id];
            vertex->setPosition(vertex->position() + fv->position * rate);
        }
    }
}

void Face::setName(const uint8_t *value)
{
    copyBytesSafe(m_name, value, sizeof(m_name));
}

void Face::setEnglishName(const uint8_t *value)
{
    copyBytesSafe(m_englishName, value, sizeof(m_englishName));
}

void Face::setWeight(float value)
{
    m_weight = value;
}

} /* namespace vpvl */
