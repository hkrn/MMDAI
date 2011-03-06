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

#define RENDER_MINSCALEDIFF   0.001f
#define RENDER_SCALESPEEDRATE 0.9f
#define RENDER_MINMOVEDIFF    0.000001f
#define RENDER_MOVESPEEDRATE  0.9f
#define RENDER_MINSPINDIFF    0.000001f
#define RENDER_SPINSPEEDRATE  0.9f

/* SceneRendererer::initialize: initialzie Renderer */
void SceneRenderer::initialize()
{
  m_width = 0;
  m_height = 0;
  m_scale = 1.0f;
  m_trans = btVector3(0.0f, 0.0f, 0.0f);
  m_rot = btQuaternion(0.0f, 0.0f, 0.0f, 1.0f);
  m_cameraTrans = btVector3(0.0f, RENDER_VIEWPOINT_Y_OFFSET, RENDER_VIEWPOINT_CAMERA_Z);

  m_currentScale = m_scale;
  m_currentTrans = m_trans;
  m_currentRot = m_rot;

  m_transMatrix.setIdentity();
  updateModelViewMatrix();
}

bool SceneRenderer::setup(int *size,
                          float *campusColor,
                          bool useShadowMapping,
                          int shadowMapTextureSize,
                          bool shadowMapLightFirst)
{
  bool ret = false;
  ret = m_engine->setup(campusColor, useShadowMapping, shadowMapTextureSize, shadowMapLightFirst);
  setSize(size[0], size[1]);
  return ret;
}

/* SceneRenderer::clear: free Renderer */
void SceneRenderer::clear()
{
  initialize();
}

/* SceneRenderer::SceneRenderer: constructor */
SceneRenderer::SceneRenderer(GLSceneRenderEngine *engine)
  : m_engine(engine)
{
  initialize();
}

/* SceneRenderer::~SceneRenderer: destructor */
SceneRenderer::~SceneRenderer()
{
  clear();
}

/* SceneRenderer::setSize: set size */
void SceneRenderer::setSize(int w, int h)
{
  if (m_width != w || m_height != h) {
    if (w > 0)
      m_width = w;
    if (h > 0)
      m_height = h;
    updateProjectionMatrix();
  }
}

/* SceneRenderer::getWidth: get width */
int SceneRenderer::getWidth() const
{
  return m_width;
}

/* SceneRenderer::getHeight: get height */
int SceneRenderer::getHeight() const
{
  return m_height;
}

/* SceneRenderer::setScale: set scale */
void SceneRenderer::setScale(float scale)
{
  m_scale = scale;
}

/* SceneRenderer::getScale: get scale */
float SceneRenderer::getScale() const
{
  return m_scale;
}

/* SceneRenderer::translate: translate */
void SceneRenderer::translate(float x, float y, float z)
{
  m_trans += btVector3(x, y, z);
}

/* Render::resetLocation: reset rotation, transition, and scale */
void SceneRenderer::resetLocation(const float *trans, const float *rot, const float scale)
{
  btMatrix3x3 bm;
  bm.setEulerZYX(MMDME_RAD(rot[0]), MMDME_RAD(rot[1]), MMDME_RAD(rot[2]));
  bm.getRotation(m_rot);
  m_trans = btVector3(trans[0], trans[1], trans[2]);
  m_scale = scale;
}

/* SceneRenderer::update: update scale */
void SceneRenderer::updateScale()
{
  float diff;

  /* if no difference, return */
  if (m_currentScale == m_scale)
    return;

  diff = fabs(m_currentScale - m_scale);
  if (diff < RENDER_MINSCALEDIFF) {
    m_currentScale = m_scale;
  } else {
    m_currentScale = m_currentScale * (RENDER_SCALESPEEDRATE) + m_scale * (1.0f - RENDER_SCALESPEEDRATE);
  }
  updateProjectionMatrix();
}

/* SceneRenderer::updateTransRotMatrix:  update trans and rotation matrix */
void SceneRenderer::updateTransRotMatrix()
{
  float diff1, diff2;
  btVector3 trans;
  btQuaternion rot;

  /* if no difference, return */
  if (m_currentRot == m_rot && m_currentTrans == m_trans)
    return;

  /* calculate difference */
  trans = m_trans;
  trans -= m_currentTrans;
  diff1 = trans.length2();
  rot = m_rot;
  rot -= m_currentRot;
  diff2 = rot.length2();

  if (diff1 > RENDER_MINMOVEDIFF)
    m_currentTrans = m_currentTrans.lerp(m_trans, 1.0f - RENDER_MOVESPEEDRATE); /* current * 0.9 + target * 0.1 */
  else
    m_currentTrans = m_trans;
  if (diff2 > RENDER_MINSPINDIFF)
    m_currentRot = m_currentRot.slerp(m_rot, 1.0f - RENDER_SPINSPEEDRATE); /* current * 0.9 + target * 0.1 */
  else
    m_currentRot = m_rot;

  updateModelViewMatrix();
}

/* SceneRenderer::rotate: rotate scene */
void SceneRenderer::rotate(float x, float y, float z)
{
  z = 0; /* unused */
  m_rot = m_rot * btQuaternion(x, 0, 0);
  m_rot = btQuaternion(0, y, 0) * m_rot;
}

/* SceneRenderer::getScreenPointPosition: convert screen position to object position */
void SceneRenderer::getScreenPointPosition(btVector3 *dst, btVector3 *src)
{
  *dst = m_transMatrixInv * (*src);
}

/* SceneRenderer::updateModelViewMatrix: update model view matrix */
void SceneRenderer::updateModelViewMatrix()
{
  m_transMatrix.setRotation(m_currentRot);
  m_transMatrix.setOrigin(m_currentTrans + m_cameraTrans);
  m_transMatrixInv = m_transMatrix.inverse();
  m_engine->updateModelViewMatrix(m_transMatrix, m_transMatrixInv);
}

/* SceneRenderer::updateDepthTextureViewParam: update center and radius information to get required range for shadow mapping */
void SceneRenderer::updateDepthTextureViewParam(PMDObject **objects, int num)
{
  int i;
  float d, dmax;
  float *r = new float[num];
  btVector3 *c = new btVector3[num];
  btVector3 cc = btVector3(0.0f, 0.0f, 0.0f);

  for (i = 0; i < num; i++) {
    PMDObject *object = objects[i];
    if (!object->isEnable())
      continue;
    r[i] = object->getPMDModel()->calculateBoundingSphereRange(&(c[i]));
    cc += c[i];
  }
  if (num != 0)
    cc /= (float) num;

  dmax = 0.0f;
  for (i = 0; i < num; i++) {
    if (!objects[i]->isEnable())
      continue;
    d = cc.distance(c[i]) + r[i];
    if (dmax < d)
      dmax = d;
  }
  m_engine->setShadowMapAutoView(cc, dmax);

  delete [] r;
  delete [] c;
}

int SceneRenderer::pickModel(PMDObject **objects,
                             int size,
                             int x,
                             int y,
                             int *allowDropPicked)
{
  return m_engine->pickModel(objects, size, x, y, m_width, m_height, m_currentScale, allowDropPicked);
}

void SceneRenderer::updateLighting(bool useCartoonRendering,
                                   bool useMMDLikeCartoon,
                                   float *lightDirection,
                                   float lightIntensy,
                                   float *lightColor)
{
  m_engine->updateLighting(useCartoonRendering, useMMDLikeCartoon, lightDirection, lightIntensy, lightColor);
}

void SceneRenderer::updateProjectionMatrix()
{
  m_engine->updateProjectionMatrix(m_width, m_height, m_currentScale);
}

} /* namespace */
