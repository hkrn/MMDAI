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

#ifndef VPVL_FACE_H_
#define VPVL_FACE_H_

#include <LinearMath/btAlignedObjectArray.h>
#include <LinearMath/btVector3.h>
#include "vpvl/Vertex.h"
#include "vpvl/common.h"

namespace vpvl
{

enum FaceType {
    kBase,
    kEyebrow,
    kEye,
    kLip,
    kOther
};

struct FaceVertex
{
    uint32_t id;
    btVector3 position;
};

class Face
{
public:
    Face();
    ~Face();

    static const uint32_t kMaxVertexID = 65536;
    static size_t totalSize(const uint8_t *data, size_t n);
    static size_t stride(const uint8_t *data);

    void read(const uint8_t *data);
    void convertIndices(const Face *base);
    void setVertices(VertexList &vertices);
    void setVertices(VertexList &vertices, float rate);

    const uint8_t *name() const {
        return m_name;
    }
    FaceType type() const {
        return m_type;
    }
    float weight() const {
        return m_weight;
    }

    void setName(const uint8_t *value) {
        copyBytesSafe(m_name, value, sizeof(m_name));
    }
    void setWeight(float value) {
        m_weight = value;
    }

private:
    uint8_t m_name[20];
    FaceType m_type;
    btAlignedObjectArray<FaceVertex *> m_vertices;
    float m_weight;
};

typedef btAlignedObjectArray<Face*> FaceList;

} /* namespace vpvl */

#endif
