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

#include <vpvl2/vpvl2.h>
#include <vpvl2/gl/Texture2D.h>
#include <vpvl2/extensions/BaseApplicationContext.h>
#include <vpvl2/extensions/XMLProject.h>
#include <vpvl2/extensions/qt/String.h>

#include "Common.h"
#include <QtCore>
#include <QtMultimedia>
#include <QQuickWindow>
#include <QOpenGLBuffer>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

#include <LinearMath/btIDebugDraw.h>
#include <IGizmo.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "ApplicationContext.h"
#include "BoneRefObject.h"
#include "CameraRefObject.h"
#include "EncodingTask.h"
#include "GraphicsDevice.h"
#include "Grid.h"
#include "LightRefObject.h"
#include "MotionProxy.h"
#include "RenderTarget.h"
#include "ModelProxy.h"
#include "ProjectProxy.h"
#include "Util.h"
#include "WorldProxy.h"
#include "VideoSurface.h"

using namespace vpvl2;
using namespace vpvl2::extensions;
using namespace vpvl2::extensions::qt;

namespace {

static const Vector3 kBoneVertices[] = {
    Vector3(0.0f,  1.0f,  0.0f),  // top
    Vector3(0.0f,  0.0f,  0.0f),  // bottom
    Vector3(0.0f,  0.1f,  -0.1f), // front
    Vector3(0.1f,  0.1f,  0.0f),  // right
    Vector3(0.0f,  0.1f,  0.1f),  // back
    Vector3(-0.1f,  0.1f,  0.0f), // left
};
static const int kBoneIndices[] = {
    0, 2, 3, 0, 3, 4, 0, 4, 5, 0, 5, 2, // top
    1, 2, 3, 1, 3, 4, 1, 4, 5, 1, 5, 2  // bottom
};

static const int kNumBoneVertices = sizeof(kBoneVertices) / sizeof(kBoneVertices[0]);
static const int kNumBoneIndices = sizeof(kBoneIndices) / sizeof(kBoneIndices[0]);

static IApplicationContext::MousePositionType convertMousePositionType(int button)
{
    switch (static_cast<Qt::MouseButton>(button)) {
    case Qt::LeftButton:
        return IApplicationContext::kMouseLeftPressPosition;
    case Qt::MiddleButton:
        return IApplicationContext::kMouseMiddlePressPosition;
    case Qt::RightButton:
        return IApplicationContext::kMouseRightPressPosition;
    default:
        return IApplicationContext::kMouseCursorPosition;
    }
}

static int convertKey(int key)
{
    switch (static_cast<Qt::Key>(key)) {
    case Qt::Key_Backspace:
        return '\b';
    case Qt::Key_Tab:
        return '\t';
    case Qt::Key_Clear:
        return 0x0c;
    case Qt::Key_Return:
        return '\r';
    case Qt::Key_Pause:
        return 0x13;
    case Qt::Key_Escape:
        return 0x1b;
    case Qt::Key_Space:
        return ' ';
    case Qt::Key_Delete:
        return 0x7f;
    case Qt::Key_Up:
        return 273;
    case Qt::Key_Down:
        return 274;
    case Qt::Key_Right:
        return 275;
    case Qt::Key_Left:
        return 276;
    case Qt::Key_Insert:
        return 277;
    case Qt::Key_Home:
        return 278;
    case Qt::Key_End:
        return 279;
    case Qt::Key_PageUp:
        return 280;
    case Qt::Key_PageDown:
        return 281;
    case Qt::Key_F1:
    case Qt::Key_F2:
    case Qt::Key_F3:
    case Qt::Key_F4:
    case Qt::Key_F5:
    case Qt::Key_F6:
    case Qt::Key_F7:
    case Qt::Key_F8:
    case Qt::Key_F9:
    case Qt::Key_F10:
    case Qt::Key_F11:
    case Qt::Key_F12:
    case Qt::Key_F13:
    case Qt::Key_F14:
    case Qt::Key_F15:
        return 282 + (key - Qt::Key_F1);
    default:
        return key;
    }
}

static int convertModifier(int value)
{
    switch (static_cast<Qt::Modifier>(value)) {
    case Qt::ShiftModifier:
        return 0x3;
    case Qt::ControlModifier:
        return 0xc0;
    case Qt::AltModifier:
        return 0x100;
    case Qt::MetaModifier:
        return 0xc00;
    default:
        return 0;
    }
}

}

class RenderTarget::DebugDrawer : public btIDebugDraw {
public:
    DebugDrawer()
        : m_flags(0),
          m_index(0)
    {
    }
    ~DebugDrawer() {
    }

    void initialize() {
        if (!m_program) {
            m_program.reset(new QOpenGLShaderProgram());
            m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/gui/grid.vsh");
            m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/gui/grid.fsh");
            m_program->bindAttributeLocation("inPosition", 0);
            m_program->bindAttributeLocation("inColor", 1);
            m_program->link();
            Q_ASSERT(m_program->isLinked());
            allocateBuffer(QOpenGLBuffer::VertexBuffer, m_vbo);
            allocateBuffer(QOpenGLBuffer::VertexBuffer, m_cbo);
            m_vao.reset(new QOpenGLVertexArrayObject());
            if (m_vao->create()) {
                m_vao->bind();
                bindAttributeBuffers();
                m_vao->release();
            }
        }
    }

    void drawContactPoint(const btVector3 &PointOnB,
                          const btVector3 &normalOnB,
                          btScalar distance,
                          int /* lifeTime */,
                          const btVector3 &color) {
        drawLine(PointOnB, PointOnB + normalOnB * distance, color);
    }
    void drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color) {
        int vertexIndex = m_index, colorIndex = m_index;
        m_vertices[vertexIndex++] = from;
        m_vertices[vertexIndex++] = to;
        m_colors[colorIndex++] = color;
        m_colors[colorIndex++] = color;
        m_index += 2;
        if (m_index >= kPreAllocatedSize) {
            flush();
        }
    }
    void drawLine(const btVector3 &from,
                  const btVector3 &to,
                  const btVector3 &fromColor,
                  const btVector3 & /* toColor */) {
        drawLine(from, to, fromColor);
    }
    void draw3dText(const btVector3 & /* location */, const char *textString) {
        VPVL2_VLOG(1, textString);
    }
    void reportErrorWarning(const char *warningString) {
        VPVL2_LOG(WARNING, warningString);
    }
    int getDebugMode() const {
        return m_flags;
    }
    void setDebugMode(int debugMode) {
        m_flags = debugMode;
    }

    void flush() {
        Q_ASSERT(m_index % 2 == 0);
        m_vbo->bind();
        m_vbo->write(0, m_vertices, m_index * sizeof(m_vertices[0]));
        m_cbo->bind();
        m_cbo->write(0, m_colors, m_index * sizeof(m_colors[0]));
        bindProgram();
        m_program->setUniformValue("modelViewProjectionMatrix", m_modelViewProjectionMatrix);
        glDrawArrays(GL_LINES, 0, m_index / 2);
        releaseProgram();
        m_index = 0;
    }
    void setModelViewProjectionMatrix(const QMatrix4x4 &value) {
        m_modelViewProjectionMatrix = value;
    }

private:
    enum {
        kPreAllocatedSize = 1024000
    };
    static void allocateBuffer(QOpenGLBuffer::Type type, QScopedPointer<QOpenGLBuffer> &buffer) {
        buffer.reset(new QOpenGLBuffer(type));
        buffer->create();
        buffer->bind();
        buffer->setUsagePattern(QOpenGLBuffer::DynamicDraw);
        buffer->allocate(sizeof(QVector3D) * kPreAllocatedSize);
        buffer->release();
    }
    void bindAttributeBuffers() {
        m_program->enableAttributeArray(0);
        m_program->enableAttributeArray(1);
        m_vbo->bind();
        m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(m_vertices[0]));
        m_cbo->bind();
        m_program->setAttributeBuffer(1, GL_FLOAT, 0, 3, sizeof(m_colors[0]));
    }
    void bindProgram() {
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        m_program->bind();
        if (m_vao->isCreated()) {
            m_vao->bind();
        }
        else {
            bindAttributeBuffers();
        }
    }
    void releaseProgram() {
        if (m_vao->isCreated()) {
            m_vao->release();
        }
        else {
            m_vbo->release();
            m_cbo->release();
            m_program->disableAttributeArray(0);
            m_program->disableAttributeArray(1);
        }
        m_program->release();
        glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
    }

    QScopedPointer<QOpenGLShaderProgram> m_program;
    QScopedPointer<QOpenGLVertexArrayObject> m_vao;
    QScopedPointer<QOpenGLBuffer> m_vbo;
    QScopedPointer<QOpenGLBuffer> m_cbo;
    QMatrix4x4 m_modelViewProjectionMatrix;
    Vector3 m_vertices[kPreAllocatedSize];
    Vector3 m_colors[kPreAllocatedSize];
    int m_flags;
    int m_index;
};

class RenderTarget::ModelDrawer : public QObject {
    Q_OBJECT

public:
    ModelDrawer(QObject *parent = 0)
        : QObject(parent),
          m_currentModelRef(0),
          m_nvertices(0),
          m_nindices(0),
          m_dirty(false)
    {
    }
    ~ModelDrawer() {
    }

    void initialize() {
        if (!m_program) {
            m_program.reset(new QOpenGLShaderProgram());
            m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/gui/grid.vsh");
            m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/gui/grid.fsh");
            m_program->bindAttributeLocation("inPosition", kPositionAttribute);
            m_program->bindAttributeLocation("inColor", kColorAttribute);
            m_program->link();
            Q_ASSERT(m_program->isLinked());
            allocateBuffer(QOpenGLBuffer::VertexBuffer, QOpenGLBuffer::DynamicDraw, m_vbo);
            allocateBuffer(QOpenGLBuffer::IndexBuffer, QOpenGLBuffer::DynamicDraw, m_ibo);
            m_vao.reset(new QOpenGLVertexArrayObject());
            if (m_vao->create()) {
                m_vao->bind();
                bindAttributeBuffers();
                m_vao->release();
            }
        }
    }
    void setModelProxyRef(const ModelProxy *value) {
        const QList<BoneRefObject *> &allBones = value ? value->allBoneRefs() : QList<BoneRefObject *>();
        QVarLengthArray<Vertex> vertices;
        QVarLengthArray<int> indices;
        bool proceed;
        vertices.reserve(allBones.size() * kNumBoneVertices);
        indices.reserve(allBones.size() * kNumBoneIndices);
        if (value && m_currentModelRef != value) {
            removeModelRef(m_currentModelRef);
            connect(value, &ModelProxy::targetBonesDidCommitTransform, this, &ModelDrawer::markDirty);
            int offset = 0;
            foreach (const BoneRefObject *bone, allBones) {
                const IBone *boneRef = bone->data();
                connect(bone, &BoneRefObject::localTranslationChanged, this, &ModelDrawer::markDirty);
                connect(bone, &BoneRefObject::localOrientationChanged, this, &ModelDrawer::markDirty);
                updateBoneVertices(boneRef, value->firstTargetBone() == bone, vertices, proceed);
                if (proceed) {
                    int newOffset = kNumBoneVertices * offset;
                    for (int i = 0; i < kNumBoneIndices; i++) {
                        int value = kBoneIndices[i] + newOffset;
                        indices.append(value);
                    }
                    offset++;
                }
            }
            m_vbo->bind();
            m_vbo->allocate(vertices.data(), vertices.size() * sizeof(vertices[0]));
            m_vbo->release();
            m_ibo->bind();
            m_ibo->allocate(indices.data(), indices.size() * sizeof(indices[0]));
            m_ibo->release();
            m_nvertices = vertices.size();
            m_nindices = indices.size();
            m_currentModelRef = value;
        }
    }
    const ModelProxy *currentModelProxyRef() const {
        return m_currentModelRef;
    }
    void setModelViewProjectionMatrix(const QMatrix4x4 &value) {
        m_modelViewProjectionMatrix = value;
    }
    void draw() {
        bindProgram();
        m_program->setUniformValue("modelViewProjectionMatrix", m_modelViewProjectionMatrix);
        glDrawElements(GL_TRIANGLES, m_nindices, GL_UNSIGNED_INT, 0);
        releaseProgram();
    }
    void update() {
        if (m_dirty && m_currentModelRef) {
            const QList<BoneRefObject *> &allBones = m_currentModelRef->allBoneRefs();
            QVarLengthArray<Vertex> vertices;
            bool proceed;
            vertices.reserve(allBones.size());
            foreach (const BoneRefObject *bone, allBones) {
                const IBone *boneRef = bone->data();
                updateBoneVertices(boneRef, m_currentModelRef->firstTargetBone() == bone, vertices, proceed);
            }
            m_vbo->bind();
            m_vbo->write(0, vertices.data(), vertices.size() * sizeof(vertices[0]));
            m_vbo->release();
            m_nvertices = vertices.size();
            m_dirty = false;
        }
    }
    void removeModelRef(const ModelProxy *modelProxyRef) {
        if (modelProxyRef) {
            disconnect(modelProxyRef, &ModelProxy::targetBonesDidCommitTransform, this, &ModelDrawer::markDirty);
            foreach (const BoneRefObject *bone, modelProxyRef->allBoneRefs()) {
                disconnect(bone, &BoneRefObject::localTranslationChanged, this, &ModelDrawer::markDirty);
                disconnect(bone, &BoneRefObject::localOrientationChanged, this, &ModelDrawer::markDirty);
            }
            if (modelProxyRef == m_currentModelRef) {
                m_currentModelRef = 0;
            }
        }
    }

public slots:
    void markDirty() {
        m_dirty = true;
    }

private:
    struct Vertex {
        Vector3 position;
        Vector3 color;
    };
    enum VertexType {
        kPositionAttribute,
        kColorAttribute
    };
    static const float kOpacity;

    static void allocateBuffer(QOpenGLBuffer::Type type, QOpenGLBuffer::UsagePattern usage, QScopedPointer<QOpenGLBuffer> &buffer) {
        buffer.reset(new QOpenGLBuffer(type));
        buffer->create();
        buffer->bind();
        buffer->setUsagePattern(usage);
        buffer->release();
    }
    void bindAttributeBuffers() {
        m_program->enableAttributeArray(kPositionAttribute);
        m_program->enableAttributeArray(kColorAttribute);
        m_vbo->bind();
        m_program->setAttributeBuffer(kPositionAttribute, GL_FLOAT, 0, 3, sizeof(Vertex));
        m_program->setAttributeBuffer(kColorAttribute, GL_FLOAT, 16, 4, sizeof(Vertex));
        m_ibo->bind();
    }
    void bindProgram() {
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        m_program->bind();
        if (m_vao->isCreated()) {
            m_vao->bind();
        }
        else {
            bindAttributeBuffers();
        }
    }
    void releaseProgram() {
        if (m_vao->isCreated()) {
            m_vao->release();
        }
        else {
            m_ibo->release();
            m_vbo->release();
            m_program->disableAttributeArray(kPositionAttribute);
            m_program->disableAttributeArray(kColorAttribute);
        }
        m_program->release();
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
    }
    void updateBoneVertices(const IBone *boneRef, bool selected, QVarLengthArray<Vertex> &vertices, bool &proceed) const {
        const Vector3 &origin = boneRef->worldTransform().getOrigin(), &delta = boneRef->destinationOrigin() - origin;
        if (boneRef->isInteractive() && !delta.isZero()) {
            static const glm::vec3 kGLMUnitY(0, 1, 0);
            const glm::vec3 scale(1, delta.length(), 1), normal(glm::normalize(glm::vec3(delta.x(), delta.y(), delta.z())));
            const glm::mat4 matrix(glm::scale(glm::toMat4(glm::rotation(kGLMUnitY, normal)), scale));
            Vector3 color(Util::toColor(Qt::blue));
            Transform transform(Transform::getIdentity());
            transform.setFromOpenGLMatrix(glm::value_ptr(matrix));
            transform.setOrigin(origin);
            if (selected) {
                color = Util::toColor(Qt::red);
            }
            else if (boneRef->hasFixedAxes()) {
                color = Util::toColor(Qt::magenta);
            }
            else if (boneRef->hasLocalAxes()) {
                color = Util::toColor(Qt::cyan);
            }
            else if (boneRef->hasInverseKinematics()) {
                color = Util::toColor(Qt::yellow);
            }
            Vertex v;
            for (int i = 0; i < kNumBoneVertices; i++) {
                v.position = transform * kBoneVertices[i];
                v.color = color;
                v.color.setW(kOpacity);
                vertices.append(v);
            }
            proceed = true;
        }
        else {
            proceed = false;
        }
    }

    const ModelProxy *m_currentModelRef;
    QScopedPointer<QOpenGLShaderProgram> m_program;
    QScopedPointer<QOpenGLVertexArrayObject> m_vao;
    QScopedPointer<QOpenGLBuffer> m_vbo;
    QScopedPointer<QOpenGLBuffer> m_ibo;
    QMatrix4x4 m_modelViewProjectionMatrix;
    int m_nvertices;
    int m_nindices;
    volatile bool m_dirty;
};

const float RenderTarget::ModelDrawer::kOpacity = 0.25;
const QVector3D RenderTarget::kDefaultShadowMapSize = QVector3D(1024, 1024, 1);

RenderTarget::RenderTarget(QQuickItem *parent)
    : QQuickItem(parent),
      m_grid(new Grid()),
      m_shadowMapSize(kDefaultShadowMapSize),
      m_editMode(SelectMode),
      m_projectProxyRef(0),
      m_currentGizmoRef(0),
      m_lastTimeIndex(0),
      m_currentTimeIndex(0),
      m_snapStepSize(5, 5, 5),
      m_visibleGizmoMasks(AxisX | AxisY | AxisZ | AxisScreen),
      m_grabbingGizmo(false),
      m_playing(false),
      m_dirty(false)
{
    connect(this, &RenderTarget::windowChanged, this, &RenderTarget::handleWindowChange);
    connect(this, &RenderTarget::shadowMapSizeChanged, this, &RenderTarget::prepareUpdatingLight);
}

RenderTarget::~RenderTarget()
{
    m_projectProxyRef = 0;
    m_currentGizmoRef = 0;
    m_lastTimeIndex = 0;
    m_currentTimeIndex = 0;
    m_grabbingGizmo = false;
    m_playing = false;
    m_dirty = false;
}

bool RenderTarget::handleMousePress(int x, int y, int button)
{
    glm::vec4 v(x - m_viewport.x(), y - m_viewport.y(), 1, 0);
    IApplicationContext::MousePositionType type = convertMousePositionType(button);
    m_applicationContext->setMousePosition(v, type);
    if (!m_applicationContext->handleMouse(v, type) && m_currentGizmoRef) {
        m_grabbingGizmo = m_currentGizmoRef->OnMouseDown(x, y);
        if (m_grabbingGizmo) {
            ModelProxy *modelProxy = m_projectProxyRef->currentModel();
            Q_ASSERT(modelProxy);
            switch (m_editMode) {
            case RotateMode:
                modelProxy->beginRotate(0);
                break;
            case MoveMode:
                modelProxy->beginTranslate(0);
                break;
            case SelectMode:
            default:
                break;
            }
            emit grabbingGizmoChanged();
        }
    }
    return m_grabbingGizmo;
}

bool RenderTarget::handleMouseMove(int x, int y,  bool pressed)
{
    glm::vec4 v(x - m_viewport.x(), y - m_viewport.y(), pressed, 0);
    m_applicationContext->setMousePosition(v, IApplicationContext::kMouseCursorPosition);
    if (m_applicationContext->handleMouse(v, IApplicationContext::kMouseCursorPosition)) {
        return true;
    }
    else if (m_currentGizmoRef) {
        m_currentGizmoRef->OnMouseMove(x, y);
        if (m_grabbingGizmo) {
            const ModelProxy *modelProxy = m_projectProxyRef->currentModel();
            Q_ASSERT(modelProxy);
            const BoneRefObject *boneProxy = modelProxy->firstTargetBone();
            Q_ASSERT(boneProxy);
            IBone *boneRef = boneProxy->data();
            Scalar rawMatrix[16];
            for (int i = 0; i < 16; i++) {
                rawMatrix[i] = static_cast<Scalar>(m_editMatrix.constData()[i]);
            }
            Transform transform(Transform::getIdentity());
            transform.setFromOpenGLMatrix(rawMatrix);
            boneRef->setLocalTranslation(transform.getOrigin());
            boneRef->setLocalOrientation(transform.getRotation());
            return true;
        }
    }
    return false;
}

bool RenderTarget::handleMouseRelease(int x, int y, int button)
{
    glm::vec4 v(x - m_viewport.x(), y - m_viewport.y(), 0, 0);
    IApplicationContext::MousePositionType type = convertMousePositionType(button);
    m_applicationContext->setMousePosition(v, type);
    if (m_applicationContext->handleMouse(v, type)) {
        return true;
    }
    else if (m_currentGizmoRef) {
        m_currentGizmoRef->OnMouseUp(x, y);
        if (m_grabbingGizmo) {
            ModelProxy *modelProxy = m_projectProxyRef->currentModel();
            Q_ASSERT(modelProxy);
            switch (m_editMode) {
            case RotateMode:
                modelProxy->endRotate();
                break;
            case MoveMode:
                modelProxy->endTranslate();
                break;
            case SelectMode:
            default:
                break;
            }
            m_grabbingGizmo = false;
            emit grabbingGizmoChanged();
            return true;
        }
    }
    return false;
}

bool RenderTarget::handleMouseWheel(int x, int y)
{
    glm::vec4 v(x, y, 0, 0);
    return m_applicationContext->handleMouse(v, IApplicationContext::kMouseWheelPosition);
}

bool RenderTarget::handleKeyPress(int key, int modifier)
{
    return m_applicationContext->handleKeyPress(convertKey(key), convertModifier(modifier));
}

void RenderTarget::toggleRunning(bool value)
{
    Q_ASSERT(window());
    if (value) {
        connect(window(), &QQuickWindow::beforeRendering, this, &RenderTarget::draw, Qt::DirectConnection);
    }
    else {
        disconnect(window(), &QQuickWindow::beforeRendering, this, &RenderTarget::draw);
    }
}

bool RenderTarget::isInitialized() const
{
    return Scene::isInitialized();
}

qreal RenderTarget::currentTimeIndex() const
{
    return m_currentTimeIndex;
}

void RenderTarget::setCurrentTimeIndex(qreal value)
{
    if (value != m_currentTimeIndex) {
        seekVideo(value);
        m_currentTimeIndex = value;
        emit currentTimeIndexChanged();
        if (QQuickWindow *win = window()) {
            win->update();
        }
    }
}

qreal RenderTarget::lastTimeIndex() const
{
    return m_lastTimeIndex;
}

void RenderTarget::setLastTimeIndex(qreal value)
{
    if (value != m_lastTimeIndex) {
        m_lastTimeIndex = value;
        emit lastTimeIndexChanged();
        if (QQuickWindow *win = window()) {
            win->update();
        }
    }
}

qreal RenderTarget::currentFPS() const
{
    return m_counter.value();
}

ProjectProxy *RenderTarget::projectProxy() const
{
    return m_projectProxyRef;
}

Grid *RenderTarget::grid() const
{
    return m_grid.data();
}

void RenderTarget::setProjectProxy(ProjectProxy *value)
{
    Q_ASSERT(value);
    connect(this, &RenderTarget::enqueuedModelsDidDelete, value, &ProjectProxy::enqueuedModelsDidDelete);
    connect(value, &ProjectProxy::gridVisibleChanged, this, &RenderTarget::toggleGridVisible);
    connect(value, &ProjectProxy::modelDidAdd, this, &RenderTarget::enqueueUploadingModel);
    connect(value, &ProjectProxy::modelDidCommitUploading, this, &RenderTarget::commitUploadingModels);
    connect(value, &ProjectProxy::modelDidRemove, this, &RenderTarget::enqueueDeletingModel);
    connect(value, &ProjectProxy::modelDidCommitDeleting, this, &RenderTarget::commitDeletingModels);
    connect(value, &ProjectProxy::effectDidAdd, this, &RenderTarget::enqueueUploadingEffect);
    connect(value, &ProjectProxy::effectDidCommitUploading, this, &RenderTarget::commitUploadingEffects);
    connect(value, &ProjectProxy::motionDidLoad, this, &RenderTarget::prepareSyncMotionState);
    connect(value, &ProjectProxy::currentModelChanged, this, &RenderTarget::updateGizmo);
    connect(value, &ProjectProxy::projectDidRelease, this, &RenderTarget::commitDeletingModels);
    connect(value, &ProjectProxy::projectWillCreate, this, &RenderTarget::disconnectProjectSignals);
    connect(value, &ProjectProxy::projectDidCreate, this, &RenderTarget::prepareUploadingModelsInProject);
    connect(value, &ProjectProxy::projectWillLoad, this, &RenderTarget::disconnectProjectSignals);
    connect(value, &ProjectProxy::projectDidLoad, this, &RenderTarget::prepareUploadingModelsInProject);
    connect(value, &ProjectProxy::undoDidPerform, this, &RenderTarget::render);
    connect(value, &ProjectProxy::undoDidPerform, this, &RenderTarget::updateGizmo);
    connect(value, &ProjectProxy::redoDidPerform, this, &RenderTarget::render);
    connect(value, &ProjectProxy::redoDidPerform, this, &RenderTarget::updateGizmo);
    connect(value, &ProjectProxy::currentTimeIndexChanged, this, &RenderTarget::seekMediaFromProject);
    connect(value, &ProjectProxy::rewindDidPerform, this, &RenderTarget::resetCurrentTimeIndex);
    connect(value, &ProjectProxy::rewindDidPerform, this, &RenderTarget::resetLastTimeIndex);
    connect(value, &ProjectProxy::rewindDidPerform, this, &RenderTarget::prepareSyncMotionState);
    connect(value->world(), &WorldProxy::simulationTypeChanged, this, &RenderTarget::prepareSyncMotionState);
    CameraRefObject *camera = value->camera();
    connect(camera, &CameraRefObject::lookAtChanged, this, &RenderTarget::markDirty);
    connect(camera, &CameraRefObject::angleChanged, this, &RenderTarget::markDirty);
    connect(camera, &CameraRefObject::distanceChanged, this, &RenderTarget::markDirty);
    connect(camera, &CameraRefObject::fovChanged, this, &RenderTarget::markDirty);
    connect(camera, &CameraRefObject::cameraDidReset, this, &RenderTarget::markDirty);
    LightRefObject *light = value->light();
    connect(light, &LightRefObject::directionChanged, this, &RenderTarget::prepareUpdatingLight);
    connect(light, &LightRefObject::shadowTypeChanged, this, &RenderTarget::prepareUpdatingLight);
    const QUrl &url = value->videoSource();
    if (!url.isEmpty() && url.isValid()) {
        setVideoUrl(url);
    }
    m_grid->setVisible(value->isGridVisible());
    m_projectProxyRef = value;
}

bool RenderTarget::isPlaying() const
{
    return m_playing;
}

void RenderTarget::setPlaying(bool value)
{
    if (m_playing != value) {
        m_playing = value;
        emit playingChanged();
    }
}

bool RenderTarget::isDirty() const
{
    return m_dirty;
}

void RenderTarget::setDirty(bool value)
{
    if (m_dirty != value) {
        m_dirty = value;
        emit dirtyChanged();
    }
}

bool RenderTarget::isSnapGizmoEnabled() const
{
    return translationGizmo()->IsUsingSnap();
}

void RenderTarget::setSnapGizmoEnabled(bool value)
{
    IGizmo *translationGizmoRef = translationGizmo();
    if (translationGizmoRef->IsUsingSnap() != value) {
        translationGizmoRef->UseSnap(value);
        emit enableSnapGizmoChanged();
    }
}

bool RenderTarget::grabbingGizmo() const
{
    return m_grabbingGizmo;
}

QRect RenderTarget::viewport() const
{
    return m_viewport;
}

void RenderTarget::setViewport(const QRect &value)
{
    if (m_viewport != value) {
        m_viewport = value;
        setDirty(true);
        emit viewportChanged();
    }
}

QVector3D RenderTarget::shadowMapSize() const
{
    return m_shadowMapSize;
}

void RenderTarget::setShadowMapSize(const QVector3D &value)
{
    Q_ASSERT(m_projectProxyRef);
    if (value != m_shadowMapSize) {
        m_projectProxyRef->setGlobalString("shadow.texture.size", value);
        m_shadowMapSize = value;
        emit shadowMapSizeChanged();
    }
}

QUrl RenderTarget::audioUrl() const
{
    return QUrl();
}

void RenderTarget::setAudioUrl(const QUrl &value)
{
    if (value != audioUrl()) {
        QScopedPointer<QAudioDecoder> decoder(new QAudioDecoder());
        decoder->setSourceFilename(value.toLocalFile());
        /* XXX: under construction */
        while (decoder->bufferAvailable()) {
            const QAudioBuffer &buffer = decoder->read();
            qDebug() << buffer.startTime() << buffer.frameCount() << buffer.sampleCount();
        }
        emit audioUrlChanged();
    }
}

QUrl RenderTarget::videoUrl() const
{
    return mediaPlayer()->media().canonicalUrl();
}

void RenderTarget::setVideoUrl(const QUrl &value)
{
    if (value != videoUrl()) {
        Q_ASSERT(m_projectProxyRef);
        QMediaPlayer *mediaPlayerRef = mediaPlayer();
        mediaPlayerRef->setMedia(value);
        m_projectProxyRef->setVideoSource(value);
        emit videoUrlChanged();
    }
}

RenderTarget::EditModeType RenderTarget::editMode() const
{
    return m_editMode;
}

void RenderTarget::setEditMode(EditModeType value)
{
    if (m_editMode != value) {
        switch (value) {
        case RotateMode:
            m_currentGizmoRef = orientationGizmo();
            break;
        case MoveMode:
            m_currentGizmoRef = translationGizmo();
            break;
        case SelectMode:
        default:
            m_currentGizmoRef = 0;
            break;
        }
        m_editMode = value;
        emit editModeChanged();
    }
}

RenderTarget::VisibleGizmoMasks RenderTarget::visibleGizmoMasks() const
{
    return m_visibleGizmoMasks;
}

void RenderTarget::setVisibleGizmoMasks(VisibleGizmoMasks value)
{
    if (value != m_visibleGizmoMasks) {
        orientationGizmo()->SetAxisMask(value);
        m_visibleGizmoMasks = value;
        emit visibleGizmoMasksChanged();
    }
}

QVector3D RenderTarget::snapGizmoStepSize() const
{
    return m_snapStepSize;
}

void RenderTarget::setSnapGizmoStepSize(const QVector3D &value)
{
    if (!qFuzzyCompare(value, m_snapStepSize)) {
        translationGizmo()->SetSnap(value.x(), value.y(), value.z());
        m_snapStepSize = value;
        emit snapGizmoStepSizeChanged();
    }
}

QMatrix4x4 RenderTarget::viewMatrix() const
{
    return QMatrix4x4(glm::value_ptr(m_viewMatrix));
}

QMatrix4x4 RenderTarget::projectionMatrix() const
{
    return QMatrix4x4(glm::value_ptr(m_projectionMatrix));
}

GraphicsDevice *RenderTarget::graphicsDevice() const
{
    return m_graphicsDevice.data();
}

void RenderTarget::handleWindowChange(QQuickWindow *window)
{
    if (window) {
        connect(window, &QQuickWindow::sceneGraphInitialized, this, &RenderTarget::initialize, Qt::DirectConnection);
        connect(window, &QQuickWindow::frameSwapped, this, &RenderTarget::synchronizeImplicitly, Qt::DirectConnection);
        window->setClearBeforeRendering(false);
    }
}

void RenderTarget::update()
{
    Q_ASSERT(window());
    window()->update();
}

void RenderTarget::render()
{
    Q_ASSERT(window());
    connect(window(), &QQuickWindow::beforeRendering, this, &RenderTarget::synchronizeExplicitly, Qt::DirectConnection);
}

void RenderTarget::exportImage(const QUrl &fileUrl, const QSize &size)
{
    Q_ASSERT(window());
    if (fileUrl.isEmpty() || !fileUrl.isValid()) {
        /* do nothing if url is empty or invalid */
        VPVL2_VLOG(2, "fileUrl is empty or invalid: url=" << fileUrl.toString().toStdString());
        return;
    }
    m_exportLocation = fileUrl;
    m_exportSize = size;
    if (!m_exportSize.isValid()) {
        m_exportSize = m_viewport.size();
    }
    connect(window(), &QQuickWindow::frameSwapped, this, &RenderTarget::drawOffscreenForImage, Qt::DirectConnection);
}

void RenderTarget::exportVideo(const QUrl &fileUrl, const QSize &size, const QString &videoType, const QString &frameImageType)
{
    Q_ASSERT(window());
    if (fileUrl.isEmpty() || !fileUrl.isValid()) {
        /* do nothing if url is empty or invalid */
        VPVL2_VLOG(2, "fileUrl is empty or invalid: url=" << fileUrl.toString().toStdString());
        return;
    }
    m_exportLocation = fileUrl;
    m_exportSize = size;
    if (!m_exportSize.isValid()) {
        m_exportSize = m_viewport.size();
    }
    EncodingTask *encodingTaskRef = encodingTask();
    encodingTaskRef->reset();
    encodingTaskRef->setSize(m_exportSize);
    encodingTaskRef->setTitle(m_projectProxyRef->title());
    encodingTaskRef->setInputImageFormat(frameImageType);
    encodingTaskRef->setOutputFormat(videoType);
    encodingTaskRef->setOutputPath(fileUrl.toLocalFile());
    connect(window(), &QQuickWindow::frameSwapped, this, &RenderTarget::drawOffscreenForVideo, Qt::DirectConnection);
}

void RenderTarget::cancelExportingVideo()
{
    Q_ASSERT(window());
    disconnect(window(), &QQuickWindow::frameSwapped, this, &RenderTarget::drawOffscreenForVideo);
    disconnect(window(), &QQuickWindow::frameSwapped, this, &RenderTarget::launchEncodingTask);
    if (m_encodingTask && m_encodingTask->isRunning()) {
        m_encodingTask->stop();
        emit encodeDidCancel();
    }
}

void RenderTarget::loadJson(const QUrl &fileUrl)
{
    QFile file(fileUrl.toLocalFile());
    if (file.open(QFile::ReadOnly)) {
        const QJsonDocument &document = QJsonDocument::fromJson(file.readAll());
        const QJsonObject &root = document.object(), &projectObject = root.value("project").toObject();
        QFileInfo fileInfo(projectObject.value("path").toString());
        if (fileInfo.exists() && fileInfo.isFile()) {
            m_projectProxyRef->loadAsync(QUrl::fromLocalFile(fileInfo.absoluteFilePath()));
        }
        else {
            m_projectProxyRef->world()->setSimulationType(projectObject.value("simulationPhysics").toBool(false) ? WorldProxy::EnableSimulationPlayOnly : WorldProxy::DisableSimulation);
            m_projectProxyRef->setTitle(projectObject.value("title").toString(tr("Untitled Project")));
            m_projectProxyRef->setScreenColor(QColor(projectObject.value("screenColor").toString("#ffffff")));
            const QJsonObject &gridObject = root.value("grid").toObject();
            m_projectProxyRef->setGridVisible(gridObject.value("visible").toBool(true));
            const QJsonObject &audioObject = root.value("audio").toObject();
            fileInfo.setFile(audioObject.value("source").toString());
            if (fileInfo.exists() && fileInfo.isFile()) {
                m_projectProxyRef->setAudioSource(QUrl::fromLocalFile(fileInfo.absoluteFilePath()));
                m_projectProxyRef->setAudioVolume(audioObject.value("volume").toDouble());
            }
            CameraRefObject *camera = m_projectProxyRef->camera();
            const QJsonObject &cameraObject = root.value("camera").toObject();
            if (cameraObject.contains("motion")) {
                fileInfo.setFile(cameraObject.value("motion").toString());
                if (fileInfo.exists() && fileInfo.isFile()) {
                    m_projectProxyRef->loadMotion(QUrl::fromLocalFile(fileInfo.absoluteFilePath()), 0, ProjectProxy::CameraMotion);
                }
            }
            else {
                camera->setFov(cameraObject.value("fov").toDouble(camera->fov()));
                camera->setDistance(cameraObject.value("distance").toDouble(camera->distance()));
            }
            const QJsonObject &lightObject = root.value("light").toObject();
            LightRefObject *light = m_projectProxyRef->light();
            light->setColor(QColor(lightObject.value("color").toString("#999999")));
            foreach (const QJsonValue &v, root.value("models").toArray()) {
                const QJsonObject &item = v.toObject();
                fileInfo.setFile(item.value("path").toString());
                if (fileInfo.exists() && fileInfo.isFile()) {
                    ModelProxy *modelProxy = m_projectProxyRef->loadModel(QUrl::fromLocalFile(fileInfo.absoluteFilePath()), QUuid::createUuid(), true);
                    modelProxy->setScaleFactor(item.value("scaleFactor").toDouble(1.0));
                    modelProxy->setOpacity(item.value("opacity").toDouble(1.0));
                    modelProxy->setEdgeWidth(item.value("edgeWidth").toDouble(1.0));
                    foreach (const QJsonValue &m, item.value("motions").toArray()) {
                        fileInfo.setFile(m.toString());
                        if (fileInfo.exists() && fileInfo.isFile()) {
                            m_projectProxyRef->loadMotion(QUrl::fromLocalFile(fileInfo.absoluteFilePath()), modelProxy, ProjectProxy::ModelMotion);
                        }
                    }
                }
            }
            foreach (const QJsonValue &v, root.value("effects").toArray()) {
                const QJsonObject &item = v.toObject();
                fileInfo.setFile(item.value("path").toString());
                if (fileInfo.exists() && fileInfo.isFile()) {
                    m_projectProxyRef->loadEffect(QUrl::fromLocalFile(fileInfo.absoluteFilePath()));
                }
            }
            const QJsonObject &videoObject = root.value("video").toObject();
            fileInfo.setFile(videoObject.value("source").toString());
            if (fileInfo.exists() && fileInfo.isFile()) {
                setVideoUrl(QUrl::fromLocalFile(fileInfo.absoluteFilePath()));
            }
            setCurrentTimeIndex(root.value("currentTimeIndex").toDouble());
        }
    }
}

void RenderTarget::resetCurrentTimeIndex()
{
    m_currentTimeIndex = 0;
    emit currentTimeIndexChanged();
}

void RenderTarget::resetLastTimeIndex()
{
    m_lastTimeIndex = 0;
    emit lastTimeIndexChanged();
}

void RenderTarget::markDirty()
{
    m_dirty = true;
}

void RenderTarget::updateGizmo()
{
    Q_ASSERT(m_projectProxyRef);
    if (const ModelProxy *modelProxy = m_projectProxyRef->currentModel()) {
        IGizmo *translationGizmoRef = translationGizmo(), *orientationGizmoRef = orientationGizmo();
        switch (modelProxy->transformType()) {
        case ModelProxy::GlobalTransform:
            translationGizmoRef->SetLocation(IGizmo::LOCATE_WORLD);
            orientationGizmoRef->SetLocation(IGizmo::LOCATE_WORLD);
            break;
        case ModelProxy::LocalTransform:
            translationGizmoRef->SetLocation(IGizmo::LOCATE_LOCAL);
            orientationGizmoRef->SetLocation(IGizmo::LOCATE_LOCAL);
            break;
        case ModelProxy::ViewTransform:
            translationGizmoRef->SetLocation(IGizmo::LOCATE_VIEW);
            orientationGizmoRef->SetLocation(IGizmo::LOCATE_VIEW);
            break;
        }
        setSnapGizmoStepSize(m_snapStepSize);
        if (const BoneRefObject *boneProxy = modelProxy->firstTargetBone()) {
            const IBone *boneRef = boneProxy->data();
            Transform transform(boneRef->localOrientation(), boneRef->localTranslation());
            Scalar rawMatrix[16];
            transform.getOpenGLMatrix(rawMatrix);
            for (int i = 0; i < 16; i++) {
                m_editMatrix.data()[i] = static_cast<qreal>(rawMatrix[i]);
            }
            const Vector3 &v = boneRef->origin();
            translationGizmoRef->SetOffset(v.x(), v.y(), v.z());
            orientationGizmoRef->SetOffset(v.x(), v.y(), v.z());
        }
    }
    else {
        m_currentGizmoRef = 0;
    }
}

void RenderTarget::updateModelBones()
{
    if (m_modelDrawer) {
        m_modelDrawer->markDirty();
        render();
    }
}

void RenderTarget::seekMediaFromProject()
{
    Q_ASSERT(m_projectProxyRef);
    seekVideo(m_projectProxyRef->currentTimeIndex());
}

void RenderTarget::handleAudioDecoderError(QAudioDecoder::Error error)
{
    const QString &message = ""; //mediaPlayer()->errorString();
    VPVL2_LOG(WARNING, "The audio " << audioUrl().toString().toStdString() << " cannot be loaded: code=" << error << " message=" << message.toStdString());
    emit errorDidHappen(QStringLiteral("%1 (code=%2)").arg(message).arg(error));
}

void RenderTarget::handleMediaPlayerError(QMediaPlayer::Error error)
{
    const QString &message = mediaPlayer()->errorString();
    VPVL2_LOG(WARNING, "The video " << videoUrl().toString().toStdString() << " cannot be loaded: code=" << error << " message=" << message.toStdString());
    emit errorDidHappen(QStringLiteral("%1 (code=%2)").arg(message).arg(error));
}

void RenderTarget::handleFileChange(const QString &filePath)
{
    Q_ASSERT(window());
    QMutexLocker locker(&m_fileChangeQueueMutex); Q_UNUSED(locker);
    connect(window(), &QQuickWindow::frameSwapped, this, &RenderTarget::consumeFileChangeQueue, Qt::DirectConnection);
    m_fileChangeQueue.enqueue(filePath);
}

void RenderTarget::consumeFileChangeQueue()
{
    Q_ASSERT(m_applicationContext && window());
    QMutexLocker locker(&m_fileChangeQueueMutex); Q_UNUSED(locker);
    disconnect(window(), &QQuickWindow::frameSwapped, this, &RenderTarget::consumeFileChangeQueue);
    while (!m_fileChangeQueue.isEmpty()) {
        m_applicationContext->reloadFile(m_fileChangeQueue.dequeue());
    }
}

void RenderTarget::toggleGridVisible()
{
    m_grid->setVisible(m_projectProxyRef->isGridVisible());
}

void RenderTarget::draw()
{
    Q_ASSERT(m_applicationContext && window());
    if (m_projectProxyRef) {
        emit renderWillPerform();
        resetOpenGLStates();
        drawShadowMap();
        updateViewport();
        clearScene();
        drawVideoFrame();
        m_applicationContext->saveDirtyEffects();
        drawGrid();
        drawScene();
        drawDebug();
        drawModelBones();
        drawCurrentGizmo();
        m_applicationContext->renderEffectParameterUIWidgets();
        bool flushed = false;
        m_counter.update(m_renderTimer.elapsed(), flushed);
        if (flushed) {
            emit currentFPSChanged();
        }
        emit renderDidPerform();
    }
}

void RenderTarget::drawOffscreenForImage()
{
    Q_ASSERT(window());
    disconnect(window(), &QQuickWindow::frameSwapped, this, &RenderTarget::drawOffscreenForImage);
    connect(window(), &QQuickWindow::frameSwapped, this, &RenderTarget::writeExportedImage);
    QOpenGLFramebufferObject fbo(m_exportSize, ApplicationContext::framebufferObjectFormat(window()));
    drawOffscreen(&fbo);
    m_exportImage = fbo.toImage();
}

void RenderTarget::drawOffscreenForVideo()
{
    Q_ASSERT(window());
    EncodingTask *encodingTaskRef = encodingTask();
    QOpenGLFramebufferObject *fbo = encodingTaskRef->generateFramebufferObject(window());
    drawOffscreen(fbo);
    if (qFuzzyIsNull(m_projectProxyRef->differenceTimeIndex(m_currentTimeIndex))) {
        encodingTaskRef->setEstimatedFrameCount(m_currentTimeIndex);
        disconnect(window(), &QQuickWindow::frameSwapped, this, &RenderTarget::drawOffscreenForVideo);
        connect(window(), &QQuickWindow::frameSwapped, this, &RenderTarget::launchEncodingTask);
    }
    else {
        const qreal &currentTimeIndex = m_currentTimeIndex;
        const QString &path = encodingTaskRef->generateFilename(currentTimeIndex);
        setCurrentTimeIndex(currentTimeIndex + 1);
        m_projectProxyRef->update(Scene::kUpdateAll);
        fbo->toImage().save(path);
        emit videoFrameDidSave(currentTimeIndex, m_projectProxyRef->durationTimeIndex());
    }
}

void RenderTarget::writeExportedImage()
{
    Q_ASSERT(window());
    disconnect(window(), &QQuickWindow::frameSwapped, this, &RenderTarget::writeExportedImage);
    QFileInfo finfo(m_exportLocation.toLocalFile());
    const QString &suffix = finfo.suffix();
    if (suffix != "bmp" && !QQuickWindow::hasDefaultAlphaBuffer()) {
        QTemporaryFile tempFile;
        if (tempFile.open()) {
            m_exportImage.save(&tempFile, "bmp");
            QSaveFile saveFile(finfo.filePath());
            if (saveFile.open(QFile::WriteOnly)) {
                const QImage image(tempFile.fileName());
                image.save(&saveFile, qPrintable(suffix));
                if (!saveFile.commit()) {
                    VPVL2_LOG(WARNING, "Cannot commit the file to: path=" << saveFile.fileName().toStdString() << " reason=" << saveFile.errorString().toStdString());
                }
            }
            else {
                VPVL2_LOG(WARNING, "Cannot open the file to commit: path=" << saveFile.fileName().toStdString() << "  reason=" << saveFile.errorString().toStdString());
            }
        }
        else {
            VPVL2_LOG(WARNING, "Cannot open temporary file: path=" << tempFile.fileName().toStdString() << "  reason=" << tempFile.errorString().toStdString());
        }
    }
    else {
        QSaveFile saveFile(finfo.filePath());
        if (saveFile.open(QFile::WriteOnly)) {
            m_exportImage.save(&saveFile, qPrintable(suffix));
            saveFile.commit();
        }
        else {
            VPVL2_LOG(WARNING, "Cannot open file to commit: path=" << saveFile.fileName().toStdString() << " " << saveFile.errorString().toStdString());
        }
    }
    m_exportImage = QImage();
    m_exportSize = QSize();
}

void RenderTarget::launchEncodingTask()
{
    Q_ASSERT(window());
    disconnect(window(), &QQuickWindow::frameSwapped, this, &RenderTarget::launchEncodingTask);
    encodingTask()->launch();
    m_exportSize = QSize();
}

void RenderTarget::prepareSyncMotionState()
{
    Q_ASSERT(window());
    connect(window(), &QQuickWindow::beforeRendering, this, &RenderTarget::synchronizeMotionState, Qt::DirectConnection);
}

void RenderTarget::prepareUpdatingLight()
{
    if (QQuickWindow *win = window()) {
        connect(win, &QQuickWindow::beforeRendering, this, &RenderTarget::performUpdatingLight, Qt::DirectConnection);
    }
}

void RenderTarget::synchronizeExplicitly()
{
    Q_ASSERT(window());
    disconnect(window(), &QQuickWindow::beforeRendering, this, &RenderTarget::synchronizeExplicitly);
    if (m_projectProxyRef) {
        m_projectProxyRef->update(Scene::kUpdateAll | Scene::kForceUpdateAllMorphs);
    }
    if (m_modelDrawer) {
        m_modelDrawer->update();
    }
    draw();
}

void RenderTarget::synchronizeMotionState()
{
    Q_ASSERT(window() && m_projectProxyRef);
    disconnect(window(), &QQuickWindow::beforeRendering, this, &RenderTarget::synchronizeMotionState);
    m_projectProxyRef->update(Scene::kUpdateAll | Scene::kForceUpdateAllMorphs | Scene::kResetMotionState);
    draw();
}

void RenderTarget::synchronizeImplicitly()
{
    Q_ASSERT(window());
    if (m_projectProxyRef) {
        int flags = m_playing ? Scene::kUpdateAll : (Scene::kUpdateCamera | Scene::kUpdateRenderEngines);
        m_projectProxyRef->update(flags);
    }
}

void RenderTarget::initialize()
{
    Q_ASSERT(window());
    if (!Scene::isInitialized()) {
        bool isCoreProfile = window()->format().profile() == QSurfaceFormat::CoreProfile;
        m_applicationContext.reset(new ApplicationContext(m_projectProxyRef, &m_config, isCoreProfile));
        gl::pushAnnotationGroup(Q_FUNC_INFO, m_applicationContext->sharedFunctionResolverInstance());
        connect(m_applicationContext.data(), &ApplicationContext::fileDidChange, this, &RenderTarget::handleFileChange);
        Scene::initialize(m_applicationContext->sharedFunctionResolverInstance());
        m_graphicsDevice.reset(new GraphicsDevice());
        m_graphicsDevice->initialize();
        emit graphicsDeviceChanged();
        m_applicationContext->initializeOpenGLContext(false);
        m_grid->load(m_applicationContext->sharedFunctionResolverInstance());
        m_applicationContext->setViewportRegion(glm::ivec4(0, 0, window()->width(), window()->height()));
        connect(window()->openglContext(), &QOpenGLContext::aboutToBeDestroyed, m_projectProxyRef, &ProjectProxy::reset, Qt::DirectConnection);
        connect(window()->openglContext(), &QOpenGLContext::aboutToBeDestroyed, this, &RenderTarget::releaseOpenGLResources, Qt::DirectConnection);
        toggleRunning(true);
        disconnect(window(), &QQuickWindow::sceneGraphInitialized, this, &RenderTarget::initialize);
        emit initializedChanged();
        m_renderTimer.start();
        gl::popAnnotationGroup(m_applicationContext->sharedFunctionResolverInstance());
    }
}

void RenderTarget::releaseOpenGLResources()
{
    gl::pushAnnotationGroup(Q_FUNC_INFO, m_applicationContext->sharedFunctionResolverInstance());
    disconnect(m_applicationContext.data(), &ApplicationContext::fileDidChange, this, &RenderTarget::handleFileChange);
    m_applicationContext->deleteAllModelProxies(m_projectProxyRef);
    m_applicationContext->release();
    m_currentGizmoRef = 0;
    m_translationGizmo.reset();
    m_orientationGizmo.reset();
    m_modelDrawer.reset();
    m_debugDrawer.reset();
    m_grid.reset();
    if (m_videoSurface) {
        m_videoSurface->release();
    }
    gl::popAnnotationGroup(m_applicationContext->sharedFunctionResolverInstance());
}

void RenderTarget::enqueueUploadingModel(ModelProxy *model, bool isProject)
{
    Q_ASSERT(window() && model && m_applicationContext);
    const QUuid &uuid = model->uuid();
    VPVL2_VLOG(1, "Enqueued uploading the model " << uuid.toString().toStdString() << " a.k.a " << model->name().toStdString());
    m_applicationContext->enqueueUploadingModel(model, isProject);
}

void RenderTarget::enqueueUploadingEffect(ModelProxy *model)
{
    Q_ASSERT(window() && model && m_applicationContext);
    const QUuid &uuid = model->uuid();
    VPVL2_VLOG(1, "Enqueued uploading the effect " << uuid.toString().toStdString());
    m_applicationContext->enqueueUploadingEffect(model);
}

void RenderTarget::enqueueDeletingModel(ModelProxy *model)
{
    Q_ASSERT(m_applicationContext);
    if (model) {
        VPVL2_VLOG(1, "The model " << model->uuid().toString().toStdString() << " a.k.a " << model->name().toStdString() << " will be released from RenderTarget");
        if (m_modelDrawer) {
            m_modelDrawer->removeModelRef(model);
        }
        m_applicationContext->enqueueDeletingModelProxy(model);
    }
}

void RenderTarget::commitUploadingModels()
{
    Q_ASSERT(window());
    connect(window(), &QQuickWindow::beforeRendering, this, &RenderTarget::performUploadingEnqueuedModels, Qt::DirectConnection);
}

void RenderTarget::commitUploadingEffects()
{
    Q_ASSERT(window());
    connect(window(), &QQuickWindow::beforeRendering, this, &RenderTarget::performUploadingEnqueuedEffects, Qt::DirectConnection);
}

void RenderTarget::commitDeletingModels()
{
    if (QQuickWindow *win = window()) {
        connect(win, &QQuickWindow::beforeRendering, this, &RenderTarget::performDeletingEnqueuedModels, Qt::DirectConnection);
    }
    else {
        performDeletingEnqueuedModels();
    }
}

void RenderTarget::performUploadingEnqueuedModels()
{
    Q_ASSERT(window() && m_applicationContext);
    disconnect(window(), &QQuickWindow::beforeRendering, this, &RenderTarget::performUploadingEnqueuedModels);
    gl::pushAnnotationGroup(Q_FUNC_INFO, m_applicationContext->sharedFunctionResolverInstance());
    QList<ApplicationContext::ModelProxyPair> succeededModelProxies, failedModelProxies;
    m_applicationContext->uploadEnqueuedModelProxies(m_projectProxyRef, succeededModelProxies, failedModelProxies);
    foreach (const ApplicationContext::ModelProxyPair &pair, succeededModelProxies) {
        ModelProxy *modelProxy = pair.first;
        VPVL2_VLOG(1, "The model " << modelProxy->uuid().toString().toStdString() << " a.k.a " << modelProxy->name().toStdString() << " is uploaded" << (pair.second ? " from the project." : "."));
        connect(modelProxy, &ModelProxy::transformTypeChanged, this, &RenderTarget::updateGizmo);
        connect(modelProxy, &ModelProxy::firstTargetBoneChanged, this, &RenderTarget::updateGizmo);
        connect(modelProxy, &ModelProxy::firstTargetBoneChanged, this, &RenderTarget::updateModelBones);
        emit uploadingModelDidSucceed(modelProxy, pair.second);
    }
    foreach (const ApplicationContext::ModelProxyPair &pair, failedModelProxies) {
        ModelProxy *modelProxy = pair.first;
        VPVL2_VLOG(1, "The model " << modelProxy->uuid().toString().toStdString() << " a.k.a " << modelProxy->name().toStdString() << " is not uploaded " << (pair.second ? " from the project." : "."));
        emit uploadingModelDidFail(modelProxy, pair.second);
    }
    emit enqueuedModelsDidUpload();
    gl::popAnnotationGroup(m_applicationContext->sharedFunctionResolverInstance());
}

void RenderTarget::performUploadingEnqueuedEffects()
{
    Q_ASSERT(window() && m_applicationContext);
    disconnect(window(), &QQuickWindow::beforeRendering, this, &RenderTarget::performUploadingEnqueuedEffects);
    gl::pushAnnotationGroup(Q_FUNC_INFO, m_applicationContext->sharedFunctionResolverInstance());
    QList<ModelProxy *> succeededEffects, failedEffects;
    m_applicationContext->uploadEnqueuedEffects(m_projectProxyRef, succeededEffects, failedEffects);
    foreach (ModelProxy *modelProxy, succeededEffects) {
        VPVL2_VLOG(1, "The effect " << modelProxy->uuid().toString().toStdString() << " a.k.a " << modelProxy->name().toStdString() << " is uploaded.");
        emit uploadingEffectDidSucceed(modelProxy);
    }
    foreach (ModelProxy *modelProxy, failedEffects) {
        VPVL2_VLOG(1, "The effect " << modelProxy->uuid().toString().toStdString() << " a.k.a " << modelProxy->name().toStdString() << " is not uploaded.");
        emit uploadingEffectDidFail(modelProxy);
    }
    emit enqueuedEffectsDidUpload();
    gl::popAnnotationGroup(m_applicationContext->sharedFunctionResolverInstance());
}

void RenderTarget::performDeletingEnqueuedModels()
{
    Q_ASSERT(m_applicationContext);
    if (QQuickWindow *win = window()) {
        disconnect(win, &QQuickWindow::beforeRendering, this, &RenderTarget::performDeletingEnqueuedModels);
    }
    gl::pushAnnotationGroup(Q_FUNC_INFO, m_applicationContext->sharedFunctionResolverInstance());
    const QList<ModelProxy *> &deletedModelProxies = m_applicationContext->deleteEnqueuedModelProxies(m_projectProxyRef);
    foreach (ModelProxy *modelProxy, deletedModelProxies) {
        VPVL2_VLOG(1, "The model " << modelProxy->uuid().toString().toStdString() << " a.k.a " << modelProxy->name().toStdString() << " is scheduled to be delete from RenderTarget and will be deleted");
        disconnect(modelProxy, &ModelProxy::transformTypeChanged, this, &RenderTarget::updateGizmo);
        disconnect(modelProxy, &ModelProxy::firstTargetBoneChanged, this, &RenderTarget::updateGizmo);
        modelProxy->deleteLater();
    }
    emit enqueuedModelsDidDelete();
    gl::popAnnotationGroup(m_applicationContext->sharedFunctionResolverInstance());
}

void RenderTarget::performUpdatingLight()
{
    Q_ASSERT(window() && m_applicationContext && m_projectProxyRef);
    disconnect(window(), &QQuickWindow::beforeRendering, this, &RenderTarget::performUpdatingLight);
    gl::pushAnnotationGroup(Q_FUNC_INFO, m_applicationContext->sharedFunctionResolverInstance());
    const LightRefObject *light = m_projectProxyRef->light();
    const qreal &shadowDistance = light->shadowDistance();
    const Vector3 &direction = light->data()->direction(),
            &eye = -direction * shadowDistance,
            &center = direction * shadowDistance;
    const glm::mediump_float &aspectRatio = m_shadowMapSize.x() / float(m_shadowMapSize.y());
    const glm::mat4 &lightView = glm::lookAt(glm::vec3(eye.x(), eye.y(), eye.z()),
                                             glm::vec3(center.x(), center.y(), center.z()),
                                             glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::mat4 &lightProjection = glm::infinitePerspective(45.0f, aspectRatio, 0.1f);
    m_applicationContext->setLightMatrices(glm::mat4(), lightView, lightProjection);
    Scene *scene = m_projectProxyRef->projectInstanceRef();
    if (light->shadowType() == LightRefObject::SelfShadow) {
        const Vector3 size(m_shadowMapSize.x(), m_shadowMapSize.y(), 1);
        m_applicationContext->createShadowMap(size);
    }
    else {
        scene->setShadowMapRef(0);
    }
    gl::popAnnotationGroup(m_applicationContext->sharedFunctionResolverInstance());
}

void RenderTarget::disconnectProjectSignals()
{
    /* disable below signals behavior while loading project */
    disconnect(m_projectProxyRef, &ProjectProxy::motionDidLoad, this, &RenderTarget::prepareSyncMotionState);
    disconnect(this, &RenderTarget::shadowMapSizeChanged, this, &RenderTarget::prepareUpdatingLight);
    disconnect(m_projectProxyRef, &ProjectProxy::rewindDidPerform, this, &RenderTarget::prepareSyncMotionState);
    disconnect(m_projectProxyRef->world(), &WorldProxy::simulationTypeChanged, this, &RenderTarget::prepareSyncMotionState);
    disconnect(m_projectProxyRef->light(), &LightRefObject::directionChanged, this, &RenderTarget::prepareUpdatingLight);
    disconnect(m_projectProxyRef->light(), &LightRefObject::shadowTypeChanged, this, &RenderTarget::prepareUpdatingLight);
}

void RenderTarget::releaseVideoSurface()
{
    /* prepareUploadingModelsInProject -> releaseVideoSurface -> resetMediaPlayer */
    disconnect(this, &RenderTarget::enqueuedModelsDidUpload, this, &RenderTarget::releaseVideoSurface);
    connect(this, &RenderTarget::videoSurfaceDidRelease, this, &RenderTarget::resetMediaPlayer);
    if (m_videoSurface) {
        gl::pushAnnotationGroup(Q_FUNC_INFO, m_applicationContext->sharedFunctionResolverInstance());
        m_videoSurface->release();
        emit videoSurfaceDidRelease();
        gl::popAnnotationGroup(m_applicationContext->sharedFunctionResolverInstance());
    }
}

void RenderTarget::resetMediaPlayer()
{
    Q_ASSERT(m_projectProxyRef);
    disconnect(this, &RenderTarget::videoSurfaceDidRelease, this, &RenderTarget::resetMediaPlayer);
    m_mediaPlayer.reset();
    m_videoSurface.reset();
    const QUrl &url = m_projectProxyRef->videoSource();
    if (!url.isEmpty() && url.isValid()) {
        setVideoUrl(url);
    }
}

void RenderTarget::prepareUploadingModelsInProject()
{
    /* must use Qt::DirectConnection to release OpenGL resources and recreate shadowmap */
    connect(this, &RenderTarget::enqueuedModelsDidUpload, this, &RenderTarget::activateProject, Qt::DirectConnection);
    /* must use Qt::DirectConnection due to VideoSurface contains OpenGL resources */
    connect(this, &RenderTarget::enqueuedModelsDidUpload, this, &RenderTarget::releaseVideoSurface, Qt::DirectConnection);
    commitUploadingModels();
}

void RenderTarget::activateProject()
{
    Q_ASSERT(m_applicationContext && m_projectProxyRef);
    disconnect(this, &RenderTarget::enqueuedModelsDidUpload, this, &RenderTarget::activateProject);
    setShadowMapSize(m_projectProxyRef->globalSetting("shadow.texture.size", kDefaultShadowMapSize));
    m_applicationContext->release();
    m_applicationContext->resetOrderIndex(m_projectProxyRef->modelProxies().count() + 1);
    m_applicationContext->createShadowMap(Vector3(m_shadowMapSize.x(), m_shadowMapSize.y(), 1));
    m_grid->setVisible(m_projectProxyRef->isGridVisible());
    connect(this, &RenderTarget::shadowMapSizeChanged, this, &RenderTarget::prepareUpdatingLight);
    connect(m_projectProxyRef, &ProjectProxy::motionDidLoad, this, &RenderTarget::prepareSyncMotionState);
    connect(m_projectProxyRef, &ProjectProxy::rewindDidPerform, this, &RenderTarget::prepareSyncMotionState);
    connect(m_projectProxyRef->world(), &WorldProxy::simulationTypeChanged, this, &RenderTarget::prepareSyncMotionState);
    connect(m_projectProxyRef->light(), &LightRefObject::directionChanged, this, &RenderTarget::prepareUpdatingLight);
    connect(m_projectProxyRef->light(), &LightRefObject::shadowTypeChanged, this, &RenderTarget::prepareUpdatingLight);
    prepareSyncMotionState();
    prepareUpdatingLight();
}

QMediaPlayer *RenderTarget::mediaPlayer() const
{
    if (!m_mediaPlayer) {
        m_mediaPlayer.reset(new QMediaPlayer());
        m_videoSurface.reset(new VideoSurface(m_mediaPlayer.data()));
        connect(m_mediaPlayer.data(), SIGNAL(error(QMediaPlayer::Error)), this, SLOT(handleMediaPlayerError(QMediaPlayer::Error)));
    }
    return m_mediaPlayer.data();
}

EncodingTask *RenderTarget::encodingTask() const
{
    if (!m_encodingTask) {
        m_encodingTask.reset(new EncodingTask());
        connect(m_encodingTask.data(), &EncodingTask::encodeDidBegin, this, &RenderTarget::encodeDidBegin);
        connect(m_encodingTask.data(), &EncodingTask::encodeDidProceed, this, &RenderTarget::encodeDidProceed);
        connect(m_encodingTask.data(), &EncodingTask::encodeDidFinish, this, &RenderTarget::encodeDidFinish);
    }
    return m_encodingTask.data();
}

IGizmo *RenderTarget::translationGizmo() const
{
    if (!m_translationGizmo) {
        m_translationGizmo.reset(CreateMoveGizmo());
        m_translationGizmo->SetSnap(m_snapStepSize.x(), m_snapStepSize.y(), m_snapStepSize.z());
        m_translationGizmo->SetEditMatrix(const_cast<float *>(m_editMatrix.data()));
    }
    return m_translationGizmo.data();
}

IGizmo *RenderTarget::orientationGizmo() const
{
    if (!m_orientationGizmo) {
        m_orientationGizmo.reset(CreateRotateGizmo());
        m_orientationGizmo->SetEditMatrix(const_cast<float *>(m_editMatrix.data()));
        m_orientationGizmo->SetAxisMask(m_visibleGizmoMasks);
    }
    return m_orientationGizmo.data();
}

void RenderTarget::resetOpenGLStates()
{
    gl::pushAnnotationGroup(Q_FUNC_INFO, m_applicationContext->sharedFunctionResolverInstance());
    window()->resetOpenGLState();
    Scene::setRequiredOpenGLState();
    gl::popAnnotationGroup(m_applicationContext->sharedFunctionResolverInstance());
}

void RenderTarget::clearScene()
{
    const QColor &color = m_projectProxyRef ? m_projectProxyRef->screenColor() : QColor(Qt::white);
    glClearColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void RenderTarget::drawVideoFrame()
{
    if (m_videoSurface) {
        gl::pushAnnotationGroup(Q_FUNC_INFO, m_applicationContext->sharedFunctionResolverInstance());
        m_videoSurface->initialize();
        m_videoSurface->renderVideoFrame();
        gl::popAnnotationGroup(m_applicationContext->sharedFunctionResolverInstance());
    }
}

void RenderTarget::drawGrid()
{
    gl::pushAnnotationGroup(Q_FUNC_INFO, m_applicationContext->sharedFunctionResolverInstance());
    m_grid->draw(m_viewProjectionMatrix);
    gl::popAnnotationGroup(m_applicationContext->sharedFunctionResolverInstance());
}

void RenderTarget::drawShadowMap()
{
    gl::pushAnnotationGroup(Q_FUNC_INFO, m_applicationContext->sharedFunctionResolverInstance());
    m_applicationContext->renderShadowMap();
    m_applicationContext->renderOffscreen();
    gl::popAnnotationGroup(m_applicationContext->sharedFunctionResolverInstance());
}

void RenderTarget::drawScene()
{
    Array<IRenderEngine *> enginesForPreProcess, enginesForStandard, enginesForPostProcess;
    Hash<HashPtr, IEffect *> nextPostEffects;
    Scene *scene = m_projectProxyRef->projectInstanceRef();
    scene->getRenderEnginesByRenderOrder(enginesForPreProcess,
                                         enginesForStandard,
                                         enginesForPostProcess,
                                         nextPostEffects);
    const bool isProjectiveShadow = m_projectProxyRef->light()->shadowType() == LightRefObject::ProjectiveShadow;
    gl::pushAnnotationGroup(Q_FUNC_INFO, m_applicationContext->sharedFunctionResolverInstance());
    for (int i = enginesForPostProcess.count() - 1; i >= 0; i--) {
        IRenderEngine *engine = enginesForPostProcess[i];
        engine->preparePostProcess();
    }
    for (int i = 0, nengines = enginesForPreProcess.count(); i < nengines; i++) {
        IRenderEngine *engine = enginesForPreProcess[i];
        engine->performPreProcess();
    }
    for (int i = 0, nengines = enginesForStandard.count(); i < nengines; i++) {
        IRenderEngine *engine = enginesForStandard[i];
        engine->renderModel();
        engine->renderEdge();
        if (isProjectiveShadow) {
            engine->renderShadow();
        }
    }
    for (int i = 0, nengines = enginesForPostProcess.count(); i < nengines; i++) {
        IRenderEngine *engine = enginesForPostProcess[i];
        IEffect *const *nextPostEffect = nextPostEffects[engine];
        engine->performPostProcess(*nextPostEffect);
    }
    gl::popAnnotationGroup(m_applicationContext->sharedFunctionResolverInstance());
}

void RenderTarget::drawDebug()
{
    Q_ASSERT(m_projectProxyRef);
    WorldProxy *worldProxy = m_projectProxyRef->world();
    if (worldProxy->isDebugEnabled() && worldProxy->simulationType() != WorldProxy::DisableSimulation) {
        gl::pushAnnotationGroup(Q_FUNC_INFO, m_applicationContext->sharedFunctionResolverInstance());
        if (!m_debugDrawer) {
            m_debugDrawer.reset(new DebugDrawer());
            m_debugDrawer->initialize();
            m_debugDrawer->setModelViewProjectionMatrix(Util::fromMatrix4(m_viewProjectionMatrix));
            m_debugDrawer->setDebugMode(btIDebugDraw::DBG_DrawAabb |
                                        // btIDebugDraw::DBG_DrawConstraintLimits |
                                        // btIDebugDraw::DBG_DrawConstraints |
                                        // btIDebugDraw::DBG_DrawContactPoints |
                                        // btIDebugDraw::DBG_DrawFeaturesText |
                                        // btIDebugDraw::DBG_DrawText |
                                        // btIDebugDraw::DBG_FastWireframe |
                                        // btIDebugDraw::DBG_DrawWireframe |
                                        0);
            worldProxy->setDebugDrawer(m_debugDrawer.data());
        }
        worldProxy->debugDraw();
        m_debugDrawer->flush();
        gl::popAnnotationGroup(m_applicationContext->sharedFunctionResolverInstance());
    }
}

void RenderTarget::drawModelBones()
{
    Q_ASSERT(m_projectProxyRef);
    ModelProxy *currentModelRef = m_projectProxyRef->currentModel();
    if (!m_playing && m_editMode == SelectMode && currentModelRef && currentModelRef->isVisible()) {
        gl::pushAnnotationGroup(Q_FUNC_INFO, m_applicationContext->sharedFunctionResolverInstance());
        if (!m_modelDrawer) {
            m_modelDrawer.reset(new ModelDrawer());
            connect(m_projectProxyRef, &ProjectProxy::currentTimeIndexChanged, m_modelDrawer.data(), &RenderTarget::ModelDrawer::markDirty);
            connect(m_projectProxyRef, &ProjectProxy::undoDidPerform, m_modelDrawer.data(), &RenderTarget::ModelDrawer::markDirty);
            connect(m_projectProxyRef, &ProjectProxy::redoDidPerform, m_modelDrawer.data(), &RenderTarget::ModelDrawer::markDirty);
            m_modelDrawer->initialize();
            m_modelDrawer->setModelViewProjectionMatrix(Util::fromMatrix4(m_viewProjectionMatrix));
            m_modelDrawer->setModelProxyRef(currentModelRef);
        }
        m_modelDrawer->setModelProxyRef(currentModelRef);
        m_modelDrawer->draw();
        gl::popAnnotationGroup(m_applicationContext->sharedFunctionResolverInstance());
    }
}

void RenderTarget::drawCurrentGizmo()
{
    if (!m_playing && m_currentGizmoRef) {
        gl::pushAnnotationGroup(Q_FUNC_INFO, m_applicationContext->sharedFunctionResolverInstance());
        m_currentGizmoRef->Draw();
        gl::popAnnotationGroup(m_applicationContext->sharedFunctionResolverInstance());
    }
}

void RenderTarget::drawOffscreen(QOpenGLFramebufferObject *fbo)
{
    gl::pushAnnotationGroup(Q_FUNC_INFO, m_applicationContext->sharedFunctionResolverInstance());
    m_applicationContext->setViewportRegion(glm::ivec4(0, 0, fbo->width(), fbo->height()));
    Scene::setRequiredOpenGLState();
    drawShadowMap();
    Q_ASSERT(fbo->isValid());
    fbo->bind();
    glViewport(0, 0, fbo->width(), fbo->height());
    clearScene();
    drawScene();
    m_applicationContext->setViewportRegion(glm::ivec4(m_viewport.x(), m_viewport.y(), m_viewport.width(), m_viewport.height()));
    QOpenGLFramebufferObject::bindDefault();
    gl::popAnnotationGroup(m_applicationContext->sharedFunctionResolverInstance());
}

void RenderTarget::updateViewport()
{
    Q_ASSERT(m_applicationContext);
    int w = m_viewport.width(), h = m_viewport.height();
    if (isDirty()) {
        glm::mat4 cameraWorld, cameraView, cameraProjection;
        gl::pushAnnotationGroup(Q_FUNC_INFO, m_applicationContext->sharedFunctionResolverInstance());
        m_applicationContext->setViewportRegion(glm::ivec4(m_viewport.x(), m_viewport.y(), w, h));
        m_applicationContext->updateCameraMatrices();
        m_applicationContext->getCameraMatrices(cameraWorld, cameraView, cameraProjection);
        m_viewMatrix = cameraView;
        m_projectionMatrix = cameraProjection;
        m_viewProjectionMatrix = cameraProjection * cameraView;
        if (m_debugDrawer) {
            m_debugDrawer->setModelViewProjectionMatrix(Util::fromMatrix4(m_viewProjectionMatrix));
        }
        if (m_modelDrawer) {
            m_modelDrawer->setModelViewProjectionMatrix(Util::fromMatrix4(m_viewProjectionMatrix));
        }
        IGizmo *translationGizmoRef = translationGizmo(), *orientationGizmoRef = orientationGizmo();
        translationGizmoRef->SetScreenDimension(w, h);
        translationGizmoRef->SetCameraMatrix(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection));
        orientationGizmoRef->SetScreenDimension(w, h);
        orientationGizmoRef->SetCameraMatrix(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection));
        emit viewMatrixChanged();
        emit projectionMatrixChanged();
        setDirty(false);
        gl::popAnnotationGroup(m_applicationContext->sharedFunctionResolverInstance());
    }
    glViewport(m_viewport.x(), m_viewport.y(), w, h);
}

void RenderTarget::seekVideo(const qreal &value)
{
    if (m_mediaPlayer) {
        const quint64 &position = qRound64((value / Scene::defaultFPS()) * 1000);
        m_mediaPlayer->setPosition(position);
    }
}

#include "RenderTarget.moc"
