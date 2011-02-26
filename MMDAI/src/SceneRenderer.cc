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

#if defined(__APPLE__)
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

namespace MMDAI {

#define SHADOW_PCF                   /* use hardware PCF for shadow mapping */
#define SHADOW_AUTO_VIEW             /* automatically define depth frustum */
#define SHADOW_AUTO_VIEW_ANGLE 15.0f /* view angle for automatic depth frustum */

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

  m_scaleCurrent = m_scale;
  m_transCurrent = m_trans;
  m_rotCurrent = m_rot;

  m_transMatrix.setIdentity();
  updateModelViewMatrix();

  m_enableShadowMapping = false;
  m_shadowMapInitialized = false;
  m_lightVec = btVector3(0.0f, 0.0f, 0.0f);
  m_shadowMapAutoViewEyePoint = btVector3(0.0f, 0.0f, 0.0f);
  m_shadowMapAutoViewRadius = 0.0f;
}

/* SceneRenderer::clear: free Renderer */
void SceneRenderer::clear()
{
  initialize();
}

/* SceneRenderer::SceneRenderer: constructor */
SceneRenderer::SceneRenderer()
{
  initialize();
}

/* SceneRenderer::~SceneRenderer: destructor */
SceneRenderer::~SceneRenderer()
{
  clear();
}

/* setup: initialize and setup Renderer */
bool SceneRenderer::setup(int *size, float *campusColor, bool useShadowMapping, int shadowMapTextureSize, bool shadowMapLightFirst)
{
  /* set clear color */
  glClearColor(campusColor[0], campusColor[1], campusColor[2], 0.0f);
  glClearStencil(0);

  /* enable depth test */
  glEnable(GL_DEPTH_TEST);

  /* enable texture */
  glEnable(GL_TEXTURE_2D);

  /* enable face culling */
  glEnable(GL_CULL_FACE);
  /* not Renderer the back surface */
  glCullFace(GL_BACK);

  /* enable alpha blending */
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  /* enable alpha test, to avoid zero-alpha surfaces to depend on the Renderering order */
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GEQUAL, 0.05f);

  /* enable lighting */
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHTING);

  /* initialization for shadow mapping */
  if (useShadowMapping)
    setShadowMapping(true, shadowMapTextureSize, shadowMapLightFirst);

  setSize(size[0], size[1]);

  return true;
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

/* SceneRenderer::initializeShadowMap: initialize OpenGL for shadow mapping */
void SceneRenderer::initializeShadowMap(int shadowMapTextureSize)
{
  static const GLdouble genfunc[][4] = {
    { 1.0, 0.0, 0.0, 0.0 },
    { 0.0, 1.0, 0.0, 0.0 },
    { 0.0, 0.0, 1.0, 0.0 },
    { 0.0, 0.0, 0.0, 1.0 },
  };

  /* initialize model view matrix */
  glPushMatrix();
  glLoadIdentity();

  /* use 4th texture unit for depth texture, make it current */
  glActiveTextureARB(GL_TEXTURE3_ARB);

  /* prepare a texture object for depth texture Renderering in frame buffer object */
  glGenTextures(1, &m_depthTextureID);
  glBindTexture(GL_TEXTURE_2D, m_depthTextureID);

  /* assign depth component to the texture */
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapTextureSize, shadowMapTextureSize, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);

  /* set texture parameters for shadow mapping */
#ifdef SHADOW_PCF
  /* use hardware PCF */
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#else
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
#endif

  /* tell OpenGL to compare the R texture coordinates to the (depth) texture value */
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

  /* also tell OpenGL to get the compasiron result as alpha value */
  glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_ALPHA);

  /* set texture coordinates generation mode to use the raw texture coordinates (S, T, R, Q) in eye view */
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
  glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
  glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
  glTexGendv(GL_S, GL_EYE_PLANE, genfunc[0]);
  glTexGendv(GL_T, GL_EYE_PLANE, genfunc[1]);
  glTexGendv(GL_R, GL_EYE_PLANE, genfunc[2]);
  glTexGendv(GL_Q, GL_EYE_PLANE, genfunc[3]);

  /* finished configuration of depth texture: unbind the texture */
  glBindTexture(GL_TEXTURE_2D, 0);

  /* allocate a frame buffer object (FBO) for depth buffer Renderering */
  glGenFramebuffersEXT(1, &m_fboID);
  /* switch to the newly allocated FBO */
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fboID);
  /* bind the texture to the FBO, telling that it should Renderer the depth information to the texture */
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, m_depthTextureID, 0);
  /* also tell OpenGL not to draw and read the color buffers */
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  /* check FBO status */
  if (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT) {
    /* cannot use FBO */
  }
  /* finished configuration of FBO, now switch to default frame buffer */
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

  /* reset the current texture unit to default */
  glActiveTextureARB(GL_TEXTURE0_ARB);

  /* restore the model view matrix */
  glPopMatrix();
}

/* SceneRenderer::setShadowMapping: switch shadow mapping */
void SceneRenderer::setShadowMapping(bool flag, int shadowMapTextureSize, bool shadowMapLightFirst)
{
  m_enableShadowMapping = flag;

  if (m_enableShadowMapping) {
    /* enabled */
    if (! m_shadowMapInitialized) {
      /* initialize now */
      initializeShadowMap(shadowMapTextureSize);
      m_shadowMapInitialized = true;
    }
    /* set how to set the comparison result value of R coordinates and texture (depth) value */
    glActiveTextureARB(GL_TEXTURE3_ARB);
    glBindTexture(GL_TEXTURE_2D, m_depthTextureID);
    if (shadowMapLightFirst) {
      /* when Renderering order is light(full) - dark(shadow part), OpenGL should set the shadow part as true */
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_GEQUAL);
    } else {
      /* when Renderering order is dark(full) - light(non-shadow part), OpenGL should set the shadow part as false */
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    }
    glDisable(GL_TEXTURE_2D);
    glActiveTextureARB(GL_TEXTURE0_ARB);
    MMDAILogInfoString("Shadow mapping enabled");
  } else {
    /* disabled */
    if (m_shadowMapInitialized) {
      /* disable depth texture unit */
      glActiveTextureARB(GL_TEXTURE3_ARB);
      glDisable(GL_TEXTURE_2D);
      glActiveTextureARB(GL_TEXTURE0_ARB);
    }
    MMDAILogInfoString("Shadow mapping disabled");
  }
}

/* SceneRenderer::RendererSceneShadowMap: shadow mapping */
void SceneRenderer::renderSceneShadowMap(Option *option, Stage *stage, PMDObject *objects, int size)
{
  short i;
  GLint viewport[4]; /* store viewport */
  GLdouble modelview[16]; /* store model view transform */
  GLdouble projection[16]; /* store projection transform */

#ifdef SHADOW_AUTO_VIEW
  float fovy;
  float eyeDist;
  btVector3 v;
#endif

  static GLfloat lightdim[] = { 0.2f, 0.2f, 0.2f, 1.0f };
  static const GLfloat lightblk[] = { 0.0f, 0.0f, 0.0f, 1.0f };

  /* Renderer the depth texture */
  /* store the current viewport */
  glGetIntegerv(GL_VIEWPORT, viewport);

  /* store the current projection matrix */
  glGetDoublev(GL_PROJECTION_MATRIX, projection);

  /* switch to FBO for depth buffer Renderering */
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fboID);

  /* clear the buffer */
  /* clear only the depth buffer, since other buffers will not be used */
  glClear(GL_DEPTH_BUFFER_BIT);

  /* set the viewport to the required texture size */
  glViewport(0, 0, option->getShadowMappingTextureSize(), option->getShadowMappingTextureSize());

  /* reset the projection matrix */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  /* set the model view matrix to make the light position as eye point and capture the whole scene in the view */
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

#ifdef SHADOW_AUTO_VIEW
  /* auto-update the view area according to the model */
  /* the model range and position has been computed at model updates before */
  /* set the view angle */
  fovy = SHADOW_AUTO_VIEW_ANGLE;
  /* set the distance to cover all the model range */
  eyeDist = m_shadowMapAutoViewRadius / sinf(fovy * 0.5f * 3.1415926f / 180.0f);
  /* set the perspective */
  gluPerspective(fovy, 1.0, 1.0, eyeDist + m_shadowMapAutoViewRadius + 50.0f); /* +50.0f is needed to cover the background */
  /* the viewpoint should be at eyeDist far toward light direction from the model center */
  v = m_lightVec * eyeDist + m_shadowMapAutoViewEyePoint;
  gluLookAt(v.x(), v.y(), v.z(), m_shadowMapAutoViewEyePoint.x(), m_shadowMapAutoViewEyePoint.y(), m_shadowMapAutoViewEyePoint.z(), 0.0, 1.0, 0.0);
#else
  /* fixed view */
  gluPerspective(25.0, 1.0, 1.0, 120.0);
  gluLookAt(30.0, 77.0, 30.0, 0.0, 17.0, 0.0, 0.0, 1.0, 0.0);
#endif

  /* keep the current model view for later process */
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

  /* do not write into frame buffer other than depth information */
  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

  /* also, lighting is not needed */
  glDisable(GL_LIGHTING);

  /* disable Renderering the front surface to get the depth of back face */
  glCullFace(GL_FRONT);

  /* disable alpha test */
  glDisable(GL_ALPHA_TEST);

  /* we are now writing to depth texture using FBO, so disable the depth texture mapping here */
  glActiveTextureARB(GL_TEXTURE3_ARB);
  glDisable(GL_TEXTURE_2D);
  glActiveTextureARB(GL_TEXTURE0_ARB);

  /* set polygon offset to avoid "moire" */
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(4.0, 4.0);

  /* Renderer objects for depth */
  /* only objects that wants to drop shadow should be Renderered here */
  for (i = 0; i < size; i++) {
    PMDObject *object = &objects[i];
    if (!object->isEnable())
      continue;
    glPushMatrix();
    object->getPMDModel()->renderForShadow();
    glPopMatrix();
  }

  /* reset the polygon offset */
  glDisable(GL_POLYGON_OFFSET_FILL);

  /* switch to default FBO */
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

  /* revert configurations to normal Renderering */
  glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixd(projection);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glEnable(GL_LIGHTING);
  glCullFace(GL_BACK);
  glEnable(GL_ALPHA_TEST);

  /* clear all the buffers */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  /* Renderer the full scene */
  /* set model view matrix, as the same as normal Renderering */
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glMultMatrixf(m_rotMatrix);

  /* Renderer the whole scene */
  if (option->getShadowMappingLightFirst()) {
    /* Renderer light setting, later Renderer only the shadow part with dark setting */
    stage->renderBackground();
    stage->renderFloor();
    for (i = 0; i < size; i++) {
      PMDObject *object = &objects[i];
      if (!object->isEnable())
        continue;
      PMDModel *model = object->getPMDModel();
      model->renderModel();
      model->renderEdge();
    }
  } else {
    /* Renderer in dark setting, later Renderer only the non-shadow part with light setting */
    /* light setting for non-toon objects */
    lightdim[0] = lightdim[1] = lightdim[2] = 0.55f - 0.2f * option->getShadowMappingSelfDensity();
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightdim);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightdim);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightblk);

    /* Renderer the non-toon objects (back, floor, non-toon models) */
    stage->renderBackground();
    stage->renderFloor();
    for (i = 0; i < size; i++) {
      PMDObject *object = &objects[i];
      if (!object->isEnable())
        continue;
      PMDModel *model = object->getPMDModel();
      if (model->getToonFlag() == true)
        continue;
      model->renderModel();
    }

    /* for toon objects, they should apply the model-defined toon texture color at texture coordinates (0, 0) for shadow Renderering */
    /* so restore the light setting */
    if (option->getUseCartoonRendering())
      updateLighting(true, option->getUseMMDLikeCartoon(), option->getLightDirection(), option->getLightIntensity(), option->getLightColor());
    /* Renderer the toon objects */
    for (i = 0; i < size; i++) {
      PMDObject *object = &objects[i];
      if (!object->isEnable())
        continue;
      PMDModel *model = object->getPMDModel();
      if (!model->getToonFlag())
        continue;
      /* set texture coordinates for shadow mapping */
      model->updateShadowColorTexCoord(option->getShadowMappingSelfDensity());
      /* tell model to Renderer with the shadow corrdinates */
      model->setSelfShadowDrawing(true);
      /* Renderer model and edge */
      model->renderModel();
      model->renderEdge();
      /* disable shadow Renderering */
      model->setSelfShadowDrawing(false);
    }
    if (!option->getUseCartoonRendering())
      updateLighting(false, option->getUseMMDLikeCartoon(), option->getLightDirection(), option->getLightIntensity(), option->getLightColor());
  }

  /* Renderer the part clipped by the depth texture */
  /* activate the texture unit for shadow mapping and make it current */
  glActiveTextureARB(GL_TEXTURE3_ARB);

  /* set texture matrix (note: matrices should be set in reverse order) */
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  /* move the range from [-1,1] to [0,1] */
  glTranslated(0.5, 0.5, 0.5);
  glScaled(0.5, 0.5, 0.5);
  /* multiply the model view matrix when the depth texture was Renderered */
  glMultMatrixd(modelview);
  /* multiply the inverse matrix of current model view matrix */
  glMultMatrixf(m_rotMatrixInv);

  /* revert to model view matrix mode */
  glMatrixMode(GL_MODELVIEW);

  /* enable texture mapping with texture coordinate generation */
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_TEXTURE_GEN_S);
  glEnable(GL_TEXTURE_GEN_T);
  glEnable(GL_TEXTURE_GEN_R);
  glEnable(GL_TEXTURE_GEN_Q);

  /* bind the depth texture Renderered at the first step */
  glBindTexture(GL_TEXTURE_2D, m_depthTextureID);

  /* depth texture set up was done, now switch current texture unit to default */
  glActiveTextureARB(GL_TEXTURE0_ARB);

  /* set depth func to allow overwrite for the same surface in the following Renderering */
  glDepthFunc(GL_LEQUAL);

  if (option->getShadowMappingLightFirst()) {
    /* the area clipped by depth texture by alpha test is dark part */
    glAlphaFunc(GL_GEQUAL, 0.1f);

    /* light setting for non-toon objects */
    lightdim[0] = lightdim[1] = lightdim[2] = 0.55f - 0.2f * option->getShadowMappingSelfDensity();
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightdim);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightdim);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightblk);

    /* Renderer the non-toon objects (back, floor, non-toon models) */
    stage->renderBackground();
    stage->renderFloor();
    for (i = 0; i < size; i++) {
      PMDObject *object = &objects[i];
      if (!object->isEnable())
        continue;
      PMDModel *model = object->getPMDModel();
      if (!model->getToonFlag())
        continue;
      model->renderModel();
    }

    /* for toon objects, they should apply the model-defined toon texture color at texture coordinates (0, 0) for shadow Renderering */
    /* so restore the light setting */
    if (option->getUseCartoonRendering())
      updateLighting(true, option->getUseMMDLikeCartoon(), option->getLightDirection(), option->getLightIntensity(), option->getLightColor());
    /* Renderer the toon objects */
    for (i = 0; i < size; i++) {
      PMDObject *object = &objects[i];
      if (!object->isEnable())
        continue;
      PMDModel *model = object->getPMDModel();
      if (!model->getToonFlag())
        continue;
      /* set texture coordinates for shadow mapping */
      model->updateShadowColorTexCoord(option->getShadowMappingSelfDensity());
      /* tell model to Renderer with the shadow corrdinates */
      model->setSelfShadowDrawing(true);
      /* Renderer model and edge */
      model->renderModel();
      /* disable shadow Renderering */
      model->setSelfShadowDrawing(false);
    }
    if (!option->getUseCartoonRendering())
      updateLighting(false, option->getUseMMDLikeCartoon(), option->getLightDirection(), option->getLightIntensity(), option->getLightColor());
  } else {
    /* the area clipped by depth texture by alpha test is light part */
    glAlphaFunc(GL_GEQUAL, 0.001f);
    stage->renderBackground();
    stage->renderFloor();
    for (i = 0; i < size; i++) {
      PMDObject *object = &objects[i];
      if (!object->isEnable())
        continue;
      object->getPMDModel()->renderModel();
    }
  }

  /* reset settings */
  glDepthFunc(GL_LESS);
  glAlphaFunc(GL_GEQUAL, 0.05f);

  glActiveTextureARB(GL_TEXTURE3_ARB);
  glDisable(GL_TEXTURE_GEN_S);
  glDisable(GL_TEXTURE_GEN_T);
  glDisable(GL_TEXTURE_GEN_R);
  glDisable(GL_TEXTURE_GEN_Q);
  glDisable(GL_TEXTURE_2D);
  glActiveTextureARB(GL_TEXTURE0_ARB);
}

/* SceneRenderer::RendererScene: Renderer scene */
void SceneRenderer::renderScene(Option *option, Stage *stage, PMDObject *objects, int size)
{
  short i;
  /* clear Renderering buffer */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);

  /* set model viwe matrix */
  glLoadIdentity();
  glMultMatrixf(m_rotMatrix);

  /* stage and shadhow */
  glPushMatrix();
  /* background */
  stage->renderBackground();
  /* enable stencil */
  glEnable(GL_STENCIL_TEST);
  glStencilFunc(GL_ALWAYS, 1, ~0);
  /* make stencil tag true */
  glStencilOp(GL_KEEP, GL_KEEP , GL_REPLACE);
  /* Renderer floor */
  stage->renderFloor();
  /* Renderer shadow stencil */
  glColorMask(0, 0, 0, 0) ;
  glDepthMask(0);
  glEnable(GL_STENCIL_TEST);
  glStencilFunc(GL_EQUAL, 1, ~0);
  /* increment 1 pixel stencil */
  glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
  /* Render model */
  glDisable(GL_DEPTH_TEST);
  for (i = 0; i < size; i++) {
    PMDObject *object = &objects[i];
    if (!object->isEnable())
      continue;
    glPushMatrix();
    glMultMatrixf(stage->getShadowMatrix());
    object->getPMDModel()->renderForShadow();
    glPopMatrix();
  }
  glEnable(GL_DEPTH_TEST);
  glColorMask(1, 1, 1, 1);
  glDepthMask(1);
  /* if stencil is 2, Renderer shadow with blend on */
  glStencilFunc(GL_EQUAL, 2, ~0);
  glDisable(GL_LIGHTING);
  glColor4f(0.1f, 0.1f, 0.1f, option->getShadowMappingSelfDensity());
  glDisable(GL_DEPTH_TEST);
  stage->renderFloor();
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_STENCIL_TEST);
  glEnable(GL_LIGHTING);
  glPopMatrix();

  /* Render model */
  for (i = 0; i < size; i++) {
    PMDObject *object = &objects[i];
    if (!object->isEnable())
      continue;
    PMDModel *model = object->getPMDModel();
    model->renderModel();
    model->renderEdge();
  }
}

/* SceneRenderer::render: Render all */
void SceneRenderer::render(Option *option, Stage *stage, PMDObject *objects, int size)
{
  /* update scale */
  updateScale();
  /* update trans and rotation matrix */
  updateTransRotMatrix();

  if (m_enableShadowMapping)
    renderSceneShadowMap(option, stage, objects, size);
  else
    renderScene(option, stage, objects, size);
}

/* SceneRenderer::pickModel: pick up a model at the screen position */
int SceneRenderer::pickModel(PMDObject *objects, int size, int x, int y, int *allowDropPicked)
{
  int i;

  GLuint selectionBuffer[512];
  GLint viewport[4];

  GLint hits;
  GLuint *data;
  GLuint minDepth = 0, minDepthAllowDrop = 0;
  int minID, minIDAllowDrop;
  GLuint depth;
  int id;

  /* get current viewport */
  glGetIntegerv(GL_VIEWPORT, viewport);
  /* set selection buffer */
  glSelectBuffer(512, selectionBuffer);
  /* begin selection mode */
  glRenderMode(GL_SELECT);
  /* save projection matrix */
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  /* set projection matrix for picking */
  glLoadIdentity();
  /* apply picking matrix */
  gluPickMatrix(x, viewport[3] - y, 15.0, 15.0, viewport);
  /* apply normal projection matrix */
  applyProjectionMatrix();
  /* switch to model view mode */
  glMatrixMode(GL_MODELVIEW);
  /* initialize name buffer */
  glInitNames();
  glPushName(0);
  /* draw models with selection names */
  for (i = 0; i < size; i++) {
    PMDObject *object = &objects[i];
    if (!object->isEnable())
      continue;
    glLoadName(i);
    object->getPMDModel()->renderForShadow();
  }

  /* restore projection matrix */
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  /* switch to model view mode */
  glMatrixMode(GL_MODELVIEW);
  /* end selection mode and get number of hits */
  hits = glRenderMode(GL_RENDER);
  if (hits == 0) return -1;
  data = &(selectionBuffer[0]);
  minID = -1;
  minIDAllowDrop = -1;
  for (i = 0; i < hits; i++) {
    depth = *(data + 1);
    id = *(data + 3);
    if (minID == -1 || minDepth > depth) {
      minDepth = depth;
      minID = id;
    }
    if (allowDropPicked && objects[id].allowMotionFileDrop()) {
      if (minIDAllowDrop == -1 || minDepthAllowDrop > depth) {
        minDepthAllowDrop = depth;
        minIDAllowDrop = id;
      }
    }
    data += *data + 3;
  }
  if (allowDropPicked)
    *allowDropPicked = minIDAllowDrop;

  return minID;
}

/* SceneRenderer::update: update scale */
void SceneRenderer::updateScale()
{
  float diff;

  /* if no difference, return */
  if (m_scaleCurrent == m_scale) return;

  diff = fabs(m_scaleCurrent - m_scale);
  if (diff < RENDER_MINSCALEDIFF) {
    m_scaleCurrent = m_scale;
  } else {
    m_scaleCurrent = m_scaleCurrent * (RENDER_SCALESPEEDRATE) + m_scale * (1.0f - RENDER_SCALESPEEDRATE);
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
  if (m_rotCurrent == m_rot && m_transCurrent == m_trans) return;

  /* calculate difference */
  trans = m_trans;
  trans -= m_transCurrent;
  diff1 = trans.length2();
  rot = m_rot;
  rot -= m_rotCurrent;
  diff2 = rot.length2();

  if (diff1 > RENDER_MINMOVEDIFF)
    m_transCurrent = m_transCurrent.lerp(m_trans, 1.0f - RENDER_MOVESPEEDRATE); /* current * 0.9 + target * 0.1 */
  else
    m_transCurrent = m_trans;
  if (diff2 > RENDER_MINSPINDIFF)
    m_rotCurrent = m_rotCurrent.slerp(m_rot, 1.0f - RENDER_SPINSPEEDRATE); /* current * 0.9 + target * 0.1 */
  else
    m_rotCurrent = m_rot;

  updateModelViewMatrix();
}

/* SceneRenderer::rotate: rotate scene */
void SceneRenderer::rotate(float x, float y, float z)
{
  z = 0;
  m_rot = m_rot * btQuaternion(x, 0, 0);
  m_rot = btQuaternion(0, y, 0) * m_rot;
}

/* SceneRenderer::getScreenPointPosition: convert screen position to object position */
void SceneRenderer::getScreenPointPosition(btVector3 *dst, btVector3 *src)
{
  *dst = m_transMatrixInv * (*src);
}

/* SceneRenderer::updateLigithing: update light */
void SceneRenderer::updateLighting(bool useCartoonRendering, bool useMMDLikeCartoon, float *lightDirection, float lightIntensy, float *lightColor)
{
  float fLightDif[4];
  float fLightSpc[4];
  float fLightAmb[4];
  int i;
  float d, a, s;

  if (!useMMDLikeCartoon) {
    /* MMDAgent original cartoon */
    d = 0.2f;
    a = lightIntensy * 2.0f;
    s = 0.4f;
  } else if (useCartoonRendering) {
    /* like MikuMikuDance */
    d = 0.0f;
    a = lightIntensy * 2.0f;
    s = lightIntensy;
  } else {
    /* no toon */
    d = lightIntensy;
    a = 1.0f;
    s = 1.0f; /* OpenGL default */
  }

  for (i = 0; i < 3; i++)
    fLightDif[i] = lightColor[i] * d;
  fLightDif[3] = 1.0f;
  for (i = 0; i < 3; i++)
    fLightAmb[i] = lightColor[i] * a;
  fLightAmb[3] = 1.0f;
  for (i = 0; i < 3; i++)
    fLightSpc[i] = lightColor[i] * s;
  fLightSpc[3] = 1.0f;

  glLightfv(GL_LIGHT0, GL_POSITION, lightDirection);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, fLightDif);
  glLightfv(GL_LIGHT0, GL_AMBIENT, fLightAmb);
  glLightfv(GL_LIGHT0, GL_SPECULAR, fLightSpc);

  /* update light direction vector */
  m_lightVec = btVector3(lightDirection[0], lightDirection[1], lightDirection[2]);
  m_lightVec.normalize();
}

/* SceneRenderer::updateProjectionMatrix: update view information */
void SceneRenderer::updateProjectionMatrix()
{
  glViewport(0, 0, m_width, m_height);

  /* camera setting */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  applyProjectionMatrix();
  glMatrixMode(GL_MODELVIEW);

}

/* SceneRenderer::applyProjectionMatirx: update projection matrix */
void SceneRenderer::applyProjectionMatrix()
{
  double aspect = (double) m_height / (double) m_width;
  double ratio = (m_scaleCurrent == 0.0f) ? 1.0 : 1.0 / m_scaleCurrent;

  glFrustum(- ratio, ratio, - aspect * ratio, aspect * ratio, RENDER_VIEWPOINT_FRUSTUM_NEAR, RENDER_VIEWPOINT_FRUSTUM_FAR);
}

/* SceneRenderer::updateModelViewMatrix: update model view matrix */
void SceneRenderer::updateModelViewMatrix()
{
  m_transMatrix.setRotation(m_rotCurrent);
  m_transMatrix.setOrigin(m_transCurrent + m_cameraTrans);
  m_transMatrixInv = m_transMatrix.inverse();
  m_transMatrix.getOpenGLMatrix(m_rotMatrix);
  m_transMatrixInv.getOpenGLMatrix(m_rotMatrixInv);
}

/* SceneRenderer::updateDepthTextureViewParam: update center and radius information to get required range for shadow mapping */
void SceneRenderer::updateDepthTextureViewParam(PMDObject *objects, int num)
{
  int i;
  float d, dmax;
  float *r = new float[num];
  btVector3 *c = new btVector3[num];
  btVector3 cc = btVector3(0.0f, 0.0f, 0.0f);

  for (i = 0; i < num; i++) {
    PMDObject *object = &objects[i];
    if (!object->isEnable())
      continue;
    r[i] = object->getPMDModel()->calculateBoundingSphereRange(&(c[i]));
    cc += c[i];
  }
  if (num != 0)
    cc /= (float) num;

  dmax = 0.0f;
  for (i = 0; i < num; i++) {
    if (!objects[i].isEnable())
      continue;
    d = cc.distance(c[i]) + r[i];
    if (dmax < d)
      dmax = d;
  }

  m_shadowMapAutoViewEyePoint = cc;
  m_shadowMapAutoViewRadius = dmax;

  delete [] r;
  delete [] c;
}

} /* namespace */

