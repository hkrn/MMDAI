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

#if defined(OPENGLES1)
#include "MMDAI/MMDAI.h"
#include "MMDAI/GLES1SceneRenderEngine.h"

#define SHADOW_PCF                   /* use hardware PCF for shadow mapping */
#define SHADOW_AUTO_VIEW             /* automatically define depth frustum */
#define SHADOW_AUTO_VIEW_ANGLE 15.0f /* view angle for automatic depth frustum */

namespace MMDAI {

GLES1SceneRenderEngine::GLES1SceneRenderEngine()
  : m_lightVec(btVector3(0.0f, 0.0f, 0.0f)),
    m_shadowMapAutoViewEyePoint(btVector3(0.0f, 0.0f, 0.0f)),
    m_shadowMapAutoViewRadius(0.0f),
    m_boxList(0),
    m_sphereList(0),
    m_depthTextureID(0),
    m_fboID(0),
    m_boxListEnabled(false),
    m_sphereListEnabled(false),
    m_enableShadowMapping(false),
    m_overrideModelViewMatrix(false),
    m_overrideProjectionMatrix(false),
    m_shadowMapInitialized(false)
{
}

GLES1SceneRenderEngine::~GLES1SceneRenderEngine()
{
}

void GLES1SceneRenderEngine::drawCube()
{
#if 0
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
#endif
}

void GLES1SceneRenderEngine::drawSphere(int lats, int longs)
{
#if 0
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
#endif
}

void GLES1SceneRenderEngine::drawConvex(btConvexShape *shape)
{
#if 0
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
#endif
}

void GLES1SceneRenderEngine::renderRigidBodies(BulletPhysics *bullet)
{
#if 0
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
            glScalef(2 * halfExtent[0], 2 * halfExtent[1], 2 * halfExtent[2]);
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
            glScalef(radius, radius, radius);
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
#endif
}

void GLES1SceneRenderEngine::renderBone(PMDBone *bone)
{
#if 0
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
         glScalef(0.1, 0.1, 0.1);
      }
      else {
         switch (type) {
         case IK_DESTINATION:
            glColor4f(0.7f, 0.2f, 0.2f, 1.0f);
            glScalef(0.25, 0.25, 0.25);
            break;
         case UNDER_IK:
            glColor4f(0.8f, 0.5f, 0.0f, 1.0f);
            glScalef(0.15, 0.15, 0.15);
            break;
         case IK_TARGET:
            glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
            glScalef(0.15, 0.15, 0.15);
            break;
         case UNDER_ROTATE:
         case TWIST:
         case FOLLOW_ROTATE:
            glColor4f(0.0f, 0.8f, 0.2f, 1.0f);
            glScalef(0.15, 0.15, 0.15);
            break;
         default:
            if (bone->hasMotionIndependency()) {
               glColor4f(0.0f, 1.0f, 1.0f, 1.0f);
               glScalef(0.25, 0.25, 0.25);
            } else {
               glColor4f(0.0f, 0.5f, 1.0f, 1.0f);
               glScalef(0.15, 0.15, 0.15);
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
#endif
}

void GLES1SceneRenderEngine::renderBones(PMDModel *model)
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
void GLES1SceneRenderEngine::renderModel(PMDModel *model)
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
   glActiveTexture(GL_TEXTURE0);
   glClientActiveTexture(GL_TEXTURE0);

   /* set lists */
   glEnableClientState(GL_VERTEX_ARRAY);
   glEnableClientState(GL_NORMAL_ARRAY);
   glVertexPointer(3, GL_FLOAT, sizeof(btVector3), model->getSkinnedVerticesPtr());
   glNormalPointer(GL_FLOAT, sizeof(btVector3), model->getSkinnedNormalsPtr());

   /* set model texture coordinates to texture unit 0 */
   glClientActiveTexture(GL_TEXTURE0);
   glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   glTexCoordPointer(2, GL_FLOAT, 0, model->getTexCoordsPtr());

   const bool enableToon = model->getToonFlag();
   const bool hasSingleSphereMap = model->hasSingleSphereMap();
   const bool hasMultipleSphereMap = model->hasMultipleSphereMap();

   if (enableToon) {
      /* set toon texture coordinates to texture unit 1 */
      glActiveTexture(GL_TEXTURE1);
      glEnable(GL_TEXTURE_2D);
      glClientActiveTexture(GL_TEXTURE1);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      if (model->isSelfShadowEnabled()) {
         /* when drawing a shadow part in shadow mapping, force toon texture coordinates to (0, 0) */
         glTexCoordPointer(2, GL_FLOAT, 0, model->getToonTexCoordsForSelfShadowPtr());
      }
      else {
         glTexCoordPointer(2, GL_FLOAT, 0, model->getToonTexCoordsPtr());
      }
      glActiveTexture(GL_TEXTURE0);
      glClientActiveTexture(GL_TEXTURE0);
   }

   if (hasSingleSphereMap) {
      /* this model contains single sphere map texture */
      /* set texture coordinate generation for sphere map on texture unit 0 */
      glEnable(GL_TEXTURE_2D);
      //glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
      //glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
      glDisable(GL_TEXTURE_2D);
   }

   if (hasMultipleSphereMap) {
      /* this model contains additional sphere map texture */
      /* set texture coordinate generation for sphere map on texture unit 2 */
      glActiveTexture(GL_TEXTURE2);
      glEnable(GL_TEXTURE_2D);
      //glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
      //glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
      glDisable(GL_TEXTURE_2D);
      glActiveTexture(GL_TEXTURE0);
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
         glActiveTexture(GL_TEXTURE0);
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
                  //glEnable(GL_TEXTURE_GEN_S);
                  //glEnable(GL_TEXTURE_GEN_T);
               }
               else {
                  /* disable generation */
                  //glDisable(GL_TEXTURE_GEN_S);
                  //glDisable(GL_TEXTURE_GEN_T);
               }
            }
         }
         else {
           glEnable(GL_TEXTURE_2D);
           glBindTexture(GL_TEXTURE_2D, 0);
         }
      }
      else {
         glDisable(GL_TEXTURE_2D);
      }

      if (enableToon) {
         /* set toon texture for texture unit 1 */
         const PMDTextureNative *native = model->getToonTextureAt(m->getToonID())->getNative();
         if (native != NULL) {
            glActiveTexture(GL_TEXTURE1);
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
            glActiveTexture(GL_TEXTURE2);
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
            }
            else {
               glBindTexture(GL_TEXTURE_2D, 0);
            }
            //glEnable(GL_TEXTURE_GEN_S);
            //glEnable(GL_TEXTURE_GEN_T);
         }
         else {
            /* disable generation */
            glActiveTexture(GL_TEXTURE2);
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
            glActiveTexture(GL_TEXTURE0);
         glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      }
   }

   glDisableClientState(GL_VERTEX_ARRAY);
   glDisableClientState(GL_NORMAL_ARRAY);
   if (enableToon) {
      glClientActiveTexture(GL_TEXTURE0);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      if (hasSingleSphereMap) {
         glActiveTexture(GL_TEXTURE0);
         //glDisable(GL_TEXTURE_GEN_S);
         //glDisable(GL_TEXTURE_GEN_T);
      }
      glClientActiveTexture(GL_TEXTURE1);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      if (hasMultipleSphereMap) {
         glActiveTexture(GL_TEXTURE2);
         //glDisable(GL_TEXTURE_GEN_S);
         //glDisable(GL_TEXTURE_GEN_T);
      }
      glActiveTexture(GL_TEXTURE0);
   } else {
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      if (hasSingleSphereMap) {
         //glDisable(GL_TEXTURE_GEN_S);
         //glDisable(GL_TEXTURE_GEN_T);
      }
      if (hasMultipleSphereMap) {
         glActiveTexture(GL_TEXTURE2);
         //glDisable(GL_TEXTURE_GEN_S);
         //glDisable(GL_TEXTURE_GEN_T);
         glActiveTexture(GL_TEXTURE0);
      }
   }

   if (hasSingleSphereMap || hasMultipleSphereMap) {
      //glDisable(GL_TEXTURE_GEN_S);
      //glDisable(GL_TEXTURE_GEN_T);
   }
   if (enableToon) {
      glActiveTexture(GL_TEXTURE1);
      glDisable(GL_TEXTURE_2D);
   }
   if (hasMultipleSphereMap) {
      glActiveTexture(GL_TEXTURE2);
      glDisable(GL_TEXTURE_2D);
   }
   glActiveTexture(GL_TEXTURE0);

   glDisable(GL_TEXTURE_2D);
   glEnable(GL_CULL_FACE);
#ifndef MMDFILES_CONVERTCOORDINATESYSTEM
   glCullFace(GL_BACK);
   glPopMatrix();
#endif
}

void GLES1SceneRenderEngine::renderEdge(PMDModel *model)
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

void GLES1SceneRenderEngine::renderShadow(PMDModel *model)
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

PMDTextureNative *GLES1SceneRenderEngine::allocateTexture(const unsigned char *data,
                                                       const int width,
                                                       const int height,
                                                       const int components)
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
   return native;
}

void GLES1SceneRenderEngine::releaseTexture(PMDTextureNative *native)
{
  if (native != NULL) {
    glDeleteTextures(1, &native->id);
    delete native;
  }
}

void GLES1SceneRenderEngine::renderModelCached(PMDModel *model, PMDRenderCacheNative **ptr)
{
  *ptr = NULL;
}

void GLES1SceneRenderEngine::renderTileTexture(PMDTexture *texture,
                                            const float *color,
                                            const float *normal,
                                            const float *vertices1,
                                            const float *vertices2,
                                            const float *vertices3,
                                            const float *vertices4,
                                            const float nX,
                                            const float nY,
                                            const bool cullFace,
                                            PMDRenderCacheNative **ptr)
{
  *ptr = NULL;

  /* register rendering command */
  if (!cullFace)
    glDisable(GL_CULL_FACE);

  glEnable(GL_TEXTURE_2D);
  glPushMatrix();
#if 0
  glNormal3f(normal[0], normal[1], normal[2]);
  glBindTexture(GL_TEXTURE_2D, texture->getNative()->id);
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, nY);
  glVertex3fv(vertices1);
  glTexCoord2f(nX, nY);
  glVertex3fv(vertices2);
  glTexCoord2f(nX, 0.0);
  glVertex3fv(vertices3);
  glTexCoord2f(0.0, 0.0);
  glVertex3fv(vertices4);
  glEnd();
#endif
  glPopMatrix();
  glDisable(GL_TEXTURE_2D);

  if (!cullFace)
    glEnable(GL_CULL_FACE);
}

void GLES1SceneRenderEngine::deleteCache(PMDRenderCacheNative **ptr)
{
  PMDRenderCacheNative *native = *ptr;
  if (native != NULL) {
    delete native;
    *ptr = 0;
  }
}

/* setup: initialize and setup Renderer */
bool GLES1SceneRenderEngine::setup(float *campusColor,
                                bool useShadowMapping,
                                int shadowMapTextureSize,
                                bool shadowMapLightFirst)
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
  setShadowMapping(useShadowMapping, shadowMapTextureSize, shadowMapLightFirst);

  return true;
}

/* GLES1SceneRenderEngine::initializeShadowMap: initialize OpenGL for shadow mapping */
void GLES1SceneRenderEngine::initializeShadowMap(int shadowMapTextureSize)
{
  (void)shadowMapTextureSize;
#if 0
  static const GLfloat genfunc[][4] = {
    { 1.0, 0.0, 0.0, 0.0 },
    { 0.0, 1.0, 0.0, 0.0 },
    { 0.0, 0.0, 1.0, 0.0 },
    { 0.0, 0.0, 0.0, 1.0 },
  };

  /* initialize model view matrix */
  glPushMatrix();
  glLoadIdentity();

  /* use 4th texture unit for depth texture, make it current */
  glActiveTexture(GL_TEXTURE3);

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
#endif
}

/* GLES1SceneRenderEngine::setShadowMapping: switch shadow mapping */
void GLES1SceneRenderEngine::setShadowMapping(bool flag, int shadowMapTextureSize, bool shadowMapLightFirst)
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
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_depthTextureID);
    if (shadowMapLightFirst) {
      /* when Renderering order is light(full) - dark(shadow part), OpenGL should set the shadow part as true */
      //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_GEQUAL);
    } else {
      /* when Renderering order is dark(full) - light(non-shadow part), OpenGL should set the shadow part as false */
      //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    }
    glDisable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    MMDAILogInfoString("Shadow mapping enabled");
  } else {
    /* disabled */
    if (m_shadowMapInitialized) {
      /* disable depth texture unit */
      glActiveTexture(GL_TEXTURE3);
      glDisable(GL_TEXTURE_2D);
      glActiveTexture(GL_TEXTURE0);
    }
    MMDAILogInfoString("Shadow mapping disabled");
  }
}

/* GLES1SceneRenderEngine::RendererSceneShadowMap: shadow mapping */
void GLES1SceneRenderEngine::renderSceneShadowMap(Option *option, Stage *stage, PMDObject **objects, int size)
{
  int i = 0;
  static GLfloat lightdim[] = { 0.2f, 0.2f, 0.2f, 1.0f };
  static const GLfloat lightblk[] = { 0.0f, 0.0f, 0.0f, 1.0f };

  /* Renderer the full scene */
  /* set model view matrix, as the same as normal Renderering */
  glMatrixMode(GL_MODELVIEW);
  applyModelViewMatrix();

  /* Renderer the whole scene */
  if (option->getShadowMappingLightFirst()) {
    /* Renderer light setting, later Renderer only the shadow part with dark setting */
    stage->renderBackground();
    stage->renderFloor();
    for (i = 0; i < size; i++) {
      PMDObject *object = objects[i];
      if (!object->isEnable())
        continue;
      PMDModel *model = object->getPMDModel();
      renderModel(model);
      renderEdge(model);
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
      PMDObject *object = objects[i];
      if (!object->isEnable())
        continue;
      PMDModel *model = object->getPMDModel();
      if (model->getToonFlag() == true)
        continue;
      renderModel(model);
    }

    /* for toon objects, they should apply the model-defined toon texture color at texture coordinates (0, 0) for shadow Renderering */
    /* so restore the light setting */
    if (option->getUseCartoonRendering())
      updateLighting(true,
                     option->getUseMMDLikeCartoon(),
                     option->getLightDirection(),
                     option->getLightIntensity(),
                     option->getLightColor());
    /* Renderer the toon objects */
    for (i = 0; i < size; i++) {
      PMDObject *object = objects[i];
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
      renderModel(model);
      renderEdge(model);
      /* disable shadow Renderering */
      model->setSelfShadowDrawing(false);
    }
    if (!option->getUseCartoonRendering())
      updateLighting(false, option->getUseMMDLikeCartoon(), option->getLightDirection(), option->getLightIntensity(), option->getLightColor());
  }

  /* Renderer the part clipped by the depth texture */
  /* activate the texture unit for shadow mapping and make it current */
  glActiveTexture(GL_TEXTURE3);

  /* set texture matrix (note: matrices should be set in reverse order) */
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  /* move the range from [-1,1] to [0,1] */
  glTranslatef(0.5, 0.5, 0.5);
  glScalef(0.5, 0.5, 0.5);
  /* multiply the model view matrix when the depth texture was Renderered */
  glMultMatrixf(m_modelView);
  /* multiply the inverse matrix of current model view matrix */
  glMultMatrixf(m_rotMatrixInv);

  /* revert to model view matrix mode */
  glMatrixMode(GL_MODELVIEW);

  /* enable texture mapping with texture coordinate generation */
  glEnable(GL_TEXTURE_2D);
  //glEnable(GL_TEXTURE_GEN_S);
  //glEnable(GL_TEXTURE_GEN_T);
  //glEnable(GL_TEXTURE_GEN_R);
  //glEnable(GL_TEXTURE_GEN_Q);

  /* bind the depth texture Renderered at the first step */
  glBindTexture(GL_TEXTURE_2D, m_depthTextureID);

  /* depth texture set up was done, now switch current texture unit to default */
  glActiveTexture(GL_TEXTURE0);

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
      PMDObject *object = objects[i];
      if (!object->isEnable())
        continue;
      PMDModel *model = object->getPMDModel();
      if (!model->getToonFlag())
        continue;
      renderModel(model);
    }

    /* for toon objects, they should apply the model-defined toon texture color at texture coordinates (0, 0) for shadow Renderering */
    /* so restore the light setting */
    if (option->getUseCartoonRendering())
      updateLighting(true,
                     option->getUseMMDLikeCartoon(),
                     option->getLightDirection(),
                     option->getLightIntensity(),
                     option->getLightColor());
    /* Renderer the toon objects */
    for (i = 0; i < size; i++) {
      PMDObject *object = objects[i];
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
      renderModel(model);
      /* disable shadow Renderering */
      model->setSelfShadowDrawing(false);
    }
    if (!option->getUseCartoonRendering())
      updateLighting(false,
                     option->getUseMMDLikeCartoon(),
                     option->getLightDirection(),
                     option->getLightIntensity(),
                     option->getLightColor());
  } else {
    /* the area clipped by depth texture by alpha test is light part */
    glAlphaFunc(GL_GEQUAL, 0.001f);
    stage->renderBackground();
    stage->renderFloor();
    for (i = 0; i < size; i++) {
      PMDObject *object = objects[i];
      if (!object->isEnable())
        continue;
      renderModel(object->getPMDModel());
    }
  }

  /* reset settings */
  glDepthFunc(GL_LESS);
  glAlphaFunc(GL_GEQUAL, 0.05f);

  glActiveTexture(GL_TEXTURE3);
  //glDisable(GL_TEXTURE_GEN_S);
  //glDisable(GL_TEXTURE_GEN_T);
  //glDisable(GL_TEXTURE_GEN_R);
  //glDisable(GL_TEXTURE_GEN_Q);
  //glDisable(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE0);
}

/* GLES1SceneRenderEngine::RendererScene: Renderer scene */
void GLES1SceneRenderEngine::renderScene(Option *option,
                                      Stage *stage,
                                      PMDObject **objects,
                                      int size)
{
  int i = 0;

  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);

  /* set model viwe matrix */
  applyModelViewMatrix();

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
    PMDObject *object = objects[i];
    if (!object->isEnable())
      continue;
    glPushMatrix();
    glMultMatrixf(stage->getShadowMatrix());
    renderShadow(object->getPMDModel());
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
    PMDObject *object = objects[i];
    if (!object->isEnable())
      continue;
    PMDModel *model = object->getPMDModel();
    renderModel(model);
    renderEdge(model);
  }
}

void GLES1SceneRenderEngine::prerender(Option *option,
                                    PMDObject **objects,
                                    int size)
{
  if (m_enableShadowMapping) {
#if 0
    int i = 0;
    GLint viewport[4]; /* store viewport */
    GLfloat projection[16]; /* store projection transform */

#ifdef SHADOW_AUTO_VIEW
    float fovy = 0.0, eyeDist = 0.0;
    btVector3 v;
#endif

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
    glGetDoublev(GL_MODELVIEW_MATRIX, m_modelView);

    /* do not write into frame buffer other than depth information */
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    /* also, lighting is not needed */
    glDisable(GL_LIGHTING);

    /* disable Renderering the front surface to get the depth of back face */
    glCullFace(GL_FRONT);

    /* disable alpha test */
    glDisable(GL_ALPHA_TEST);

    /* we are now writing to depth texture using FBO, so disable the depth texture mapping here */
    glActiveTexture(GL_TEXTURE3);
    glDisable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);

    /* set polygon offset to avoid "moire" */
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(4.0, 4.0);

    /* Renderer objects for depth */
    /* only objects that wants to drop shadow should be Renderered here */
    for (i = 0; i < size; i++) {
      PMDObject *object = objects[i];
      if (!object->isEnable())
        continue;
      glPushMatrix();
      renderShadow(object->getPMDModel());
      glPopMatrix();
    }

    /* reset the polygon offset */
    glDisable(GL_POLYGON_OFFSET_FILL);

    /* switch to default FBO */
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    /* revert configurations to normal Renderering */
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(projection);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glEnable(GL_LIGHTING);
    glCullFace(GL_BACK);
    glEnable(GL_ALPHA_TEST);
#endif
    /* clear all the buffers */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  }
  else {
    /* clear Renderering buffer */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  }
}

/* GLES1SceneRenderEngine::render: Render all */
void GLES1SceneRenderEngine::render(Option *option,
                                 Stage *stage,
                                 PMDObject **objects,
                                 int size)
{
  if (m_enableShadowMapping)
    renderSceneShadowMap(option, stage, objects, size);
  else
    renderScene(option, stage, objects, size);
}

/* GLES1SceneRenderEngine::pickModel: pick up a model at the screen position */
int GLES1SceneRenderEngine::pickModel(PMDObject **objects,
                                   int size,
                                   int x,
                                   int y,
                                   int width,
                                   int height,
                                   double scale,
                                   int *allowDropPicked)
{
#if 0
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
  applyProjectionMatrix(width, height, scale);
  /* switch to model view mode */
  glMatrixMode(GL_MODELVIEW);
  /* initialize name buffer */
  glInitNames();
  glPushName(0);

  /* draw models with selection names */
  for (i = 0; i < size; i++) {
    PMDObject *object = objects[i];
    if (!object->isEnable())
      continue;
    glLoadName(i);
    renderShadow(object->getPMDModel());
  }

  /* restore projection matrix */
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  /* switch to model view mode */
  glMatrixMode(GL_MODELVIEW);
  /* end selection mode and get number of hits */
  hits = glRenderMode(GL_RENDER);
  if (hits == 0)
    return -1;

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
    if (allowDropPicked && objects[id]->allowMotionFileDrop()) {
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
#else
  return 0;
#endif
}

/* GLES1SceneRenderEngine::updateLigithing: update light */
void GLES1SceneRenderEngine::updateLighting(bool useCartoonRendering,
                                         bool useMMDLikeCartoon,
                                         float *lightDirection,
                                         float lightIntensy,
                                         float *lightColor)
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

/* GLES1SceneRenderEngine::updateProjectionMatrix: update view information */
void GLES1SceneRenderEngine::updateProjectionMatrix(const int width,
                                                 const int height,
                                                 const double scale)
{
  glViewport(0, 0, width, height);
  /* camera setting */
  glMatrixMode(GL_PROJECTION);
  applyProjectionMatrix(width, height, scale);
  glMatrixMode(GL_MODELVIEW);
}

static void MMDAIGLFrustum(float *result, float left, float right, float bottom, float top, float near, float far)
{
  const float a = (right + left) / (right - left);
  const float b = (top + bottom) / (top - bottom);
  const float c = ((far + near) / (far - near)) * -1;
  const float d = ((-2 * far * near) / (far - near));
  const float e = (2 * near) / (right - left);
  const float f = (2 * near) / (top - bottom);
  const float matrix[16] = {
    e, 0, 0, 0,
    0, f, 0, 0,
    a, b, c, -1,
    0, 0, d, 0
  };
  memcpy(result, matrix, sizeof(matrix));
}

/* GLES1SceneRenderEngine::applyProjectionMatirx: update projection matrix */
void GLES1SceneRenderEngine::applyProjectionMatrix(const int width,
                                                const int height,
                                                const double scale)
{
  if (width == 0 || height == 0)
    return;
  if (m_overrideProjectionMatrix) {
    glLoadMatrixf(m_newProjectionMatrix);
    m_overrideProjectionMatrix = false;
  }
  else {
    const float aspect = (float) height / (float) width;
    const float ratio = (scale == 0.0) ? 1.0f : 1.0f / scale;
    float result[16];
    glLoadIdentity();
    MMDAIGLFrustum(result, - ratio, ratio, - aspect * ratio, aspect * ratio, RENDER_VIEWPOINT_FRUSTUM_NEAR, RENDER_VIEWPOINT_FRUSTUM_FAR);
    glMultMatrixf(result);
  }
}

void GLES1SceneRenderEngine::applyModelViewMatrix()
{
  glLoadIdentity();
  if (m_overrideModelViewMatrix) {
    glLoadMatrixf(m_newModelViewMatrix);
    m_overrideModelViewMatrix = false;
  }
  else {
    glMultMatrixf(m_rotMatrix);
  }
}

void GLES1SceneRenderEngine::updateModelViewMatrix(const btTransform &transMatrix,
                                                const btTransform &transMatrixInv)
{
  transMatrix.getOpenGLMatrix(m_rotMatrix);
  transMatrixInv.getOpenGLMatrix(m_rotMatrixInv);
}

void GLES1SceneRenderEngine::setModelViewMatrix(const btScalar modelView[16])
{
  m_overrideModelViewMatrix = true;
  memcpy(m_newModelViewMatrix, modelView, sizeof(m_newModelViewMatrix));
}

void GLES1SceneRenderEngine::setProjectionMatrix(const btScalar projection[16])
{
  m_overrideProjectionMatrix = true;
  memcpy(m_newProjectionMatrix, projection, sizeof(m_newProjectionMatrix));
}

void GLES1SceneRenderEngine::setShadowMapAutoView(const btVector3 &eyePoint,
                                               const float radius)
{
  m_shadowMapAutoViewEyePoint = btVector3(eyePoint);
  m_shadowMapAutoViewRadius = radius;
}

} /* namespace */

#endif

