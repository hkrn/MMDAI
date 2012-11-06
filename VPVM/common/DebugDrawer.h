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

#ifndef VPVM_DEBUGDRAWER_H_
#define VPVM_DEBUGDRAWER_H_

#include "vpvl2/qt/CString.h"
#include "SceneLoader.h"
#include "SceneWidget.h"
#include "VertexBundle.h"
#include "util.h"

#include <QtCore/QObject>
#include <btBulletDynamicsCommon.h>
#include <vpvl2/Common.h>
#include <vpvl2/qt/World.h>

namespace vpvm {

using namespace vpvl2;
using namespace vpvl2::qt;

class DebugDrawer : public btIDebugDraw
{
public:
    typedef QSet<const IBone *> BoneSet;
    static const Scalar kLength = 2.0;
    static const Vector3 kRed;
    static const Vector3 kGreen;
    static const Vector3 kBlue;

    DebugDrawer()
        : m_vbo(QGLBuffer::VertexBuffer),
          m_ibo(QGLBuffer::IndexBuffer),
          m_flags(0),
          m_visible(true),
          m_initialized(false)
    {
    }
    ~DebugDrawer() {
    }

    void draw3dText(const btVector3 & /* location */, const char *textString) {
        qDebug("[INFO]: %s\n", textString);
    }
    void drawContactPoint(const btVector3 &PointOnB,
                          const btVector3 &normalOnB,
                          btScalar distance,
                          int /* lifeTime */,
                          const btVector3 &color)
    {
        drawLine(PointOnB, PointOnB + normalOnB * distance, color);
    }
    void drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color) {
        Vertex vertices[2];
        vertices[0].set(from, color);
        vertices[1].set(to, color);
        m_vbo.write(0, &vertices[0], sizeof(vertices));
        glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 0);
    }
    void drawLine(const btVector3 &from,
                  const btVector3 &to,
                  const btVector3 &fromColor,
                  const btVector3 & /* toColor */)
    {
        drawLine(from, to, fromColor);
    }
    void reportErrorWarning(const char *warningString) {
        qWarning("[ERROR]: %s\n", warningString);
    }
    int getDebugMode() const {
        return m_flags;
    }
    void setDebugMode(int debugMode) {
        m_flags = debugMode;
    }

    void load() {
        if (!m_initialized) {
            m_bundle.initialize(QGLContext::currentContext());
            m_program.addShaderFromSourceFile(QGLShader::Vertex, ":shaders/grid.vsh");
            m_program.addShaderFromSourceFile(QGLShader::Fragment, ":shaders/grid.fsh");
            m_program.link();
            m_vbo.setUsagePattern(QGLBuffer::DynamicDraw);
            m_vbo.create();
            m_vbo.bind();
            m_vbo.allocate(sizeof(Vertex) * 2);
            m_vbo.release();
            m_ibo.setUsagePattern(QGLBuffer::StaticDraw);
            m_ibo.create();
            m_ibo.bind();
            static const int indices[] = { 0, 1 };
            m_ibo.allocate(indices, sizeof(indices));
            m_ibo.release();
            m_bundle.create();
            m_bundle.bind();
            bindVertexBundle(false);
            m_bundle.release();
            releaseVertexBundle(false);
            m_initialized = true;
        }
    }
    void setVisible(bool value) {
        m_visible = value;
    }
    void drawShape(btDiscreteDynamicsWorld *world,
                   btCollisionShape *shape,
                   const SceneLoader *loader,
                   const btTransform &transform,
                   const btVector3 &color) {
        if (m_program.isLinked()) {
            beginDrawing(loader, 0);
            world->debugDrawObject(transform, shape, color);
            flushDrawing();
        }
    }
    void drawWorld(const SceneLoader *loader) {
        if (m_program.isLinked()) {
            World *world = loader->worldRef();
            btDiscreteDynamicsWorld *dynamicWorldRef = world->dynamicWorldRef();
            btIDebugDraw *drawer = dynamicWorldRef->getDebugDrawer();
            setDebugMode(DBG_DrawWireframe | DBG_DrawAabb | DBG_DrawConstraints);
            dynamicWorldRef->setDebugDrawer(this);
            beginDrawing(loader, 0);
            dynamicWorldRef->debugDrawWorld();
            flushDrawing();
            setDebugMode(DBG_NoDebug);
            dynamicWorldRef->setDebugDrawer(drawer);
        }
    }

    void drawModelBones(const IModel *model, const SceneLoader *loader, const BoneSet &selectedBones) {
        if (!model || !m_visible || !m_program.isLinked())
            return;
        Array<IBone *> bones;
        model->getBoneRefs(bones);
        const int nbones = bones.count();
        /* シェーダのパラメータ設定 */
        beginDrawing(loader, model);
        /* IK ボーンの収集 */
        BoneSet bonesForIK;
        Array<IBone *> linkedBones;
        for (int i = 0; i < nbones; i++) {
            IBone *bone = bones[i];
            if (bone->hasInverseKinematics()) {
                linkedBones.clear();
                bonesForIK.insert(bone);
                bonesForIK.insert(bone->targetBoneRef());
                bone->getEffectorBones(linkedBones);
                const int nlinks = linkedBones.count();
                for (int j = 0; j < nlinks; j++) {
                    IBone *linkedBone = linkedBones[j];
                    bonesForIK.insert(linkedBone);
                }
            }
        }
        const IBone *specialBone = findSpecialBone(model);
        const Vector3 &origin = specialBone ? specialBone->worldTransform().getOrigin() : kZeroV3;
        /* ボーンの表示(レンダリング) */
        for (int i = 0; i < nbones; i++) {
            const IBone *bone = bones[i];
            /* 操作不可能の場合はスキップ */
            if (!bone->isInteractive())
                continue;
            /* 全ての親ボーンもとい特殊枠にあるボーンへの接続表示対策 */
            bool skipDrawingLine = (bone->destinationOrigin() == origin);
            drawBone(bone, selectedBones, bonesForIK, skipDrawingLine);
        }
        flushDrawing();
    }
    void drawMovableBone(const IBone *bone, const IModel *model, const SceneLoader *loader) {
        if (!bone || !bone->isMovable() || !m_program.isLinked())
            return;
        beginDrawing(loader, model);
        drawSphere(bone->worldTransform().getOrigin(), 0.5, Vector3(0.0f, 1.0f, 1.0f));
        flushDrawing();
    }
    void drawBoneTransform(const IBone *bone, const IModel *model, const SceneLoader *loader, int mode) {
        /* 固定軸がある場合は軸表示なし */
        if (!m_visible || !bone || !m_program.isLinked() || bone->hasFixedAxes())
            return;
        /* ボーン表示 */
        BoneSet selectedBones;
        selectedBones.insert(bone);
        //drawBone(bone, selectedBones, BoneSet(), false);
        /* シェーダのパラメータ設定 */
        beginDrawing(loader, model);
        /* 軸表示 */
        if (mode == 'V') {
            /* モデルビュー行列を元に軸表示 */
            const Transform &transform = bone->worldTransform();
            const Vector3 &origin = bone->worldTransform().getOrigin();
            QMatrix4x4 world, view, projection;
            loader->getCameraMatrices(world, view, projection);
            drawLine(origin, transform * (vec2vec(view.row(0)) * kLength), kRed);
            drawLine(origin, transform * (vec2vec(view.row(1)) * kLength), kGreen);
            drawLine(origin, transform * (vec2vec(view.row(2)) * kLength), kBlue);
        }
        else if (mode == 'L') {
            if (bone->hasLocalAxes()) {
                /* 子ボーンの方向をX軸、手前の方向をZ軸として設定する */
                const Transform &transform = bone->worldTransform();
                const Vector3 &origin = transform.getOrigin();
                Matrix3x3 axes = Matrix3x3::getIdentity();
                bone->getLocalAxes(axes);
                drawLine(origin, transform * (axes[0] * kLength), kRed);
                drawLine(origin, transform * (axes[1] * kLength), kGreen);
                drawLine(origin, transform * (axes[2] * kLength), kBlue);
            }
            else {
                /* 現在のボーン位置と回転量を乗算した軸を表示 */
                const Transform &transform = bone->worldTransform();
                const Vector3 &origin = transform.getOrigin();
                drawLine(origin, transform * Vector3(kLength, 0, 0), kRed);
                drawLine(origin, transform * Vector3(0, kLength, 0), kGreen);
                drawLine(origin, transform * Vector3(0, 0, kLength), kBlue);
            }
        }
        else {
            /* 現在のボーン位置に対する固定軸を表示 */
            const Vector3 &origin = bone->worldTransform().getOrigin();
            drawLine(origin, origin + Vector3(kLength, 0, 0), kRed);
            drawLine(origin, origin + Vector3(0, kLength, 0), kGreen);
            drawLine(origin, origin + Vector3(0, 0, kLength), kBlue);
        }
        flushDrawing();
    }

private:
    struct Vertex {
        Vertex() {}
        Vertex(const Vector3 &v, const QColor &c)
            : position(v),
              color(c.redF(), c.greenF(), c.blueF())
        {
        }
        void set(const Vector3 &v, const Vector3 &c) {
            position = v;
            color = c;
        }
        Vector3 position;
        Vector3 color;
    };
    static const IBone *findSpecialBone(const IModel *model) {
        static const CString kRoot("Root");
        Array<ILabel *> labels;
        model->getLabelRefs(labels);
        const int nlabels = labels.count();
        for (int i = 0; i < nlabels; i++) {
            const ILabel *label = labels[i];
            const int nchildren = label->count();
            if (label->isSpecial()) {
                /* 特殊枠でかつ先頭ボーンかどうか */
                if (nchildren > 0 && label->name()->equals(&kRoot)) {
                    const IBone *bone = label->bone(0);
                    return bone;
                }
            }
        }
        return 0;
    }

    void drawBone(const IBone *bone, const BoneSet &selected, const BoneSet &IK, bool skipDrawingLine) {
        static const Scalar sphereRadius = 0.2f;
        if (!bone || !bone->isVisible())
            return;
        const Vector3 &dest = bone->destinationOrigin();
        const Vector3 &origin = bone->worldTransform().getOrigin();
        Vector3 color;
        /* 選択中の場合は赤色で表示 */
        if (selected.contains(bone)) {
            drawSphere(origin, sphereRadius, Vector3(1.0f, 0.0f, 0.0f));
            color = kRed;
        }
        /* 固定軸(例:捻り)ありのボーンの場合は球体のみ紫色、接続部分を青で表示 */
        else if (bone->hasFixedAxes()) {
            drawSphere(origin, sphereRadius, Vector3(1.0, 0.0, 1.0f));
            color = kBlue;
        }
        /* IK ボーンの場合は橙色で表示 */
        else if (IK.contains(bone)) {
            drawSphere(origin, sphereRadius, Vector3(1.0f, 0.75f, 0.0f));
            color.setValue(1, 0.75, 0);
        }
        /* 上記以外の場合は青色で表示 */
        else {
            drawSphere(origin, sphereRadius, Vector3(0.0f, 0.0f, 1.0f));
            color = kBlue;
        }
        if (!skipDrawingLine) {
            /* 描写 */
            Transform tr = Transform::getIdentity();
            const Scalar &coneRadius = 0.05f;//btMin(0.1, childOrigin.distance(origin) * 0.1);
            /* ボーン接続を表示するための頂点設定 */
            tr.setOrigin(Vector3(coneRadius, 0.0f, 0.0f));
            drawLine(tr * origin, dest, color);
            tr.setOrigin(Vector3(-coneRadius, 0.0f, 0.0f));
            drawLine(tr * origin, dest, color);
        }
    }
    void beginDrawing(const SceneLoader *loader, const IModel *model) {
        QMatrix4x4 world, view, projection;
        loader->getCameraMatrices(world, view, projection);
        if (model) {
            const Vector3 &position = model->position();
            const Quaternion &rotation = model->rotation();
            world.translate(position.x(), position.y(), position.z());
            world.rotate(QQuaternion(rotation.w(), rotation.x(), rotation.y(), rotation.z()));
        }
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        m_program.bind();
        m_program.setUniformValue("modelViewProjectionMatrix", projection * view * world);
        bindVertexBundle(false); // XXX: VAO doesn't work
    }
    void flushDrawing() {
        releaseVertexBundle(false);  // XXX: VAO doesn't work
        m_program.release();
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
    }
    void bindVertexBundle(bool bundle) {
        if (!bundle || !m_bundle.bind()) {
            m_vbo.bind();
            m_ibo.bind();
            m_program.setAttributeBuffer("inPosition", GL_FLOAT, 0, 3, sizeof(Vertex));
            m_program.setAttributeBuffer("inColor", GL_FLOAT, 16, 3, sizeof(Vertex));
            m_program.enableAttributeArray("inPosition");
            m_program.enableAttributeArray("inColor");
        }
    }
    void releaseVertexBundle(bool bundle) {
        if (!bundle || !m_bundle.release()) {
            m_vbo.release();
            m_ibo.release();
        }
    }

    QGLShaderProgram m_program;
    VertexBundle m_bundle;
    QGLBuffer m_vbo;
    QGLBuffer m_ibo;
    int m_flags;
    bool m_visible;
    bool m_initialized;

    Q_DISABLE_COPY(DebugDrawer)
};

const float DebugDrawer::kLength;
const Vector3 DebugDrawer::kRed   = Vector3(1, 0, 0);
const Vector3 DebugDrawer::kGreen = Vector3(0, 1, 0);
const Vector3 DebugDrawer::kBlue  = Vector3(0, 0, 1);

} /* namespace vpvm */

#endif // DEBUGDRAWER_H
