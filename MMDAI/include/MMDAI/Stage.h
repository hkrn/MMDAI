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

#ifndef MMDAI_STAGE_H_
#define MMDAI_STAGE_H_

#include <MMDME/Common.h>
#include <MMDME/PMDModel.h>

#include "MMDAI/SceneRenderEngine.h"
#include "MMDAI/TileTexture.h"

namespace MMDAI {

class IModelLoader;


class Stage
{
public:
    Stage(SceneRenderEngine *engine);
    ~Stage();

    void setSize(float *size, float numx, float numy);
    bool loadFloor(IModelLoader *loader, BulletPhysics *bullet);
    bool loadBackground(IModelLoader *loader, BulletPhysics *bullet);
    bool loadStagePMD(IModelLoader *loader, BulletPhysics *bullet);
    void renderFloor();
    void renderBackground();
    void renderPMD();
    void updateShadowMatrix(float lightDirection[4]);
    float *getShadowMatrix() const;

private:
    void releaseFloorBody();
    void initialize();
    void clear();
    void makeFloorBody(float width, float depth);

    SceneRenderEngine *m_engine;
    PMDRenderCacheNative *m_cache;
    TileTexture *m_floor;
    TileTexture *m_background;
    PMDModel *m_model;
    bool m_hasPMD;
    bool m_listIndexPMDValid;
    BulletPhysics *m_bullet;
    btRigidBody *m_floorBody;
    float m_floorShadow[4][4];

    MMDME_DISABLE_COPY_AND_ASSIGN(Stage);
};

} /* namespace */

#endif // MMDAI_STAGE_H_
