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

/* PMDTextureLoader:lookup: lookup texture in cache */
PMDTexture *PMDTextureLoader::lookup(const char *fileName, bool *alreadyFailRet)
{
   TextureLink *tmp = m_root;

   while (tmp) {
      if (strcmp(tmp->name, fileName) == 0) {
         /* if exist but texture is NULL, it has been failed */
         *alreadyFailRet = (tmp->texture == NULL) ? true : false;
         return tmp->texture;
      }
      tmp = tmp->next;
   }
   *alreadyFailRet = false;

   return NULL;
}

/* PMDTextureLoader::store: add a texture to cache */
void PMDTextureLoader::store(PMDTexture *tex, const char *fileName)
{
   TextureLink *newLink = new TextureLink;

   newLink->name = strdup(fileName);
   newLink->texture = tex;
   newLink->next = m_root;
   m_root = newLink;
}

/* PMDTextureLoader::initialize: initialize texture loader  */
void PMDTextureLoader::initialize()
{
   m_root = NULL;
   m_hasError = false;
}

/* PMDTextureLoader::clear: free texture loader  */
void PMDTextureLoader::clear()
{
   TextureLink *tmp = m_root;
   TextureLink *next;

   while (tmp) {
      next = tmp->next;
      free(tmp->name);
      if(tmp->texture != NULL)
         delete tmp->texture;
      delete tmp;
      tmp = next;
   }
   initialize();
}

/* PMDTextureLoader::PMDTextureLoader: constructor */
PMDTextureLoader::PMDTextureLoader()
{
   initialize();
}

/* PMDTextureLoader::~PMDTextureLoader: destructor */
PMDTextureLoader::~PMDTextureLoader()
{
   clear();
}

/* PMDTextureLoader::load: load texture from file name (multi-byte char) */
PMDTexture *PMDTextureLoader::load(const char *fileName)
{
   PMDTexture *tex;
   bool already_fail;

   /* consult cache */
   tex = lookup(fileName, &already_fail);
   /* when exist but has failed, return error without trying to load */
   if (already_fail) return NULL;
   if (tex == NULL) {
      /* not exist, try to load */
      tex = new PMDTexture;
      if (tex->load(fileName) == false) {
         /* failed, store with failed status */
         store(NULL, fileName);
         m_hasError = true;
         return NULL;
      }
      /* succeeded, store it */
      store(tex, fileName);
   }
   return tex;
}

/* PMDTextureLoader::getErrorTextureString: get newline-separated list of error textures */
void PMDTextureLoader::getErrorTextureString(char *buf, int maxlen)
{
   TextureLink *tmp = m_root;

   strcpy(buf, "");
   if (!m_hasError) return;
   for (tmp = m_root; tmp; tmp = tmp->next) {
      if (tmp->texture == NULL) {
         strncat(buf, tmp->name, maxlen);
         strncat(buf, "\n", maxlen);
      }
   }
}

/* PMDTextureLoader::release: free texture loader */
void PMDTextureLoader::release()
{
   clear();
}
