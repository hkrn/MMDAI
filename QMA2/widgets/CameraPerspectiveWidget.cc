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

using namespace vpvl2;

CameraPerspectiveWidget::CameraPerspectiveWidget(QWidget *parent) :
    QWidget(parent),
    m_currentPosition(0.0f, 0.0f, 0.0f),
    m_currentAngle(0.0f, 0.0f, 0.0f),
    m_currentFovy(0.0f),
    m_currentDistance(0.0f)
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    QGridLayout *gridLayout = new QGridLayout();
    /* 前 */
    m_frontLabel = new QPushButton();
    connect(m_frontLabel, SIGNAL(clicked()), this, SLOT(setCameraPerspectiveFront()));
    gridLayout->addWidget(m_frontLabel, 0, 0);
    /* 後ろ */
    m_backLabel = new QPushButton();
    connect(m_backLabel, SIGNAL(clicked()), this, SLOT(setCameraPerspectiveBack()));
    gridLayout->addWidget(m_backLabel, 0, 1);
    /* トップ */
    m_topLabel = new QPushButton();
    connect(m_topLabel, SIGNAL(clicked()), this, SLOT(setCameraPerspectiveTop()));
    gridLayout->addWidget(m_topLabel, 0, 2);
    /* 左 */
    m_leftLabel = new QPushButton();
    connect(m_leftLabel, SIGNAL(clicked()), this, SLOT(setCameraPerspectiveLeft()));
    gridLayout->addWidget(m_leftLabel, 1, 0);
    /* 右 */
    m_rightLabel = new QPushButton();
    connect(m_rightLabel, SIGNAL(clicked()), this, SLOT(setCameraPerspectiveRight()));
    gridLayout->addWidget(m_rightLabel, 1, 1);
    /* カメラ視点 */
    m_cameraLabel = new QPushButton();
    connect(m_cameraLabel, SIGNAL(clicked()), this, SIGNAL(cameraPerspectiveDidReset()));
    gridLayout->addWidget(m_cameraLabel, 1, 2);
    m_presetGroup = new QGroupBox();
    m_presetGroup->setLayout(gridLayout);
    mainLayout->addWidget(m_presetGroup);
    /* 位置(X,Y,Z) */
    QFormLayout *formLayout = new QFormLayout();
    const Scalar &zfar = 10000;
    m_px = new QDoubleSpinBox();
    m_px->setRange(-zfar, zfar);
    connect(m_px, SIGNAL(valueChanged(double)), this, SLOT(updatePositionX(double)));
    formLayout->addRow("X", m_px);
    m_py = new QDoubleSpinBox();
    m_py->setRange(-zfar, zfar);
    connect(m_py, SIGNAL(valueChanged(double)), this, SLOT(updatePositionY(double)));
    formLayout->addRow("Y", m_py);
    m_pz = new QDoubleSpinBox();
    m_pz->setRange(-zfar, zfar);
    connect(m_pz, SIGNAL(valueChanged(double)), this, SLOT(updatePositionZ(double)));
    formLayout->addRow("Z", m_pz);
    m_positionGroup = new QGroupBox();
    m_positionGroup->setLayout(formLayout);
    /* 回転(X,Y,Z) */
    formLayout = new QFormLayout();
    m_rx = new QDoubleSpinBox();
    m_rx->setRange(-180.0, 180.0);
    m_rx->setSingleStep(0.1);
    connect(m_rx, SIGNAL(valueChanged(double)), this, SLOT(updateRotationX(double)));
    formLayout->addRow("X", m_rx);
    m_ry = new QDoubleSpinBox();
    m_ry->setSingleStep(0.1);
    m_ry->setRange(-180.0, 180.0);
    connect(m_ry, SIGNAL(valueChanged(double)), this, SLOT(updateRotationY(double)));
    formLayout->addRow("Y", m_ry);
    m_rz = new QDoubleSpinBox();
    m_rz->setSingleStep(0.1);
    m_rz->setRange(-180.0, 180.0);
    connect(m_rz, SIGNAL(valueChanged(double)), this, SLOT(updateRotationZ(double)));
    formLayout->addRow("Z", m_rz);
    m_rotationGroup = new QGroupBox();
    m_rotationGroup->setLayout(formLayout);
    QLayout *subLayout = new QHBoxLayout();
    subLayout->addWidget(m_positionGroup);
    subLayout->addWidget(m_rotationGroup);
    mainLayout->addLayout(subLayout);
    subLayout = new QHBoxLayout();
    /* 視野角 */
    m_fovyLabel = new QLabel();
    subLayout->addWidget(m_fovyLabel);
    m_fovy = new QDoubleSpinBox();
    m_fovy->setSingleStep(0.01);
    m_fovy->setRange(0.1, 90.0);
    connect(m_fovy, SIGNAL(valueChanged(double)), this, SLOT(updateFovy(double)));
    subLayout->addWidget(m_fovy);
    /* 距離 */
    m_distanceLabel = new QLabel();
    subLayout->addWidget(m_distanceLabel);
    m_distance = new QDoubleSpinBox();
    m_distance->setSingleStep(1.0);
    m_distance->setRange(0.01, zfar);
    connect(m_distance, SIGNAL(valueChanged(double)), this, SLOT(updateDistance(double)));
    subLayout->addWidget(m_distance);
    mainLayout->addLayout(subLayout);
    mainLayout->addStretch();
    retranslate();
    setLayout(mainLayout);
}

void CameraPerspectiveWidget::setCameraPerspective(const Scene::ICamera *camera)
{
    disconnect(m_px, SIGNAL(valueChanged(double)), this, SLOT(updatePositionX(double)));
    disconnect(m_py, SIGNAL(valueChanged(double)), this, SLOT(updatePositionY(double)));
    disconnect(m_pz, SIGNAL(valueChanged(double)), this, SLOT(updatePositionZ(double)));
    disconnect(m_rx, SIGNAL(valueChanged(double)), this, SLOT(updateRotationX(double)));
    disconnect(m_ry, SIGNAL(valueChanged(double)), this, SLOT(updateRotationY(double)));
    disconnect(m_rz, SIGNAL(valueChanged(double)), this, SLOT(updateRotationZ(double)));
    disconnect(m_fovy, SIGNAL(valueChanged(double)), this, SLOT(updateFovy(double)));
    disconnect(m_distance, SIGNAL(valueChanged(double)), this, SLOT(updateDistance(double)));
    m_currentPosition = camera->position();
    m_currentAngle = camera->angle();
    m_currentFovy = camera->fovy();
    m_currentDistance = camera->distance();
    m_px->setValue(m_currentPosition.x());
    m_py->setValue(m_currentPosition.y());
    m_pz->setValue(m_currentPosition.z());
    m_rx->setValue(m_currentAngle.x());
    m_ry->setValue(m_currentAngle.y());
    m_rz->setValue(m_currentAngle.z());
    m_fovy->setValue(m_currentFovy);
    m_distance->setValue(m_currentDistance);
    connect(m_px, SIGNAL(valueChanged(double)), this, SLOT(updatePositionX(double)));
    connect(m_py, SIGNAL(valueChanged(double)), this, SLOT(updatePositionY(double)));
    connect(m_pz, SIGNAL(valueChanged(double)), this, SLOT(updatePositionZ(double)));
    connect(m_rx, SIGNAL(valueChanged(double)), this, SLOT(updateRotationX(double)));
    connect(m_ry, SIGNAL(valueChanged(double)), this, SLOT(updateRotationY(double)));
    connect(m_rz, SIGNAL(valueChanged(double)), this, SLOT(updateRotationZ(double)));
    connect(m_fovy, SIGNAL(valueChanged(double)), this, SLOT(updateFovy(double)));
    connect(m_distance, SIGNAL(valueChanged(double)), this, SLOT(updateDistance(double)));
}

void CameraPerspectiveWidget::retranslate()
{
    m_presetGroup->setTitle(tr("Preset"));
    m_positionGroup->setTitle(tr("Position"));
    m_rotationGroup->setTitle(tr("Rotation"));
    m_frontLabel->setText(tr("Front"));
    m_backLabel->setText(tr("Back"));
    m_topLabel->setText(tr("Top"));
    m_leftLabel->setText(tr("Left"));
    m_rightLabel->setText(tr("Right"));
    m_cameraLabel->setText(tr("Camera"));
    m_fovyLabel->setText(tr("Fovy"));
    m_distanceLabel->setText(tr("Distance"));
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
    float fovy = static_cast<float>(value);
    m_currentFovy = fovy;
    emit cameraPerspectiveDidChange(createCamera());
}

void CameraPerspectiveWidget::updateDistance(double value)
{
    float distance = static_cast<float>(value);
    m_currentDistance = distance;
    emit cameraPerspectiveDidChange(createCamera());
}

QSharedPointer<Scene::ICamera> CameraPerspectiveWidget::createCamera() const
{
    QSharedPointer<Scene::ICamera> camera(Scene::createCamera());
    camera->setAngle(m_currentAngle);
    camera->setPosition(m_currentPosition);
    camera->setFovy(m_currentFovy);
    camera->setDistance(m_currentDistance);
    return camera;
}
