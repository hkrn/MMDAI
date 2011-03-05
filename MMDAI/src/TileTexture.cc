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

/* headers */

#include "MMDAI/MMDAI.h"

namespace MMDAI {

/* TileTexture::resetDisplayList: reset display list */
void TileTexture::resetDisplayList()
{
  m_engine->deleteCache(&m_cache);
}

/* TileTexture::initialize: initialize texture */
void TileTexture::initialize()
{
  int i, j;

  m_cache = NULL;
  m_isLoaded = false;
  m_listIndex = 0;
  m_listIndexValid = false;

  for (i = 0; i < 4; i++)
    for (j = 0; j < 3; j++)
      m_vertices[i][j] = 0.0f;
  m_numx = 1.0f;
  m_numy = 1.0f;
}

/* TileTexture::clear: free texture */
void TileTexture::clear()
{
  if (m_isLoaded)
    m_texture.release();
  initialize();
}

/* TileTexture::TileTexture: constructor */
TileTexture::TileTexture(GLSceneRenderEngine *engine)
  : m_engine(engine)
{
  initialize();
}

/* TileTexture: destructor */
TileTexture::~TileTexture()
{
  resetDisplayList();
  clear();
}

/* TileTexture::load: load a texture from file name (wide char) */
bool TileTexture::load(PMDModelLoader *loader, GLSceneRenderEngine *engine)
{
  m_texture.setRenderEngine(engine);
  bool ret = loader->loadImageTexture(&m_texture);
  if (ret) {
    m_isLoaded = true;
    resetDisplayList();
  }
  return ret;
}

/* TileTexture::render: render the textures */
void TileTexture::render(bool cullFace, const float normal[3])
{
  static const float color[] = { 0.65f, 0.65f, 0.65f, 1.0f };
  if (m_isLoaded == false)
    return;
  m_engine->renderTileTexture(&m_texture,
                              color,
                              normal,
                              m_vertices[0],
                              m_vertices[1],
                              m_vertices[2],
                              m_vertices[3],
                              m_numx,
                              m_numy,
                              cullFace,
                              &m_cache);
}

/* TileTexture::getSize: get texture size */
float TileTexture::getSize(int i, int j) const
{
  return m_vertices[i][j];
}

/* TileTexture::setSize: set texture size */
void TileTexture::setSize(float v00, float v01, float v02,
                          float v10, float v11, float v12,
                          float v20, float v21, float v22,
                          float v30, float v31, float v32,
                          float numx, float numy)
{
  m_vertices[0][0] = v00;
  m_vertices[0][1] = v01;
  m_vertices[0][2] = v02;
  m_vertices[1][0] = v10;
  m_vertices[1][1] = v11;
  m_vertices[1][2] = v12;
  m_vertices[2][0] = v20;
  m_vertices[2][1] = v21;
  m_vertices[2][2] = v22;
  m_vertices[3][0] = v30;
  m_vertices[3][1] = v31;
  m_vertices[3][2] = v32;

  m_numx = numx;
  m_numy = numy;

  resetDisplayList();
}

} /* namespace */

