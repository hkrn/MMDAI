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

#include "MMDAI/MMDAI.h"

namespace MMDAI {

GLSceneRenderEngine::GLSceneRenderEngine()
  : m_boxList(0),
    m_sphereList(0),
    m_boxListEnabled(false),
    m_sphereListEnabled(false)
{
}

GLSceneRenderEngine::~GLSceneRenderEngine()
{
}

void GLSceneRenderEngine::drawCube()
{
   static const GLfloat vertices [8][3] = {
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

void GLSceneRenderEngine::drawSphere(int lats, int longs)
{
   for (int i = 0; i <= lats; i++) {
      const double lat0 = BULLETPHYSICS_PI * (-0.5 + (double) (i - 1) / lats);
      const double z0 = sin(lat0);
      const double zr0 = cos(lat0);
      const double lat1 = BULLETPHYSICS_PI * (-0.5 + (double) i / lats);
      const double z1 = sin(lat1);
      const double zr1 = cos(lat1);

      glBegin(GL_QUAD_STRIP);
      for (int j = 0; j <= longs; j++) {
         const double lng = 2 * BULLETPHYSICS_PI * (double) (j - 1) / longs;
         const double x = cos(lng);
         const double y = sin(lng);

         glNormal3f((GLfloat)(x * zr0), (GLfloat)(y * zr0), (GLfloat)z0);
         glVertex3f((GLfloat)(x * zr0), (GLfloat)(y * zr0), (GLfloat)z0);
         glNormal3f((GLfloat)(x * zr1), (GLfloat)(y * zr1), (GLfloat)z1);
         glVertex3f((GLfloat)(x * zr1), (GLfloat)(y * zr1), (GLfloat)z1);
      }
      glEnd();
   }
}

void GLSceneRenderEngine::drawConvex(btConvexShape *shape)
{
   btShapeHull *hull = new btShapeHull(shape);
   hull->buildHull(shape->getMargin());

   if (hull->numTriangles () > 0) {
      int index = 0;
      const unsigned int *idx = hull->getIndexPointer();
      const btVector3 *vtx = hull->getVertexPointer();
      glBegin (GL_TRIANGLES);
      for (int i = 0; i < hull->numTriangles (); i++) {
         const int i1 = index++;
         const int i2 = index++;
         const int i3 = index++;
         btAssert(i1 < hull->numIndices () &&
                  i2 < hull->numIndices () &&
                  i3 < hull->numIndices ());

         const int index1 = idx[i1];
         const int index2 = idx[i2];
         const int index3 = idx[i3];
         btAssert(index1 < hull->numVertices () &&
                  index2 < hull->numVertices () &&
                  index3 < hull->numVertices ());
         const btVector3 v1 = vtx[index1];
         const btVector3 v2 = vtx[index2];
         const btVector3 v3 = vtx[index3];
         btVector3 normal = (v3 - v1).cross(v2 - v1);
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

void GLSceneRenderEngine::renderRigidBodies(BulletPhysics *bullet)
{
   GLfloat color[] = {0.8f, 0.8f, 0.0f, 1.0f};
   GLint polygonMode[2] = { 0, 0 };
   btRigidBody* body = NULL;
   btScalar m[16];
   btCollisionShape* shape = NULL;
   btVector3 halfExtent;
   btDiscreteDynamicsWorld *world = bullet->getWorld();
   const btSphereShape* sphereShape;
   float radius;
   const int numObjects = world->getNumCollisionObjects();

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
         body = btRigidBody::upcast(world->getCollisionObjectArray()[i]);
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
            }
            else {
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
            }
            else {
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
               drawConvex(static_cast<btConvexShape*>(shape));
            }
         }
         glPopMatrix();
      }
   }
   if (polygonMode[1] != GL_LINE) {
      glPolygonMode(GL_FRONT_AND_BACK, polygonMode[1]);
   }
}

void GLSceneRenderEngine::renderBone(PMDBone *bone)
{
   btScalar m[16];
   PMDBone *parentBone = bone->getParentBone();
   const btTransform *trans = bone->getTransform();
   const unsigned char type = bone->getType();
   const bool isSimulated = bone->isSimulated();

   /* do not draw IK target bones if the IK chain is under simulation */
   if (type == IK_TARGET && parentBone && parentBone->isSimulated())
     return;

   trans->getOpenGLMatrix(m);

   /* draw node */
   glPushMatrix();
   glMultMatrixf(m);
   if (type != NO_DISP) { /* do not draw invisible bone nodes */
      if (isSimulated) {
         /* under physics simulation */
         glColor4f(0.8f, 0.8f, 0.0f, 1.0f);
         glScaled(0.1, 0.1, 0.1);
      }
      else {
         switch (type) {
         case IK_DESTINATION:
            glColor4f(0.7f, 0.2f, 0.2f, 1.0f);
            glScaled(0.25, 0.25, 0.25);
            break;
         case UNDER_IK:
            glColor4f(0.8f, 0.5f, 0.0f, 1.0f);
            glScaled(0.15, 0.15, 0.15);
            break;
         case IK_TARGET:
            glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
            glScaled(0.15, 0.15, 0.15);
            break;
         case UNDER_ROTATE:
         case TWIST:
         case FOLLOW_ROTATE:
            glColor4f(0.0f, 0.8f, 0.2f, 1.0f);
            glScaled(0.15, 0.15, 0.15);
            break;
         default:
            if (bone->hasMotionIndependency()) {
               glColor4f(0.0f, 1.0f, 1.0f, 1.0f);
               glScaled(0.25, 0.25, 0.25);
            } else {
               glColor4f(0.0f, 0.5f, 1.0f, 1.0f);
               glScaled(0.15, 0.15, 0.15);
            }
            break;
         }
      }
      drawCube();
   }
   glPopMatrix();

   if (!parentBone || type == IK_DESTINATION)
     return;

   /* draw line from parent */
   glPushMatrix();
   if (type == NO_DISP) {
      glColor4f(0.5f, 0.4f, 0.5f, 1.0f);
   }
   else if (isSimulated) {
      glColor4f(0.7f, 0.7f, 0.0f, 1.0f);
   }
   else if (type == UNDER_IK || type == IK_TARGET) {
      glColor4f(0.8f, 0.5f, 0.3f, 1.0f);
   }
   else {
      glColor4f(0.5f, 0.6f, 1.0f, 1.0f);
   }

   glBegin(GL_LINES);
   const btVector3 a = parentBone->getTransform()->getOrigin();
   const btVector3 b = trans->getOrigin();
   glVertex3f(a.x(), a.y(), a.z());
   glVertex3f(b.x(), b.y(), b.z());
   glEnd();

   glPopMatrix();
}

void GLSceneRenderEngine::renderBones(PMDModel *model)
{
   glDisable(GL_DEPTH_TEST);
   glDisable(GL_LIGHTING);
   glDisable(GL_TEXTURE_2D);

   /* draw bones */
   const int nbones = model->getNumBone();
   PMDBone *bones = model->getBonesPtr();
   for (int i = 0; i < nbones; i++)
      renderBone(&bones[i]);

   glEnable(GL_DEPTH_TEST);
   glEnable(GL_LIGHTING);
}

/* needs multi-texture function on OpenGL: */
/* texture unit 0: model texture */
/* texture unit 1: toon texture for toon shading */
/* texture unit 2: additional sphere map texture, if exist */
void GLSceneRenderEngine::renderModel(PMDModel *model)
{
   const btVector3 *vertices = model->getVerticesPtr();
   if (!vertices)
     return;

#ifndef MMDFILES_CONVERTCOORDINATESYSTEM
   glPushMatrix();
   glScalef(1.0f, 1.0f, -1.0f); /* from left-hand to right-hand */
   glCullFace(GL_FRONT);
#endif

   /* activate texture unit 0 */
   glActiveTextureARB(GL_TEXTURE0_ARB);
   glClientActiveTextureARB(GL_TEXTURE0_ARB);

   /* set lists */
   glEnableClientState(GL_VERTEX_ARRAY);
   glEnableClientState(GL_NORMAL_ARRAY);
   glVertexPointer(3, GL_FLOAT, sizeof(btVector3), model->getSkinnedVerticesPtr());
   glNormalPointer(GL_FLOAT, sizeof(btVector3), model->getSkinnedNormalsPtr());

   /* set model texture coordinates to texture unit 0 */
   glClientActiveTextureARB(GL_TEXTURE0_ARB);
   glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   glTexCoordPointer(2, GL_FLOAT, 0, model->getTexCoordsPtr());

   const bool enableToon = model->getToonFlag();
   const bool hasSingleSphereMap = model->hasSingleSphereMap();
   const bool hasMultipleSphereMap = model->hasMultipleSphereMap();

   if (enableToon) {
      /* set toon texture coordinates to texture unit 1 */
      glActiveTextureARB(GL_TEXTURE1_ARB);
      glEnable(GL_TEXTURE_2D);
      glClientActiveTextureARB(GL_TEXTURE1_ARB);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      if (model->isSelfShadowEnabled()) {
         /* when drawing a shadow part in shadow mapping, force toon texture coordinates to (0, 0) */
         glTexCoordPointer(2, GL_FLOAT, 0, model->getToonTexCoordsForSelfShadowPtr());
      }
      else {
         glTexCoordPointer(2, GL_FLOAT, 0, model->getToonTexCoordsPtr());
      }
      glActiveTextureARB(GL_TEXTURE0_ARB);
      glClientActiveTextureARB(GL_TEXTURE0_ARB);
   }

   if (hasSingleSphereMap) {
      /* this model contains single sphere map texture */
      /* set texture coordinate generation for sphere map on texture unit 0 */
      glEnable(GL_TEXTURE_2D);
      glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
      glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
      glDisable(GL_TEXTURE_2D);
   }

   if (hasMultipleSphereMap) {
      /* this model contains additional sphere map texture */
      /* set texture coordinate generation for sphere map on texture unit 2 */
      glActiveTextureARB(GL_TEXTURE2_ARB);
      glEnable(GL_TEXTURE_2D);
      glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
      glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
      glDisable(GL_TEXTURE_2D);
      glActiveTextureARB(GL_TEXTURE0_ARB);
   }

   /* calculate alpha value, applying model global alpha */
   const float modelAlpha = model->getGlobalAlpha();
   const unsigned short *surfaceData = model->getSurfacesPtr();

   /* render per material */
   const int nmaterials = model->getNumMaterial();
   for (int i = 0; i < nmaterials; i++) {
      float c[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
      PMDMaterial *m = model->getMaterialAt(i);
      /* set colors */
      double alpha = m->getAlpha();
      c[3] = alpha * modelAlpha;
      if (c[3] > 0.99f) {
        c[3] = 1.0f; /* clamp to 1.0 */
      }
      if (enableToon) {
         /* use averaged color of diffuse and ambient for both */
         m->copyAvgcol(c);
         glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, &(c[0]));
         m->copySpecular(c);
         glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, &(c[0]));
      }
      else {
         /* use each color */
         m->copyDiffuse(c);
         glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, &(c[0]));
         m->copyAmbient(c);
         glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, &(c[0]));
         m->copySpecular(c);
         glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, &(c[0]));
      }
      glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, m->getShiness());

      /* disable face culling for transparent materials */
      if (alpha < 1.0f) {
         glDisable(GL_CULL_FACE);
      }
      else {
         glEnable(GL_CULL_FACE);
      }

      /* if using multiple texture units, set current unit to 0 */
      if (enableToon || hasMultipleSphereMap) {
         glActiveTextureARB(GL_TEXTURE0_ARB);
      }

      const PMDTexture *tex = m->getTexture();
      if (tex != NULL) {
         /* bind model texture */
         const PMDTextureNative *native = tex->getNative();
         if (native != NULL) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, native->id);
            if (hasSingleSphereMap) {
               if (tex->isSphereMap()) {
                  /* this is sphere map */
                  /* enable texture coordinate generation */
                  if (tex->isSphereMapAdd())
                     glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
                  glEnable(GL_TEXTURE_GEN_S);
                  glEnable(GL_TEXTURE_GEN_T);
               }
               else {
                  /* disable generation */
                  glDisable(GL_TEXTURE_GEN_S);
                  glDisable(GL_TEXTURE_GEN_T);
               }
            }
         }
      }
      else {
         glDisable(GL_TEXTURE_2D);
      }

      if (enableToon) {
         /* set toon texture for texture unit 1 */
         const PMDTextureNative *native = model->getToonTextureAt(m->getToonID())->getNative();
         if (native != NULL) {
            glActiveTextureARB(GL_TEXTURE1_ARB);
            glBindTexture(GL_TEXTURE_2D, native->id);
            /* set GL_CLAMP_TO_EDGE for toon texture to avoid texture interpolation at edge */
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
         }
      }

      if (hasMultipleSphereMap) {
         const PMDTexture *addtex = m->getAdditionalTexture();
         if (addtex) {
            /* this material has additional sphere map texture, bind it at texture unit 2 */
            glActiveTextureARB(GL_TEXTURE2_ARB);
            glEnable(GL_TEXTURE_2D);
            if (addtex->isSphereMapAdd()) {
               glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
            }
            else {
               glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            }
            const PMDTextureNative *native = addtex->getNative();
            if (native != NULL) {
               glBindTexture(GL_TEXTURE_2D, native->id);
               glEnable(GL_TEXTURE_GEN_S);
               glEnable(GL_TEXTURE_GEN_T);
            }
         }
         else {
            /* disable generation */
            glActiveTextureARB(GL_TEXTURE2_ARB);
            glDisable(GL_TEXTURE_2D);
         }
      }

      /* draw elements */
      const int nsurfaces = m->getNumSurface();
      glDrawElements(GL_TRIANGLES, nsurfaces, GL_UNSIGNED_SHORT, surfaceData);

      /* move surface pointer to next material */
      surfaceData += nsurfaces;

      /* reset some parameters */
      if (tex && tex->isSphereMap() && tex->isSphereMapAdd()) {
         if (enableToon)
            glActiveTextureARB(GL_TEXTURE0_ARB);
         glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      }
   }

   glDisableClientState(GL_VERTEX_ARRAY);
   glDisableClientState(GL_NORMAL_ARRAY);
   if (enableToon) {
      glClientActiveTextureARB(GL_TEXTURE0_ARB);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      if (hasSingleSphereMap) {
         glActiveTextureARB(GL_TEXTURE0_ARB);
         glDisable(GL_TEXTURE_GEN_S);
         glDisable(GL_TEXTURE_GEN_T);
      }
      glClientActiveTextureARB(GL_TEXTURE1_ARB);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      if (hasMultipleSphereMap) {
         glActiveTextureARB(GL_TEXTURE2_ARB);
         glDisable(GL_TEXTURE_GEN_S);
         glDisable(GL_TEXTURE_GEN_T);
      }
      glActiveTextureARB(GL_TEXTURE0_ARB);
   } else {
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      if (hasSingleSphereMap) {
         glDisable(GL_TEXTURE_GEN_S);
         glDisable(GL_TEXTURE_GEN_T);
      }
      if (hasMultipleSphereMap) {
         glActiveTextureARB(GL_TEXTURE2_ARB);
         glDisable(GL_TEXTURE_GEN_S);
         glDisable(GL_TEXTURE_GEN_T);
         glActiveTextureARB(GL_TEXTURE0_ARB);
      }
   }

   if (hasSingleSphereMap || hasMultipleSphereMap) {
      glDisable(GL_TEXTURE_GEN_S);
      glDisable(GL_TEXTURE_GEN_T);
   }
   if (enableToon) {
      glActiveTextureARB(GL_TEXTURE1_ARB);
      glDisable(GL_TEXTURE_2D);
   }
   if (hasMultipleSphereMap) {
      glActiveTextureARB(GL_TEXTURE2_ARB);
      glDisable(GL_TEXTURE_2D);
   }
   glActiveTextureARB(GL_TEXTURE0_ARB);

   glDisable(GL_TEXTURE_2D);
   glEnable(GL_CULL_FACE);
#ifndef MMDFILES_CONVERTCOORDINATESYSTEM
   glCullFace(GL_BACK);
   glPopMatrix();
#endif
}

void GLSceneRenderEngine::renderEdge(PMDModel *model)
{
   const btVector3 *vertices = model->getVerticesPtr();
   const bool enableToon = model->getToonFlag();
   const unsigned int nsurfaces = model->getNumSurfaceForEdge();
   if (!vertices || !enableToon || nsurfaces == 0)
     return;

#ifndef MMDFILES_CONVERTCOORDINATESYSTEM
   glPushMatrix();
   glScalef(1.0f, 1.0f, -1.0f);
   glCullFace(GL_BACK);
#else
   /* draw back surface only */
   glCullFace(GL_FRONT);
#endif

   /* calculate alpha value */
   const float modelAlpha = model->getGlobalAlpha();
   const float *edgeColors = model->getEdgeColors();

   glDisable(GL_LIGHTING);
   glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer(3, GL_FLOAT, sizeof(btVector3), model->getEdgeVerticesPtr());
   glColor4f(edgeColors[0], edgeColors[1], edgeColors[2], edgeColors[3] * modelAlpha);
   glDrawElements(GL_TRIANGLES, nsurfaces, GL_UNSIGNED_SHORT, model->getSurfacesForEdgePtr());
   glDisableClientState(GL_VERTEX_ARRAY);
   glEnable(GL_LIGHTING);

   /* draw front again */
#ifndef MMDFILES_CONVERTCOORDINATESYSTEM
   glPopMatrix();
   glCullFace(GL_FRONT);
#else
   glCullFace(GL_BACK);
#endif
}

void GLSceneRenderEngine::renderShadow(PMDModel *model)
{
   const btVector3 *vertices = model->getVerticesPtr();
   if (!vertices)
     return;

   glDisable(GL_CULL_FACE);
   glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer(3, GL_FLOAT, sizeof(btVector3), model->getSkinnedVerticesPtr());
   glDrawElements(GL_TRIANGLES, model->getNumSurface(), GL_UNSIGNED_SHORT, model->getSurfacesPtr());
   glDisableClientState(GL_VERTEX_ARRAY);
   glEnable(GL_CULL_FACE);
}

void GLSceneRenderEngine::bindTexture(const unsigned char *data,
                                    const int width,
                                    const int height,
                                    const int components,
                                    PMDTextureNative **ptr)
{
   /* generate texture */
   GLuint format = 0;
   PMDTextureNative *native = new PMDTextureNative;
   glGenTextures(1, &native->id);
   glBindTexture(GL_TEXTURE_2D, native->id);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   if (components == 3) {
      format = GL_RGB;
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   } else {
      format = GL_RGBA;
      glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
   }
   glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

   /* set highest priority to this texture to tell OpenGL to keep textures in GPU memory */
   GLfloat priority = 1.0f;
   glPrioritizeTextures(1, &native->id, &priority);
   *ptr = native;
}

void GLSceneRenderEngine::deleteTexture(PMDTextureNative **ptr)
{
  PMDTextureNative *native = *ptr;
  if (native != NULL) {
    glDeleteTextures(1, &native->id);
    delete native;
    *ptr = 0;
  }
}

} /* namespace */

