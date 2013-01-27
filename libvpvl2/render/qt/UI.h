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

#pragma once
#ifndef VPVL2_RENDER_QT_UI_H_
#define VPVL2_RENDER_QT_UI_H_

#include <vpvl2/vpvl2.h>
#include <vpvl2/IEffect.h>
#include <vpvl2/extensions/icu/Encoding.h>
#include <vpvl2/extensions/icu/StringMap.h>
#include <vpvl2/qt/RenderContext.h>

#include <QBasicTimer>
#include <QGLWidget>
#include <QProgressDialog>
#include <QSettings>

namespace vpvl2
{
class Factory;
class Scene;

namespace extensions
{
namespace gl
{
class SimpleShadowMap;
}
}
namespace qt
{
class TextureDrawHelper;
}

namespace render
{
namespace qt
{

using namespace vpvl2::extensions;
using namespace vpvl2::qt;

class UI : public QGLWidget
{
public:
    UI(const QGLFormat &format);
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
    void createEncoding(QSettings *settings);
    void renderDepth();
    void renderWindow();
    void setMousePositions(QMouseEvent *event);
    bool loadScene();
    IModel *createModelAsync(const QString &path);
    IMotion *createMotionAsync(const QString &path, IModel *model);
    IModel *addModel(const QString &path, QProgressDialog &dialog, int index);
    IMotion *addMotion(const QString &path, IModel *model);
    IMotion *loadMotion(const QString &path, IModel *model);

    QScopedPointer<QSettings> m_settings;
    QScopedPointer<World> m_world;
    QScopedPointer<RenderContext> m_renderContext;
    QScopedPointer<Scene> m_scene;
    QScopedPointer<Factory> m_factory;
    QScopedPointer<IEncoding> m_encoding;
    QScopedPointer<TextureDrawHelper> m_helper;
    QBasicTimer m_updateTimer;
    QElapsedTimer m_refreshTimer;
    QPoint m_prevPos;
    StringMap m_stringMapRef;
    Encoding::Dictionary m_dictionary;
    float m_prevElapsed;
    float m_currentFrameIndex;
};

} /* namespace qt */
} /* namespace render */
} /* namespace vpvl2 */

#endif
