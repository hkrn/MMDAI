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

#include "MMDME/MMDME.h"

namespace MMDAI {

/* PMDTexture::initialize: initialize texture */
void PMDTexture::initialize()
{
   m_engine = NULL;
   m_native = NULL;
   m_isSphereMap = false;
   m_isSphereMapAdd = false;
   m_width = 0;
   m_height = 0;
   m_components = 3;
   m_textureData = NULL;
}

/* PMDTexture::clear: free texture */
void PMDTexture::clear()
{
   m_engine->deleteTexture(m_native);
   if (m_textureData)
      MMDAIMemoryRelease(m_textureData);
   delete m_native;
   initialize();
}

/* constructor */
PMDTexture::PMDTexture()
{
   initialize();
}

/* ~PMDTexture: destructor */
PMDTexture::~PMDTexture()
{
   clear();
}

void PMDTexture::loadBytes(const unsigned char *data, size_t size, int width, int height, int components, bool isSphereMap, bool isSphereMapAdd)
{
   assert(m_engine);
   m_engine->deleteTexture(m_native);
   if (m_textureData)
      MMDAIMemoryRelease(m_textureData);
   delete m_native;
   m_native = new PMDTextureNative;

   m_width = width;
   m_height = height;
   m_components = components;
   m_textureData = static_cast<unsigned char *>(MMDAIMemoryAllocate(size));
   m_isSphereMap = isSphereMap;
   m_isSphereMapAdd = isSphereMapAdd;
   if (m_textureData == NULL)
     return;
   memcpy(m_textureData, data, size);

   if (m_isSphereMap || m_isSphereMapAdd) {
      /* swap vertically */
      for (int h = 0; h < m_height / 2; h++) {
         unsigned char *l1 = m_textureData + h * m_width * m_components;
         unsigned char *l2 = m_textureData + (m_height - 1 - h) * m_width * m_components;
         for (int w = 0 ; w < m_width * m_components; w++) {
            unsigned char tmp = l1[w];
            l1[w] = l2[w];
            l2[w] = tmp;
         }
      }
   }

   m_engine->bindTexture(data, width, height, components, m_native);
}

void PMDTexture::setRenderEngine(GLPMDRenderEngine *engine)
{
  m_engine = engine;
}

/* PMDTexture::getID: get OpenGL texture ID */
PMDTextureNative *PMDTexture::getNative() const
{
   return m_native;
}

/* PMDTexture::isSphereMap: return true if this texture is sphere map */
bool PMDTexture::isSphereMap() const
{
   return m_isSphereMap;
}

/* PMDTexture::isSphereMapAdd: return true if this is sphere map to add */
bool PMDTexture::isSphereMapAdd() const
{
   return m_isSphereMapAdd;
}

/* PMDTexture::release: free texture */
void PMDTexture::release()
{
   clear();
}

} /* namespace */

