/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
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

#include <qglobal.h>

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
          m_broadphase(-internal::kWorldAabbSize, internal::kWorldAabbSize),
          m_world(&m_dispatcher, &m_broadphase, &m_solver, &m_config),
          m_filtered(false)
    {
    }
    ~HandlesStaticWorld()
    {
        const btCollisionObjectArray &objects = m_world.getCollisionObjectArray();
        const int nobjects = m_world.getNumCollisionObjects();
        for (int i = nobjects - 1; i >= 0; i--) {
            btCollisionObject *object = objects.at(i);
            btRigidBody *body = btRigidBody::upcast(object);
            btMotionState *state = 0;
            if (body && (state = body->getMotionState()))
                delete state;
            m_world.removeCollisionObject(object);
            btBvhTriangleMeshShape *shape = static_cast<btBvhTriangleMeshShape *>(body->getCollisionShape());
            btStridingMeshInterface *mesh = shape->getMeshInterface();
            delete mesh;
            delete shape;
            delete object;
        }
    }

    btDiscreteDynamicsWorld *world() {
        return &m_world;
    }
    void addRigidBody(btRigidBody *body) {
        m_world.addRigidBody(body);
        m_bodies.append(body);
    }
    void filterObjects(const QList<btRigidBody *> &visibles) {
        const btCollisionObjectArray &objects = m_world.getCollisionObjectArray();
        const int nobjects = m_world.getNumCollisionObjects();
        for (int i = nobjects - 1; i >= 0; i--) {
            btCollisionObject *object = objects.at(i);
            btRigidBody *body = btRigidBody::upcast(object);
            if (!visibles.contains(body))
                m_world.removeCollisionObject(body);
        }
        m_filtered = true;
    }
    void restoreObjects() {
        if (!m_filtered)
            return;
        const btCollisionObjectArray &objects = m_world.getCollisionObjectArray();
        const int nobjects = m_world.getNumCollisionObjects();
        for (int i = nobjects - 1; i >= 0; i--)
            m_world.removeCollisionObject(objects[i]);
        foreach (btRigidBody *body, m_bodies)
            m_world.addRigidBody(body);
        /*
         * filterObjects で削除された btRigidBody のボーンが古い位置情報のままになっているため、
         * stepSimulation で MotionState を経由して新しい位置情報に更新させる必要がある
         */
        m_world.stepSimulation(1);
        m_filtered = false;
    }

private:
    QList<btRigidBody *> m_bodies;
    btDefaultCollisionConfiguration m_config;
    btCollisionDispatcher m_dispatcher;
    btAxisSweep3 m_broadphase;
    btSequentialImpulseConstraintSolver m_solver;
    btDiscreteDynamicsWorld m_world;
    bool m_filtered;
};

namespace {

const QColor &kRed = QColor::fromRgb(255, 0, 0, 127);
const QColor &kGreen = QColor::fromRgb(0, 255, 0, 127);
const QColor &kBlue = QColor::fromRgb(0, 0, 255, 127);
const QColor &kYellow = QColor::fromRgb(255, 255, 0, 127);

class BoneHandleMotionState : public btMotionState
{
public:
    BoneHandleMotionState() : m_bone(0) {}
    virtual ~BoneHandleMotionState() {}

    void setWorldTransform(const btTransform & /* worldTrans */) {
    }
    void setBone(vpvl::Bone *value) { m_bone = value; }

protected:
    vpvl::Bone *m_bone;
};

class RotationHandleMotionState : public BoneHandleMotionState
{
public:
    RotationHandleMotionState() : BoneHandleMotionState() {}

    void getWorldTransform(btTransform &worldTrans) const {
        worldTrans.setIdentity();
        if (m_bone)
            worldTrans.setOrigin(m_bone->localTransform().getOrigin());
    }
};

class TranslationHandleMotionState : public BoneHandleMotionState
{
public:
    TranslationHandleMotionState() : BoneHandleMotionState() {}

    void getWorldTransform(btTransform &worldTrans) const {
        if (m_bone)
            worldTrans = m_bone->localTransform();
        else
            worldTrans.setIdentity();
    }
};

static void LoadStaticModel(const aiMesh *mesh, Handles::Model &model)
{
    /* Open Asset Import Library を使って読み込んだモデルを VBO が利用出来る形に再構築 */
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
    model.body = 0;
}

static void LoadTrackableModel(const aiMesh *mesh,
                               HandlesStaticWorld *world,
                               BoneHandleMotionState *state,
                               Handles::Model &model)
{
    /* ハンドルのモデルを読み込んだ上で衝突判定を行うために作られたフィールドに追加する */
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
    const btScalar &mass = 0.0f;
    const btVector3 localInertia(0.0f, 0.0f, 0.0f);
    btBvhTriangleMeshShape *shape = new btBvhTriangleMeshShape(triangleMesh, true);
    btRigidBody::btRigidBodyConstructionInfo info(mass, state, shape, localInertia);
    btRigidBody *body = new btRigidBody(info);
    /*
     * Bone の位置情報を元に動かす静的なオブジェクトであるため KinematicObject として処理する
     * これを行わないと stepSimulation で進めても MotionState で Bone の位置情報を引いて更新する処理が行われない
     */
    body->setActivationState(DISABLE_DEACTIVATION);
    body->setCollisionFlags(body->getCollisionFlags() | btRigidBody::CF_KINEMATIC_OBJECT);
    body->setUserPointer(&model);
    world->addRigidBody(body);
    model.body = body;
}

}

Handles::Handles(SceneWidget *parent)
    : QObject(parent),
      m_bone(0),
      m_world(0),
      m_widget(parent),
      m_trackedHandle(0),
      m_prevPos3D(0.0f, 0.0f, 0.0f),
      m_prevAngle(0.0f),
      m_width(0),
      m_height(0),
      m_visibilityFlags(kVisibleAll),
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
    /* ウィンドウの大きさが変わったら判定がずれないように全ての画像のハンドルの位置情報を更新しておく */
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

bool Handles::testHitModel(const vpvl::Vector3 &rayFrom,
                           const vpvl::Vector3 &rayTo,
                           bool setTracked,
                           int &flags,
                           vpvl::Vector3 &pick)
{
    flags = kNone;
    if (m_bone) {
        btCollisionWorld::ClosestRayResultCallback callback(rayFrom,rayTo);
        m_world->world()->rayTest(rayFrom, rayTo, callback);
        m_trackedHandle = 0;
        if (callback.hasHit()) {
            btRigidBody *body = btRigidBody::upcast(callback.m_collisionObject);
            Handles::Model *model = static_cast<Handles::Model *>(body->getUserPointer());
            if (m_bone->isMovable() && m_visibilityFlags & kMove) {
                if (model == &m_translationHandle.x && (m_visibilityFlags & kX))
                    flags = kView | kMove | kX;
                else if (model == &m_translationHandle.y && (m_visibilityFlags & kY))
                    flags = kView | kMove | kY;
                else if (model == &m_translationHandle.z && (m_visibilityFlags & kZ))
                    flags = kView | kMove | kZ;
            }
            if (m_bone->isRotateable() && m_visibilityFlags & kRotate) {
                if (model == &m_rotationHandle.x && (m_visibilityFlags & kX))
                    flags = kView | kRotate | kX;
                else if (model == &m_rotationHandle.y && (m_visibilityFlags & kY))
                    flags = kView | kRotate | kY;
                else if (model == &m_rotationHandle.z && (m_visibilityFlags & kZ))
                    flags = kView | kRotate | kZ;
            }
            if (setTracked)
                m_trackedHandle = model;
            pick = callback.m_hitPointWorld;
            return flags != kNone;
        }
    }
    return false;
}

bool Handles::testHitImage(const QPointF &p,
                           int &flags,
                           QRectF &rect)
{
    const QPointF pos(p.x(), m_height - p.y());
    flags = kNone;
    if (m_enableMove) {
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

const vpvl::Vector3 Handles::angle(const vpvl::Vector3 &pos) const
{
    return (pos - m_bone->localTransform().getOrigin()).normalized();
}

void Handles::setPoint3D(const vpvl::Vector3 &value)
{
    m_prevPos3D = value;
}

void Handles::setPoint2D(const QPointF &value)
{
    m_prevPos2D = value;
}

void Handles::setAngle(float value)
{
    m_prevAngle = value;
}

const vpvl::Vector3 Handles::diffPoint3D(const vpvl::Vector3 &value) const
{
    return value - m_prevPos3D;
}

const QPointF Handles::diffPoint2D(const QPointF &value) const
{
    return value - m_prevPos2D;
}

float Handles::diffAngle(float value) const
{
    return value - m_prevAngle;
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
            static_cast<BoneHandleMotionState *>(state)->setBone(value);
    }
    updateBone();
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

void Handles::setVisibilityFlags(int value)
{
    if (value == kVisibleAll) {
        m_world->restoreObjects();
    }
    else {
        QList<btRigidBody *> bodies;
        if (value & kMove) {
            if (value & kX)
                bodies.append(m_translationHandle.x.body);
            else if (value & kY)
                bodies.append(m_translationHandle.y.body);
            else if (value & kZ)
                bodies.append(m_translationHandle.z.body);
        }
        if (value & kRotate) {
            if (value & kX)
                bodies.append(m_rotationHandle.x.body);
            else if (value & kY)
                bodies.append(m_rotationHandle.y.body);
            else if (value & kZ)
                bodies.append(m_rotationHandle.z.body);
        }
        m_world->filterObjects(bodies);
    }
    m_visibilityFlags = value;
}

void Handles::updateBone()
{
    /* ボーンの位置情報を更新したら stepSimulation で MotionState を経由してハンドルの位置情報に反映させる */
    m_world->world()->stepSimulation(1);
}

void Handles::drawImageHandles()
{
    QGLFunctions func(QGLContext::currentContext());
    func.glUseProgram(0);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, m_width, 0, m_height, -1, 1);
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
	QGLFunctions func(QGLContext::currentContext());
    scene->getModelViewMatrix(matrix);
    func.glUniformMatrix4fv(modelViewMatrix, 1, GL_FALSE, matrix);
    scene->getProjectionMatrix(matrix);
    func.glUniformMatrix4fv(projectionMatrix, 1, GL_FALSE, matrix);
    const vpvl::Transform &boneTransform = m_bone->localTransform();
    vpvl::Transform transform = vpvl::Transform::getIdentity();
    transform.setOrigin(m_bone->localTransform().getOrigin());
    transform.getOpenGLMatrix(matrix);
    func.glUniformMatrix4fv(boneMatrix, 1, GL_FALSE, matrix);
    if (m_bone->isRotateable() && m_visibilityFlags & kRotate) {
        drawModel(m_rotationHandle.x, kRed, kX);
        drawModel(m_rotationHandle.y, kGreen, kY);
        drawModel(m_rotationHandle.z, kBlue, kZ);
    }
    boneTransform.getOpenGLMatrix(matrix);
    func.glUniformMatrix4fv(boneMatrix, 1, GL_FALSE, matrix);
    if (m_bone->isMovable() && m_visibilityFlags & kMove) {
        drawModel(m_translationHandle.x, kRed, kX);
        drawModel(m_translationHandle.y, kGreen, kY);
        drawModel(m_translationHandle.z, kBlue, kZ);
        drawModel(m_translationHandle.axisX, kRed, kX);
        drawModel(m_translationHandle.axisY, kGreen, kY);
        drawModel(m_translationHandle.axisZ, kBlue, kZ);
    }
    m_program.release();
}

void Handles::drawModel(const Handles::Model &model,
                        const QColor &color,
                        int requiredVisibilityFlags)
{
    if (m_visibilityFlags & requiredVisibilityFlags) {
        const Handles::Vertex &ptr = model.vertices.at(0);
        const GLfloat *vertexPtr = reinterpret_cast<const GLfloat *>(&ptr.position.x());
        int inPosition = m_program.attributeLocation("inPosition");
        m_program.setUniformValue("color", &model == m_trackedHandle ? kYellow : color);
        m_program.enableAttributeArray(inPosition);
        m_program.setAttributeArray(inPosition, vertexPtr, 4, sizeof(Handles::Vertex));
        glDrawElements(GL_TRIANGLES, model.indices.count(), GL_UNSIGNED_SHORT, &model.indices[0]);
        m_program.disableAttributeArray(inPosition);
    }
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
    /* 回転軸ハンドル (3つのドーナツ状のメッシュが入ってる) */
    QFile rotationHandleFile(":models/rotation.3ds");
    if (rotationHandleFile.open(QFile::ReadOnly)) {
        const QByteArray &rotationHandleBytes = rotationHandleFile.readAll();
        vpvl::Asset *asset = new vpvl::Asset();
        asset->load(reinterpret_cast<const uint8_t *>(rotationHandleBytes.constData()), rotationHandleBytes.size());
        aiMesh **meshes = asset->getScene()->mMeshes;
        LoadTrackableModel(meshes[1], m_world, new RotationHandleMotionState(), m_rotationHandle.x);
        LoadTrackableModel(meshes[0], m_world, new RotationHandleMotionState(), m_rotationHandle.y);
        LoadTrackableModel(meshes[2], m_world, new RotationHandleMotionState(), m_rotationHandle.z);
        m_rotationHandle.asset = asset;
    }
    /* 移動軸ハンドル (3つのコーン状のメッシュと3つの細長いシリンダー計6つのメッシュが入ってる) */
    QFile translationHandleFile(":models/translation.3ds");
    if (translationHandleFile.open(QFile::ReadOnly)) {
        const QByteArray &translationHandleBytes = translationHandleFile.readAll();
        vpvl::Asset *asset = new vpvl::Asset();
        asset->load(reinterpret_cast<const uint8_t *>(translationHandleBytes.constData()), translationHandleBytes.size());
        aiMesh **meshes = asset->getScene()->mMeshes;
        LoadTrackableModel(meshes[0], m_world, new TranslationHandleMotionState(), m_translationHandle.x);
        LoadTrackableModel(meshes[2], m_world, new TranslationHandleMotionState(), m_translationHandle.y);
        LoadTrackableModel(meshes[1], m_world, new TranslationHandleMotionState(), m_translationHandle.z);
        LoadStaticModel(meshes[3], m_translationHandle.axisX);
        LoadStaticModel(meshes[5], m_translationHandle.axisY);
        LoadStaticModel(meshes[4], m_translationHandle.axisZ);
        m_translationHandle.asset = asset;
    }
}

