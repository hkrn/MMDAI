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
#include <vpvl2/qt/World.h>

#include "SceneLoader.h"
#include "Handles.h"
#include "SceneWidget.h"
#include "TextureDrawHelper.h"
#include "util.h"

#include <vpvl2/vpvl2.h>
#include <aiPostProcess.h>
#include <aiScene.h>

namespace vpvm
{

using namespace vpvl2;

class Handles::StaticWorld {
public:
    StaticWorld()
        : m_dispatcher(&m_config),
          m_broadphase(-qt::World::kAabbSize, qt::World::kAabbSize),
          m_world(&m_dispatcher, &m_broadphase, &m_solver, &m_config)
    {
    }
    ~StaticWorld()
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
    }
    void restoreObjects() {
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
    }

private:
    QList<btRigidBody *> m_bodies;
    btDefaultCollisionConfiguration m_config;
    btCollisionDispatcher m_dispatcher;
    btAxisSweep3 m_broadphase;
    btSequentialImpulseConstraintSolver m_solver;
    btDiscreteDynamicsWorld m_world;
};

namespace {

const QColor kRed = QColor::fromRgb(255, 0, 0, 127);
const QColor kGreen = QColor::fromRgb(0, 255, 0, 127);
const QColor kBlue = QColor::fromRgb(0, 0, 255, 127);
const QColor kYellow = QColor::fromRgb(255, 255, 0, 127);

class BoneHandleMotionState : public btMotionState
{
public:
    BoneHandleMotionState(Handles *handles)
        : m_handles(handles)
    {
    }
    virtual ~BoneHandleMotionState() {}

    void getWorldTransform(btTransform &worldTrans) const {
        worldTrans = m_handles->modelHandleTransform();
    }
    void setWorldTransform(const btTransform & /* worldTrans */) {
    }

protected:
    Handles *m_handles;
};

static void UILoadStaticModel(const aiMesh *mesh, Handles::Model &model)
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

static void UILoadTrackableModel(const aiMesh *mesh,
                                 Handles::StaticWorld *world,
                                 BoneHandleMotionState *state,
                                 Handles::Model &model)
{
    /* ハンドルのモデルを読み込んだ上で衝突判定を行うために作られたフィールドに追加する */
    UILoadStaticModel(mesh, model);
    QScopedPointer<btTriangleMesh> triangleMesh(new btTriangleMesh());
    const Array<Handles::Vertex> &vertices = model.vertices;
    const int nfaces = vertices.count() / 3;
    for (int i = 0; i < nfaces; i++) {
        int index = i * 3;
        triangleMesh->addTriangle(vertices[index + 0].position,
                                  vertices[index + 1].position,
                                  vertices[index + 2].position);
    }
    const btScalar &mass = 0.0f;
    const btVector3 localInertia(0.0f, 0.0f, 0.0f);
    QScopedPointer<btBvhTriangleMeshShape> shape(new btBvhTriangleMeshShape(triangleMesh.take(), true));
    btRigidBody::btRigidBodyConstructionInfo info(mass, state, shape.take(), localInertia);
    QScopedPointer<btRigidBody> body(new btRigidBody(info));
    /*
     * Bone の位置情報を元に動かす静的なオブジェクトであるため KinematicObject として処理する
     * これを行わないと stepSimulation で進めても MotionState で Bone の位置情報を引いて更新する処理が行われない
     */
    body->setActivationState(DISABLE_DEACTIVATION);
    body->setCollisionFlags(body->getCollisionFlags() | btRigidBody::CF_KINEMATIC_OBJECT);
    body->setUserPointer(&model);
    world->addRigidBody(body.data());
    model.body = body.take();
}

static void UIInitializeRenderingModel(const SceneLoader *loader,
                                       const IModel *model,
                                       const Transform &transform,
                                       QGLShaderProgram *program)
{
    QGLFunctions func(QGLContext::currentContext());
    QMatrix4x4 world, view, projection;
    loader->getCameraMatrices(world, view, projection);
    if (model) {
        const Vector3 &position = model->position();
        world.translate(position.x(), position.y(), position.z());
    }
    program->setUniformValue("modelViewProjectionMatrix", projection * view * world);
    int boneMatrix = program->uniformLocation("boneMatrix");
    float matrix[16];
    transform.getOpenGLMatrix(matrix);
    func.glUniformMatrix4fv(boneMatrix, 1, GL_FALSE, matrix);
}

}

Handles::Handles(SceneLoader *loaderRef, const QSize &size)
    : QObject(),
      m_helper(new TextureDrawHelper(size)),
      m_world(new Handles::StaticWorld()),
      m_boneRef(0),
      m_loaderRef(loaderRef),
      m_trackedHandleRef(0),
      m_constraint(kLocal),
      m_prevPos3D(0.0f, 0.0f, 0.0f),
      m_prevAngle(0.0f),
      m_visibilityFlags(kVisibleAll),
      m_visible(true)
{
}

bool Handles::isToggleButton(int value)
{
    return value & kGlobal || value & kLocal || value & kView;
}

Handles::~Handles()
{
    QGLContext *context = const_cast<QGLContext *>(QGLContext::currentContext());
    context->deleteTexture(m_x.enableMove.textureID);
    context->deleteTexture(m_y.enableMove.textureID);
    context->deleteTexture(m_z.enableMove.textureID);
    context->deleteTexture(m_x.disableMove.textureID);
    context->deleteTexture(m_y.disableMove.textureID);
    context->deleteTexture(m_z.disableMove.textureID);
    context->deleteTexture(m_x.enableRotate.textureID);
    context->deleteTexture(m_y.enableRotate.textureID);
    context->deleteTexture(m_z.enableRotate.textureID);
    context->deleteTexture(m_x.disableRotate.textureID);
    context->deleteTexture(m_y.disableRotate.textureID);
    context->deleteTexture(m_z.disableRotate.textureID);
    context->deleteTexture(m_global.textureID);
    context->deleteTexture(m_local.textureID);
    context->deleteTexture(m_view.textureID);
}

void Handles::load()
{
    m_helper->load();
    loadImageHandles();
    bool isShaderLoaded = true;
    isShaderLoaded &= m_program.addShaderFromSourceFile(QGLShader::Vertex, ":shaders/handle.vsh");
    isShaderLoaded &= m_program.addShaderFromSourceFile(QGLShader::Fragment, ":shaders/handle.fsh");
    isShaderLoaded &= m_program.link();
    if (isShaderLoaded)
        loadModelHandles();
}

void Handles::resize(const QSize &size)
{
    /* ウィンドウの大きさが変わったら判定がずれないように全ての画像のハンドルの位置情報を更新しておく */
    int width = size.width();
    qreal baseX = width - 104, baseY = 4, xoffset = 32, yoffset = 40;
    m_helper->resize(size);
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
    m_view.rect.setTopLeft(QPointF(baseX + (m_global.size.width() - m_view.size.width()) / 2, baseY + yoffset * 2));
    m_view.rect.setSize(m_view.size);
}

bool Handles::testHitModel(const Vector3 &rayFrom,
                           const Vector3 &rayTo,
                           bool setTracked,
                           int &flags,
                           Vector3 &pick)
{
    flags = kNone;
    if (m_boneRef) {
        btCollisionWorld::ClosestRayResultCallback callback(rayFrom,rayTo);
        m_world->world()->rayTest(rayFrom, rayTo, callback);
        m_trackedHandleRef = 0;
        if (callback.hasHit()) {
            const btRigidBody *body = btRigidBody::upcast(callback.m_collisionObject);
            Handles::Model *model = static_cast<Handles::Model *>(body->getUserPointer());
            if (m_boneRef->isMovable() && m_visibilityFlags & kMove) {
                if (model == &m_translationHandle.x && (m_visibilityFlags & kX))
                    flags = kModel | kMove | kX;
                else if (model == &m_translationHandle.y && (m_visibilityFlags & kY))
                    flags = kModel | kMove | kY;
                else if (model == &m_translationHandle.z && (m_visibilityFlags & kZ))
                    flags = kModel | kMove | kZ;
            }
            if (m_boneRef->isRotateable() && m_visibilityFlags & kRotate) {
                if (model == &m_rotationHandle.x && (m_visibilityFlags & kX))
                    flags = kModel | kRotate | kX;
                else if (model == &m_rotationHandle.y && (m_visibilityFlags & kY))
                    flags = kModel | kRotate | kY;
                else if (model == &m_rotationHandle.z && (m_visibilityFlags & kZ))
                    flags = kModel | kRotate | kZ;
            }
            if (setTracked)
                m_trackedHandleRef = model;
            pick = callback.m_hitPointWorld;
            return flags != kNone;
        }
    }
    return false;
}

bool Handles::testHitImage(const QPointF &p,
                           bool movable,
                           bool rotateable,
                           int &flags,
                           QRectF &rect)
{
    const QPointF pos(p.x(), m_helper->size().height() - p.y());
    flags = kNone;
    if (movable) {
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
    if (rotateable) {
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
    switch (m_constraint) {
    case kView:
        if (m_view.rect.contains(pos)) {
            rect = m_local.rect;
            flags = kView;
        }
        break;
    case kLocal:
        if (m_local.rect.contains(pos)) {
            rect = m_local.rect;
            flags = kLocal;
        }
        break;
    case kGlobal:
        if (m_global.rect.contains(pos)) {
            rect = m_global.rect;
            flags = kGlobal;
        }
        break;
    case Handles::kNone:
    case Handles::kEnable:
    case Handles::kDisable:
    case Handles::kMove:
    case Handles::kRotate:
    case Handles::kX:
    case Handles::kY:
    case Handles::kZ:
    case Handles::kModel:
    case Handles::kVisibleMove:
    case Handles::kVisibleRotate:
    case Handles::kVisibleAll:
    default:
        break;
    }
    return flags != kNone;
}

btScalar Handles::angle(const Vector3 &pos) const
{
    return pos.angle(m_boneRef->worldTransform().getOrigin());
}

int Handles::modeFromConstraint() const
{
    switch (m_constraint) {
    case kView:
        return 'V';
    case kLocal:
        return 'L';
    case kGlobal:
        return 'G';
    case Handles::kNone:
    case Handles::kEnable:
    case Handles::kDisable:
    case Handles::kMove:
    case Handles::kRotate:
    case Handles::kX:
    case Handles::kY:
    case Handles::kZ:
    case Handles::kModel:
    case Handles::kVisibleMove:
    case Handles::kVisibleRotate:
    case Handles::kVisibleAll:
    default:
        return 'L';
    }
}

const Transform Handles::modelHandleTransform() const
{
    Transform transform = Transform::getIdentity();
    if (m_boneRef) {
        int mode = modeFromConstraint();
        if (mode == 'G') {
            transform.setOrigin(m_boneRef->worldTransform().getOrigin());
        }
        else if (mode == 'L') {
            transform = m_boneRef->worldTransform();
        }
        else if (mode == 'V') {
            const Matrix3x3 &basis = m_loaderRef->sceneRef()->camera()->modelViewTransform().getBasis();
            btMatrix3x3 newBasis;
            newBasis[0] = basis * Vector3(1, 0, 0);
            newBasis[1] = basis * Vector3(0, 1, 0);
            newBasis[2] = basis * Vector3(0, 0, 1);
            transform.setOrigin(m_boneRef->worldTransform().getOrigin());
            transform.setBasis(newBasis);
        }
    }
    return transform;
}

void Handles::setPoint3D(const Vector3 &value)
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

const Vector3 Handles::diffPoint3D(const Vector3 &value) const
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

void Handles::setBone(IBone *value)
{
    m_boneRef = value;
    updateBone();
}

void Handles::setState(Flags value)
{
    m_constraint = value;
}

void Handles::setVisible(bool value)
{
    m_visible = value;
}

void Handles::setVisibilityFlags(int value)
{
    /* 一回全ての剛体を物理世界にいれておき、表示オブジェクトがあればそれのみあたるよう剛体をフィルタリングする手法を取る */
    m_world->restoreObjects();
    if (value != kVisibleAll) {
        QList<btRigidBody *> bodies;
        if (value & kMove) {
            if (value & kX)
                bodies.append(m_translationHandle.x.body);
            if (value & kY)
                bodies.append(m_translationHandle.y.body);
            if (value & kZ)
                bodies.append(m_translationHandle.z.body);
        }
        if (value & kRotate) {
            if (value & kX)
                bodies.append(m_rotationHandle.x.body);
            if (value & kY)
                bodies.append(m_rotationHandle.y.body);
            if (value & kZ)
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

void Handles::drawImageHandles(IBone *bone)
{
    if (!m_visible)
        return;
    if (bone && bone->isMovable()) {
        m_helper->draw(m_x.enableMove.rect, m_x.enableMove.textureID);
        m_helper->draw(m_y.enableMove.rect, m_y.enableMove.textureID);
        m_helper->draw(m_z.enableMove.rect, m_z.enableMove.textureID);
    }
    else {
        m_helper->draw(m_x.disableMove.rect, m_x.disableMove.textureID);
        m_helper->draw(m_y.disableMove.rect, m_y.disableMove.textureID);
        m_helper->draw(m_z.disableMove.rect, m_z.disableMove.textureID);
    }
    if (bone && bone->isRotateable()) {
        m_helper->draw(m_x.enableRotate.rect, m_x.enableRotate.textureID);
        m_helper->draw(m_y.enableRotate.rect, m_y.enableRotate.textureID);
        m_helper->draw(m_z.enableRotate.rect, m_z.enableRotate.textureID);
    }
    else {
        m_helper->draw(m_x.disableRotate.rect, m_x.disableRotate.textureID);
        m_helper->draw(m_y.disableRotate.rect, m_y.disableRotate.textureID);
        m_helper->draw(m_z.disableRotate.rect, m_z.disableRotate.textureID);
    }
    switch (m_constraint) {
    case kView:
        m_helper->draw(m_view.rect, m_view.textureID);
        break;
    case kLocal:
        m_helper->draw(m_local.rect, m_local.textureID);
        break;
    case kGlobal:
        m_helper->draw(m_global.rect, m_global.textureID);
        break;
    case Handles::kNone:
    case Handles::kEnable:
    case Handles::kDisable:
    case Handles::kMove:
    case Handles::kRotate:
    case Handles::kX:
    case Handles::kY:
    case Handles::kZ:
    case Handles::kModel:
    case Handles::kVisibleMove:
    case Handles::kVisibleRotate:
    case Handles::kVisibleAll:
    default:
        break;
    }
}

void Handles::drawRotationHandle(const IModel *model)
{
    if (!m_visible || !m_program.isLinked() || !m_boneRef)
        return;
    glDisable(GL_DEPTH_TEST);
    m_program.bind();
    UIInitializeRenderingModel(m_loaderRef, model, modelHandleTransform(), &m_program);
    if (m_boneRef->isRotateable() && m_visibilityFlags & kRotate) {
        drawModel(m_rotationHandle.x, kRed, kX);
        drawModel(m_rotationHandle.y, kGreen, kY);
        drawModel(m_rotationHandle.z, kBlue, kZ);
    }
    m_program.release();
    glEnable(GL_DEPTH_TEST);
}

void Handles::drawMoveHandle(const IModel *model)
{
    if (!m_visible || !m_program.isLinked() || !m_boneRef)
        return;
    glDisable(GL_DEPTH_TEST);
    m_program.bind();
    UIInitializeRenderingModel(m_loaderRef, model, modelHandleTransform(), &m_program);
    if (m_boneRef->isMovable() && m_visibilityFlags & kMove) {
        drawModel(m_translationHandle.x, kRed, kX);
        drawModel(m_translationHandle.y, kGreen, kY);
        drawModel(m_translationHandle.z, kBlue, kZ);
        drawModel(m_translationHandle.axisX, kRed, kX);
        drawModel(m_translationHandle.axisY, kGreen, kY);
        drawModel(m_translationHandle.axisZ, kBlue, kZ);
    }
    m_program.release();
    glEnable(GL_DEPTH_TEST);
}

void Handles::drawModel(const Handles::Model &model,
                        const QColor &color,
                        int requiredVisibilityFlags)
{
    if (m_visibilityFlags & requiredVisibilityFlags) {
        const Handles::Vertex &ptr = model.vertices.at(0);
        const GLfloat *vertexPtr = reinterpret_cast<const GLfloat *>(&ptr.position.x());
        int inPosition = m_program.attributeLocation("inPosition");
        m_program.setUniformValue("color", &model == m_trackedHandleRef ? kYellow : color);
        m_program.enableAttributeArray(inPosition);
        m_program.setAttributeArray(inPosition, vertexPtr, 4, sizeof(Handles::Vertex));
        glDrawElements(GL_TRIANGLES, model.indices.count(), GL_UNSIGNED_SHORT, &model.indices[0]);
        m_program.disableAttributeArray(inPosition);
    }
}

void Handles::loadImageHandles()
{
    QImage image;
    QGLContext *context = const_cast<QGLContext *>(QGLContext::currentContext());
    image.load(":icons/x-enable-move.png");
    m_x.enableMove.size = image.size();
    m_x.enableMove.textureID = context->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
    image.load(":icons/x-enable-rotate.png");
    m_x.enableRotate.size = image.size();
    m_x.enableRotate.textureID = context->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
    image.load(":icons/y-enable-move.png");
    m_y.enableMove.size = image.size();
    m_y.enableMove.textureID = context->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
    image.load(":icons/y-enable-rotate.png");
    m_y.enableRotate.size = image.size();
    m_y.enableRotate.textureID = context->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
    image.load(":icons/z-enable-move.png");
    m_z.enableMove.size = image.size();
    m_z.enableMove.textureID = context->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
    image.load(":icons/z-enable-rotate.png");
    m_z.enableRotate.size = image.size();
    m_z.enableRotate.textureID = context->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
    image.load(":icons/x-disable-move.png");
    m_x.disableMove.size = image.size();
    m_x.disableMove.textureID = context->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
    image.load(":icons/x-disable-rotate.png");
    m_x.disableRotate.size = image.size();
    m_x.disableRotate.textureID = context->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
    image.load(":icons/y-disable-move.png");
    m_y.disableMove.size = image.size();
    m_y.disableMove.textureID = context->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
    image.load(":icons/y-disable-rotate.png");
    m_y.disableRotate.size = image.size();
    m_y.disableRotate.textureID = context->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
    image.load(":icons/z-disable-move.png");
    m_z.disableMove.size = image.size();
    m_z.disableMove.textureID = context->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
    image.load(":icons/z-disable-rotate.png");
    m_z.disableRotate.size = image.size();
    m_z.disableRotate.textureID = context->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
    image.load(":icons/global.png");
    m_global.size = image.size();
    m_global.textureID = context->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
    image.load(":icons/local.png");
    m_local.size = image.size();
    m_local.textureID = context->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
    image.load(":icons/view.png");
    m_view.size = image.size();
    m_view.textureID = context->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
}

void Handles::loadModelHandles()
{
    /* 回転軸ハンドル (3つのドーナツ状のメッシュが入ってる) */
    QFile rotationHandleFile(":models/rotation.3ds");
    if (rotationHandleFile.open(QFile::ReadOnly)) {
        const QByteArray &rotationHandleBytes = rotationHandleFile.readAll();
        const uint8_t *data = reinterpret_cast<const uint8_t *>(rotationHandleBytes.constData());
        size_t size =  rotationHandleBytes.size();
        const aiScene *scene = m_rotationHandle.importer.ReadFileFromMemory(data, size, aiProcessPreset_TargetRealtime_Fast);
        aiMesh **meshes = scene->mMeshes;
        UILoadTrackableModel(meshes[1], m_world.data(), new BoneHandleMotionState(this), m_rotationHandle.x);
        UILoadTrackableModel(meshes[0], m_world.data(), new BoneHandleMotionState(this), m_rotationHandle.y);
        UILoadTrackableModel(meshes[2], m_world.data(), new BoneHandleMotionState(this), m_rotationHandle.z);
    }
    /* 移動軸ハンドル (3つのコーン状のメッシュと3つの細長いシリンダー計6つのメッシュが入ってる) */
    QFile translationHandleFile(":models/translation.3ds");
    if (translationHandleFile.open(QFile::ReadOnly)) {
        const QByteArray &translationHandleBytes = translationHandleFile.readAll();
        const uint8_t *data = reinterpret_cast<const uint8_t *>(translationHandleBytes.constData());
        size_t size =  translationHandleBytes.size();
        const aiScene *scene = m_translationHandle.importer.ReadFileFromMemory(data, size, aiProcessPreset_TargetRealtime_Fast);
        aiMesh **meshes = scene->mMeshes;
        UILoadTrackableModel(meshes[0], m_world.data(), new BoneHandleMotionState(this), m_translationHandle.x);
        UILoadTrackableModel(meshes[2], m_world.data(), new BoneHandleMotionState(this), m_translationHandle.y);
        UILoadTrackableModel(meshes[1], m_world.data(), new BoneHandleMotionState(this), m_translationHandle.z);
        UILoadStaticModel(meshes[3], m_translationHandle.axisX);
        UILoadStaticModel(meshes[5], m_translationHandle.axisY);
        UILoadStaticModel(meshes[4], m_translationHandle.axisZ);
    }
}

} /* namespace vpvm */
