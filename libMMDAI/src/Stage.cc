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

#include "MMDAI/Option.h"
#include "MMDAI/Stage.h"

/* findPlane: calculate plane */
static void findPlane(GLfloat plane[4], TileTexture *t)
{
  GLfloat vec0x, vec0y, vec0z, vec1x, vec1y, vec1z;

  /* need 2 vectors to find cross product */
  vec0x = t->getSize(2, 0) - t->getSize(1, 0);
  vec0y = t->getSize(2, 1) - t->getSize(1, 1);
  vec0z = t->getSize(2, 2) - t->getSize(1, 2);

  vec1x = t->getSize(3, 0) - t->getSize(1, 0);
  vec1y = t->getSize(3, 1) - t->getSize(1, 1);
  vec1z = t->getSize(3, 2) - t->getSize(1, 2);

  /* find cross product to get A, B, and C of plane equation */
  plane[0] =   vec0y * vec1z - vec0z * vec1y;
  plane[1] = -(vec0x * vec1z - vec0z * vec1x);
  plane[2] =   vec0x * vec1y - vec0y * vec1x;
  plane[3] = -(plane[0] * t->getSize(1, 0) + plane[1] * t->getSize(1, 1) + plane[2] * t->getSize(1, 2));
}

/* shadowMatrix: calculate shadow projection matrix */
static void shadowMatrix(GLfloat shadowMat[4][4], GLfloat groundplane[4], GLfloat lightpos[4])
{
  GLfloat dot;

  /* find dot product between light position vector and ground plane normal */
  dot = groundplane[0] * lightpos[0] +
        groundplane[1] * lightpos[1] +
        groundplane[2] * lightpos[2] +
        groundplane[3] * lightpos[3];

  shadowMat[0][0] = dot - lightpos[0] * groundplane[0];
  shadowMat[1][0] = 0.f - lightpos[0] * groundplane[1];
  shadowMat[2][0] = 0.f - lightpos[0] * groundplane[2];
  shadowMat[3][0] = 0.f - lightpos[0] * groundplane[3];

  shadowMat[0][1] = 0.f - lightpos[1] * groundplane[0];
  shadowMat[1][1] = dot - lightpos[1] * groundplane[1];
  shadowMat[2][1] = 0.f - lightpos[1] * groundplane[2];
  shadowMat[3][1] = 0.f - lightpos[1] * groundplane[3];

  shadowMat[0][2] = 0.f - lightpos[2] * groundplane[0];
  shadowMat[1][2] = 0.f - lightpos[2] * groundplane[1];
  shadowMat[2][2] = dot - lightpos[2] * groundplane[2];
  shadowMat[3][2] = 0.f - lightpos[2] * groundplane[3];

  shadowMat[0][3] = 0.f - lightpos[3] * groundplane[0];
  shadowMat[1][3] = 0.f - lightpos[3] * groundplane[1];
  shadowMat[2][3] = 0.f - lightpos[3] * groundplane[2];
  shadowMat[3][3] = dot - lightpos[3] * groundplane[3];
}

/* Stage::Stage: constructor */
Stage::Stage()
{
  initialize();
}

/* Stage::~Stage: destructor */
Stage::~Stage()
{
  clear();
}

/* Stage::makeFloorBody: create a rigid body for floor */
void Stage::makeFloorBody(float width, float depth)
{
  btVector3 localInertia(0, 0, 0);
  btScalar mass = 0.0f;
  btCollisionShape *colShape;
  btTransform startTransform;
  btDefaultMotionState* myMotionState;

  releaseFloorBody();

  colShape = new btBoxShape(btVector3(width, 10.0f, depth)); /* <- can free memory ? */
  if (mass != 0.0f)
    colShape->calculateLocalInertia(mass, localInertia);
  startTransform.setIdentity();
  startTransform.setOrigin(btVector3(0.0f, -9.99f, 0.0f));
  myMotionState = new btDefaultMotionState(startTransform);
  btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
  rbInfo.m_linearDamping = 0.5f;
  rbInfo.m_angularDamping = 0.5f;
  rbInfo.m_restitution = 0.8f;
  rbInfo.m_friction = 0.5f;
  m_floorBody = new btRigidBody(rbInfo);

  if (m_bullet)
    m_bullet->getWorld()->addRigidBody(m_floorBody);
}

/* Stage::releaseFloorBody: release rigid body for floor */
void Stage::releaseFloorBody()
{
  if (m_floorBody) {
    if (m_floorBody->getMotionState())
      delete m_floorBody->getMotionState();
    if (m_bullet)
      m_bullet->getWorld()->removeCollisionObject(m_floorBody);
    delete m_floorBody;
    m_floorBody = NULL;
  }
}

/* Stage::initialize: initialize stage */
void Stage::initialize()
{
  int i, j;

  m_hasPMD = false;
  m_listIndexPMD = 0;
  m_listIndexPMDValid = false;
  m_bullet = NULL;
  m_floorBody = NULL;
  for (i = 0; i < 4 ; i++)
    for (j = 0; j < 4; j++)
      m_floorShadow[i][j] = 0.0f;
}

/* Stage::clear: free stage */
void Stage::clear()
{
  releaseFloorBody();
  initialize();
}

/* Stage::setSize: set size of floor and background */
void Stage::setSize(float *size, float numx, float numy)
{
  float w = size[0]; /* width */
  float d = size[1]; /* depth */
  float h = size[2]; /* height */
  m_floor.setSize(-w, 0.0f, d,
                  w, 0.0f, d,
                  w, 0.0f, -d,
                  -w, 0.0f, -d,
                  numx, numy);
  m_background.setSize(-w, 0.0f, -d,
                       w, 0.0f, -d,
                       w, h, -d,
                       -w, h, -d,
                       numx, numy);
  makeFloorBody(w, d);
}

/* Stage::loadFloor: load floor image */
bool Stage::loadFloor(PMDModelLoader *loader, BulletPhysics *bullet)
{
  bool ret;

  if (m_bullet == NULL)
    m_bullet = bullet;

  ret = m_floor.load(loader);
  if (ret) {
    if (m_hasPMD) {
      m_pmd.release();
      m_hasPMD = false;
    }
  } else {
    MMDAILogInfo("! Error: Stage: unable to load floor \"%s\"", loader->getLocation());
  }

  return ret;
}

/* Stage::loadBackground: load background image */
bool Stage::loadBackground(PMDModelLoader *loader, BulletPhysics *bullet)
{
  bool ret;

  if (m_bullet == NULL)
    m_bullet = bullet;

  ret = m_background.load(loader);
  if (ret) {
    if (m_hasPMD) {
      m_pmd.release();
      m_hasPMD = false;
    }
  } else {
    MMDAILogInfo("! Error: Stage: unable to load background \"%s\"", loader->getLocation());
  }
  return ret;
}

/* Stage::loadStagePMD: load stage pmd */
bool Stage::loadStagePMD(PMDModelLoader *loader, BulletPhysics *bullet)
{
  bool ret;

  if (m_bullet == NULL)
    m_bullet = bullet;

  ret = m_pmd.load(loader, bullet);
  if (ret) {
    m_pmd.setToonFlag(false);
    m_pmd.updateSkin();
    m_hasPMD = true;
    if (m_listIndexPMDValid) {
      glDeleteLists(m_listIndexPMD, 1);
      m_listIndexPMDValid = false;
    }
  } else {
    MMDAILogInfo("! Error: Stage: unable to load stage PMD \"%s\"", loader->getLocation());
  }

  return ret;
}

/* Stage::renderFloor: render the floor */
void Stage::renderFloor()
{
  const float normal[3] = {0.0f, 1.0f, 0.0f};

  if (m_hasPMD)
    renderPMD();
  else
    m_floor.render(false, normal);
}

/* Stage::renderBackground: render the background */
void Stage::renderBackground()
{
  const float normal[3] = {0.0f, 0.0f, 1.0f};

  if (!m_hasPMD)
    m_background.render(true, normal);
}

/* Stage::renderPMD: render the stage pmd */
void Stage::renderPMD()
{
  if (m_listIndexPMDValid) {
    glCallList(m_listIndexPMD);
    return;
  }

  m_listIndexPMD = glGenLists(1);
  glNewList(m_listIndexPMD, GL_COMPILE);
  glPushMatrix();
  m_pmd.renderModel();
  glPopMatrix();
  glEndList();
  m_listIndexPMDValid = true;
}

/* Stage::updateShadowMatrix: update shadow projection matrix */
void Stage::updateShadowMatrix(float lightDirection[4])
{
  GLfloat floorPlane[4];

  findPlane(floorPlane, &m_floor);
  shadowMatrix(m_floorShadow, floorPlane, lightDirection);
}

/* Stage::getShadowMatrix: get shadow projection matrix */
GLfloat *Stage::getShadowMatrix() const
{
  return (GLfloat *) m_floorShadow;
}
