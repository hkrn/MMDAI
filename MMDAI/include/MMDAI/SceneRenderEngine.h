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

#ifndef MMDAI_SCENERENDERENGINE_H_
#define MMDAI_SCENERENDERENGINE_H_

#include <LinearMath/btVector3.h>
#include <LinearMath/btTransform.h>
#include <MMDME/PMDRenderEngine.h>

namespace MMDAI {

class BulletPhysics;
class PMDBone;
class PMDModel;
class IModelLoader;
class PMDObject;
class PMDTexture;
class Stage;

typedef struct PMDTextureNative PMDTextureNative;
typedef struct PMDRenderCacheNative PMDRenderCacheNative;

class SceneRenderEngine : public PMDRenderEngine {
public:
    virtual ~SceneRenderEngine() {}

    virtual PMDModel *allocateModel() = 0;
    virtual bool loadModel(PMDModel *model, IModelLoader *loader, BulletPhysics *bullet) = 0;
    virtual void releaseModel(PMDModel *model) = 0;

    virtual void renderModelCached(PMDModel *model,
                                   PMDRenderCacheNative **ptr) = 0;
    virtual void renderTileTexture(PMDTexture *texture,
                                   const float *color,
                                   const float *normal,
                                   const float *vertices1,
                                   const float *vertices2,
                                   const float *vertices3,
                                   const float *vertices4,
                                   const float nX,
                                   const float nY,
                                   const bool cullFace,
                                   PMDRenderCacheNative **ptr) = 0;
    virtual void deleteCache(PMDRenderCacheNative **ptr) = 0;

    virtual bool setup() = 0;
    virtual void initializeShadowMap() = 0;
    virtual void setShadowMapping() = 0;
    virtual void prerender(PMDObject **objects,
                           int size) = 0;
    virtual void render(PMDObject **objects,
                        int size,
                        Stage *stage) = 0;
    virtual int pickModel(PMDObject **objects,
                          int size,
                          int x,
                          int y,
                          int *allowDropPicked) = 0;
    virtual void updateLighting() = 0;
    virtual void setViewport(const int width, const int height) = 0;
    virtual void setShadowMapAutoView(const btVector3 &eyePoint,
                                      const float radius) = 0;
    virtual void setModelView(const btTransform &modelView) = 0;
    virtual void setProjection(const float projection[16]) = 0;
};

} /* namespace */

#endif
