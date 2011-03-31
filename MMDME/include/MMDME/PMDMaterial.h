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

#ifndef MMDME_PMDMATERIAL_H_
#define MMDME_PMDMATERIAL_H_

#include "MMDME/Common.h"
#include "MMDME/PMDFile.h"
#include "MMDME/PMDTexture.h"

namespace MMDAI {

class PMDModelLoader;
class PMDRenderEngine;

class PMDMaterial
{
public:
   PMDMaterial(PMDRenderEngine *engine);
   virtual ~PMDMaterial();

   bool setup(PMDFile_Material *m, PMDModelLoader *loader);
   bool hasSingleSphereMap() const;
   bool hasMultipleSphereMap() const;
   void copyDiffuse(float *c);
   void copyAvgcol(float *c);
   void copyAmbient(float *c);
   void copySpecular(float *c);
   float getAlpha() const;
   float getShiness() const;
   unsigned int getNumSurface() const;
   unsigned char getToonID() const;
   bool getEdgeFlag() const;
   PMDTexture *getTexture();
   PMDTexture *getAdditionalTexture();

private:
   void initialize();
   void clear();

   float m_diffuse[3];
   float m_ambient[3];
   float m_avgcol[3];
   float m_specular[3];
   float m_alpha;
   float m_shiness;
   unsigned int m_numSurface;
   unsigned char m_toonID;
   bool m_edgeFlag;
   PMDTexture m_texture;
   PMDTexture m_additionalTexture;
   PMDRenderEngine *m_engine;

   MMDME_DISABLE_COPY_AND_ASSIGN(PMDMaterial);
};

} /* namespace */

#endif

