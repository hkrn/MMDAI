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

#include "PMDTexture.h"

#include <stdlib.h>
#include <string.h>

/* PMDTexture::initialize: initialize texture */
void PMDTexture::initialize()
{
   m_id = PMDTEXTURE_UNINITIALIZEDID;
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
   if (m_id != PMDTEXTURE_UNINITIALIZEDID)
      glDeleteTextures(1, &m_id);
   if (m_textureData)
      free(m_textureData);
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
   clear();

   m_width = width;
   m_height = height;
   m_components = components;
   m_textureData = static_cast<unsigned char *>(malloc(size));
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

   /* generate texture */
   GLuint format = 0;
   glGenTextures(1, &m_id);
   glBindTexture(GL_TEXTURE_2D, m_id);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   if (m_components == 3) {
      format = GL_RGB;
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   } else {
      format = GL_RGBA;
      glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
   }
   glTexImage2D(GL_TEXTURE_2D, 0, format, m_width, m_height, 0, format, GL_UNSIGNED_BYTE, m_textureData);

   /* set highest priority to this texture to tell OpenGL to keep textures in GPU memory */
   GLfloat priority = 1.0f;
   glPrioritizeTextures(1, &m_id, &priority);
}

/* PMDTexture::getID: get OpenGL texture ID */
GLuint PMDTexture::getID()
{
   return m_id;
}

/* PMDTexture::isSphereMap: return true if this texture is sphere map */
bool PMDTexture::isSphereMap()
{
   return m_isSphereMap;
}

/* PMDTexture::isSphereMapAdd: return true if this is sphere map to add */
bool PMDTexture::isSphereMapAdd()
{
   return m_isSphereMapAdd;
}

/* PMDTexture::release: free texture */
void PMDTexture::release()
{
   clear();
}
