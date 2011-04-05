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

   bool setup(const PMDFile_Material *m, PMDModelLoader *loader);

   inline void copyDiffuse(float c[4]) {
       for (int i = 0; i < 3; i++)
           c[i] = m_diffuse[i];
   }
   inline void copyAvgcol(float c[4]) {
       for (int i = 0; i < 3; i++)
           c[i] = m_avgcol[i];
   }
   inline void copyAmbient(float c[4]) {
       for (int i = 0; i < 3; i++)
           c[i] = m_ambient[i];
   }
   inline void copySpecular(float c[4]) {
       for (int i = 0; i < 3; i++)
           c[i] = m_specular[i];
   }
   inline const bool hasSingleSphereMap() const {
       return m_texture.isSPH() && !m_additionalTexture.isSPA();
   }
   inline const bool hasMultipleSphereMap() const {
       return m_additionalTexture.isSPA();
   }
   inline const float getAlpha() const {
       return m_alpha;
   }
   inline const float getShiness() const {
       return m_shiness;
   }
   inline const unsigned int countSurfaces() const {
       return m_numSurface;
   }
   inline const unsigned char getToonID() const {
       return m_toonID;
   }
   inline const bool hasEdge() const {
       return m_edgeFlag;
   }
   inline PMDTexture *getTexture() {
       return &m_texture;
   }
   inline PMDTexture *getAdditionalTexture() {
       return &m_additionalTexture;
   }

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

