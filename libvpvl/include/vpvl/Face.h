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

#ifndef VPVL_FACE_H_
#define VPVL_FACE_H_

#include "vpvl/Common.h"
#include "vpvl/Vertex.h"

namespace vpvl
{

struct FaceVertex;

/**
 * @file
 * @author Nagoya Institute of Technology Department of Computer Science
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * Face class represents a face (Vertex Morphing) of a Polygon Model Data object.
 */

class VPVL_API Face
{
public:

    /**
     * Type of face kinds.
     */
    enum Type
    {
        kBase,
        kEyeblow,
        kEye,
        kLip,
        kOther
    };

    Face();
    ~Face();

    static const int kNameSize = 20;
    static const int kMaxVertexID = 65536;

    /**
     * Get byte size of all faces.
     *
     * @param data The buffer to read
     * @param rest Rest size of the buffer
     * @param count Count of all faces
     * @param ok True if the buffer is valid
     * @return Byte size of faces passed by count
     */
    static size_t totalSize(const uint8_t *data, size_t rest, size_t count, bool &ok);

    /**
     * Get stride of this face.
     *
     * @param data The buffer to read
     * @return Stride of the face
     */
    static size_t stride(const uint8_t *data);

    /**
     * Read and parse the buffer with id and sets it's result to the class.
     *
     * @param data The buffer to read and parse
     */
    void read(const uint8_t *data);

    size_t estimateSize() const;

    void write(uint8_t *data) const;

    /**
     * Convert indices of this face by the face.
     *
     * @param face A face to convert indices
     */
    void convertIndices(const Face *base);

    /**
     * Transform vertices with this face.
     *
     * @param vertices The vertices to transform
     */
    void setVertices(VertexList &vertices);

    /**
     * Transform vertices with this face by rate.
     *
     * @param vertices The vertices to transform
     */
    void setVertices(VertexList &vertices, float rate);

    /**
     * Get the name of this face.
     *
     * @return the name of this face
     */
    const uint8_t *name() const {
        return m_name;
    }

    /**
     * Get the name of this face in English.
     *
     * @return the name of this face in English.
     */
    const uint8_t *englishName() const {
        return m_englishName;
    }

    /**
     * Get the type of this face.
     *
     * @return the type of this face
     */
    Type type() const {
        return m_type;
    }

    /**
     * Get weight of this face.
     *
     * @return weight of this face
     */
    float weight() const {
        return m_weight;
    }

    /**
     * Set the name of this face.
     *
     * @param value the name of this face
     */
    void setName(const uint8_t *value) {
        copyBytesSafe(m_name, value, sizeof(m_name));
    }

    /**
     * Set the name of this face in English.
     *
     * @param value the name of this face in English
     */
    void setEnglishName(const uint8_t *value) {
        copyBytesSafe(m_englishName, value, sizeof(m_englishName));
    }

    /**
     * Set weight of this face.
     *
     * @param weight A weight value of this face
     */
    void setWeight(float value) {
        m_weight = value;
    }

private:
    uint8_t m_name[kNameSize + 1];
    uint8_t m_englishName[kNameSize + 1];
    Type m_type;
    Array<FaceVertex *> m_vertices;
    float m_weight;

    VPVL_DISABLE_COPY_AND_ASSIGN(Face)
};

typedef Array<Face*> FaceList;

} /* namespace vpvl */

#endif
