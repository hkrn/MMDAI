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

#include "MMDME/PMDMaterial.h"
#include "MMDME/PMDModelLoader.h"
#include "MMDME/PMDTexture.h"

#include <string.h>

/* PMDMaterial::initialize: initialize material */
void PMDMaterial::initialize()
{
   int i;

   for (i = 0; i < 3; i++) {
      m_diffuse[i] = 0.0f;
      m_ambient[i] = 0.0f;
      m_avgcol[i] = 0.0f;
      m_specular[i] = 0.0f;
   }
   m_alpha = 0.0f;
   m_shiness = 0.0f;
   m_numSurface = 0;
   m_toonID = 0;
   m_edgeFlag = false;
}

/* PMDMaterial::clear: free material */
void PMDMaterial::clear()
{
   /* actual texture data will be released inside textureLoader, so just reset pointer here */
   initialize();
}

/* PMDMaterial:: constructor */
PMDMaterial::PMDMaterial()
{
   initialize();
}

/* ~PMDMaterial:: destructor */
PMDMaterial::~PMDMaterial()
{
   clear();
}

/* PMDMaterial::setup: initialize and setup material */
bool PMDMaterial::setup(PMDFile_Material *m, PMDModelLoader *loader)
{
   char *p;
   char name[21];

   clear();

   /* colors */
   for (int i = 0; i < 3; i++) {
      m_diffuse[i] = m->diffuse[i];
      m_ambient[i] = m->ambient[i];
      /* calculate average color of diffuse and ambient for toon rendering */
      m_avgcol[i] = (m_diffuse[i] + m_ambient[i]) * 0.5f;
      m_specular[i] = m->specular[i];
   }
   m_alpha = m->alpha;
   m_shiness = m->shiness;

   /* number of surface indices whose material should be assigned by this */
   m_numSurface = m->numSurfaceIndex;

   /* toon texture ID */
   if (m->toonID == 0xff)
      m_toonID = 0;
   else
      m_toonID = m->toonID + 1;
   /* edge drawing flag */
   m_edgeFlag = m->edgeFlag ? true : false;

   /* load model texture */
   strncpy(name, m->textureFile, 20);
   name[20] = '\0';
   if (strlen(name) > 0) {
      p = strchr(name, '*');
      if (p) {
         /* has extra sphere map */
         *p = '\0';
         printf("%s %s\n", name,  p + 1);
         fflush(stdout);
         if (!loader->loadModelTexture(name, &m_texture))
           return false;
         if (!loader->loadModelTexture(p + 1, &m_additionalTexture))
           return false;
      } else {
         if (!loader->loadModelTexture(name, &m_texture))
            return false;
      }
   }

   return true;
}

/* PMDMaterial::hasSingleSphereMap: return if it has single sphere maps */
bool PMDMaterial::hasSingleSphereMap() const
{
   return m_texture.isSphereMap() && !m_additionalTexture.isSphereMapAdd();
}

/* PMDMaterial::hasMultipleSphereMap: return if it has multiple sphere map */
bool PMDMaterial::hasMultipleSphereMap() const
{
   return m_additionalTexture.isSphereMapAdd();
}

/* PMDMaterial::copyDiffuse: get diffuse colors */
void PMDMaterial::copyDiffuse(float *c)
{
   int i;

   for (i = 0; i < 3; i++)
      c[i] = m_diffuse[i];
}

/* PMDMaterial::copyAvgcol: get average colors of diffuse and ambient */
void PMDMaterial::copyAvgcol(float *c)
{
   int i;

   for (i = 0; i < 3; i++)
      c[i] = m_avgcol[i];
}

/* PMDMaterial::copyAmbient: get ambient colors */
void PMDMaterial::copyAmbient(float *c)
{
   int i;

   for (i = 0; i < 3; i++)
      c[i] = m_ambient[i];
}

/* PMDMaterial::copySpecular: get specular colors */
void PMDMaterial::copySpecular(float *c)
{
   int i;

   for (i = 0; i < 3; i++)
      c[i] = m_specular[i];
}

/* PMDMaterial::getAlpha: get alpha */
float PMDMaterial::getAlpha() const
{
   return m_alpha;
}

/* PMDMaterial::getShiness: get shiness */
float PMDMaterial::getShiness() const
{
   return m_shiness;
}

/* PMDMaterial::getNumSurface: get number of surface */
unsigned int PMDMaterial::getNumSurface() const
{
   return m_numSurface;
}

/* PMDMaterial::getToonID: get toon index */
unsigned char PMDMaterial::getToonID() const
{
   return m_toonID;
}

/* PMDMaterial::getEdgeFlag: get edge flag */
bool PMDMaterial::getEdgeFlag() const
{
   return m_edgeFlag;
}

/* PMDMaterial::getTexture: get texture */
PMDTexture *PMDMaterial::getTexture()
{
   return &m_texture;
}

/* getAdditionalTexture: get additional sphere map */
PMDTexture *PMDMaterial::getAdditionalTexture()
{
   return &m_additionalTexture;
}
