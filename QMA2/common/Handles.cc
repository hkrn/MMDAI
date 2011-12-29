/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn                                    */
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

#include "Handles.h"
#include "SceneWidget.h"
#include "World.h"
#include "util.h"

#include <vpvl/vpvl.h>
#include <aiScene.h>

class HandlesStaticWorld {
public:
    HandlesStaticWorld()
        : m_dispatcher(&m_config),
          m_broadphase(btVector3(-400.0f, -400.0f, -400.0f), btVector3(400.0f, 400.0, 400.0f), 1024),
          m_world(&m_dispatcher, &m_broadphase, &m_solver, &m_config)
    {
    }
    ~HandlesStaticWorld()
    {
    }

    void addTriangleMeshShape(btTriangleMeshShape *shape, btTriangleMesh *mesh) {
        m_shapes.push_back(shape);
        m_meshes.push_back(mesh);
    }
    btDiscreteDynamicsWorld *world() {
        return &m_world;
    }
    void deleteAllObjects() {
        const int nobjects = m_world.getNumCollisionObjects();
        for (int i = nobjects - 1; i >= 0; i--) {
            btCollisionObject *object = m_world.getCollisionObjectArray().at(i);
            btRigidBody *body = btRigidBody::upcast(object);
            btMotionState *state = 0;
            if (body && (state = body->getMotionState()))
                delete state;
            m_world.removeCollisionObject(object);
            delete object;
        }
        const int nshapes = m_shapes.size();
        for (int i = 0; i < nshapes; i++) {
            btTriangleMeshShape *shape = m_shapes[i];
            btTriangleMesh *mesh = m_meshes[i];
            delete shape;
            delete mesh;
        }
    }

private:
    btDefaultCollisionConfiguration m_config;
    btCollisionDispatcher m_dispatcher;
    btAxisSweep3 m_broadphase;
    btSequentialImpulseConstraintSolver m_solver;
    btDiscreteDynamicsWorld m_world;
    btAlignedObjectArray<btTriangleMeshShape *> m_shapes;
    btAlignedObjectArray<btTriangleMesh *> m_meshes;
};

namespace {

const QColor &kRed = QColor::fromRgb(255, 0, 0, 127);
const QColor &kGreen = QColor::fromRgb(0, 255, 0, 127);
const QColor &kBlue = QColor::fromRgb(0, 0, 255, 127);

class MotionState : public btMotionState
{
public:
    MotionState() : m_bone(0) {}
    ~MotionState() {}

    void getWorldTransform(btTransform &worldTrans) const {
        worldTrans.setBasis(btMatrix3x3::getIdentity());
        if (m_bone)
            worldTrans.setOrigin(m_bone->position() + m_bone->originPosition());
        else
            worldTrans.setOrigin(btVector3(0, 0, 0));
    }
    void setWorldTransform(const btTransform & /* worldTrans */) {
    }
    void setBone(vpvl::Bone *value) { m_bone = value; }

private:
    vpvl::Bone *m_bone;
};

static void LoadStaticModel(const aiMesh *mesh, Handles::Model &model)
{
    const aiVector3D *meshVertices = mesh->mVertices;
    const aiVector3D *meshNormals = mesh->mNormals;
    const unsigned int nfaces = mesh->mNumFaces;
    int index = 0;
    for (unsigned int i = 0; i < nfaces; i++) {
        const struct aiFace &face = mesh->mFaces[i];
        const unsigned int nindices = face.mNumIndices;
        for (unsigned int j = 0; j < nindices; j++) {
            const int vertexIndex = face.mIndices[j];
            const aiVector3D &v = meshVertices[vertexIndex];
            const aiVector3D &n = meshNormals[vertexIndex];
            Handles::Vertex vertex;
            vertex.position.setValue(v.x, v.y, v.z);
            vertex.position.setW(1);
            vertex.normal.setValue(n.x, n.y, n.z);
            model.vertices.add(vertex);
            model.indices.add(index++);
        }
    }
}

static void LoadTrackableModel(const aiMesh *mesh, Handles::Model &model, HandlesStaticWorld *world)
{
    LoadStaticModel(mesh, model);
    btTriangleMesh *triangleMesh = new btTriangleMesh();
    const vpvl::Array<Handles::Vertex> &vertices = model.vertices;
    const int nfaces = vertices.count() / 3;
    for (int i = 0; i < nfaces; i++) {
        int index = i * 3;
        triangleMesh->addTriangle(vertices[index + 0].position,
                                  vertices[index + 1].position,
                                  vertices[index + 2].position);
    }
    /* TODO: track moving */
    const btScalar &mass = 0.0f;
    const btVector3 localInertia(0.0f, 0.0f, 0.0f);
    btBvhTriangleMeshShape *shape = new btBvhTriangleMeshShape(triangleMesh, true);
    btMotionState *state = new MotionState();
    btRigidBody::btRigidBodyConstructionInfo info(mass, state, shape, localInertia);
    btRigidBody *body = new btRigidBody(info);
    body->setActivationState(DISABLE_DEACTIVATION);
    body->setCollisionFlags(body->getCollisionFlags() | btRigidBody::CF_KINEMATIC_OBJECT);
    body->setUserPointer(&model);
    world->addTriangleMeshShape(shape, triangleMesh);
    world->world()->addRigidBody(body);
}

}

Handles::Handles(SceneWidget *parent)
    : m_bone(0),
      m_world(0),
      m_widget(parent),
      m_width(0),
      m_height(0),
      m_enableMove(false),
      m_enableRotate(false),
      m_isLocal(true),
      m_visible(true)
{
    m_world = new HandlesStaticWorld();
    m_rotationHandle.asset = 0;
    m_translationHandle.asset = 0;
}

Handles::~Handles()
{
    m_widget->deleteTexture(m_x.enableMove.textureID);
    m_widget->deleteTexture(m_y.enableMove.textureID);
    m_widget->deleteTexture(m_z.enableMove.textureID);
    m_widget->deleteTexture(m_x.disableMove.textureID);
    m_widget->deleteTexture(m_y.disableMove.textureID);
    m_widget->deleteTexture(m_z.disableMove.textureID);
    m_widget->deleteTexture(m_x.enableRotate.textureID);
    m_widget->deleteTexture(m_y.enableRotate.textureID);
    m_widget->deleteTexture(m_z.enableRotate.textureID);
    m_widget->deleteTexture(m_x.disableRotate.textureID);
    m_widget->deleteTexture(m_y.disableRotate.textureID);
    m_widget->deleteTexture(m_z.disableRotate.textureID);
    m_widget->deleteTexture(m_global.textureID);
    m_widget->deleteTexture(m_local.textureID);
    m_world->deleteAllObjects();
    delete m_world;
    delete m_rotationHandle.asset;
    delete m_translationHandle.asset;
}

void Handles::load()
{
    loadImageHandles();
    bool isShaderLoaded = true;
    isShaderLoaded &= m_program.addShaderFromSourceFile(QGLShader::Vertex, ":shaders/handle.vsh");
    isShaderLoaded &= m_program.addShaderFromSourceFile(QGLShader::Fragment, ":shaders/handle.fsh");
    isShaderLoaded &= m_program.link();
    if (isShaderLoaded)
        loadModelHandles();
}

void Handles::resize(int width, int height)
{
    qreal baseX = width - 104, baseY = 4, xoffset = 32, yoffset = 40;
    m_width = width;
    m_height = height;
    m_x.enableMove.rect.setTopLeft(QPointF(baseX, baseY));
    m_x.enableMove.rect.setSize(m_x.enableMove.size);
    m_y.enableMove.rect.setTopLeft(QPointF(baseX + xoffset, baseY));
    m_y.enableMove.rect.setSize(m_y.enableMove.size);
    m_z.enableMove.rect.setTopLeft(QPointF(baseX + xoffset * 2, baseY));
    m_z.enableMove.rect.setSize(m_z.enableMove.size);
    m_x.disableMove.rect.setTopLeft(QPointF(baseX, baseY));
    m_x.disableMove.rect.setSize(m_x.disableMove.size);
    m_y.disableMove.rect.setTopLeft(QPointF(baseX + xoffset, baseY));
    m_y.disableMove.rect.setSize(m_y.disableMove.size);
    m_z.disableMove.rect.setTopLeft(QPointF(baseX + xoffset * 2,  baseY));
    m_z.disableMove.rect.setSize(m_z.disableMove.size);
    m_x.enableRotate.rect.setTopLeft(QPointF(baseX, baseY + yoffset));
    m_x.enableRotate.rect.setSize(m_x.enableRotate.size);
    m_y.enableRotate.rect.setTopLeft(QPointF(baseX + xoffset, baseY + yoffset));
    m_y.enableRotate.rect.setSize(m_y.enableRotate.size);
    m_z.enableRotate.rect.setTopLeft(QPointF(baseX + xoffset * 2, baseY + yoffset));
    m_z.enableRotate.rect.setSize(m_z.enableRotate.size);
    m_x.disableRotate.rect.setTopLeft(QPointF(baseX, baseY + yoffset));
    m_x.disableRotate.rect.setSize(m_x.disableRotate.size);
    m_y.disableRotate.rect.setTopLeft(QPointF(baseX + xoffset, baseY + yoffset));
    m_y.disableRotate.rect.setSize(m_y.disableRotate.size);
    m_z.disableRotate.rect.setTopLeft(QPointF(baseX + xoffset * 2, baseY + yoffset));
    m_z.disableRotate.rect.setSize(m_z.disableRotate.size);
    m_global.rect.setTopLeft(QPointF(baseX, baseY + yoffset * 2));
    m_global.rect.setSize(m_global.size);
    m_local.rect.setTopLeft(QPointF(baseX + (m_global.size.width() - m_local.size.width()) / 2, baseY + yoffset * 2));
    m_local.rect.setSize(m_local.size);
}

bool Handles::testHit(const QPointF &p,
                      const vpvl::Vector3 &rayFrom,
                      const vpvl::Vector3 &rayTo,
                      int &flags,
                      QRectF &rect)
{
    const QPointF pos(p.x(), m_height - p.y());
    flags = kNone;
    btCollisionWorld::ClosestRayResultCallback callback(rayFrom,rayTo);
    m_world->world()->rayTest(rayFrom, rayTo, callback);
    if (callback.hasHit()) {
        const btVector3 &pick = callback.m_hitPointWorld;
        btRigidBody *body = btRigidBody::upcast(callback.m_collisionObject);
        Handles::Model *model = static_cast<Handles::Model *>(body->getUserPointer());
        /* TODO: implement emit signal of rotation handles */
        if (model == &m_translationHandle.x)
            qDebug() << "TX" << pick.x() << pick.y() << pick.z();
        else if (model == &m_translationHandle.y)
            qDebug() << "TY" << pick.x() << pick.y() << pick.z();
        else if (model == &m_translationHandle.z)
            qDebug() << "TZ" << pick.x() << pick.y() << pick.z();
        else if (model == &m_rotationHandle.x)
            qDebug() << "RX" << pick.x() << pick.y() << pick.z();
        else if (model == &m_rotationHandle.y)
            qDebug() << "RY" << pick.x() << pick.y() << pick.z();
        else if (model == &m_rotationHandle.z)
            qDebug() << "RZ" << pick.x() << pick.y() << pick.z();
    }
    else if (m_enableMove) {
        if (m_x.enableMove.rect.contains(pos)) {
            rect = m_x.enableMove.rect;
            flags = kEnable | kMove | kX;
        }
        else if (m_y.enableMove.rect.contains(pos)) {
            rect = m_y.enableMove.rect;
            flags = kEnable | kMove | kY;
        }
        else if (m_z.enableMove.rect.contains(pos)) {
            rect = m_z.enableMove.rect;
            flags = kEnable | kMove | kZ;
        }
    }
    else {
        if (m_x.disableMove.rect.contains(pos)) {
            rect = m_x.disableMove.rect;
            flags = kDisable | kMove | kX;
        }
        else if (m_y.disableMove.rect.contains(pos)) {
            rect = m_y.disableMove.rect;
            flags = kDisable | kMove | kY;
        }
        else if (m_z.disableMove.rect.contains(pos)) {
            rect = m_z.disableMove.rect;
            flags = kDisable | kMove | kZ;
        }
    }
    if (m_enableRotate) {
        if (m_x.enableRotate.rect.contains(pos)) {
            rect = m_x.enableRotate.rect;
            flags = kEnable | kRotate | kX;
        }
        else if (m_y.enableRotate.rect.contains(pos)) {
            rect = m_y.enableRotate.rect;
            flags = kEnable | kRotate | kY;
        }
        else if (m_z.enableRotate.rect.contains(pos)) {
            rect = m_z.enableRotate.rect;
            flags = kEnable | kRotate | kZ;
        }
    }
    else {
        if (m_x.disableRotate.rect.contains(pos)) {
            rect = m_x.disableRotate.rect;
            flags = kDisable | kRotate | kX;
        }
        else if (m_y.disableRotate.rect.contains(pos)) {
            rect = m_y.disableRotate.rect;
            flags = kDisable | kRotate | kY;
        }
        else if (m_z.disableRotate.rect.contains(pos)) {
            rect = m_z.disableRotate.rect;
            flags = kDisable | kRotate | kZ;
        }
    }
    if (m_isLocal) {
        if (m_local.rect.contains(pos)) {
            rect = m_local.rect;
            flags = kLocal;
        }
    }
    else {
        if (m_global.rect.contains(pos)) {
            rect = m_global.rect;
            flags = kGlobal;
        }
    }
    return flags != kNone;
}

void Handles::draw()
{
    if (!m_visible)
        return;
    drawModelHandles();
    drawImageHandles();
}

void Handles::setBone(vpvl::Bone *value)
{
    m_bone = value;
    btDiscreteDynamicsWorld *world = m_world->world();
    int nobjects = world->getNumCollisionObjects();
    for (int i = 0; i < nobjects; i++) {
        btCollisionObject *object = world->getCollisionObjectArray().at(i);
        btRigidBody *body = btRigidBody::upcast(object);
        btMotionState *state = 0;
        if (body && (state = body->getMotionState()))
            static_cast<MotionState *>(state)->setBone(value);
    }
    world->stepSimulation(1);
}

void Handles::setMovable(bool value)
{
    m_enableMove = value;
}

void Handles::setRotateable(bool value)
{
    m_enableRotate = value;
}

void Handles::setLocal(bool value)
{
    m_isLocal = value;
}

void Handles::setVisible(bool value)
{
    m_visible = value;
}

void Handles::drawImageHandles()
{
    glUseProgram(0);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, m_width, 0, m_height);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    if (m_enableMove) {
        m_widget->drawTexture(m_x.enableMove.rect, m_x.enableMove.textureID);
        m_widget->drawTexture(m_y.enableMove.rect, m_y.enableMove.textureID);
        m_widget->drawTexture(m_z.enableMove.rect, m_z.enableMove.textureID);
    }
    else {
        m_widget->drawTexture(m_x.disableMove.rect, m_x.disableMove.textureID);
        m_widget->drawTexture(m_y.disableMove.rect, m_y.disableMove.textureID);
        m_widget->drawTexture(m_z.disableMove.rect, m_z.disableMove.textureID);
    }
    if (m_enableRotate) {
        m_widget->drawTexture(m_x.enableRotate.rect, m_x.enableRotate.textureID);
        m_widget->drawTexture(m_y.enableRotate.rect, m_y.enableRotate.textureID);
        m_widget->drawTexture(m_z.enableRotate.rect, m_z.enableRotate.textureID);
    }
    else {
        m_widget->drawTexture(m_x.disableRotate.rect, m_x.disableRotate.textureID);
        m_widget->drawTexture(m_y.disableRotate.rect, m_y.disableRotate.textureID);
        m_widget->drawTexture(m_z.disableRotate.rect, m_z.disableRotate.textureID);
    }
    if (m_isLocal)
        m_widget->drawTexture(m_local.rect, m_local.textureID);
    else
        m_widget->drawTexture(m_global.rect, m_global.textureID);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void Handles::drawModelHandles()
{
    if (!m_program.isLinked() || !m_bone)
        return;
    float matrix[16];
    const vpvl::Scene *scene = m_widget->scene();
    m_program.bind();
    int modelViewMatrix = m_program.uniformLocation("modelViewMatrix");
    int projectionMatrix = m_program.uniformLocation("projectionMatrix");
    int boneMatrix = m_program.uniformLocation("boneMatrix");
    scene->getModelViewMatrix(matrix);
    glUniformMatrix4fv(modelViewMatrix, 1, GL_FALSE, matrix);
    scene->getProjectionMatrix(matrix);
    glUniformMatrix4fv(projectionMatrix, 1, GL_FALSE, matrix);
    vpvl::Transform transform;
    transform.setIdentity();
    transform.setOrigin(m_bone->position() + m_bone->originPosition());
    transform.getOpenGLMatrix(matrix);
    glUniformMatrix4fv(boneMatrix, 1, GL_FALSE, matrix);
    drawModel(m_rotationHandle.x, kRed);
    drawModel(m_rotationHandle.y, kGreen);
    drawModel(m_rotationHandle.z, kBlue);
    drawModel(m_translationHandle.x, kRed);
    drawModel(m_translationHandle.y, kGreen);
    drawModel(m_translationHandle.z, kBlue);
    drawModel(m_translationHandle.axisX, kRed);
    drawModel(m_translationHandle.axisY, kGreen);
    drawModel(m_translationHandle.axisZ, kBlue);
    m_program.release();
}

void Handles::drawModel(const Handles::Model &model, const QColor &color)
{
    const Handles::Vertex &ptr = model.vertices.at(0);
    const GLfloat *vertexPtr = reinterpret_cast<const GLfloat *>(&ptr.position.x());
    int inPosition = m_program.attributeLocation("inPosition");
    m_program.setUniformValue("color", color);
    m_program.enableAttributeArray(inPosition);
    m_program.setAttributeArray(inPosition, vertexPtr, 4, sizeof(Handles::Vertex));
    glDrawElements(GL_TRIANGLES, model.indices.count(), GL_UNSIGNED_SHORT, &model.indices[0]);
    m_program.disableAttributeArray(inPosition);
}

void Handles::loadImageHandles()
{
    QImage image;
    image.load(":icons/x-enable-move.png");
    m_x.enableMove.size = image.size();
    m_x.enableMove.textureID = m_widget->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
    image.load(":icons/x-enable-rotate.png");
    m_x.enableRotate.size = image.size();
    m_x.enableRotate.textureID = m_widget->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
    image.load(":icons/y-enable-move.png");
    m_y.enableMove.size = image.size();
    m_y.enableMove.textureID = m_widget->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
    image.load(":icons/y-enable-rotate.png");
    m_y.enableRotate.size = image.size();
    m_y.enableRotate.textureID = m_widget->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
    image.load(":icons/z-enable-move.png");
    m_z.enableMove.size = image.size();
    m_z.enableMove.textureID = m_widget->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
    image.load(":icons/z-enable-rotate.png");
    m_z.enableRotate.size = image.size();
    m_z.enableRotate.textureID = m_widget->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
    image.load(":icons/x-disable-move.png");
    m_x.disableMove.size = image.size();
    m_x.disableMove.textureID = m_widget->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
    image.load(":icons/x-disable-rotate.png");
    m_x.disableRotate.size = image.size();
    m_x.disableRotate.textureID = m_widget->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
    image.load(":icons/y-disable-move.png");
    m_y.disableMove.size = image.size();
    m_y.disableMove.textureID = m_widget->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
    image.load(":icons/y-disable-rotate.png");
    m_y.disableRotate.size = image.size();
    m_y.disableRotate.textureID = m_widget->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
    image.load(":icons/z-disable-move.png");
    m_z.disableMove.size = image.size();
    m_z.disableMove.textureID = m_widget->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
    image.load(":icons/z-disable-rotate.png");
    m_z.disableRotate.size = image.size();
    m_z.disableRotate.textureID = m_widget->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
    image.load(":icons/global.png");
    m_global.size = image.size();
    m_global.textureID = m_widget->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
    image.load(":icons/local.png");
    m_local.size = image.size();
    m_local.textureID = m_widget->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
}

void Handles::loadModelHandles()
{
    QFile rotationHandleFile(":models/rotation.3ds");
    if (rotationHandleFile.open(QFile::ReadOnly)) {
        const QByteArray &rotationHandleBytes = rotationHandleFile.readAll();
        vpvl::Asset *asset = new vpvl::Asset();
        asset->load(reinterpret_cast<const uint8_t *>(rotationHandleBytes.constData()), rotationHandleBytes.size());
        aiMesh **meshes = asset->getScene()->mMeshes;
        LoadTrackableModel(meshes[0], m_rotationHandle.x, m_world);
        LoadTrackableModel(meshes[1], m_rotationHandle.y, m_world);
        LoadTrackableModel(meshes[2], m_rotationHandle.z, m_world);
        m_rotationHandle.asset = asset;
    }
    QFile translationHandleFile(":models/translation.3ds");
    if (translationHandleFile.open(QFile::ReadOnly)) {
        const QByteArray &translationHandleBytes = translationHandleFile.readAll();
        vpvl::Asset *asset = new vpvl::Asset();
        asset->load(reinterpret_cast<const uint8_t *>(translationHandleBytes.constData()), translationHandleBytes.size());
        aiMesh **meshes = asset->getScene()->mMeshes;
        LoadTrackableModel(meshes[0], m_translationHandle.y, m_world);
        LoadTrackableModel(meshes[1], m_translationHandle.x, m_world);
        LoadTrackableModel(meshes[2], m_translationHandle.z, m_world);
        LoadStaticModel(meshes[3], m_translationHandle.axisX);
        LoadStaticModel(meshes[4], m_translationHandle.axisY);
        LoadStaticModel(meshes[5], m_translationHandle.axisZ);
        m_translationHandle.asset = asset;
    }
}
