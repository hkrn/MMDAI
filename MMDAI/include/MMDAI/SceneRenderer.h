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

#ifndef SCENERENDERER_H
#define SCENERENDERER_H

#include <MMDME/MMDME.h>

#include "MMDAI/GLSceneRenderEngine.h"

namespace MMDAI {

#define RENDER_VIEWPOINT_CAMERA_Z     -100.0f
#define RENDER_VIEWPOINT_Y_OFFSET     -13.0f
#define RENDER_VIEWPOINT_FRUSTUM_NEAR 5.0f
#define RENDER_VIEWPOINT_FRUSTUM_FAR  2000.0f

class PMDObject;
class Option;
class Stage;

/* Render: render */
class SceneRenderer
{
private:

  GLSceneRenderEngine *m_engine;

  int m_width;             /* window width */
  int m_height;            /* winodw height */
  float m_scale;           /* target scale */
  btVector3 m_trans;       /* target trans vector */
  btQuaternion m_rot;      /* target rotation */
  btVector3 m_cameraTrans; /* position of camera */

  float m_currentScale;         /* current scale */
  btVector3 m_currentTrans;     /* current trans vector */
  btQuaternion m_currentRot;    /* current rotation */
  btTransform m_transMatrix;    /* current trans vector + rotation matrix */
  btTransform m_transMatrixInv; /* current trans vector + inverse of rotation matrix */

  float m_backgroundColor[3]; /* background color */

  /* updateProjectionMatrix: update view information */
  void updateProjectionMatrix();

  /* applyProjectionMatirx: update projection matrix */
  void applyProjectionMatrix();

  /* updateModelViewMatrix: update model view matrix */
  void updateModelViewMatrix();

  /* Render::initialize: initialzie Render */
  void initialize();

  /* Render::clear: free Render */
  void clear();

  MMDME_DISABLE_COPY_AND_ASSIGN(SceneRenderer);

public:

  /* SceneRender: constructor */
  SceneRenderer(GLSceneRenderEngine *engine);

  /* ~SceneRender: destructor */
  ~SceneRenderer();

  /* setup: initialize and setup Render */
  bool setup(int *size,
             float *campusColor,
             bool useShadowMapping,
             int shadowMapTextureSize,
             bool shadowMapLightFirst);

  /* setSize: set size */
  void setSize(int w, int h);

  /* getWidth: get width */
  int getWidth() const;

  /* getHeight: get height */
  int getHeight() const;

  /* setScale: set scale */
  void setScale(float scale);

  /* getScale: get scale */
  float getScale() const;

  /* translate: translate */
  void translate(float x, float y, float z);

  /* rotate: rotate scene */
  void rotate(float x, float y, float z);

  /* Render::resetLocation: reset rotation, transition, and scale */
  void resetLocation(const float *trans, const float *rot, const float scale);

  /* pickModel: pick up a model at the screen position */
  int pickModel(PMDObject **objects, int size, int x, int y, int *allowDropPicked);

  /* updateLigithing: update light */
  void updateLighting(bool useCartoonRendering, bool useMMDLikeCartoon, float *lightDirection, float lightIntensy, float *lightColor);

  /* updateDepthTextureViewParam: update center and radius information to get required range for shadow mapping */
  void updateDepthTextureViewParam(PMDObject **objects, int num);

  /* getScreenPointPosition: convert screen position to object position */
  void getScreenPointPosition(btVector3 *dst, btVector3 *src);

  /* update: update scale */
  void updateScale();

  /* updateTransRotMatrix:  update trans and rotation matrix */
  void updateTransRotMatrix();
};

} /* namespace */

#endif // SCENERENDERER_H

