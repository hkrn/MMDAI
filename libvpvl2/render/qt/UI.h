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

#ifndef VPVL2_RENDER_QT_UI_H_
#define VPVL2_RENDER_QT_UI_H_

#include "vpvl2/Scene.h"

#ifndef VPVL2_NO_BULLET
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#else
VPVL2_DECLARE_HANDLE(btDiscreteDynamicsWorld)
#endif

#include <QtGui/QtGui>
#include <QtOpenGL/QtOpenGL>

namespace vpvl2
{
class Factory;
class IEffect;
class IModel;
class IMotion;

namespace render
{
namespace qt
{
class Delegate;
class String;

class UI : public QGLWidget
{
public:
    UI();
    ~UI();

    void load(const QString &filename);
    void rotate(float x, float y);
    void translate(float x, float y);

protected:
    void closeEvent(QCloseEvent *event);
    void initializeGL();
    void timerEvent(QTimerEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void resizeGL(int w, int h);
    void paintGL();

private:
    void setMousePositions(QMouseEvent *event);
    bool loadScene();
    IModel *createModelAsync(const QString &path) const;
    IEffect *createEffectAsync(const IString *path);
    IEffect *createEffectAsync(const IModel *model, const IString *dir);
    IMotion *createMotionAsync(const QString &path, IModel *model) const;
    IModel *addModel(const QString &path, QProgressDialog &dialog);
    IMotion *addMotion(const QString &path, IModel *model);
    IMotion *loadMotion(const QString &path, IModel *model);

#ifndef VPVL2_NO_BULLET
    btDefaultCollisionConfiguration m_config;
    btCollisionDispatcher m_dispatcher;
    btAxisSweep3 m_broadphase;
    btSequentialImpulseConstraintSolver m_solver;
    btDiscreteDynamicsWorld m_world;
#endif /* VPVL2_NO_BULLET */
    QSettings *m_settings;
    QElapsedTimer m_timer;
    QGLFramebufferObject *m_fbo;
    QPoint m_prevPos;
    QMatrix4x4 m_projectionMatrix;
    QMatrix4x4 m_modelViewMatrix;
    typedef QPair<QRegExp, IEffect *> EffectAttachment;
    typedef QPair<void *, QList<EffectAttachment> > OffscreenRenderTarget;
    QList<OffscreenRenderTarget> m_offscreens;
    QHash<QString, IEffect *> m_effectCaches;
    QMutex m_effectCacheLock;
    Delegate *m_delegate;
    Scene m_scene;
    Factory *m_factory;
    IEncoding *m_encoding;
    GLuint m_depthTextureID;
    float m_prevElapsed;
    float m_currentFrameIndex;
};

}
}
}

#endif
