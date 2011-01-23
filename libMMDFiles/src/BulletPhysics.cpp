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

#include "BulletPhysics.h"

/* BulletPhysics::initialize: initialize BulletPhysics */
void BulletPhysics::initialize()
{
   m_collisionConfig = NULL;
   m_dispatcher = NULL;
   m_overlappingPairCache = NULL;
   m_solver = NULL;
   m_world = NULL;

   m_fps = 0;

   m_boxListEnabled = false;
   m_sphereListEnabled = false;
}

/* BulletPhysics::clear: free BulletPhysics */
void BulletPhysics::clear()
{
#if 0
   int i, numObject;
   btCollisionObject *obj;
   btRigidBody *body;

   if (m_world) {
      /* release remaining objects within the world */
      numObject = m_world->getNumCollisionObjects();
      for (i = 0; i < numObject; i++) {
         obj = m_world->getCollisionObjectArray()[i];
         body = btRigidBody::upcast(obj);
         if (body && body->getMotionState())
            delete body->getMotionState();
         m_world->removeCollisionObject(obj);
         delete obj;
      }
   }
   if (m_world)
      delete m_world;
   if (m_solver)
      delete m_solver;
   if (m_overlappingPairCache)
      delete m_overlappingPairCache;
   if (m_dispatcher)
      delete m_dispatcher;
   if (m_collisionConfig)
      delete m_collisionConfig;
#endif
   initialize();
}

/* BulletPhysics::BulletPhysics: constructor */
BulletPhysics::BulletPhysics()
{
   initialize();
}

/* BulletPhysics::~BulletPhysics: destructor */
BulletPhysics::~BulletPhysics()
{
   clear();
}

/* BulletPhysics::setup: initialize and setup BulletPhysics */
void BulletPhysics::setup(int simulationFps)
{
   float dist = 400.0f;

   clear();

   /* store values */
   m_fps = simulationFps;
   m_subStep = btScalar(1.0 / btScalar(m_fps));

   /* make a collision configuration */
   m_collisionConfig = new btDefaultCollisionConfiguration();

   /* make a collision dispatcher from the configuration for sequenciall processing */
   m_dispatcher = new btCollisionDispatcher(m_collisionConfig);

   /* set broadphase */
   m_overlappingPairCache = new btAxisSweep3(btVector3(-dist, -dist, -dist), btVector3(dist, dist, dist), 1024);

   /* make a sequencial constraint solver */
   m_solver = new btSequentialImpulseConstraintSolver();

   /* create simulation world */
   m_world = new btDiscreteDynamicsWorld(m_dispatcher, m_overlappingPairCache, m_solver, m_collisionConfig);

   /* set default gravity */
   /* some tweak for the simulation to match that of MMD... */
   m_world->setGravity(btVector3(0.0f, -9.8f * 2, 0.0f));

   /* a weird configuration to use 120Hz simulation */
   /* change the number of constraint solving iteration to be inversely propotional to simulation rate */
   /* is this a bug of bulletphysics? */
   m_world->getSolverInfo().m_numIterations = (int) (10 * 60 / m_fps);
}

/* BulletPhysics::update: step the simulation world forward */
void BulletPhysics::update(float deltaFrame)
{
   btScalar sec = deltaFrame / 30.0f; /* convert frame to second */

   if (sec > 1.0) {
      /* long pause, just move ahead at one step */
      m_world->stepSimulation(sec, 1, sec);
   } else {
      /* progress by (1.0/fps) sub step */
      m_world->stepSimulation(sec, m_fps, m_subStep);
   }
}

/* BulletPhysics::getWorld: get simulation world */
btDiscreteDynamicsWorld *BulletPhysics::getWorld()
{
   return m_world;
}

/* drawCube: draw a cube */
static void drawCube()
{
   static GLfloat vertices [8][3] = {
      { -0.5f, -0.5f, 0.5f},
      { 0.5f, -0.5f, 0.5f},
      { 0.5f, 0.5f, 0.5f},
      { -0.5f, 0.5f, 0.5f},
      { 0.5f, -0.5f, -0.5f},
      { -0.5f, -0.5f, -0.5f},
      { -0.5f, 0.5f, -0.5f},
      { 0.5f, 0.5f, -0.5f}
   };

   glBegin(GL_POLYGON);
   glVertex3fv(vertices[0]);
   glVertex3fv(vertices[1]);
   glVertex3fv(vertices[2]);
   glVertex3fv(vertices[3]);
   glEnd();
   glBegin(GL_POLYGON);
   glVertex3fv(vertices[4]);
   glVertex3fv(vertices[5]);
   glVertex3fv(vertices[6]);
   glVertex3fv(vertices[7]);
   glEnd();
   glBegin(GL_POLYGON);
   glVertex3fv(vertices[1]);
   glVertex3fv(vertices[4]);
   glVertex3fv(vertices[7]);
   glVertex3fv(vertices[2]);
   glEnd();
   glBegin(GL_POLYGON);
   glVertex3fv(vertices[5]);
   glVertex3fv(vertices[0]);
   glVertex3fv(vertices[3]);
   glVertex3fv(vertices[6]);
   glEnd();
   glBegin(GL_POLYGON);
   glVertex3fv(vertices[3]);
   glVertex3fv(vertices[2]);
   glVertex3fv(vertices[7]);
   glVertex3fv(vertices[6]);
   glEnd();
   glBegin(GL_POLYGON);
   glVertex3fv(vertices[1]);
   glVertex3fv(vertices[0]);
   glVertex3fv(vertices[5]);
   glVertex3fv(vertices[4]);
   glEnd();
}

/* drawSphere: draw a sphere */
static void drawSphere(int lats, int longs)
{
   int i, j;
   double lat0;
   double z0;
   double zr0;
   double lat1;
   double z1;
   double zr1;
   double lng;
   double x;
   double y;

   for (i = 0; i <= lats; i++) {
      lat0 = BULLETPHYSICS_PI * (-0.5 + (double) (i - 1) / lats);
      z0 = sin(lat0);
      zr0 = cos(lat0);
      lat1 = BULLETPHYSICS_PI * (-0.5 + (double) i / lats);
      z1 = sin(lat1);
      zr1 = cos(lat1);

      glBegin(GL_QUAD_STRIP);
      for (j = 0; j <= longs; j++) {
         lng = 2 * BULLETPHYSICS_PI * (double) (j - 1) / longs;
         x = cos(lng);
         y = sin(lng);

         glNormal3f((GLfloat)(x * zr0), (GLfloat)(y * zr0), (GLfloat)z0);
         glVertex3f((GLfloat)(x * zr0), (GLfloat)(y * zr0), (GLfloat)z0);
         glNormal3f((GLfloat)(x * zr1), (GLfloat)(y * zr1), (GLfloat)z1);
         glVertex3f((GLfloat)(x * zr1), (GLfloat)(y * zr1), (GLfloat)z1);
      }
      glEnd();
   }
}

/* drawConvex: draw a convex shape */
static void drawConvex(btConvexShape* shape)
{
   int i;
   int i1, i2, i3;
   int index = 0;
   int index1;
   int index2;
   int index3;
   btVector3 v1, v2, v3;
   btVector3 normal;

   const unsigned int* idx;
   const btVector3* vtx;
   btShapeHull *hull = new btShapeHull(shape);

   hull->buildHull(shape->getMargin());
   if (hull->numTriangles () > 0) {
      index = 0;
      idx = hull->getIndexPointer();
      vtx = hull->getVertexPointer();
      glBegin (GL_TRIANGLES);
      for (i = 0; i < hull->numTriangles (); i++) {
         i1 = index++;
         i2 = index++;
         i3 = index++;
         btAssert(i1 < hull->numIndices () &&
                  i2 < hull->numIndices () &&
                  i3 < hull->numIndices ());

         index1 = idx[i1];
         index2 = idx[i2];
         index3 = idx[i3];
         btAssert(index1 < hull->numVertices () &&
                  index2 < hull->numVertices () &&
                  index3 < hull->numVertices ());
         v1 = vtx[index1];
         v2 = vtx[index2];
         v3 = vtx[index3];
         normal = (v3 - v1).cross(v2 - v1);
         normal.normalize ();

         glNormal3f(normal.getX(), normal.getY(), normal.getZ());
         glVertex3f (v1.x(), v1.y(), v1.z());
         glVertex3f (v2.x(), v2.y(), v2.z());
         glVertex3f (v3.x(), v3.y(), v3.z());
      }
      glEnd ();
   }

   delete hull;
}

/* debugDisplay: render rigid bodies */
void BulletPhysics::debugDisplay()
{
   GLfloat color[] = {0.8f, 0.8f, 0.0f, 1.0f};
   GLint polygonMode[2];
   btRigidBody* body;
   btScalar m[16];
   btCollisionShape* shape;
   btVector3 halfExtent;
   const btSphereShape* sphereShape;
   float radius;


   const int numObjects = m_world->getNumCollisionObjects();

   /* draw in wire frame */
   glGetIntegerv(GL_POLYGON_MODE, polygonMode);
   if (polygonMode[1] != GL_LINE)
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glDisable(GL_TEXTURE_2D);
   for (int i = 0; i < numObjects; i++) {
      /* set color */
      color[1] = 0.8f / (float) ((i % 5) + 1) + 0.2f;
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
      /* draw */
      {
         body = btRigidBody::upcast(m_world->getCollisionObjectArray()[i]);
         body->getWorldTransform().getOpenGLMatrix(m);
         shape = body->getCollisionShape();
         glPushMatrix();
         glMultMatrixf(m);
         switch (shape->getShapeType()) {
         case BOX_SHAPE_PROXYTYPE: {
            const btBoxShape* boxShape = static_cast<const btBoxShape*>(shape);
            halfExtent = boxShape->getHalfExtentsWithMargin();
            glScaled(2 * halfExtent[0], 2 * halfExtent[1], 2 * halfExtent[2]);
            if (m_boxListEnabled) {
               glCallList(m_boxList);
            } else {
               m_boxList = glGenLists(1);
               glNewList(m_boxList, GL_COMPILE);
               drawCube();
               glEndList();
               m_boxListEnabled = true;
            }
            break;
         }
         case SPHERE_SHAPE_PROXYTYPE: {
            sphereShape = static_cast<const btSphereShape*>(shape);
            radius = sphereShape->getMargin(); /* radius doesn't include the margin, so draw with margin */
            glScaled(radius, radius, radius);
            if (m_sphereListEnabled) {
               glCallList(m_sphereList);
            } else {
               m_sphereList = glGenLists(1);
               glNewList(m_sphereList, GL_COMPILE);
               drawSphere(10, 10);
               glEndList();
               m_sphereListEnabled = true;
            }
            break;
         }
         default:
            if (shape->isConvex()) {
               drawConvex((btConvexShape*)shape);
            }
         }
         glPopMatrix();
      }
   }
   if (polygonMode[1] != GL_LINE) {
      glPolygonMode(GL_FRONT_AND_BACK, polygonMode[1]);
   }
}
