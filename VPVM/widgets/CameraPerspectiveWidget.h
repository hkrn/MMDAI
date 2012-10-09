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

#ifndef VPVM_PERSPECTIONWIDGET_H
#define VPVM_PERSPECTIONWIDGET_H

#include <QtGui/QWidget>
#include <vpvl2/Common.h>
#include <vpvl2/Scene.h>

namespace vpvl2 {
class IBone;
}

class QDoubleSpinBox;
class QGroupBox;
class QLabel;
class QPushButton;
class QRadioButton;

namespace vpvm
{

using namespace vpvl2;

class CameraPerspectiveWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CameraPerspectiveWidget(QWidget *parent = 0);

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
    QSharedPointer<ICamera> createCamera() const;

    Vector3 m_currentPosition;
    Vector3 m_currentAngle;
    QGroupBox *m_presetGroup;
    QGroupBox *m_positionGroup;
    QGroupBox *m_rotationGroup;
    QPushButton *m_frontButton;
    QPushButton *m_backButton;
    QPushButton *m_topButton;
    QPushButton *m_leftButton;
    QPushButton *m_rightButton;
    QPushButton *m_cameraButton;
    QLabel *m_fovyLabel;
    QLabel *m_distanceLabel;
    QDoubleSpinBox *m_px;
    QDoubleSpinBox *m_py;
    QDoubleSpinBox *m_pz;
    QDoubleSpinBox *m_rx;
    QDoubleSpinBox *m_ry;
    QDoubleSpinBox *m_rz;
    QGroupBox *m_followGroup;
    QRadioButton *m_followNone;
    QRadioButton *m_followModel;
    QRadioButton *m_followBone;
    QDoubleSpinBox *m_fovy;
    QDoubleSpinBox *m_distance;
    QPushButton *m_initializeButton;
    Scalar m_currentFovy;
    Scalar m_currentDistance;
    bool m_enableFollowingModel;
    bool m_enableFollowingBone;

    Q_DISABLE_COPY(CameraPerspectiveWidget)
};

} /* namespace vpvl2 */

#endif // PERSPECTIONWIDGET_H
