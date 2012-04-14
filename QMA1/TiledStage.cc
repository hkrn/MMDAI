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

#include <QtOpenGL/QtOpenGL>

#include "World.h"
#include <btBulletDynamicsCommon.h>
#include <QtCore/QtCore>

class TiledStageInternal : protected QGLFunctions {
public:
    struct TileStageVertex {
        vpvl::Vector3 position;
        vpvl::Vector3 normal;
        vpvl::Vector3 texcoord;
        vpvl::Color color;
    };

    TiledStageInternal(const vpvl::Scene *scene, const QVector3D &normal)
        : m_scene(scene),
          m_listID(0),
          m_textureID(0)
    {
        TileStageVertex vertex;
        vertex.position.setZero();
        vertex.normal.setValue(normal.x(), normal.y(), normal.z());
        vertex.color.setValue(0.1f, 0.1f, 0.1f, 0.6f);
        vertex.texcoord.setValue(0.0f, 1.0f, 0.0f);
        m_vertices.add(vertex);
        vertex.texcoord.setValue(1.0f, 1.0f, 0.0f);
        m_vertices.add(vertex);
        vertex.texcoord.setValue(1.0f, 0.0f, 0.0f);
        m_vertices.add(vertex);
        vertex.texcoord.setValue(0.0f, 0.0f, 0.0f);
        m_vertices.add(vertex);
        const uint16_t indices[] = { 3, 2, 0, 0, 2, 1 };
        memcpy(m_indices, indices, sizeof(indices));
        initializeGLFunctions(QGLContext::currentContext());
        glGenBuffers(sizeof(m_buffers) / sizeof(GLuint), m_buffers);
        glBindBuffer(GL_ARRAY_BUFFER, m_buffers[0]);
        glBufferData(GL_ARRAY_BUFFER, m_vertices.count() * sizeof(TileStageVertex), &m_vertices[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_indices), m_indices, GL_STATIC_DRAW);
        m_matrix.setToIdentity();
    }
    ~TiledStageInternal() {
        glDeleteBuffers(sizeof(m_buffers) / sizeof(GLuint), m_buffers);
        deleteList();
        if (m_textureID)
            glDeleteTextures(1, &m_textureID);
        m_listID = 0;
        m_textureID = 0;
    }
    void load(const QString &path) {
        QGLContext *context = const_cast<QGLContext *>(QGLContext::currentContext());
        m_textureID = context->bindTexture(path);
    }
    void render(bool cullface, bool hasColor) {
        const float color[] = { 0.65f, 0.65f, 0.65f, 1.0f };
        float matrix[16];
        m_scene->getProjectionMatrix(matrix);
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(matrix);
        m_scene->getModelViewMatrix(matrix);
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(matrix);
        if (m_listID) {
            glCallList(m_listID);
        }
        else {
            static TileStageVertex v;
            m_listID = glGenLists(1);
            glNewList(m_listID, GL_COMPILE_AND_EXECUTE);
            if (!cullface)
                glDisable(GL_CULL_FACE);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
            glDisable(GL_LIGHTING);
            glActiveTexture(GL_TEXTURE0);
            glClientActiveTexture(GL_TEXTURE0);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, m_textureID);
            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_NORMAL_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glBindBuffer(GL_ARRAY_BUFFER, m_buffers[0]);
            glVertexPointer(3, GL_FLOAT, sizeof(v), reinterpret_cast<const GLvoid *>(0));
            size_t offset = reinterpret_cast<const uint8_t *>(&v.normal) - reinterpret_cast<const uint8_t *>(&v.position);
            glNormalPointer(GL_FLOAT, sizeof(v), reinterpret_cast<const GLvoid *>(offset));
            if (hasColor) {
                offset = reinterpret_cast<const uint8_t *>(&v.normal) - reinterpret_cast<const uint8_t *>(&v.position);
                glColorPointer(4, GL_FLOAT, sizeof(v), reinterpret_cast<const GLvoid *>(offset));
            }
            offset = reinterpret_cast<const uint8_t *>(&v.texcoord) - reinterpret_cast<const uint8_t *>(&v.position);
            glTexCoordPointer(2, GL_FLOAT, sizeof(v), reinterpret_cast<const GLvoid *>(offset));
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[1]);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_NORMAL_ARRAY);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_TEXTURE_2D);
            if (!cullface)
                glEnable(GL_CULL_FACE);
            glEndList();
        }
    }
    const QMatrix3x4 &vertices() const {
        return m_matrix;
    }
    void setVertices(const QMatrix3x4 &vertices) {
        for (int i = 0; i < 4; i++)
            m_vertices[i].position.setValue(vertices(i, 0), vertices(i, 1), vertices(i, 2));
        glDeleteBuffers(1, &m_buffers[0]);
        glGenBuffers(1, &m_buffers[0]);
        glBindBuffer(GL_ARRAY_BUFFER, m_buffers[0]);
        glBufferData(GL_ARRAY_BUFFER, m_vertices.count() * sizeof(TileStageVertex), &m_vertices[0], GL_STATIC_DRAW);
        m_matrix = vertices;
    }

private:
    void deleteList() {
        if (m_listID) {
            glDeleteLists(m_listID, 1);
            m_listID = 0;
        }
    }

    const vpvl::Scene *m_scene;
    QMatrix3x4 m_matrix;
    vpvl::Array<TileStageVertex> m_vertices;
    uint16_t m_indices[6];
    GLuint m_listID;
    GLuint m_buffers[2];
    GLuint m_textureID;
};

TiledStage::TiledStage(const vpvl::Scene *scene, internal::World *world)
    : m_scene(scene),
      m_floor(0),
      m_background(0),
      m_floorRigidBody(0),
      m_world(world)
{
    initializeGLFunctions(QGLContext::currentContext());
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
    m_floor = new TiledStageInternal(m_scene, QVector3D(0.0f, 1.0f, 0.0f));
    m_floor->load(path);
    setSize(25.0f, 40.0f, 25.0f);
}

void TiledStage::loadBackground(const QString &path)
{
    delete m_background;
    m_background = new TiledStageInternal(m_scene, QVector3D(0.0f, 0.0f, 1.0f));
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
