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
/* - Neither the name of the MMDAgent project team nor the names of  */
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

/* headers */

#include "MMDME/MMDME.h"

namespace MMDAI {

PMDFace::PMDFace()
    : m_name(NULL),
    m_type(PMD_FACE_OTHER),
    m_numVertex(0),
    m_vertex(NULL),
    m_weight(0.0f)
{
}

PMDFace::~PMDFace()
{
    release();
}

void PMDFace::release()
{
    MMDAIMemoryRelease(m_name);
    MMDAIMemoryRelease(m_vertex);

    m_name = NULL;
    m_type = PMD_FACE_OTHER;
    m_numVertex = 0;
    m_vertex = NULL;
    m_weight = 0.0f;
}

void PMDFace::setup(const PMDFile_Face *face, const PMDFile_Face_Vertex *faceVertexList)
{
    uint32_t i = 0;
    char name[21];

    release();

    /* name */
    MMDAIStringCopySafe(name, face->name, sizeof(name));
    m_name = MMDAIStringClone(name);

    /* type */
    m_type = face->type;

    /* number of vertices */
    m_numVertex = face->numVertex;

    if (m_numVertex) {
        /* vertex list */
        m_vertex = static_cast<PMDFaceVertex *>(MMDAIMemoryAllocate(sizeof(PMDFaceVertex) * m_numVertex));
        if (m_vertex == NULL) {
            MMDAILogWarnString("cannot allocate memory");
            return;
        }
        for (i = 0; i < m_numVertex; i++) {
            m_vertex[i].id = faceVertexList[i].vertexID;
            m_vertex[i].pos.setValue(faceVertexList[i].pos[0], faceVertexList[i].pos[1], faceVertexList[i].pos[2]);
        }
    }

#ifdef MMDFILES_CONVERTCOORDINATESYSTEM
    /* left-handed system: PMD, DirectX */
    /* right-handed system: OpenGL, bulletphysics */
    /* reverse Z value on vertices */
    for (i = 0; i < m_numVertex; i++) {
        m_vertex[i].pos.setZ(- m_vertex[i].pos.z());
    }
#endif

    MMDAILogDebugSJIS("name=\"%s\", type=%d, numVertex=%d", m_name, m_type, m_numVertex);
}

void PMDFace::convertIndex(PMDFace *base)
{
    assert(m_vertex != NULL);

    if (m_type != PMD_FACE_BASE) {
        for (uint32_t i = 0; i < m_numVertex; i++) {
            uint32_t relID = m_vertex[i].id;
            if (relID >= base->m_numVertex) /* a workaround for some models with corrupted face index values... */
                relID -= kMaxVertexID;
            m_vertex[i].id = base->m_vertex[relID].id;
        }
    } else {
        /* for base face, just do workaround */
        for (uint32_t i = 0; i < m_numVertex; i++) {
            if (m_vertex[i].id >= kMaxVertexID) /* a workaround for some models with corrupted face index values... */
                m_vertex[i].id -= kMaxVertexID;
        }
    }
}

void PMDFace::apply(btVector3 *vertexList)
{
    assert(vertexList != NULL && m_vertex != NULL);

    for (uint32_t i = 0; i < m_numVertex; i++)
        vertexList[m_vertex[i].id] = m_vertex[i].pos;
}

void PMDFace::add(btVector3 *vertexList, float rate)
{
    assert(vertexList != NULL && m_vertex != NULL);

    for (uint32_t i = 0; i < m_numVertex; i++)
        vertexList[m_vertex[i].id] += m_vertex[i].pos * rate;
}

} /* namespace */
