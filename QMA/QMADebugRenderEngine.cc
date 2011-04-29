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

#include "QMADebugRenderEngine.h"

#include <MMDAI/MMDAI.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btShapeHull.h>

QMADebugRenderEngine::QMADebugRenderEngine(MMDAI::SceneController *controller)
    : m_controller(controller),
      m_world(0),
      m_boxList(0),
      m_sphereList(0),
      m_debugMode(btIDebugDraw::DBG_NoDebug),
      m_boxListEnabled(false),
      m_sphereListEnabled(false),
      m_renderBones(false),
      m_renderRigidBodies(false)
{
}

QMADebugRenderEngine::~QMADebugRenderEngine()
{
    m_world = 0;
    m_controller = 0;
    if (m_sphereListEnabled) {
        glDeleteLists(m_sphereList, 1);
        m_sphereList = 0;
        m_sphereListEnabled = false;
    }
    if (m_boxListEnabled) {
        glDeleteLists(m_boxList, 1);
        m_boxList = 0;
        m_boxListEnabled = false;
    }
}

void QMADebugRenderEngine::initialize()
{
    m_world = m_controller->getPhysicalEngine()->getWorld();
    m_world->setDebugDrawer(this);
}

void QMADebugRenderEngine::render()
{
    if (m_renderRigidBodies)
        renderRigidBodies();
    if (m_renderBones)
        renderModelBones();
    m_world->debugDrawWorld();
}

void QMADebugRenderEngine::drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color)
{
    glColor3fv(color);
    glBegin(GL_LINE);
    glVertex3fv(from);
    glVertex3fv(to);
    glEnd();
}

void QMADebugRenderEngine::drawContactPoint(const btVector3 &PointOnB, const btVector3 &normalOnB, btScalar distance, int lifeTime, const btVector3 &color)
{
    Q_UNUSED(PointOnB);
    Q_UNUSED(normalOnB);
    Q_UNUSED(distance);
    Q_UNUSED(lifeTime);
    Q_UNUSED(color);
}

void QMADebugRenderEngine::reportErrorWarning(const char *warningString)
{
    MMDAILogWarnString(warningString);
}

void QMADebugRenderEngine::draw3dText(const btVector3 &location, const char *textString)
{
    Q_UNUSED(location);
    Q_UNUSED(textString);
}

void QMADebugRenderEngine::renderRigidBodies()
{
    GLfloat color[] = {0.8f, 0.8f, 0.0f, 1.0f};
    GLint polygonMode[2] = { 0, 0 };
    btRigidBody* body = NULL;
    btScalar m[16];
    btCollisionShape* shape = NULL;
    btVector3 halfExtent;
    btDiscreteDynamicsWorld *world = m_controller->getPhysicalEngine()->getWorld();
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

void QMADebugRenderEngine::renderModelBones()
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    /* draw bones */
    int nmodels = m_controller->getMaxObjects();
    for (int i = 0; i < nmodels; i++) {
        MMDAI::PMDObject *object = m_controller->getObjectAt(i);
        if (object && object->isEnable()) {
            MMDAI::PMDModel *model = object->getModel();
            const int nbones = model->countBones();
            for (int j = 0; j < nbones; j++) {
                renderModelBone(model->getBoneAt(j));
            }
        }
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void QMADebugRenderEngine::drawCube()
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

void QMADebugRenderEngine::drawSphere(int lats, int longs)
{
    for (int i = 0; i <= lats; i++) {
        const float lat0 = M_PI * (-0.5 + static_cast<float>(i - 1) / lats);
        const float z0 = sin(lat0);
        const float zr0 = cos(lat0);
        const float lat1 = M_PI * (-0.5 + static_cast<float>(i) / lats);
        const float z1 = sin(lat1);
        const float zr1 = cos(lat1);

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= longs; j++) {
            const float lng = 2 * M_PI * static_cast<float>(j - 1) / longs;
            const float x = cos(lng);
            const float y = sin(lng);

            glNormal3f(x * zr0, y * zr0, z0);
            glVertex3f(x * zr0, y * zr0, z0);
            glNormal3f(x * zr1, y * zr1, z1);
            glVertex3f(x * zr1, y * zr1, z1);
        }
        glEnd();
    }
}

void QMADebugRenderEngine::drawConvex(btConvexShape *shape)
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

            //glNormal3f(normal.getX(), normal.getY(), normal.getZ());
            glNormal3fv(normal);
            glVertex3fv(v1);
            glVertex3fv(v2);
            glVertex3fv(v3);
        }
        glEnd ();
    }

    delete hull;
}

void QMADebugRenderEngine::renderModelBone(MMDAI::PMDBone *bone)
{
    btScalar m[16];
    MMDAI::PMDBone *parentBone = bone->getParentBone();
    const btTransform trans = bone->getTransform();
    const unsigned char type = bone->getType();
    const bool isSimulated = bone->isSimulated();

    /* do not draw IK target bones if the IK chain is under simulation */
    if (type == MMDAI::IK_TARGET && parentBone && parentBone->isSimulated())
        return;

    trans.getOpenGLMatrix(m);

    /* draw node */
    glPushMatrix();
    glMultMatrixf(m);
    if (type != MMDAI::NO_DISP) { /* do not draw invisible bone nodes */
        if (isSimulated) {
            /* under physics simulation */
            glColor4f(0.8f, 0.8f, 0.0f, 1.0f);
            glScalef(0.1, 0.1, 0.1);
        }
        else {
            switch (type) {
            case MMDAI::IK_DESTINATION:
                glColor4f(0.7f, 0.2f, 0.2f, 1.0f);
                glScalef(0.25, 0.25, 0.25);
                break;
            case MMDAI::UNDER_IK:
                glColor4f(0.8f, 0.5f, 0.0f, 1.0f);
                glScalef(0.15, 0.15, 0.15);
                break;
            case MMDAI::IK_TARGET:
                glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
                glScalef(0.15, 0.15, 0.15);
                break;
            case MMDAI::UNDER_ROTATE:
            case MMDAI::TWIST:
            case MMDAI::FOLLOW_ROTATE:
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

    if (!parentBone || type == MMDAI::IK_DESTINATION)
        return;

    /* draw line from parent */
    glPushMatrix();
    if (type == MMDAI::NO_DISP) {
        glColor4f(0.5f, 0.4f, 0.5f, 1.0f);
    }
    else if (isSimulated) {
        glColor4f(0.7f, 0.7f, 0.0f, 1.0f);
    }
    else if (type == MMDAI::UNDER_IK || type == MMDAI::IK_TARGET) {
        glColor4f(0.8f, 0.5f, 0.3f, 1.0f);
    }
    else {
        glColor4f(0.5f, 0.6f, 1.0f, 1.0f);
    }

    const btVector3 a = parentBone->getTransform().getOrigin();
    const btVector3 b = trans.getOrigin();
    glBegin(GL_LINES);
    glVertex3fv(a);
    glVertex3fv(b);
    glEnd();
    glPopMatrix();
}
