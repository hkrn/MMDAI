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

#include <GLee.h>

#include "MMDME/PMDModel.h"

#include "PMDInternal.h"

/* PMDModel::renderModel: render the model */
/* needs multi-texture function on OpenGL: */
/* texture unit 0: model texture */
/* texture unit 1: toon texture for toon shading */
/* texture unit 2: additional sphere map texture, if exist */
void PMDModel::renderModel()
{
   unsigned long i;
   float c[4];
   PMDMaterial *m;
   float modelAlpha;
   unsigned short *surfaceData;

   if (!m_vertexList) return;

#ifndef MMDFILES_CONVERTCOORDINATESYSTEM
   glPushMatrix();
   glScalef(1.0f, 1.0f, -1.0f); /* from left-hand to right-hand */
   glCullFace(GL_FRONT);
#endif

   /* activate texture unit 0 */
   glActiveTextureARB(GL_TEXTURE0_ARB);
   glClientActiveTextureARB(GL_TEXTURE0_ARB);

   /* set lists */
   glEnableClientState(GL_VERTEX_ARRAY);
   glEnableClientState(GL_NORMAL_ARRAY);
   glVertexPointer(3, GL_FLOAT, sizeof(btVector3), m_skinnedVertexList);
   glNormalPointer(GL_FLOAT, sizeof(btVector3), m_skinnedNormalList);
   /* set model texture coordinates to texture unit 0 */
   glClientActiveTextureARB(GL_TEXTURE0_ARB);
   glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   glTexCoordPointer(2, GL_FLOAT, 0, m_texCoordList);
   if (m_toon) {
      /* set toon texture coordinates to texture unit 1 */
      glActiveTextureARB(GL_TEXTURE1_ARB);
      glEnable(GL_TEXTURE_2D);
      glClientActiveTextureARB(GL_TEXTURE1_ARB);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      if (m_selfShadowDrawing)
         /* when drawing a shadow part in shadow mapping, force toon texture coordinates to (0, 0) */
         glTexCoordPointer(2, GL_FLOAT, 0, m_toonTexCoordListForShadowMap);
      else
         glTexCoordPointer(2, GL_FLOAT, 0, m_toonTexCoordList);
      glActiveTextureARB(GL_TEXTURE0_ARB);
      glClientActiveTextureARB(GL_TEXTURE0_ARB);
   }
   if (m_hasSingleSphereMap) {
      /* this model contains single sphere map texture */
      /* set texture coordinate generation for sphere map on texture unit 0 */
      glEnable(GL_TEXTURE_2D);
      glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
      glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
      glDisable(GL_TEXTURE_2D);
   }
   if (m_hasMultipleSphereMap) {
      /* this model contains additional sphere map texture */
      /* set texture coordinate generation for sphere map on texture unit 2 */
      glActiveTextureARB(GL_TEXTURE2_ARB);
      glEnable(GL_TEXTURE_2D);
      glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
      glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
      glDisable(GL_TEXTURE_2D);
      glActiveTextureARB(GL_TEXTURE0_ARB);
   }

   /* calculate alpha value, applying model global alpha */
   modelAlpha = m_globalAlpha;
   surfaceData = m_surfaceList;

   /* render per material */
   for (i = 0; i < m_numMaterial; i++) {
      m = &(m_material[i]);
      /* set colors */
      c[3] = m->getAlpha() * modelAlpha;
      if (c[3] > 0.99f) c[3] = 1.0f; /* clamp to 1.0 */
      if (m_toon) {
         /* use averaged color of diffuse and ambient for both */
         m->copyAvgcol(c);
         glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, &(c[0]));
         m->copySpecular(c);
         glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, &(c[0]));
      } else {
         /* use each color */
         m->copyDiffuse(c);
         glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, &(c[0]));
         m->copyAmbient(c);
         glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, &(c[0]));
         m->copySpecular(c);
         glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, &(c[0]));
      }
      glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, m->getShiness());

      /* disable face culling for transparent materials */
      if (m->getAlpha() < 1.0f)
         glDisable(GL_CULL_FACE);
      else
         glEnable(GL_CULL_FACE);

      /* if using multiple texture units, set current unit to 0 */
      if (m_toon || m_hasMultipleSphereMap)
         glActiveTextureARB(GL_TEXTURE0_ARB);

      if (m->getTexture()) {
         /* bind model texture */
         glEnable(GL_TEXTURE_2D);
         glBindTexture(GL_TEXTURE_2D, m->getTexture()->getID());
         if (m_hasSingleSphereMap) {
            if (m->getTexture()->isSphereMap()) {
               /* this is sphere map */
               /* enable texture coordinate generation */
               if (m->getTexture()->isSphereMapAdd())
                  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
               glEnable(GL_TEXTURE_GEN_S);
               glEnable(GL_TEXTURE_GEN_T);
            } else {
               /* disable generation */
               glDisable(GL_TEXTURE_GEN_S);
               glDisable(GL_TEXTURE_GEN_T);
            }
         }
      } else {
         glDisable(GL_TEXTURE_2D);
      }

      if (m_toon) {
         /* set toon texture for texture unit 1 */
         glActiveTextureARB(GL_TEXTURE1_ARB);
         glBindTexture(GL_TEXTURE_2D, m_toonTextureID[m->getToonID()]);
         /* set GL_CLAMP_TO_EDGE for toon texture to avoid texture interpolation at edge */
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      }

      if (m_hasMultipleSphereMap) {
         if (m->getAdditionalTexture()) {
            /* this material has additional sphere map texture, bind it at texture unit 2 */
            glActiveTextureARB(GL_TEXTURE2_ARB);
            glEnable(GL_TEXTURE_2D);
            if (m->getAdditionalTexture()->isSphereMapAdd())
               glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
            else
               glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            glBindTexture(GL_TEXTURE_2D, m->getAdditionalTexture()->getID());
            glEnable(GL_TEXTURE_GEN_S);
            glEnable(GL_TEXTURE_GEN_T);
         } else {
            /* disable generation */
            glActiveTextureARB(GL_TEXTURE2_ARB);
            glDisable(GL_TEXTURE_2D);
         }
      }

      /* draw elements */
      glDrawElements(GL_TRIANGLES, m->getNumSurface(), GL_UNSIGNED_SHORT, surfaceData);

      /* move surface pointer to next material */
      surfaceData += m->getNumSurface();

      /* reset some parameters */
      if (m->getTexture() && m->getTexture()->isSphereMap() && m->getTexture()->isSphereMapAdd()) {
         if (m_toon)
            glActiveTextureARB(GL_TEXTURE0_ARB);
         glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      }
   }

   glDisableClientState(GL_VERTEX_ARRAY);
   glDisableClientState(GL_NORMAL_ARRAY);
   if (m_toon) {
      glClientActiveTextureARB(GL_TEXTURE0_ARB);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      if (m_hasSingleSphereMap) {
         glActiveTextureARB(GL_TEXTURE0_ARB);
         glDisable(GL_TEXTURE_GEN_S);
         glDisable(GL_TEXTURE_GEN_T);
      }
      glClientActiveTextureARB(GL_TEXTURE1_ARB);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      if (m_hasMultipleSphereMap) {
         glActiveTextureARB(GL_TEXTURE2_ARB);
         glDisable(GL_TEXTURE_GEN_S);
         glDisable(GL_TEXTURE_GEN_T);
      }
      glActiveTextureARB(GL_TEXTURE0_ARB);
   } else {
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      if (m_hasSingleSphereMap) {
         glDisable(GL_TEXTURE_GEN_S);
         glDisable(GL_TEXTURE_GEN_T);
      }
      if (m_hasMultipleSphereMap) {
         glActiveTextureARB(GL_TEXTURE2_ARB);
         glDisable(GL_TEXTURE_GEN_S);
         glDisable(GL_TEXTURE_GEN_T);
         glActiveTextureARB(GL_TEXTURE0_ARB);
      }
   }

   if (m_hasSingleSphereMap || m_hasMultipleSphereMap) {
      glDisable(GL_TEXTURE_GEN_S);
      glDisable(GL_TEXTURE_GEN_T);
   }
   if (m_toon) {
      glActiveTextureARB(GL_TEXTURE1_ARB);
      glDisable(GL_TEXTURE_2D);
   }
   if (m_hasMultipleSphereMap) {
      glActiveTextureARB(GL_TEXTURE2_ARB);
      glDisable(GL_TEXTURE_2D);
   }
   glActiveTextureARB(GL_TEXTURE0_ARB);

   glDisable(GL_TEXTURE_2D);
   glEnable(GL_CULL_FACE);
#ifndef MMDFILES_CONVERTCOORDINATESYSTEM
   glCullFace(GL_BACK);
   glPopMatrix();
#endif
}

/* PMDModel::renderEdge: render toon edge */
void PMDModel::renderEdge()
{
   float modelAlpha;

   if (!m_vertexList) return;

   if (!m_toon) return;

   if (m_numSurfaceForEdge == 0) return;

#ifndef MMDFILES_CONVERTCOORDINATESYSTEM
   glPushMatrix();
   glScalef(1.0f, 1.0f, -1.0f);
   glCullFace(GL_BACK);
#else
   /* draw back surface only */
   glCullFace(GL_FRONT);
#endif

   /* calculate alpha value */
   modelAlpha = m_globalAlpha;

   glDisable(GL_LIGHTING);
   glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer(3, GL_FLOAT, sizeof(btVector3), m_edgeVertexList);
   glColor4f(m_edgeColor[0], m_edgeColor[1], m_edgeColor[2], m_edgeColor[3] * modelAlpha);
   glDrawElements(GL_TRIANGLES, m_numSurfaceForEdge, GL_UNSIGNED_SHORT, m_surfaceListForEdge);
   glDisableClientState(GL_VERTEX_ARRAY);
   glEnable(GL_LIGHTING);

   /* draw front again */
#ifndef MMDFILES_CONVERTCOORDINATESYSTEM
   glPopMatrix();
   glCullFace(GL_FRONT);
#else
   glCullFace(GL_BACK);
#endif
}

/* PMDModel::renderForShadow: render for shadow */
void PMDModel::renderForShadow()
{
   if (!m_vertexList) return;

   glDisable(GL_CULL_FACE);
   glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer(3, GL_FLOAT, sizeof(btVector3), m_skinnedVertexList);
   glDrawElements(GL_TRIANGLES, m_numSurface, GL_UNSIGNED_SHORT, m_surfaceList);
   glDisableClientState(GL_VERTEX_ARRAY);
   glEnable(GL_CULL_FACE);
}

/* PMDModel::renderDebug: render for debug view */
void PMDModel::renderDebug()
{
   unsigned short i;

   if (!m_vertexList) return;

   glDisable(GL_DEPTH_TEST);
   glDisable(GL_LIGHTING);
   glDisable(GL_TEXTURE_2D);

   /* draw bones */
   for (i = 0; i < m_numBone; i++)
      m_boneList[i].renderDebug();

   glEnable(GL_DEPTH_TEST);
   glEnable(GL_LIGHTING);
}
