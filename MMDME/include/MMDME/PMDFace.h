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

#ifndef MMDME_PMDFACE_H_
#define MMDME_PMDFACE_H_

#include <btBulletDynamicsCommon.h>

#include "MMDME/Common.h"
#include "MMDME/PMDFile.h"

namespace MMDAI {

typedef struct _PMDFaceVertex {
    unsigned long id;
    btVector3 pos;
} PMDFaceVertex;

class PMDFace
{
public:
    PMDFace();
    ~PMDFace();

    void setup(const PMDFile_Face *face, const PMDFile_Face_Vertex *faceVertexList);
    void convertIndex(PMDFace *base);
    void apply(btVector3 *vertexList);
    void add(btVector3 *vertexList, float rate);

    const char *getName() const {
        return m_name;
    }
    const float getWeight() const {
        return m_weight;
    }
    void setWeight(const float value) {
        m_weight = value;
    }

private:
    static const unsigned int kMaxVertexID = 65536;

    void initialize();
    void clear();

    char *m_name;
    unsigned char m_type;
    unsigned int m_numVertex;
    PMDFaceVertex *m_vertex;
    float m_weight;

    MMDME_DISABLE_COPY_AND_ASSIGN(PMDFace);
};

} /* namespace */

#endif

