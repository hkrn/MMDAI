/**

 Copyright (c) 2010-2013  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#include "vpvl2/qt/DebugDrawer.h"

#include <QtCore>
#include "vpvl2/vpvl2.h"
#include "vpvl2/extensions/World.h"
#include "vpvl2/extensions/gl/ShaderProgram.h"
#include "vpvl2/extensions/gl/VertexBundle.h"
#include "vpvl2/extensions/gl/VertexBundleLayout.h"
#include "vpvl2/extensions/icu4c/String.h"
#include "vpvl2/extensions/icu4c/StringMap.h"
#include "vpvl2/qt/Util.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshift-op-parentheses"
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#pragma clang diagnostic pop

#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>

namespace vpvl2
{
namespace qt
{

using namespace vpvl2;
using namespace vpvl2::extensions;
using namespace vpvl2::extensions::icu4c;
using namespace vpvl2::qt;

const Scalar DebugDrawer::kLength  = 2.0f;
const Vector3 DebugDrawer::kRed   = Vector3(1, 0, 0);
const Vector3 DebugDrawer::kGreen = Vector3(0, 1, 0);
const Vector3 DebugDrawer::kBlue  = Vector3(0, 0, 1);

class DebugDrawer::PrivateShaderProgram : public ShaderProgram {
public:
    enum VertexType {
        kPosition,
        kColor
    };

    PrivateShaderProgram()
        : ShaderProgram(),
          m_modelViewProjectionMatrix(-1)
    {
    }
    ~PrivateShaderProgram() {
        m_modelViewProjectionMatrix = -1;
    }

    void addShaderFromFile(const QString &path, GLuint type) {
        QFile file(path);
        if (file.open(QFile::ReadOnly | QFile::Unbuffered)) {
            vsize size = file.size();
            uchar *address = file.map(0, size);
            String s(UnicodeString(reinterpret_cast<const char *>(address), size));
            addShaderSource(&s, type);
            file.unmap(address);
        }
    }
    bool link() {
        glBindAttribLocation(m_program, kPosition, "inPosition");
        glBindAttribLocation(m_program, kColor, "inColor");
        bool ok = ShaderProgram::link();
        if (ok) {
            m_modelViewProjectionMatrix = glGetUniformLocation(m_program, "modelViewProjectionMatrix");
        }
        return ok;
    }
    void enableAttributes() {
        glEnableVertexAttribArray(kPosition);
        glEnableVertexAttribArray(kColor);
    }
    void setUniformValues(const float *matrix) {
        glUniformMatrix4fv(m_modelViewProjectionMatrix, 1, GL_FALSE, matrix);
    }

private:
    GLint m_modelViewProjectionMatrix;
};

DebugDrawer::DebugDrawer(const IApplicationContext *applicationContextRef, StringMap *settingsRef)
    : m_applicationContextRef(applicationContextRef),
      m_configRef(settingsRef),
      m_program(new PrivateShaderProgram()),
      m_bundle(new VertexBundle()),
      m_layout(new VertexBundleLayout()),
      m_flags(0),
      m_index(0),
      m_visible(true)
{
}

DebugDrawer::~DebugDrawer()
{
    m_flags = 0;
    m_index = 0;
    m_visible = false;
}

void DebugDrawer::draw3dText(const btVector3 & /* location */, const char *textString)
{
    qDebug("[INFO]: %s\n", textString);
}

void DebugDrawer::drawContactPoint(const btVector3 &PointOnB,
                                   const btVector3 &normalOnB,
                                   btScalar distance,
                                   int /* lifeTime */,
                                   const btVector3 &color)
{
    drawLine(PointOnB, PointOnB + normalOnB * distance, color);
}

void DebugDrawer::drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color)
{
    m_vertices.append(Vertex(from, color));
    m_vertices.append(Vertex(to, color));
    m_indices.append(m_index++);
    m_indices.append(m_index++);
}

void DebugDrawer::drawLine(const btVector3 &from,
                           const btVector3 &to,
                           const btVector3 &fromColor,
                           const btVector3 & /* toColor */)
{
    drawLine(from, to, fromColor);
}

void DebugDrawer::reportErrorWarning(const char *warningString)
{
    qWarning("[ERROR]: %s\n", warningString);
}

int DebugDrawer::getDebugMode() const
{
    return m_flags;
}

void DebugDrawer::setDebugMode(int debugMode)
{
    m_flags = debugMode;
}

void DebugDrawer::load()
{
    QDir dir(Util::toQString(m_configRef->value("dir.system.shaders",
                                                UnicodeString::fromUTF8(":shaders/gui"))));
    m_program->create();
    m_program->addShaderFromFile(dir.absoluteFilePath("grid.vsh"), GL_VERTEX_SHADER);
    m_program->addShaderFromFile(dir.absoluteFilePath("grid.fsh"), GL_FRAGMENT_SHADER);
    if (m_program->link()) {
        m_bundle->create(VertexBundle::kVertexBuffer, 0, GL_DYNAMIC_DRAW, 0, 0);
        m_bundle->create(VertexBundle::kIndexBuffer, 0, GL_DYNAMIC_DRAW, 0, 0);
        m_layout->create();
        m_layout->bind();
        bindVertexBundle(false);
        m_program->enableAttributes();
        m_layout->unbind();
        releaseVertexBundle(false);
    }
}

void DebugDrawer::setVisible(bool value)
{
    m_visible = value;
}

void DebugDrawer::drawShape(btDiscreteDynamicsWorld *world,
                            btCollisionShape *shape,
                            const btTransform &transform,
                            const btVector3 &color)
{
    if (m_program->isLinked()) {
        beginDrawing(0);
        world->debugDrawObject(transform, shape, color);
        flushDrawing();
    }
}

void DebugDrawer::drawWorld(World *world, int flags)
{
    if (m_program->isLinked()) {
        btDiscreteDynamicsWorld *dynamicWorldRef = world->dynamicWorldRef();
        btIDebugDraw *drawer = dynamicWorldRef->getDebugDrawer();
        setDebugMode(flags);
        dynamicWorldRef->setDebugDrawer(this);
        beginDrawing(0);
        dynamicWorldRef->debugDrawWorld();
        flushDrawing();
        setDebugMode(DBG_NoDebug);
        dynamicWorldRef->setDebugDrawer(drawer);
    }
}

void DebugDrawer::drawModelBones(const IModel *model, const BoneSet &selectedBones)
{
    if (!model || !m_visible || !m_program->isLinked())
        return;
    Array<IBone *> bones;
    model->getBoneRefs(bones);
    const int nbones = bones.count();
    /* シェーダのパラメータ設定 */
    beginDrawing(model);
    /* IK ボーンの収集 */
    BoneSet bonesForIK;
    Array<IBone *> linkedBones;
    for (int i = 0; i < nbones; i++) {
        IBone *bone = bones[i];
        if (bone->hasInverseKinematics()) {
            linkedBones.clear();
            bonesForIK.insert(bone);
            bonesForIK.insert(bone->effectorBoneRef());
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

void DebugDrawer::drawMovableBone(const IBone *bone, const IModel *model)
{
    if (!bone || !bone->isMovable() || !m_program->isLinked())
        return;
    beginDrawing(model);
    drawSphere(bone->worldTransform().getOrigin(), 0.5, Vector3(0.0f, 1.0f, 1.0f));
    flushDrawing();
}

void DebugDrawer::drawBoneTransform(const IBone *bone, const IModel *model, int mode)
{
    /* 固定軸がある場合は軸表示なし */
    if (!m_visible || !bone || !m_program->isLinked() || bone->hasFixedAxes())
        return;
    /* ボーン表示 */
    BoneSet selectedBones;
    selectedBones.insert(bone);
    //drawBone(bone, selectedBones, BoneSet(), false);
    /* シェーダのパラメータ設定 */
    beginDrawing(model);
    /* 軸表示 */
    if (mode == 'V') {
        /* モデルビュー行列を元に軸表示 */
        const Transform &transform = bone->worldTransform();
        const Vector3 &origin = bone->worldTransform().getOrigin();
        float viewMatrix[16];
        m_applicationContextRef->getMatrix(viewMatrix, model, IApplicationContext::kCameraMatrix | IApplicationContext::kViewMatrix);
        const glm::mat4 &view = glm::make_mat4(viewMatrix);
        const glm::vec4 &x = glm::row(view, 0), &y = glm::row(view, 1), &z = glm::row(view, 2);
        drawLine(origin, transform * (Vector3(x.x, x.y, x.z) * kLength), kRed);
        drawLine(origin, transform * (Vector3(y.x, y.y, y.z) * kLength), kGreen);
        drawLine(origin, transform * (Vector3(z.x, z.y, z.z) * kLength), kBlue);
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

const IBone *DebugDrawer::findSpecialBone(const IModel *model)
{
    static const String kRoot("Root");
    Array<ILabel *> labels;
    model->getLabelRefs(labels);
    const int nlabels = labels.count();
    for (int i = 0; i < nlabels; i++) {
        const ILabel *label = labels[i];
        const int nchildren = label->count();
        if (label->isSpecial()) {
            /* 特殊枠でかつ先頭ボーンかどうか */
            if (nchildren > 0 && label->name()->equals(&kRoot)) {
                const IBone *bone = label->boneRef(0);
                return bone;
            }
        }
    }
    return 0;
}

void DebugDrawer::drawBone(const IBone *bone, const BoneSet &selected, const BoneSet &IK, bool skipDrawingLine)
{
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

void DebugDrawer::beginDrawing(const IModel *model)
{
    float worldViewProjectionMatrix[16];
    m_applicationContextRef->getMatrix(worldViewProjectionMatrix,
                                  model,
                                  IApplicationContext::kCameraMatrix |
                                  IApplicationContext::kWorldMatrix |
                                  IApplicationContext::kViewMatrix |
                                  IApplicationContext::kProjectionMatrix);
    m_program->bind();
    m_program->setUniformValues(worldViewProjectionMatrix);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    bindVertexBundle(false); // XXX: VAO doesn't work
    m_program->enableAttributes(); // same problem as above (VAO doesn't work)
}

void DebugDrawer::flushDrawing()
{
    int nindices = m_indices.count();
    if (nindices > 0) {
        m_bundle->allocate(VertexBundle::kVertexBuffer, GL_DYNAMIC_DRAW, nindices * sizeof(m_vertices[0]), &m_vertices[0]);
        m_bundle->allocate(VertexBundle::kIndexBuffer, GL_DYNAMIC_DRAW, nindices * sizeof(m_indices[0]), &m_indices[0]);
        glDrawElements(GL_LINES, nindices, GL_UNSIGNED_INT, 0);
        m_program->unbind();
        m_vertices.clear();
        m_indices.clear();
        m_index = 0;
    }
    releaseVertexBundle(false);  // XXX: VAO doesn't work
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
}

void DebugDrawer::bindVertexBundle(bool bundle)
{
    if (!bundle || !m_layout->bind()) {
        static Vertex v;
        const vsize offset = reinterpret_cast<const uint8 *>(&v.color) - reinterpret_cast<const uint8 *>(&v.position);
        m_bundle->bind(VertexBundle::kVertexBuffer, 0);
        glVertexAttribPointer(PrivateShaderProgram::kPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
        glVertexAttribPointer(PrivateShaderProgram::kColor, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void *>(offset));
        m_bundle->bind(VertexBundle::kIndexBuffer, 0);
    }
}

void DebugDrawer::releaseVertexBundle(bool bundle)
{
    if (!bundle || !m_layout->release()) {
        m_bundle->unbind(VertexBundle::kVertexBuffer);
        m_bundle->unbind(VertexBundle::kIndexBuffer);
    }
}

} /* namespace qt */
} /* namespace vpvl2 */
