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

#ifndef MMDME_PMDMATERIAL_H_
#define MMDME_PMDMATERIAL_H_

#include "MMDME/Common.h"
#include "MMDME/PMDFile.h"
#include "MMDME/PMDTexture.h"

namespace MMDAI {

class PMDModelLoader;
class PMDRenderEngine;

/* PMDMaterial: material of PMD */
class PMDMaterial
{
private:

   float m_diffuse[3];  /* diffuse color */
   float m_ambient[3];  /* ambient color */
   float m_avgcol[3];   /* average of diffuse and ambient */
   float m_specular[3]; /* specular color */
   float m_alpha;       /* alpha color */
   float m_shiness;     /* shiness intensity */

   unsigned int m_numSurface; /* number of surface indices for this material */

   unsigned char m_toonID; /* toon index */
   bool m_edgeFlag;        /* true if edge should be drawn */

   PMDTexture m_texture;            /* pointer to texture */
   PMDTexture m_additionalTexture;  /* pointer to additional sphere map */

   PMDRenderEngine *m_engine;

   /* initialize: initialize material */
   void initialize();

   /* clear: free material */
   void clear();

   MMDME_DISABLE_COPY_AND_ASSIGN(PMDMaterial);

public:

   /* PMDMaterial: constructor */
   PMDMaterial(PMDRenderEngine *engine);

   /* ~PMDMaterial: destructor */
   virtual ~PMDMaterial();

   /* setup: initialize and setup material */
   bool setup(PMDFile_Material *m, PMDModelLoader *loader);

   /* hasSingleSphereMap: return if it has single sphere maps */
   bool hasSingleSphereMap() const;

   /* hasMultipleSphereMap: return if it has multiple sphere map */
   bool hasMultipleSphereMap() const;

   /* copyDiffuse: get diffuse colors */
   void copyDiffuse(float *c);

   /* copyAvgcol: get average colors of diffuse and ambient */
   void copyAvgcol(float *c);

   /* copyAmbient: get ambient colors */
   void copyAmbient(float *c);

   /* copySpecular: get specular colors */
   void copySpecular(float *c);

   /* getAlpha: get alpha color */
   float getAlpha() const;

   /* getShiness: get shiness intensity */
   float getShiness() const;

   /* getNumSurface: get number of surface */
   unsigned int getNumSurface() const;

   /* getToonID: get toon index */
   unsigned char getToonID() const;

   /* getEdgeFlag: get edge flag */
   bool getEdgeFlag() const;

   /* getTexture: get texture */
   PMDTexture *getTexture();

   /* getAdditionalTexture: get additional sphere map */
   PMDTexture *getAdditionalTexture();
};

} /* namespace */

#endif

