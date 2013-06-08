/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
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

#ifndef VPVM_HANDLES_H_
#define VPVM_HANDLES_H_

#include <vpvl2/Common.h>
#include <assimp/assimp.hpp>
#include <assimp/aiMesh.h>

#include <QObject>
#include <QColor>
#include <QRectF>
#include <QScopedPointer>
#include <QSize>

namespace vpvl2 {
class IBone;
class IModel;
class IRenderContext;
namespace extensions {
namespace gl {
class ShaderProgram;
}
}
namespace qt {
class TextureDrawHelper;
}
}

class btBvhTriangleMeshShape;
class btMotionState;
class btRigidBody;
class btTriangleMesh;

namespace vpvm
{

using namespace vpvl2;
using namespace vpvl2::qt;
class SceneLoader;

class Handles : public QObject
{
    Q_OBJECT

public:
    class StaticWorld;

    enum Flags {
        kNone          = 0x0,
        kEnable        = 0x1,
        kDisable       = 0x2,
        kMove          = 0x4,
        kRotate        = 0x8,
        kX             = 0x10,
        kY             = 0x20,
        kZ             = 0x40,
        kGlobal        = 0x80,
        kLocal         = 0x100,
        kView          = 0x200,
        kModel         = 0x400,
        kOperation     = kGlobal | kLocal | kView,
        kVisibleMove   = kMove   | kX | kY | kZ | kModel,
        kVisibleRotate = kRotate | kX | kY | kZ | kModel,
        kVisibleAll    = kMove   | kRotate | kX | kY | kZ | kModel
    };

    static bool isToggleButton(int value);

    Handles(SceneLoader *loaderRef, IRenderContext *renderContextRef, const QSize &size);
    ~Handles();

    void loadImageHandles();
    void loadModelHandles();
    void resize(const QSize &size);
    bool testHitModel(const Vector3 &rayFrom,
                      const Vector3 &rayTo,
                      bool setTracked,
                      int &flags,
                      Vector3 &pick);
    bool testHitImage(const QPointF &p,
                      bool movable,
                      bool rotateable,
                      int &flags,
                      QRectF &rect);
    void drawImageHandles(IBone *bone);
    void drawRotationHandle();
    void drawMoveHandle();
    btScalar angle(const Vector3 &pos) const;

    void setPoint3D(const Vector3 &value);
    void setPoint2D(const QPointF &value);
    void setAngle(float value);
    void setRotateDirection(bool value);
    const Vector3 diffPoint3D(const Vector3 &value) const;
    const QPointF diffPoint2D(const QPointF &value) const;
    float diffAngle(float value) const;
    IBone *currentBone() const { return m_boneRef; }
    bool isPoint3DZero() const { return m_prevPos3D.isZero(); }
    bool isAngleZero() const { return m_prevAngle == 0.0f; }
    Flags constraint() const { return m_constraint; }
    int modeFromConstraint() const;
    const Transform modelHandleTransform() const;

    void setState(Flags value);
    void setBone(IBone *value);
    void setLocal(bool value);
    void setVisible(bool value);
    void setVisibilityFlags(int value);

private slots:
    void updateHandleModel();

private:
    class Model;
    class PrivateShaderProgram;
    struct Texture {
        void load(const QString &path);
        QSize size;
        QRectF rect;
        intptr_t textureID;
    };
    struct ImageHandle {
        Texture enableMove;
        Texture disableMove;
        Texture enableRotate;
        Texture disableRotate;
    };
    struct Vertex {
        Vector4 position;
        Vector3 normal;
    };
    struct RotationHandle {
        Assimp::Importer importer;
        QScopedPointer<Model> x;
        QScopedPointer<Model> y;
        QScopedPointer<Model> z;
    };
    struct TranslationHandle : public RotationHandle {
        QScopedPointer<Model> axisX;
        QScopedPointer<Model> axisY;
        QScopedPointer<Model> axisZ;
    };

    void drawModel(Model *model, const QColor &color, int requiredVisibilityFlags);
    void beginDrawing();
    void flushDrawing();

    QScopedPointer<PrivateShaderProgram> m_program;
    QScopedPointer<TextureDrawHelper> m_helper;
    QScopedPointer<StaticWorld> m_world;
    IRenderContext *m_renderContextRef;
    IBone *m_boneRef;
    SceneLoader *m_loaderRef;
    RotationHandle m_rotationHandle;
    TranslationHandle m_translationHandle;
    Model *m_trackedHandleRef;
    ImageHandle m_x;
    ImageHandle m_y;
    ImageHandle m_z;
    Texture m_global;
    Texture m_local;
    Texture m_view;
    Flags m_constraint;
    Vector3 m_prevPos3D;
    QPointF m_prevPos2D;
    float m_prevAngle;
    int m_visibilityFlags;
    bool m_visible;
    bool m_handleModelsAreLoaded;

    Q_DISABLE_COPY(Handles)
};

} /* namespace vpvm */

#endif // HANDLES_H
