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

Handles::Handles(SceneWidget *parent)
    : m_world(0),
      m_widget(parent),
      m_width(0),
      m_height(0),
      m_enableMove(false),
      m_enableRotate(false),
      m_isLocal(true),
      m_visible(true)
{
    m_world = new internal::World(30);
    m_rotationHandle.asset = 0;
    m_translateHandle.asset = 0;
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
    delete m_translateHandle.asset;
}

void Handles::load()
{
    loadImageHandles();
    m_program.addShaderFromSourceFile(QGLShader::Vertex, ":shaders/handle.vsh");
    m_program.addShaderFromSourceFile(QGLShader::Fragment, ":shaders/handle.fsh");
    if (m_program.link())
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
    m_world->mutableWorld()->rayTest(rayFrom, rayTo, callback);
    if (callback.hasHit()) {
        btRigidBody *body = btRigidBody::upcast(callback.m_collisionObject);
        Q_UNUSED(body);
        btVector3 pick = callback.m_hitPointWorld;
        Handles::Model *model = static_cast<Handles::Model *>(body->getUserPointer());
        /* TODO: implement emit signal of rotation handles */
        if (model == &m_rotationHandle.x)
            qDebug() << "X" << pick.x() << pick.y() << pick.z();
        else if (model == &m_rotationHandle.y)
            qDebug() << "Y" << pick.x() << pick.y() << pick.z();
        else if (model == &m_rotationHandle.z)
            qDebug() << "Z" << pick.x() << pick.y() << pick.z();
    }
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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    drawModelHandles();
    drawImageHandles();
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
    float m[16];
    const vpvl::Scene *scene = m_widget->scene();
#if 0
    int modelViewMatrix = m_program.uniformLocation("modelViewMatrix");
    int projectionMatrix = m_program.uniformLocation("projectionMatrix");
    int lightPosition = m_program.uniformLocation("lightPosition");
    m_program.bind();
    scene->getModelViewMatrix(m);
    m_program.setUniformValue(modelViewMatrix, internal::toMatrix4x4(m));
    scene->getProjectionMatrix(m);
    m_program.setUniformValue(projectionMatrix, internal::toMatrix4x4(m));
    drawModelHandle(m_rotationHandle.x, QColor::fromRgb(255, 0, 0));
    m_program.release();
#else
    glMatrixMode(GL_PROJECTION);
    scene->getProjectionMatrix(m);
    glLoadMatrixf(m);
    glMatrixMode(GL_MODELVIEW);
    scene->getModelViewMatrix(m);
    glLoadMatrixf(m);
    drawModelHandle(m_rotationHandle.x, QColor::fromRgb(255, 0, 0));
    drawModelHandle(m_rotationHandle.y, QColor::fromRgb(0, 255, 0));
    drawModelHandle(m_rotationHandle.z, QColor::fromRgb(0, 0, 255));
#endif
}

void Handles::drawModelHandle(const Handles::Model &model, const QColor &color)
{
    const Handles::Vertex &ptr = model.vertices.at(0);
    const GLfloat *vertexPtr = reinterpret_cast<const GLfloat *>(&ptr.position.x());
    const GLfloat *normalPtr = reinterpret_cast<const GLfloat *>(&ptr.normal.x());
#if 0
    int inPosition = m_program.attributeLocation("inPosition");
    m_program.setUniformValue("color", color);
    m_program.enableAttributeArray(inPosition);
    m_program.setAttributeArray(inPosition, vertexPtr, 3, sizeof(Handles::Vertex));
    glDrawElements(GL_TRIANGLES, model.indices.count(), GL_UNSIGNED_SHORT, &model.indices[0]);
    m_program.disableAttributeArray(inPosition);
#else
    Q_UNUSED(color);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(Handles::Vertex), vertexPtr);
    glNormalPointer(GL_FLOAT, sizeof(Handles::Vertex), normalPtr);
    glDrawElements(GL_TRIANGLES, model.indices.count(), GL_UNSIGNED_SHORT, &model.indices[0]);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
#endif
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

static void LoadHandleModel(const aiMesh *mesh, Handles::Model &model, internal::World *world)
{
    const aiVector3D *meshVertices = mesh->mVertices;
    const aiVector3D *meshNormals = mesh->mNormals;
    const unsigned int nfaces = mesh->mNumFaces;
    int index = 0;
    btTriangleMesh *triangleMesh = new btTriangleMesh();
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
        triangleMesh->addTriangle(model.vertices.at(i * 3 + 0).position,
                                  model.vertices.at(i * 3 + 1).position,
                                  model.vertices.at(i * 3 + 2).position);
    }
    const btScalar &mass = 0.0f;
    const btVector3 localInertia(0.0f, 0.0f, 0.0f);
    btBvhTriangleMeshShape *shape = new btBvhTriangleMeshShape(triangleMesh, true);
    btDefaultMotionState *state = new btDefaultMotionState();
    btRigidBody::btRigidBodyConstructionInfo info(mass, state, shape, localInertia);
    btRigidBody *body = new btRigidBody(info);
    body->setUserPointer(&model);
    world->addTriangleMeshShape(shape, triangleMesh);
    world->mutableWorld()->addRigidBody(body);
}

void Handles::loadModelHandles()
{
    QFile rotationHandleFile(":models/rotation.3ds");
    if (rotationHandleFile.open(QFile::ReadOnly)) {
        const QByteArray &rotationHandleBytes = rotationHandleFile.readAll();
        vpvl::Asset *asset = new vpvl::Asset();
        asset->load(reinterpret_cast<const uint8_t *>(rotationHandleBytes.constData()), rotationHandleBytes.size());
        const aiScene *scene = asset->getScene();
        LoadHandleModel(scene->mMeshes[0], m_rotationHandle.x, m_world);
        LoadHandleModel(scene->mMeshes[1], m_rotationHandle.y, m_world);
        LoadHandleModel(scene->mMeshes[2], m_rotationHandle.z, m_world);
        m_rotationHandle.asset = asset;
    }
    QFile translateHandleFile(":models/translation.3ds");
    if (translateHandleFile.open(QFile::ReadOnly)) {
        const QByteArray &translateHandleBytes = translateHandleFile.readAll();
        vpvl::Asset *asset = new vpvl::Asset();
        asset->load(reinterpret_cast<const uint8_t *>(translateHandleBytes.constData()), translateHandleBytes.size());
        // const aiScene *scene = handle->getScene();
        // LoadHandleModel(scene->mMeshes[0], m_translateHandle.x);
        // LoadHandleModel(scene->mMeshes[1], m_translateHandle.y);
        // LoadHandleModel(scene->mMeshes[2], m_translateHandle.z);
        m_translateHandle.asset = asset;
    }
}
