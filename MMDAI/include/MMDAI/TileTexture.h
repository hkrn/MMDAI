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

#ifndef MMDAI_TILETEXTURE_H_
#define MMDAI_TILETEXTURE_H_

#include <MMDME/Common.h>
#include <MMDME/PMDModelLoader.h>
#include <MMDME/PMDTexture.h>

#include "MMDAI/SceneRenderEngine.h"

namespace MMDAI {

class TileTexture
{
public:
    TileTexture(SceneRenderEngine *engine);
    ~TileTexture();

    bool load(PMDModelLoader *loader, SceneRenderEngine *engine);
    void render(bool cullFace, const float normal[3]);
    void setSize(float v00, float v01, float v02,
                 float v10, float v11, float v12,
                 float v20, float v21, float v22,
                 float v30, float v31, float v32,
                 float x, float y);
    float getSize(int i, int j) const;

private:
    void resetDisplayList();
    void initialize();
    void clear();

    SceneRenderEngine *m_engine;
    PMDRenderCacheNative *m_cache;
    PMDTexture m_texture;
    bool m_isLoaded;
    float m_vertices[4][3];
    float m_numx;
    float m_numy;

    MMDME_DISABLE_COPY_AND_ASSIGN(TileTexture);
};

} /* namespace */

#endif // MMDAI_TILETEXTURE_H_

