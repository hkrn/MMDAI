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

#ifndef HANDLES_H
#define HANDLES_H

#include <QtOpenGL/QtOpenGL>
#include <QtCore/QRectF>
#include <QtCore/QSize>

#include <vpvl/Common.h>

namespace internal {
class TextureDrawHelper;
}

namespace vpvl {
class Asset;
class Bone;
}

class btBvhTriangleMeshShape;
class btRigidBody;
class btTriangleMesh;
class HandlesStaticWorld;
class SceneWidget;

class Handles : public QObject
{
    Q_OBJECT

public:
    struct Texture {
        QSize size;
        QRectF rect;
        GLuint textureID;
    };
    struct ImageHandle {
        Texture enableMove;
        Texture disableMove;
        Texture enableRotate;
        Texture disableRotate;
    };
    struct Vertex {
        vpvl::Vector3 position;
        vpvl::Vector3 normal;
    };
    struct Model {
        vpvl::Array<Vertex> vertices;
        vpvl::Array<uint16_t> indices;
        btRigidBody *body;
    };
    struct RotationHandle {
        vpvl::Asset *asset;
        Model x;
        Model y;
        Model z;
        GLuint indicesBuffer;
        GLuint verticesBuffer;
    };
    struct TranslationHandle : public RotationHandle {
        Model axisX;
        Model axisY;
        Model axisZ;
    };

    enum Flags {
        kNone       = 0x0,
        kEnable     = 0x1,
        kDisable    = 0x2,
        kMove       = 0x4,
        kRotate     = 0x8,
        kX          = 0x10,
        kY          = 0x20,
        kZ          = 0x40,
        kGlobal     = 0x80,
        kLocal      = 0x100,
        kView       = 0x200,
        kVisibleAll = kMove | kRotate | kX | kY | kZ | kView
    };

    Handles(SceneWidget *parent);
    ~Handles();

    void load();
    void resize(int width, int height);
    bool testHitModel(const vpvl::Vector3 &rayFrom,
                      const vpvl::Vector3 &rayTo,
                      bool setTracked,
                      int &flags,
                      vpvl::Vector3 &pick);
    bool testHitImage(const QPointF &p,
                      bool movable,
                      bool rotateable,
                      int &flags,
                      QRectF &rect);
    void drawImageHandles(bool movable, bool rotateable);
    void drawModelHandles();
    const btScalar angle(const vpvl::Vector3 &pos) const;

    void setPoint3D(const vpvl::Vector3 &value);
    void setPoint2D(const QPointF &value);
    void setAngle(float value);
    void setRotateDirection(bool value);
    const vpvl::Vector3 diffPoint3D(const vpvl::Vector3 &value) const;
    const QPointF diffPoint2D(const QPointF &value) const;
    float diffAngle(float value) const;
    bool isPoint3DZero() const { return m_prevPos3D.isZero(); }
    bool isAngleZero() const { return m_prevAngle == 0.0f; }

    bool isLocal() const { return m_isLocal; }
    void setBone(vpvl::Bone *value);
    void setLocal(bool value);
    void setVisible(bool value);
    void setVisibilityFlags(int value);

private slots:
    void updateBone();

private:
    void drawModel(const Handles::Model &model,
                   const QColor &color,
                   int requiredVisibilityFlags);
    void loadImageHandles();
    void loadModelHandles();

    internal::TextureDrawHelper *m_helper;
    vpvl::Bone *m_bone;
    HandlesStaticWorld *m_world;
    SceneWidget *m_widget;
    QGLShaderProgram m_program;
    RotationHandle m_rotationHandle;
    TranslationHandle m_translationHandle;
    Model *m_trackedHandle;
    ImageHandle m_x;
    ImageHandle m_y;
    ImageHandle m_z;
    Texture m_global;
    Texture m_local;
    vpvl::Vector3 m_prevPos3D;
    QPointF m_prevPos2D;
    float m_prevAngle;
    int m_width;
    int m_height;
    int m_visibilityFlags;
    bool m_isLocal;
    bool m_visible;

    Q_DISABLE_COPY(Handles)
};

#endif // HANDLES_H
