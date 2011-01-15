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

#ifndef TILETEXTURE_H
#define TILETEXTURE_H

#include "MMDFiles.h"

class TileTexture
{
private:

  PMDTexture m_texture;     /* texture */
  bool m_isLoaded;
  GLuint m_listIndex;       /* display list */
  bool m_listIndexValid;    /* true if m_listIndex was registered */

  GLfloat m_vertices[4][3]; /* position */
  GLfloat m_numx;
  GLfloat m_numy;

  /* resetDisplayList: reset display list */
  void resetDisplayList();

  /* initialize: initialize texture */
  void initialize();

  /* clear: free texture */
  void clear();

public:

  /* TileTexture: constructor */
  TileTexture();

  /* TileTexture: destructor */
  ~TileTexture();

  /* load: load a texture from file name (wide char) */
  bool load(const char *fileName);

  /* render: render the textures */
  void render(bool cullFace, const float normal[3]);

  /* setSize: set texture size */
  void setSize(float v00, float v01, float v02,
               float v10, float v11, float v12,
               float v20, float v21, float v22,
               float v30, float v31, float v32,
               float x, float y);

  /* getSize: get texture size */
  GLfloat getSize(int i, int j);
};

#endif // TILETEXTURE_H
