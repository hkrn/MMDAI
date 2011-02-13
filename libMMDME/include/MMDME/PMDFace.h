/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2010  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn (libMMDAI)                         */
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

/* PMDFaceVertex: vertex of this model */
typedef struct _PMDFaceVertex {
   unsigned long id; /* vertex index of this model to be controlled */
   btVector3 pos;    /* position to be placed if this face rate is 1.0 */
} PMDFaceVertex;

/* PMDFace: face of PMD */
class PMDFace
{
private:

   static const unsigned int kMaxVertexID = 65536;

   char *m_name;              /* name of this face */
   unsigned char m_type;      /* face type (PMD_FACE_TYPE) */
   unsigned int m_numVertex;  /* number of vertices controlled by this face */
   PMDFaceVertex *m_vertex;   /* vertices controlled by this face */
   float m_weight;            /* current weight of this face */

   /* initialize: initialize face */
   void initialize();

   /* clear: free face */
   void clear();

   MMDME_DISABLE_COPY_AND_ASSIGN(PMDFace);

public:

   /* PMDFace: constructor */
   PMDFace();

   /* ~PMDFace: destructor */
   ~PMDFace();

   /* setup: initialize and setup face */
   void setup(PMDFile_Face *face, PMDFile_Face_Vertex *faceVertexList);

   /* convertIndex: convert base-relative index to model vertex index */
   void convertIndex(PMDFace *base);

   /* apply: apply this face morph to model vertices */
   void apply(btVector3 *vertexList);

   /* add: add this face morph to model vertices with a certain rate */
   void add(btVector3 *vertexList, float rate);

   /* getName: get name */
   const char *getName() const;

   /* getWeight: get weight */
   float getWeight() const;

   /* setWeight: set weight */
   void setWeight(float f);
};

#endif

