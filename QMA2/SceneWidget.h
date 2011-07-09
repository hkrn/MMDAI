/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn                                    */
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

#ifndef SCENEWIDGET_H
#define SCENEWIDGET_H

#include <QtCore/QtCore>
#include <GL/glew.h>
#include <QtOpenGL/QGLWidget>

#include <LinearMath/btVector3.h>
#include <vpvl/Bone.h>

namespace vpvl {
class PMDModel;
class Scene;
class VMDMotion;
class XModel;
}

class Grid;
class World;
class QProgressDialog;

class SceneWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit SceneWidget(QSettings *settings, QWidget *parent = 0);
    ~SceneWidget();

    void setCurrentFPS(int value);

    const QHash<QString, vpvl::PMDModel *> &models() const {
        return m_models;
    }
    vpvl::PMDModel *selectedModel() const {
        return m_selected;
    }
    void setSelectedModel(vpvl::PMDModel *value) {
        m_selected = value;
        emit modelDidSelect(value);
    }

    static const QString toUnicodeModelName(const vpvl::PMDModel *model);

public slots:
    void addModel();
    void insertMotionToAllModels();
    void insertMotionToSelectedModel();
    void addAsset();
    void setCamera();
    void deleteSelectedModel();
    void resetCamera();
    void rotate(float x, float y);
    void translate(float x, float y);
    void setCameraPerspective(btVector3 *pos, btVector3 *angle, float *fovy, float *distance);
    void zoom(bool up, const Qt::KeyboardModifiers &modifiers);
    void zoomIn() { zoom(true, Qt::NoModifier); }
    void zoomOut() { zoom(false, Qt::NoModifier); }
    void rotateUp() { rotate(10.0f, 0.0f); }
    void rotateDown() { rotate(-10.0f, 0.0f); }
    void rotateLeft() { rotate(0.0f, 10.0f); }
    void rotateRight() { rotate(0.0f, -10.0f); }
    void translateUp() { translate(0.0f, 1.0f); }
    void translateDown() { translate(0.0f, -1.0f); }
    void translateLeft() { translate(-1.0f, 0.0f); }
    void translateRight() { translate(1.0f, 0.0f); }

signals:
    void modelDidAdd(vpvl::PMDModel *model);
    void modelDidDelete(vpvl::PMDModel *model);
    void modelDidSelect(vpvl::PMDModel *model);
    void motionDidAdd(vpvl::VMDMotion *motion);
    void assetDidAdd(vpvl::XModel *model);
    void cameraMotionDidSet(vpvl::VMDMotion *motion);
    void cameraPerspectiveDidSet(const btVector3 &pos, const btVector3 &angle, float fovy, float distance);
    void surfaceDidUpdate();
    void fpsDidUpdate(int fps);

protected:
    void closeEvent(QCloseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
    void initializeGL();
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void paintGL();
    void resizeGL(int w, int h);
    void timerEvent(QTimerEvent *event);
    void wheelEvent(QWheelEvent *event);

private:
    void initializeSurface();
    vpvl::XModel *addAssetInternal(const QString &baseName, const QDir &dir);
    vpvl::PMDModel *addModelInternal(const QString &baseName, const QDir &dir);
    vpvl::VMDMotion *addMotionInternal(vpvl::PMDModel *model, const QString &path);
    vpvl::VMDMotion *setCameraInternal(const QString &path);
    void pickBones(const QPoint &point, float approx, vpvl::BoneList &pickBones);
    void getObjectCoordinate(const QPoint &point, btVector3 &coordinate);
    void setLighting();
    bool loadTexture(const QString &path, GLuint &textureID);
    bool loadToonTexture(const QString &name, const QDir &model, GLuint &textureID);
    void loadModel(vpvl::PMDModel *model, const QDir &dir);
    void unloadModel(const vpvl::PMDModel *model);
    void drawModel(const vpvl::PMDModel *model);
    void drawModelEdge(const vpvl::PMDModel *model);
    void drawModelShadow(const vpvl::PMDModel *model);
    void drawModelBones(const vpvl::PMDModel *model);
    void loadAsset(vpvl::XModel *model, const QString &name, const QDir &dir);
    void unloadAsset(const vpvl::XModel *model, const QString &name);
    void drawAsset(const vpvl::XModel *model);
    void drawSurface();
    void drawGrid();
    void updateFPS();
    void startSceneUpdateTimer();
    void stopSceneUpdateTimer();
    QProgressDialog *getProgressDialog(const QString &label, int max);
    const QString openFileDialog(const QString &name, const QString &desc, const QString &exts);

    vpvl::Scene *m_scene;
    vpvl::VMDMotion *m_camera;
    vpvl::PMDModel *m_selected;
    Grid *m_grid;
    World *m_world;
    QSettings *m_settings;
    QHash<QString, vpvl::PMDModel *> m_models;
    QHash<QString, vpvl::XModel *> m_assets;
    QList<vpvl::VMDMotion *> m_motions;
    QTime m_timer;
    QPoint m_prevPos;
    GLuint m_gridListID;
    int m_frameCount;
    int m_currentFPS;
    int m_defaultFPS;
    int m_interval;
    int m_internalTimerID;
};

#endif // SCENEWIDGET_H
