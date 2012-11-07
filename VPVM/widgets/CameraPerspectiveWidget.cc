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

#include "widgets/CameraPerspectiveWidget.h"

#include <QtGui/QtGui>
#include <vpvl2/vpvl2.h>

/* lupdate cannot parse tr() syntax correctly */

namespace vpvm
{

using namespace vpvl2;

static const Scalar kMaxFar = 10000;
static const Scalar kMaxAngle = 360;

CameraPerspectiveWidget::CameraPerspectiveWidget(QWidget *parent)
    : QWidget(parent),
      m_currentPosition(0.0f, 0.0f, 0.0f),
      m_currentAngle(0.0f, 0.0f, 0.0f),
      m_presetGroup(new QGroupBox()),
      m_positionGroup(new QGroupBox()),
      m_rotationGroup(new QGroupBox()),
      m_frontButton(new QPushButton()),
      m_backButton(new QPushButton()),
      m_topButton(new QPushButton()),
      m_leftButton(new QPushButton()),
      m_rightButton(new QPushButton()),
      m_cameraButton(new QPushButton()),
      m_fovyLabel(new QLabel()),
      m_distanceLabel(new QLabel()),
      m_px(createSpinBox(1, -kMaxFar, kMaxFar)),
      m_py(createSpinBox(1, -kMaxFar, kMaxFar)),
      m_pz(createSpinBox(1, -kMaxFar, kMaxFar)),
      m_rx(createSpinBox(0.1, -kMaxAngle, kMaxAngle)),
      m_ry(createSpinBox(0.1, -kMaxAngle, kMaxAngle)),
      m_rz(createSpinBox(0.1, -kMaxAngle, kMaxAngle)),
      m_followGroup(new QGroupBox()),
      m_followNone(new QRadioButton()),
      m_followModel(new QRadioButton()),
      m_followBone(new QRadioButton()),
      m_fovy(createSpinBox(0.1, 0.1, 125)),
      m_distance(createSpinBox(1, -kMaxFar, kMaxFar)),
      m_initializeButton(new QPushButton()),
      m_currentFovy(0.0f),
      m_currentDistance(0.0f),
      m_enableFollowingModel(false),
      m_enableFollowingBone(false)
{
    QScopedPointer<QVBoxLayout> mainLayout(new QVBoxLayout());
    QScopedPointer<QGridLayout> gridLayout(new QGridLayout());
    /* 前 */
    connect(m_frontButton.data(), SIGNAL(clicked()), this, SLOT(setCameraPerspectiveFront()));
    gridLayout->addWidget(m_frontButton.data(), 0, 0);
    /* 後ろ */
    connect(m_backButton.data(), SIGNAL(clicked()), this, SLOT(setCameraPerspectiveBack()));
    gridLayout->addWidget(m_backButton.data(), 0, 1);
    /* トップ */
    connect(m_topButton.data(), SIGNAL(clicked()), this, SLOT(setCameraPerspectiveTop()));
    gridLayout->addWidget(m_topButton.data(), 0, 2);
    /* 左 */
    connect(m_leftButton.data(), SIGNAL(clicked()), this, SLOT(setCameraPerspectiveLeft()));
    gridLayout->addWidget(m_leftButton.data(), 1, 0);
    /* 右 */
    connect(m_rightButton.data(), SIGNAL(clicked()), this, SLOT(setCameraPerspectiveRight()));
    gridLayout->addWidget(m_rightButton.data(), 1, 1);
    /* カメラ視点 */
    connect(m_cameraButton.data(), SIGNAL(clicked()), this, SIGNAL(cameraPerspectiveDidReset()));
    gridLayout->addWidget(m_cameraButton.data(), 1, 2);
    m_presetGroup->setLayout(gridLayout.take());
    mainLayout->addWidget(m_presetGroup.data());
    /* 位置(X,Y,Z) */
    QScopedPointer<QFormLayout> formLayout(new QFormLayout());
    connect(m_px.data(), SIGNAL(valueChanged(double)), this, SLOT(updatePositionX(double)));
    formLayout->addRow("X", m_px.data());
    connect(m_py.data(), SIGNAL(valueChanged(double)), this, SLOT(updatePositionY(double)));
    formLayout->addRow("Y", m_py.data());
    connect(m_pz.data(), SIGNAL(valueChanged(double)), this, SLOT(updatePositionZ(double)));
    formLayout->addRow("Z", m_pz.data());
    m_positionGroup->setLayout(formLayout.take());
    /* 回転(X,Y,Z) */
    formLayout.reset(new QFormLayout());
    connect(m_rx.data(), SIGNAL(valueChanged(double)), this, SLOT(updateRotationX(double)));
    formLayout->addRow("X", m_rx.data());
    connect(m_ry.data(), SIGNAL(valueChanged(double)), this, SLOT(updateRotationY(double)));
    formLayout->addRow("Y", m_ry.data());
    connect(m_rz.data(), SIGNAL(valueChanged(double)), this, SLOT(updateRotationZ(double)));
    formLayout->addRow("Z", m_rz.data());
    m_rotationGroup->setLayout(formLayout.take());
    QScopedPointer<QLayout> subLayout(new QHBoxLayout());
    subLayout->addWidget(m_positionGroup.data());
    subLayout->addWidget(m_rotationGroup.data());
    mainLayout->addLayout(subLayout.take());
    /* 追従 */
    m_followNone->setChecked(true);
    subLayout.reset(new QHBoxLayout());
    subLayout->addWidget(m_followNone.data());
    subLayout->addWidget(m_followModel.data());
    subLayout->addWidget(m_followBone.data());
    m_followGroup->setLayout(subLayout.take());
    mainLayout->addWidget(m_followGroup.data());
    subLayout.reset(new QHBoxLayout());
    /* 視野角 */
    subLayout->addWidget(m_fovyLabel.data());
    connect(m_fovy.data(), SIGNAL(valueChanged(double)), this, SLOT(updateFovy(double)));
    subLayout->addWidget(m_fovy.data());
    /* 距離 */
    subLayout->addWidget(m_distanceLabel.data());
    connect(m_distance.data(), SIGNAL(valueChanged(double)), this, SLOT(updateDistance(double)));
    subLayout->addWidget(m_distance.data());
    mainLayout->addLayout(subLayout.take());
    /* 初期化 */
    connect(m_initializeButton.data(), SIGNAL(clicked()), this, SLOT(initializeCamera()));
    mainLayout->addWidget(m_initializeButton.data(), 0, Qt::AlignCenter);
    mainLayout->addStretch();
    retranslate();
    setLayout(mainLayout.take());
}

void CameraPerspectiveWidget::setCameraPerspective(const ICamera *camera)
{
    disconnect(m_px.data(), SIGNAL(valueChanged(double)), this, SLOT(updatePositionX(double)));
    disconnect(m_py.data(), SIGNAL(valueChanged(double)), this, SLOT(updatePositionY(double)));
    disconnect(m_pz.data(), SIGNAL(valueChanged(double)), this, SLOT(updatePositionZ(double)));
    disconnect(m_rx.data(), SIGNAL(valueChanged(double)), this, SLOT(updateRotationX(double)));
    disconnect(m_ry.data(), SIGNAL(valueChanged(double)), this, SLOT(updateRotationY(double)));
    disconnect(m_rz.data(), SIGNAL(valueChanged(double)), this, SLOT(updateRotationZ(double)));
    disconnect(m_fovy.data(), SIGNAL(valueChanged(double)), this, SLOT(updateFovy(double)));
    disconnect(m_distance.data(), SIGNAL(valueChanged(double)), this, SLOT(updateDistance(double)));
    m_currentPosition = camera->lookAt();
    m_currentAngle = camera->angle();
    m_currentFovy = camera->fov();
    m_currentDistance = camera->distance();
    m_px->setValue(m_currentPosition.x());
    m_py->setValue(m_currentPosition.y());
    m_pz->setValue(m_currentPosition.z());
    m_rx->setValue(m_currentAngle.x());
    m_ry->setValue(m_currentAngle.y());
    m_rz->setValue(m_currentAngle.z());
    m_fovy->setValue(m_currentFovy);
    m_distance->setValue(m_currentDistance);
    connect(m_px.data(), SIGNAL(valueChanged(double)), this, SLOT(updatePositionX(double)));
    connect(m_py.data(), SIGNAL(valueChanged(double)), this, SLOT(updatePositionY(double)));
    connect(m_pz.data(), SIGNAL(valueChanged(double)), this, SLOT(updatePositionZ(double)));
    connect(m_rx.data(), SIGNAL(valueChanged(double)), this, SLOT(updateRotationX(double)));
    connect(m_ry.data(), SIGNAL(valueChanged(double)), this, SLOT(updateRotationY(double)));
    connect(m_rz.data(), SIGNAL(valueChanged(double)), this, SLOT(updateRotationZ(double)));
    connect(m_fovy.data(), SIGNAL(valueChanged(double)), this, SLOT(updateFovy(double)));
    connect(m_distance.data(), SIGNAL(valueChanged(double)), this, SLOT(updateDistance(double)));
}

void CameraPerspectiveWidget::retranslate()
{
    m_presetGroup->setTitle(vpvm::CameraPerspectiveWidget::tr("Preset"));
    m_positionGroup->setTitle(vpvm::CameraPerspectiveWidget::tr("Position"));
    m_rotationGroup->setTitle(vpvm::CameraPerspectiveWidget::tr("Rotation"));
    m_frontButton->setText(vpvm::CameraPerspectiveWidget::tr("Front"));
    m_backButton->setText(vpvm::CameraPerspectiveWidget::tr("Back"));
    m_topButton->setText(vpvm::CameraPerspectiveWidget::tr("Top"));
    m_leftButton->setText(vpvm::CameraPerspectiveWidget::tr("Left"));
    m_rightButton->setText(vpvm::CameraPerspectiveWidget::tr("Right"));
    m_cameraButton->setText(vpvm::CameraPerspectiveWidget::tr("Camera"));
    m_fovyLabel->setText(vpvm::CameraPerspectiveWidget::tr("Fovy"));
    m_distanceLabel->setText(vpvm::CameraPerspectiveWidget::tr("Distance"));
    m_followGroup->setTitle(vpvm::CameraPerspectiveWidget::tr("Follow"));
    m_followNone->setText(vpvm::CameraPerspectiveWidget::tr("None"));
    m_followModel->setText(vpvm::CameraPerspectiveWidget::tr("Model"));
    m_followBone->setText(vpvm::CameraPerspectiveWidget::tr("Bone"));
    m_initializeButton->setText(vpvm::CameraPerspectiveWidget::tr("Initialize"));
}

void CameraPerspectiveWidget::setCameraPerspectiveFront()
{
    m_currentAngle = kZeroV3;
    emit cameraPerspectiveDidChange(createCamera());
}

void CameraPerspectiveWidget::setCameraPerspectiveBack()
{
    m_currentAngle = Vector3(0, 180, 0);
    emit cameraPerspectiveDidChange(createCamera());
}

void CameraPerspectiveWidget::setCameraPerspectiveTop()
{
    m_currentAngle = Vector3(90, 0, 0);
    emit cameraPerspectiveDidChange(createCamera());
}

void CameraPerspectiveWidget::setCameraPerspectiveBottom()
{
    m_currentAngle = Vector3(-90, 0, 0);
    emit cameraPerspectiveDidChange(createCamera());
}

void CameraPerspectiveWidget::setCameraPerspectiveLeft()
{
    m_currentAngle = Vector3(0, -90, 0);
    emit cameraPerspectiveDidChange(createCamera());
}

void CameraPerspectiveWidget::setCameraPerspectiveRight()
{
    m_currentAngle = Vector3(0, 90, 0);
    emit cameraPerspectiveDidChange(createCamera());
}

void CameraPerspectiveWidget::updatePositionX(double value)
{
    m_currentPosition.setX(value);
    emit cameraPerspectiveDidChange(createCamera());
}

void CameraPerspectiveWidget::updatePositionY(double value)
{
    m_currentPosition.setY(value);
    emit cameraPerspectiveDidChange(createCamera());
}

void CameraPerspectiveWidget::updatePositionZ(double value)
{
    m_currentPosition.setZ(value);
    emit cameraPerspectiveDidChange(createCamera());
}

void CameraPerspectiveWidget::updateRotationX(double value)
{
    m_currentAngle.setX(value);
    emit cameraPerspectiveDidChange(createCamera());
}

void CameraPerspectiveWidget::updateRotationY(double value)
{
    m_currentAngle.setY(value);
    emit cameraPerspectiveDidChange(createCamera());
}

void CameraPerspectiveWidget::updateRotationZ(double value)
{
    m_currentAngle.setZ(value);
    emit cameraPerspectiveDidChange(createCamera());
}

void CameraPerspectiveWidget::updateFovy(double value)
{
    m_currentFovy = Scalar(value);
    emit cameraPerspectiveDidChange(createCamera());
}

void CameraPerspectiveWidget::updateDistance(double value)
{
    m_currentDistance = Scalar(value);
    emit cameraPerspectiveDidChange(createCamera());
}

void CameraPerspectiveWidget::initializeCamera()
{
    QScopedPointer<ICamera> camera(Scene::createCamera());
    m_currentAngle = camera->angle();
    m_currentPosition = camera->lookAt();
    m_currentFovy = camera->fov();
    m_currentDistance = camera->distance();
    emit cameraPerspectiveDidChange(createCamera());
}

void CameraPerspectiveWidget::setPositionFromModel(const Vector3 &value)
{
    if (m_followModel->isChecked()) {
        m_currentPosition = value;
        emit cameraPerspectiveDidChange(createCamera());
    }
}

void CameraPerspectiveWidget::setPositionFromBone(const Vector3 &value)
{
    if (m_followBone->isChecked()) {
        m_currentPosition = value;
        emit cameraPerspectiveDidChange(createCamera());
    }
}

void CameraPerspectiveWidget::setPositionFromBone(const QList<IBone *> &bones)
{
    if (!bones.isEmpty() && m_followBone->isChecked()) {
        m_currentPosition = bones.first()->worldTransform().getOrigin();
        emit cameraPerspectiveDidChange(createCamera());
    }
}

QDoubleSpinBox *CameraPerspectiveWidget::createSpinBox(double step, double min, double max)
{
    QScopedPointer<QDoubleSpinBox> spinBox(new QDoubleSpinBox());
    spinBox->setAlignment(Qt::AlignRight);
    spinBox->setRange(min, max);
    spinBox->setSingleStep(step);
    return spinBox.take();
}

QSharedPointer<ICamera> CameraPerspectiveWidget::createCamera() const
{
    QSharedPointer<ICamera> camera(Scene::createCamera());
    camera->setAngle(m_currentAngle);
    camera->setLookAt(m_currentPosition);
    camera->setFov(m_currentFovy);
    camera->setDistance(m_currentDistance);
    return camera;
}

} /* namespace vpvm */
