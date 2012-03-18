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

#ifndef DEBUGDRAWER_H
#define DEBUGDRAWER_H

#include "SceneLoader.h"
#include "SceneWidget.h"
#include "util.h"

#include <QtCore/QObject>
#include <btBulletDynamicsCommon.h>
#include <vpvl/vpvl.h>
#include <vpvl/gl2/Renderer.h>

namespace internal {

class DebugDrawer : public btIDebugDraw
{
public:
    DebugDrawer(vpvl::Scene *scene)
        : m_scene(scene),
          m_flags(0),
          m_visible(true)
    {}
    virtual ~DebugDrawer() {}

    static void setLine(const btVector3 &from, const btVector3 &to, QVector3D *lines) {
        lines[0].setX(from.x());
        lines[0].setY(from.y());
        lines[0].setZ(from.z());
        lines[1].setX(to.x());
        lines[1].setY(to.y());
        lines[1].setZ(to.z());
    }
    void draw3dText(const btVector3 & /* location */, const char *textString) {
        fprintf(stderr, "[INFO]: %s\n", textString);
    }
    void drawContactPoint(const btVector3 &PointOnB, const btVector3 &normalOnB, btScalar distance, int /* lifeTime */, const btVector3 &color) {
        QVector3D lines[2];
        setLine(PointOnB, PointOnB + normalOnB * distance, lines);
        m_program.setUniformValue("color", QColor::fromRgb(color.x() * 255, color.y() * 255, color.z() * 255));
        m_program.setAttributeArray("inPosition", lines);
        glDrawArrays(GL_LINES, 0, 2);
    }
    void drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color) {
        QVector3D lines[2];
        setLine(from, to, lines);
        m_program.setUniformValue("color", QColor::fromRgb(color.x() * 255, color.y() * 255, color.z() * 255));
        m_program.setAttributeArray("inPosition", lines);
        glDrawArrays(GL_LINES, 0, 2);
    }
    void drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &fromColor, const btVector3 & /* toColor */) {
        QVector3D lines[2];
        setLine(from, to, lines);
        m_program.setUniformValue("color", QColor::fromRgb(fromColor.x() * 255, fromColor.y() * 255, fromColor.z() * 255));
        m_program.setAttributeArray("inPosition", lines);
        glDrawArrays(GL_LINES, 0, 2);
    }
    void reportErrorWarning(const char *warningString) {
        fprintf(stderr, "[ERROR]: %s\n", warningString);
    }
    int getDebugMode() const {
        return m_flags;
    }
    void setDebugMode(int debugMode) {
        m_flags = debugMode;
    }

    void initialize() {
        m_program.addShaderFromSourceFile(QGLShader::Vertex, ":shaders/handle.vsh");
        m_program.addShaderFromSourceFile(QGLShader::Fragment, ":shaders/handle.fsh");
        m_program.link();
    }
    void setVisible(bool value) {
        m_visible = value;
    }

    void drawModelBones(const vpvl::PMDModel *model, const QList<vpvl::Bone *> &selected) {
        if (!m_visible || !model || !m_program.isLinked())
            return;
        vpvl::Array<vpvl::Vector3> vertices;
        static const int indices[] = {
            0, 1, 2, 3
        };
        const vpvl::BoneList &bones = model->bones();
        const int nbones = bones.count();
        float matrix[16];
        vpvl::Transform tr = vpvl::Transform::getIdentity();
        QGLFunctions func(QGLContext::currentContext());
        glDisable(GL_DEPTH_TEST);
        m_program.bind();
        m_scene->getModelViewMatrix(matrix);
        int modelViewMatrix = m_program.uniformLocation("modelViewMatrix");
        func.glUniformMatrix4fv(modelViewMatrix, 1, GL_FALSE, matrix);
        m_scene->getProjectionMatrix(matrix);
        int projectionMatrix = m_program.uniformLocation("projectionMatrix");
        func.glUniformMatrix4fv(projectionMatrix, 1, GL_FALSE, matrix);
        m_program.setUniformValue("boneMatrix", QMatrix4x4());
        m_program.enableAttributeArray("inPosition");
        QSet<vpvl::Bone *> linkedIKBones, targetBones, destinationBones;
        const vpvl::IKList &IKs = model->IKs();
        int nIKs = IKs.count();
        for (int i = 0; i < nIKs; i++) {
            vpvl::IK *ik = IKs[i];
            const vpvl::BoneList &bones = ik->linkedBones();
            int nbones = bones.count();
            targetBones.insert(ik->targetBone());
            destinationBones.insert(ik->destinationBone());
            for (int j = 0; j < nbones; j++) {
                vpvl::Bone *bone = bones[j];
                linkedIKBones.insert(bone);
            }
        }
        static const QColor kColorRed = QColor::fromRgbF(1.0, 0.0, 0.0);
        static const QColor kColorOrange = QColor::fromRgbF(1.0, 0.75, 0.0);
        static const QColor kColorBlue = QColor::fromRgbF(0.0, 0.0, 1.0);
        for (int i = 0; i < nbones; i++) {
            const vpvl::Bone *bone = bones[i], *child = bone->child();
            if (!bone->isMovable() && !bone->isRotateable())
                continue;
            /* 子ボーンが「全ての親」の場合はスキップしておく */
            if (child->originPosition() == vpvl::kZeroV)
                continue;
            const vpvl::Transform &boneTransform = bone->localTransform(),
                    &childTransform = child->localTransform();
            const vpvl::Vector3 &origin = boneTransform.getOrigin(),
                    &childOrigin = childTransform.getOrigin();
            const vpvl::Scalar &s = 0.075f;//btMin(0.1, childOrigin.distance(origin) * 0.1);
            vertices.clear();
            tr.setOrigin(vpvl::Vector3(s, 0.0f, 0.0f));
            vertices.add(tr * origin);
            vertices.add(childOrigin);
            tr.setOrigin(vpvl::Vector3(-s, 0.0f, 0.0f));
            vertices.add(tr * origin);
            vertices.add(childOrigin);
            vpvl::Bone *mutableBone = const_cast<vpvl::Bone *>(bone);
            if (selected.contains(mutableBone)) {
                drawSphere(origin, 0.1f, vpvl::Vector3(1.0f, 0.0f, 0.0f));
                m_program.setUniformValue("color", kColorRed);
            }
            else if (linkedIKBones.contains(mutableBone)) {
                drawSphere(origin, 0.1f, vpvl::Vector3(1.0f, 0.75f, 0.0f));
                m_program.setUniformValue("color", kColorOrange);
            }
            else {
                drawSphere(origin, 0.1f, vpvl::Vector3(0.0f, 0.0f, 1.0f));
                m_program.setUniformValue("color", kColorBlue);
            }
            m_program.setAttributeArray("inPosition",
                                        reinterpret_cast<const GLfloat *>(&vertices[0]),
                                        3,
                                        sizeof(vpvl::Vector3));
            glDrawElements(GL_LINE_LOOP, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_INT, indices);
        }
        m_program.release();
        glEnable(GL_DEPTH_TEST);
    }
    void drawBoneTransform(vpvl::Bone *bone) {
        if (!m_visible || !bone || !m_program.isLinked())
            return;
        float matrix[16];
        QGLFunctions func(QGLContext::currentContext());
        glDisable(GL_DEPTH_TEST);
        m_program.bind();
        m_scene->getModelViewMatrix(matrix);
        int modelViewMatrix = m_program.uniformLocation("modelViewMatrix");
        func.glUniformMatrix4fv(modelViewMatrix, 1, GL_FALSE, matrix);
        m_scene->getProjectionMatrix(matrix);
        int projectionMatrix = m_program.uniformLocation("projectionMatrix");
        func.glUniformMatrix4fv(projectionMatrix, 1, GL_FALSE, matrix);
        m_program.setUniformValue("boneMatrix", QMatrix4x4());
        m_program.enableAttributeArray("inPosition");
        const QString &name = internal::toQString(bone);
        const vpvl::Bone *child = bone->child();
        if ((name.indexOf("指") != -1
             || name.endsWith("腕")
             || name.endsWith("ひじ")
             || name.endsWith("手首")
             ) && child) {
            /* 子ボーンの方向をX軸、手前の方向をZ軸として設定する */
            const vpvl::Vector3 &boneOrigin = bone->originPosition();
            const vpvl::Vector3 &childOrigin = child->originPosition();
            /* 外積を使ってそれぞれの軸を求める */
            const vpvl::Vector3 &axisX = (childOrigin - boneOrigin).normalized();
            vpvl::Vector3 tmp1 = axisX;
            name.startsWith("左") ? tmp1.setY(-axisX.y()) : tmp1.setX(-axisX.x());
            vpvl::Vector3 axisZ = axisX.cross(tmp1).normalized(), tmp2 = axisX;
            tmp2.setZ(-axisZ.z());
            vpvl::Vector3 axisY = tmp2.cross(-axisX).normalized();
#if 1
            const vpvl::Transform &transform = bone->localTransform();
            const vpvl::Vector3 &origin = transform.getOrigin();
            drawLine(origin, transform * (axisX * 2), vpvl::Vector3(1, 0, 0));
            drawLine(origin, transform * (axisY * 2), vpvl::Vector3(0, 1, 0));
            drawLine(origin, transform * (axisZ * 2), vpvl::Vector3(0, 0, 1));
#else
            btMatrix3x3 matrix(
                        axisX.x(), axisX.y(), axisX.z(),
                        axisY.x(), axisY.y(), axisY.z(),
                        axisZ.x(), axisZ.y(), axisZ.z()
                        );
            const vpvl::Vector3 &origin = bone->localTransform().getOrigin();
            vpvl::Transform transform(matrix, pos);
            drawLine(origin, transform * vpvl::Vector3(1, 0, 0), vpvl::Vector3(1, 0, 0));
            drawLine(origin, transform * vpvl::Vector3(0, 1, 0), vpvl::Vector3(0, 1, 0));
            drawLine(origin, transform * vpvl::Vector3(0, 0, 1), vpvl::Vector3(0, 0, 1));
#endif
        }
        else {
            const vpvl::Transform &transform = bone->localTransform();
            const vpvl::Vector3 &origin = transform.getOrigin();
            drawLine(origin, transform * vpvl::Vector3(2, 0, 0), vpvl::Vector3(1, 0, 0));
            drawLine(origin, transform * vpvl::Vector3(0, 2, 0), vpvl::Vector3(0, 1, 0));
            drawLine(origin, transform * vpvl::Vector3(0, 0, 2), vpvl::Vector3(0, 0, 1));
        }
        m_program.release();
    }

private:
    QGLShaderProgram m_program;
    vpvl::Scene *m_scene;
    int m_flags;
    bool m_visible;

    Q_DISABLE_COPY(DebugDrawer)
};

}

#endif // DEBUGDRAWER_H
