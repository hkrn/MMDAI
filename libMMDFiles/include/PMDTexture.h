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

#define PMDTEXTURE_UNINITIALIZEDID 0xFFFFFFFF

/* PMDTexture: texture of PMD */
class PMDTexture
{
private:

   GLuint m_id;                  /* OpenGL texture id */
   bool m_isSphereMap;           /* true if this texture is sphere map (.sph or .spa) */
   bool m_isSphereMapAdd;        /* true if this is sphere map to add (.spa) */
   long m_width;                 /* texture image width */
   long m_height;                /* texture image height */
   unsigned char m_components;   /* number of components (3 for RGB, 4 for RGBA) */
   unsigned char *m_textureData; /* texel data */

private:

   /* loadBMP: load BMP texture */
   bool loadBMP(const char *fileName);

   /* loadTGA: load TGA texture */
   bool loadTGA(const char *fileName);

   /* loadPNG: load PNG texture */
   bool loadPNG(const char *fileName);

   /* loadJPG: load JPG texture */
   bool loadJPG(const char *fileName);

   /* initialize: initialize texture */
   void initialize();

   /* clear: free texture */
   void clear();

public:

   /* PMDTexture: constructor */
   PMDTexture();

   /* ~PMDTexture: destructor */
   ~PMDTexture();

   /* load: load from file name */
   bool load(const char *fileName);

   /* getID: get OpenGL texture ID */
   GLuint getID();

   /* isSphereMap: return true if this texture is sphere map */
   bool isSphereMap();

   /* isSphereMapAdd: return true if this is sphere map to add */
   bool isSphereMapAdd();

   /* release: free texture */
   void release();
};
