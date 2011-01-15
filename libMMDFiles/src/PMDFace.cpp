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

#include "MMDFiles.h"

/* PMDFace::initialize: initialize face */
void PMDFace::initialize()
{
   m_name = NULL;
   m_type = PMD_FACE_OTHER;
   m_numVertex = 0;
   m_vertex = NULL;
   m_weight = 0.0f;
}

/* PMDFace::clear: free face */
void PMDFace::clear()
{
   if(m_name)
      free(m_name);
   if (m_vertex)
      free(m_vertex);

   initialize();
}

/* PMDFace::PMDFace: constructor */
PMDFace::PMDFace()
{
   initialize();
}

/* PMDFace::~PMDFace: destructor */
PMDFace::~PMDFace()
{
   clear();
}

/* PMDFace::setup: initialize and setup face */
void PMDFace::setup(PMDFile_Face *face, PMDFile_Face_Vertex *faceVertexList)
{
   unsigned long i;
   char name[21];

   clear();

   /* name */
   strncpy(name, face->name, 20);
   name[20] = '\0';
   m_name = strdup(name);

   /* type */
   m_type = face->type;

   /* number of vertices */
   m_numVertex = face->numVertex;

   if (m_numVertex) {
      /* vertex list */
      m_vertex = (PMDFaceVertex *) malloc(sizeof(PMDFaceVertex) * m_numVertex);
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
}

/* PMDFace::convertIndex: convert base-relative index to model vertex index */
void PMDFace::convertIndex(PMDFace *base)
{
   unsigned long i, relID;

   if (m_vertex == NULL)
      return;

   if (m_type != PMD_FACE_BASE) {
      for (i = 0; i < m_numVertex; i++) {
         relID = m_vertex[i].id;
         if (relID >= base->m_numVertex) /* a workaround for some models with corrupted face index values... */
            relID -= PMDFACE_MAXVERTEXID;
         m_vertex[i].id = base->m_vertex[relID].id;
      }
   } else {
      /* for base face, just do workaround */
      for (i = 0; i < m_numVertex; i++) {
         if (m_vertex[i].id >= PMDFACE_MAXVERTEXID) /* a workaround for some models with corrupted face index values... */
            m_vertex[i].id -= PMDFACE_MAXVERTEXID;
      }
   }
}

/* PMDFace::apply: apply this face morph to model vertices */
void PMDFace::apply(btVector3 *vertexList)
{
   unsigned long i;

   if (m_vertex == NULL)
      return;

   for (i = 0; i < m_numVertex; i++)
      vertexList[m_vertex[i].id] = m_vertex[i].pos;
}

/* PMDFace::add: add this face morph to model vertices with a certain rate */
void PMDFace::add(btVector3 *vertexList, float rate)
{
   unsigned long i;

   if (m_vertex == NULL)
      return;

   for (i = 0; i < m_numVertex; i++)
      vertexList[m_vertex[i].id] += m_vertex[i].pos * rate;
}

/* PMDFace::getName: get name */
char *PMDFace::getName()
{
   return m_name;
}

/* PMDFace::getWeight: get weight */
float PMDFace::getWeight()
{
   return m_weight;
}

/* PMDFace::setWeight: set weight */
void PMDFace::setWeight(float f)
{
   m_weight = f;
}
