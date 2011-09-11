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

#include "TiledStage.h"

#include "Delegate.h"
#include "World.h"
#include <btBulletDynamicsCommon.h>
#include <QtCore/QtCore>

class TiledStageInternal {
public:
    TiledStageInternal(Delegate *delegate, const QVector3D &normal)
        : m_delegate(delegate),
          m_listID(0),
          m_textureID(0)
    {
        const float normals[] = {
            normal.x(), normal.y(), normal.z(),
            normal.x(), normal.y(), normal.z(),
            normal.x(), normal.y(), normal.z(),
            normal.x(), normal.y(), normal.z()
        };
        const float colors[] = {
            0.1f, 0.1f, 0.1f, 0.6f,
            0.1f, 0.1f, 0.1f, 0.6f,
            0.1f, 0.1f, 0.1f, 0.6f,
            0.1f, 0.1f, 0.1f, 0.6f
        };
        const float coords[] = {
            0.0f, 1.0f,
            1.0f, 1.0f,
            1.0f, 0.0f,
            0.0f, 0.0f
        };
        const uint16_t indices[] = { 3, 2, 0, 0, 2, 1 };
        memcpy(m_normals, normals, sizeof(normals));
        memcpy(m_colors, colors, sizeof(colors));
        memcpy(m_texcoords, coords, sizeof(coords));
        memcpy(m_indices, indices, sizeof(indices));
        glGenBuffers(sizeof(m_buffers) / sizeof(GLuint), m_buffers);
        glBindBuffer(GL_ARRAY_BUFFER, m_buffers[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(m_normals), m_normals, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, m_buffers[2]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(m_colors), m_colors, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, m_buffers[3]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(m_texcoords), m_texcoords, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[4]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_indices), m_indices, GL_STATIC_DRAW);
        m_matrix.setToIdentity();
    }
    ~TiledStageInternal() {
        glDeleteBuffers(sizeof(m_buffers) / sizeof(GLuint), m_buffers);
        deleteList();
        if (m_textureID)
            glDeleteTextures(1, &m_textureID);
        m_delegate = 0;
        m_listID = 0;
        m_textureID = 0;
    }
    void load(const QString &path) {
        m_delegate->loadTexture(path.toStdString(), m_textureID);
    }
    void render(bool cullface, bool hasColor) {
        const float color[] = { 0.65f, 0.65f, 0.65f, 1.0f };
        if (m_listID) {
            glCallList(m_listID);
        }
        else {
            m_listID = glGenLists(1);
            glNewList(m_listID, GL_COMPILE_AND_EXECUTE);
            if (!cullface)
                glDisable(GL_CULL_FACE);
            glPushMatrix();
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, m_textureID);
#if 1
            glActiveTexture(GL_TEXTURE0);
            glClientActiveTexture(GL_TEXTURE0);
            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_NORMAL_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glBindBuffer(GL_ARRAY_BUFFER, m_buffers[0]);
            glVertexPointer(3, GL_DOUBLE, 0, 0);
            glBindBuffer(GL_ARRAY_BUFFER, m_buffers[1]);
            glNormalPointer(GL_FLOAT, 0, 0);
            if (hasColor) {
                glBindBuffer(GL_ARRAY_BUFFER, m_buffers[2]);
                glColorPointer(4, GL_FLOAT, 0, 0);
            }
            glBindBuffer(GL_ARRAY_BUFFER, m_buffers[3]);
            glTexCoordPointer(2, GL_FLOAT, 0, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[4]);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_NORMAL_ARRAY);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#else
            glNormal3f(normal.x(), normal.y(), normal.z());
            glColor4(0.1f, 0.1f, 0.1f, 0.6f);
            glBegin(GL_QUADS);
            glTexCoord2f(0.0f, m_y);
            glVertex3d(m_matrix(0, 0), m_matrix(0, 1), m_matrix(0, 2));
            glTexCoord2f(m_x, m_y);
            glVertex3d(m_matrix(1, 0), m_matrix(1, 1), m_matrix(1, 2));
            glTexCoord2f(m_x, 0.0f);
            glVertex3d(m_matrix(2, 0), m_matrix(2, 1), m_matrix(2, 2));
            glTexCoord2f(0.0f, 0.0f);
            glVertex3d(m_matrix(3, 0), m_matrix(3, 1), m_matrix(3, 2));
            glEnd();
#endif
            glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_TEXTURE_2D);
            glPopMatrix();
            if (!cullface)
                glEnable(GL_CULL_FACE);
            glEndList();
        }
    }
    const QMatrix3x4 &vertices() const {
        return m_matrix;
    }
    void setVertices(const QMatrix3x4 &vertices) {
        vertices.copyDataTo(m_vertices);
        glBindBuffer(GL_ARRAY_BUFFER, m_buffers[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices), m_vertices, GL_STATIC_DRAW);
        m_matrix = vertices;
    }

private:
    void deleteList() {
        if (m_listID) {
            glDeleteLists(m_listID, 1);
            m_listID = 0;
        }
    }

    Delegate *m_delegate;
    QMatrix3x4 m_matrix;
    qreal m_vertices[12];
    float m_normals[12];
    float m_colors[12];
    float m_texcoords[8];
    uint16_t m_indices[6];
    GLuint m_listID;
    GLuint m_buffers[5];
    GLuint m_textureID;
};

namespace
{

void FindPlane(TiledStageInternal *internal, QVector4D &plane)
{
    const QMatrix3x4 &v = internal->vertices();
    float v0x = v(2, 0) - v(1, 0);
    float v0y = v(2, 1) - v(1, 1);
    float v0z = v(2, 2) - v(1, 2);
    float v1x = v(3, 0) - v(1, 0);
    float v1y = v(3, 1) - v(1, 1);
    float v1z = v(3, 2) - v(1, 2);
    plane.setX(  v0y * v1z - v0z * v1y);
    plane.setY(-(v0x * v1z - v0z * v1x));
    plane.setZ(  v0x * v1y - v0y * v1x);
    plane.setW(-(plane.x() * v(1, 0) + plane.y() * v(1, 1) + plane.z() * v(1, 2)));
}

void ShadowMatrix(const QVector4D &plane, const QVector4D &position, QMatrix4x4 &matrix)
{
    qreal dot = QVector4D::dotProduct(plane, position);
    matrix(0, 0) =  dot - position.x() * plane.x();
    matrix(1, 0) = 0.0f - position.x() * plane.y();
    matrix(2, 0) = 0.0f - position.x() * plane.z();
    matrix(3, 0) = 0.0f - position.x() * plane.w();
    matrix(0, 1) = 0.0f - position.y() * plane.x();
    matrix(1, 1) =  dot - position.y() * plane.y();
    matrix(2, 1) = 0.0f - position.y() * plane.z();
    matrix(3, 1) = 0.0f - position.y() * plane.w();
    matrix(0, 2) = 0.0f - position.z() * plane.x();
    matrix(1, 2) = 0.0f - position.z() * plane.y();
    matrix(2, 2) =  dot - position.z() * plane.z();
    matrix(3, 2) = 0.0f - position.z() * plane.w();
    matrix(0, 3) = 0.0f - position.w() * plane.x();
    matrix(1, 3) = 0.0f - position.w() * plane.y();
    matrix(2, 3) = 0.0f - position.w() * plane.z();
    matrix(3, 3) =  dot - position.w() * plane.w();
}

}

TiledStage::TiledStage(Delegate *delegate, World *world)
    : m_floor(0),
      m_background(0),
      m_floorRigidBody(0),
      m_delegate(delegate),
      m_world(world)
{
    m_matrix.setToIdentity();
}

TiledStage::~TiledStage()
{
    destroyFloor();
    delete m_floor;
    m_floor = 0;
    delete m_background;
    m_background = 0;
}

void TiledStage::loadFloor(const QString &path)
{
    delete m_floor;
    m_floor = new TiledStageInternal(m_delegate, QVector3D(0.0f, 1.0f, 0.0f));
    m_floor->load(path);
    setSize(25.0f, 40.0f, 25.0f);
}

void TiledStage::loadBackground(const QString &path)
{
    delete m_background;
    m_background = new TiledStageInternal(m_delegate, QVector3D(0.0f, 0.0f, 1.0f));
    m_background->load(path);
    setSize(25.0f, 40.0f, 25.0f);
}

void TiledStage::renderFloor()
{
    if (m_floor)
        m_floor->render(false, true);
}

void TiledStage::renderBackground()
{
    if (m_background)
        m_background->render(false, false);
}

void TiledStage::setSize(float width, float height, float depth)
{
    QMatrix3x4 vertices;
    if (m_floor) {
        vertices(0, 0) = -width;
        vertices(0, 1) = 0.0f;
        vertices(0, 2) = depth;
        vertices(1, 0) = width;
        vertices(1, 1) = 0.0f;
        vertices(1, 2) = depth;
        vertices(2, 0) = width;
        vertices(2, 1) = 0.0f;
        vertices(2, 2) = -depth;
        vertices(3, 0) = -width;
        vertices(3, 1) = 0.0f;
        vertices(3, 2) = -depth;
        m_floor->setVertices(vertices);
        buildFloor(width, height);
    }
    if (m_background) {
        vertices(0, 0) = -width;
        vertices(0, 1) = 0.0f;
        vertices(0, 2) = -depth;
        vertices(1, 0) = width;
        vertices(1, 1) = 0.0f;
        vertices(1, 2) = -depth;
        vertices(2, 0) = width;
        vertices(2, 1) = height;
        vertices(2, 2) = -depth;
        vertices(3, 0) = -width;
        vertices(3, 1) = height;
        vertices(3, 2) = -depth;
        m_background->setVertices(vertices);
    }
}

void TiledStage::updateShadowMatrix(const btVector3 &position)
{
    if (m_floor) {
        QVector4D plane, direction;
        direction.setX(position.x());
        direction.setY(position.y());
        direction.setZ(position.z());
        direction.setW(0.0f);
        FindPlane(m_floor, plane);
        ShadowMatrix(plane, direction, m_matrix);
    }
}

const qreal *TiledStage::shadowMatrix() const
{
    return m_matrix.constData();
}

void TiledStage::buildFloor(float width, float height)
{
    btVector3 localIneteria(0.0f, 0.0f, 0.0f);
    btScalar mass = 0.0f;
    btCollisionShape *shape = 0;
    btTransform startTransform;
    btDefaultMotionState *state = 0;
    destroyFloor();
    shape = new btBoxShape(btVector3(width, 10.0f, height));
    if (mass != 0.0f)
        shape->calculateLocalInertia(mass, localIneteria);
    startTransform.setIdentity();
    startTransform.setOrigin(btVector3(0.0f, -9.99f, 0.0f));
    state = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo builder(mass, state, shape, localIneteria);
    builder.m_linearDamping = 0.5f;
    builder.m_angularDamping = 0.5f;
    builder.m_restitution = 0.8f;
    builder.m_friction = 0.5f;
    m_floorRigidBody = new btRigidBody(builder);
    m_world->mutableWorld()->addRigidBody(m_floorRigidBody);
}

void TiledStage::destroyFloor()
{
    if (m_floorRigidBody) {
        delete m_floorRigidBody->getMotionState();
        m_world->mutableWorld()->removeCollisionObject(m_floorRigidBody);
        delete m_floorRigidBody;
        m_floorRigidBody = 0;
    }
}
