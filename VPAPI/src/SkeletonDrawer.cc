/**

 Copyright (c) 2010-2014  hkrn

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

#include "SkeletonDrawer.h"

#include <vpvl2/vpvl2.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "BoneRefObject.h"
#include "ModelProxy.h"
#include "Util.h"

namespace {

using namespace vpvl2;

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

}

SkeletonDrawer::SkeletonDrawer(QObject *parent)
    : QObject(parent),
      m_currentModelRef(0),
      m_nvertices(0),
      m_nindices(0),
      m_dirty(false)
{
}

SkeletonDrawer::~SkeletonDrawer()
{
}

void SkeletonDrawer::initialize()
{
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

void SkeletonDrawer::setModelProxyRef(const ModelProxy *value)
{
    const QList<BoneRefObject *> &allBones = value ? value->allBoneRefs() : QList<BoneRefObject *>();
    QVarLengthArray<Vertex> vertices;
    QVarLengthArray<int> indices;
    bool proceed;
    vertices.reserve(allBones.size() * kNumBoneVertices);
    indices.reserve(allBones.size() * kNumBoneIndices);
    if (value && m_currentModelRef != value) {
        removeModelRef(m_currentModelRef);
        connect(value, &ModelProxy::targetBonesDidCommitTransform, this, &SkeletonDrawer::markDirty);
        connect(value, &ModelProxy::firstTargetBoneChanged, this, &SkeletonDrawer::markDirty);
        int offset = 0;
        foreach (const BoneRefObject *bone, allBones) {
            const IBone *boneRef = bone->data();
            connect(bone, &BoneRefObject::localTranslationChanged, this, &SkeletonDrawer::markDirty);
            connect(bone, &BoneRefObject::localOrientationChanged, this, &SkeletonDrawer::markDirty);
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

const ModelProxy *SkeletonDrawer::currentModelProxyRef() const
{
    return m_currentModelRef;
}

void SkeletonDrawer::setViewProjectionMatrix(const QMatrix4x4 &value)
{
    m_viewProjectionMatrix = value;
}

void SkeletonDrawer::draw(const QMatrix4x4 &worldMatrix)
{
    bindProgram();
    m_program->setUniformValue("modelViewProjectionMatrix", m_viewProjectionMatrix * worldMatrix);
    glDrawElements(GL_TRIANGLES, m_nindices, GL_UNSIGNED_INT, 0);
    releaseProgram();
}

void SkeletonDrawer::update()
{
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

void SkeletonDrawer::removeModelRef(const ModelProxy *modelProxyRef)
{
    if (modelProxyRef) {
        disconnect(modelProxyRef, &ModelProxy::targetBonesDidCommitTransform, this, &SkeletonDrawer::markDirty);
        disconnect(modelProxyRef, &ModelProxy::firstTargetBoneChanged, this, &SkeletonDrawer::markDirty);
        foreach (const BoneRefObject *bone, modelProxyRef->allBoneRefs()) {
            disconnect(bone, &BoneRefObject::localTranslationChanged, this, &SkeletonDrawer::markDirty);
            disconnect(bone, &BoneRefObject::localOrientationChanged, this, &SkeletonDrawer::markDirty);
        }
        if (modelProxyRef == m_currentModelRef) {
            m_currentModelRef = 0;
        }
    }
}

void SkeletonDrawer::markDirty()
{
    m_dirty = true;
    emit modelDidMarkDirty();
}

const float SkeletonDrawer::kOpacity = 0.25;

void SkeletonDrawer::allocateBuffer(QOpenGLBuffer::Type type, QOpenGLBuffer::UsagePattern usage, QScopedPointer<QOpenGLBuffer> &buffer)
{
    buffer.reset(new QOpenGLBuffer(type));
    buffer->create();
    buffer->bind();
    buffer->setUsagePattern(usage);
    buffer->release();
}

void SkeletonDrawer::bindAttributeBuffers()
{
    m_program->enableAttributeArray(kPositionAttribute);
    m_program->enableAttributeArray(kColorAttribute);
    m_vbo->bind();
    m_program->setAttributeBuffer(kPositionAttribute, GL_FLOAT, 0, 3, sizeof(Vertex));
    m_program->setAttributeBuffer(kColorAttribute, GL_FLOAT, 16, 4, sizeof(Vertex));
    m_ibo->bind();
}

void SkeletonDrawer::bindProgram()
{
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

void SkeletonDrawer::releaseProgram()
{
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

void SkeletonDrawer::updateBoneVertices(const IBone *boneRef, bool selected, QVarLengthArray<Vertex> &vertices, bool &proceed) const
{
    const Vector3 &origin = boneRef->worldTransform().getOrigin(), &delta = boneRef->destinationOrigin() - origin;
    if (boneRef->isInteractive() && !delta.isZero()) {
        static const glm::vec3 kGLMUnitY(0, 1, 0);
        const glm::vec3 scale(1, delta.length(), 1), normal(glm::normalize(glm::vec3(delta.x(), delta.y(), delta.z())));
        const glm::mat4 matrix(glm::scale(glm::toMat4(glm::rotation(kGLMUnitY, normal)), scale));
        Vector3 color(Util::toColorRGB(Util::kBlue));
        Scalar opacity(kOpacity);
        Transform transform(Transform::getIdentity());
        transform.setFromOpenGLMatrix(glm::value_ptr(matrix));
        transform.setOrigin(origin);
        if (selected) {
            color = Util::toColorRGB(Util::kRed);
            opacity = 1.0;
        }
        else if (boneRef->hasFixedAxes()) {
            color = Util::toColorRGB(Qt::magenta);
        }
        else if (boneRef->hasLocalAxes()) {
            color = Util::toColorRGB(Qt::cyan);
        }
        else if (boneRef->hasInverseKinematics()) {
            color = Util::toColorRGB(Util::kYellow);
        }
        Vertex v;
        for (int i = 0; i < kNumBoneVertices; i++) {
            v.position = transform * kBoneVertices[i];
            v.color = color;
            v.color.setW(opacity);
            vertices.append(v);
        }
        proceed = true;
    }
    else {
        proceed = false;
    }
}

