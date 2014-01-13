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

#ifndef SKELETONDRAWER_H
#define SKELETONDRAWER_H

#include <vpvl2/Common.h>

#include <QObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QScopedPointer>
#include <QVarLengthArray>

class ModelProxy;

namespace vpvl2 {
class IBone;
}

class SkeletonDrawer : public QObject
{
    Q_OBJECT

public:
    SkeletonDrawer(QObject *parent = 0);
    ~SkeletonDrawer();

    void initialize();
    void setModelProxyRef(const ModelProxy *value);
    const ModelProxy *currentModelProxyRef() const;
    void setViewProjectionMatrix(const QMatrix4x4 &value);
    void draw();
    void update();
    void removeModelRef(const ModelProxy *modelProxyRef);

public slots:
    void markDirty();

signals:
    void modelDidMarkDirty();

private:
    struct Vertex {
        vpvl2::Vector3 position;
        vpvl2::Vector3 color;
    };
    enum VertexType {
        kPositionAttribute,
        kColorAttribute
    };
    static const float kOpacity;
    static void allocateBuffer(QOpenGLBuffer::Type type, QOpenGLBuffer::UsagePattern usage, QScopedPointer<QOpenGLBuffer> &buffer);

    void bindAttributeBuffers();
    void bindProgram();
    void releaseProgram();
    void updateBoneVertices(const vpvl2::IBone *boneRef, bool selected, QVarLengthArray<Vertex> &vertices, bool &proceed) const;

    const ModelProxy *m_currentModelRef;
    QScopedPointer<QOpenGLShaderProgram> m_program;
    QScopedPointer<QOpenGLVertexArrayObject> m_vao;
    QScopedPointer<QOpenGLBuffer> m_vbo;
    QScopedPointer<QOpenGLBuffer> m_ibo;
    QMatrix4x4 m_viewProjectionMatrix;
    int m_nvertices;
    int m_nindices;
    volatile bool m_dirty;
};

#endif

