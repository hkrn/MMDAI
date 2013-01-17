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

#ifndef VPVM_PERSPECTIONWIDGET_H
#define VPVM_PERSPECTIONWIDGET_H

#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QWidget>
#else
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QWidget>
#endif
#include <vpvl2/Common.h>
#include <vpvl2/Scene.h>
#include <vpvl2/qt/RenderContext.h> /* for using moc generate workaround */

namespace vpvl2 {
class IBone;
}

namespace vpvm
{

using namespace vpvl2;
class SceneLoader;

class CameraPerspectiveWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CameraPerspectiveWidget(SceneLoader *sceneLoaderRef, QWidget *parent = 0);

public slots:
    void setCameraPerspective(const ICamera *camera);

signals:
    void cameraPerspectiveDidChange(const QSharedPointer<ICamera> &camera);
    void cameraPerspectiveDidReset();

private slots:
    void retranslate();
    void setCameraPerspectiveFront();
    void setCameraPerspectiveBack();
    void setCameraPerspectiveTop();
    void setCameraPerspectiveBottom();
    void setCameraPerspectiveLeft();
    void setCameraPerspectiveRight();
    void updatePositionX(double value);
    void updatePositionY(double value);
    void updatePositionZ(double value);
    void updateRotationX(double value);
    void updateRotationY(double value);
    void updateRotationZ(double value);
    void updateFovy(double value);
    void updateDistance(double value);
    void initializeCamera();
    void setPositionFromModel(const Vector3 &value);
    void setPositionFromBone(const Vector3 &value);
    void setPositionFromBone(const QList<IBone *> &bones);

private:
    static QDoubleSpinBox *createSpinBox(double step, double min, double max);
    QSharedPointer<ICamera> createCamera() const;

    SceneLoader *m_sceneLoaderRef;
    Vector3 m_currentPosition;
    Vector3 m_currentAngle;
    QScopedPointer<QGroupBox> m_presetGroup;
    QScopedPointer<QGroupBox> m_positionGroup;
    QScopedPointer<QGroupBox> m_rotationGroup;
    QScopedPointer<QPushButton> m_frontButton;
    QScopedPointer<QPushButton> m_backButton;
    QScopedPointer<QPushButton> m_topButton;
    QScopedPointer<QPushButton> m_leftButton;
    QScopedPointer<QPushButton> m_rightButton;
    QScopedPointer<QPushButton> m_cameraButton;
    QScopedPointer<QLabel> m_fovyLabel;
    QScopedPointer<QLabel> m_distanceLabel;
    QScopedPointer<QDoubleSpinBox> m_px;
    QScopedPointer<QDoubleSpinBox> m_py;
    QScopedPointer<QDoubleSpinBox> m_pz;
    QScopedPointer<QDoubleSpinBox> m_rx;
    QScopedPointer<QDoubleSpinBox> m_ry;
    QScopedPointer<QDoubleSpinBox> m_rz;
    QScopedPointer<QGroupBox> m_followGroup;
    QScopedPointer<QRadioButton> m_followNone;
    QScopedPointer<QRadioButton> m_followModel;
    QScopedPointer<QRadioButton> m_followBone;
    QScopedPointer<QDoubleSpinBox> m_fovy;
    QScopedPointer<QDoubleSpinBox> m_distance;
    QScopedPointer<QPushButton> m_initializeButton;
    Scalar m_currentFovy;
    Scalar m_currentDistance;
    bool m_enableFollowingModel;
    bool m_enableFollowingBone;

    Q_DISABLE_COPY(CameraPerspectiveWidget)
};

} /* namespace vpvl2 */

#endif // PERSPECTIONWIDGET_H
