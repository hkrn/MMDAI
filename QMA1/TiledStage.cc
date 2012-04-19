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
#include <vpvl2/vpvl2.h>

using namespace vpvl2;

class TiledStage::PrivateContext : protected QGLFunctions {
public:
    struct Vertex {
        QVector3D position;
        QVector3D normal;
        QVector2D texcoord;
    };

    PrivateContext(const Scene *scene, const QVector3D &normal, bool hasColor, bool cullFace = false)
        : m_scene(scene),
          m_textureID(0),
          m_cullFace(cullFace),
          m_hasColor(hasColor)
    {
        Vertex vertex;
        vertex.normal = normal;
        vertex.texcoord.setX(0.0f);
        vertex.texcoord.setY(1.0f);
        m_vertices.add(vertex);
        vertex.texcoord.setX(1.0f);
        vertex.texcoord.setY(1.0f);
        m_vertices.add(vertex);
        vertex.texcoord.setX(1.0f);
        vertex.texcoord.setY(0.0f);
        m_vertices.add(vertex);
        vertex.texcoord.setX(0.0f);
        vertex.texcoord.setY(0.0f);
        m_vertices.add(vertex);
        const uint16_t indices[] = { 3, 2, 0, 0, 2, 1 };
        memcpy(m_indices, indices, sizeof(indices));
        m_matrix.setToIdentity();
        initializeGLFunctions(QGLContext::currentContext());
    }
    ~PrivateContext() {
        glDeleteBuffers(sizeof(m_buffers) / sizeof(GLuint), m_buffers);
        if (m_textureID)
            glDeleteTextures(1, &m_textureID);
        m_textureID = 0;
    }

    void load(const QString &path) {
        glGenBuffers(sizeof(m_buffers) / sizeof(GLuint), m_buffers);
        glBindBuffer(GL_ARRAY_BUFFER, m_buffers[0]);
        glBufferData(GL_ARRAY_BUFFER, m_vertices.count() * sizeof(Vertex), &m_vertices[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_indices), m_indices, GL_STATIC_DRAW);
        m_program.addShaderFromSourceFile(QGLShader::Vertex, ":shaders/stage.vsh");
        m_program.addShaderFromSourceFile(QGLShader::Fragment, ":shaders/stage.fsh");
        m_program.link();
        QGLContext *context = const_cast<QGLContext *>(QGLContext::currentContext());
        QGLContext::BindOptions options = QGLContext::LinearFilteringBindOption|QGLContext::InvertedYBindOption;
        m_textureID = context->bindTexture(QImage(path), GL_TEXTURE_2D, GL_RGBA, options);
    }
    void render() {
        if (!m_program.isLinked())
            return;
        if (!m_cullFace)
            glDisable(GL_CULL_FACE);
        QMatrix4x4 modelView4x4, projection4x4;
        float modelViewMatrixf[16], projectionMatrixf[16], normalMatrix[9];
        const Scene::IMatrices *matrices = m_scene->matrices();
        matrices->getModelView(modelViewMatrixf);
        matrices->getProjection(projectionMatrixf);
        for (int i = 0; i < 16; i++) {
            modelView4x4.data()[i] = modelViewMatrixf[i];
            projection4x4.data()[i] = projectionMatrixf[i];
        }
        const QMatrix4x4 &modelViewProjection4x4 = projection4x4 * modelView4x4;
        m_program.bind();
        m_program.setUniformValue("modelViewProjectionMatrix", modelViewProjection4x4);
        matrices->getNormal(normalMatrix);
        glUniformMatrix3fv(m_program.uniformLocation("normalMatrix"), 1, GL_FALSE, normalMatrix);
        static const Vertex v;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_textureID);
        glBindBuffer(GL_ARRAY_BUFFER, m_buffers[0]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[1]);
        m_program.setUniformValue("mainTexture", 0);
        m_program.setUniformValue("lightPosition", toQVector3D(m_scene->light()->direction()));
        int inPosition = m_program.attributeLocation("inPosition");
        glEnableVertexAttribArray(inPosition);
        glVertexAttribPointer(inPosition, 3, GL_FLOAT, GL_FALSE, sizeof(v), 0);
        size_t offset = reinterpret_cast<const uint8_t *>(&v.normal) - reinterpret_cast<const uint8_t *>(&v.position);
        int inNormal = m_program.attributeLocation("inNormal");
        glEnableVertexAttribArray(inNormal);
        glVertexAttribPointer(inNormal, 3, GL_FLOAT, GL_FALSE, sizeof(v), reinterpret_cast<const void *>(offset));
        offset = reinterpret_cast<const uint8_t *>(&v.texcoord) - reinterpret_cast<const uint8_t *>(&v.position);
        int inTexCoord = m_program.attributeLocation("inTexCoord");
        glEnableVertexAttribArray(inTexCoord);
        glVertexAttribPointer(inTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(v), reinterpret_cast<const void *>(offset));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        if (!m_cullFace)
            glEnable(GL_CULL_FACE);
        m_program.release();
    }
    const QMatrix3x4 &vertices() const {
        return m_matrix;
    }
    void setVertices(const QMatrix3x4 &vertices) {
        for (int i = 0; i < 4; i++) {
            QVector3D &position = m_vertices[i].position;
            position.setX(vertices(i, 0));
            position.setY(vertices(i, 1));
            position.setZ(vertices(i, 2));
        }
        glDeleteBuffers(1, &m_buffers[0]);
        glGenBuffers(1, &m_buffers[0]);
        glBindBuffer(GL_ARRAY_BUFFER, m_buffers[0]);
        glBufferData(GL_ARRAY_BUFFER, m_vertices.count() * sizeof(Vertex), &m_vertices[0], GL_STATIC_DRAW);
        m_matrix = vertices;
    }

private:
    static const inline QVector3D toQVector3D(const Vector3 &value) {
        QVector3D v;
        v.setX(value.x());
        v.setY(value.y());
        v.setZ(value.z());
        return v;
    }

    const Scene *m_scene;
    QMatrix3x4 m_matrix;
    Array<Vertex> m_vertices;
    uint16_t m_indices[6];
    QGLShaderProgram m_program;
    GLuint m_buffers[2];
    GLuint m_textureID;
    const bool m_cullFace;
    const bool m_hasColor;
};

TiledStage::TiledStage(const Scene *scene, internal::World *world)
    : m_scene(scene),
      m_floor(0),
      m_background(0),
      m_floorRigidBody(0),
      m_world(world)
{
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
    m_floor = new PrivateContext(m_scene, QVector3D(0.0f, 1.0f, 0.0f), true);
    m_floor->load(path);
    setSize(25.0f, 40.0f, 25.0f);
}

void TiledStage::loadBackground(const QString &path)
{
    delete m_background;
    m_background = new PrivateContext(m_scene, QVector3D(0.0f, 0.0f, 1.0f), false);
    m_background->load(path);
    setSize(25.0f, 40.0f, 25.0f);
}

void TiledStage::renderFloor()
{
    if (m_floor)
        m_floor->render();
}

void TiledStage::renderBackground()
{
    if (m_background)
        m_background->render();
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
