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

#include "models/BoneMotionModel.h"
#include "common/util.h"
#include "dialogs/BoneDialog.h"

#include <QtGui/QtGui>
#include <vpvl2/vpvl2.h>

namespace {

static void UISetPositionSpinBoxRange(QScopedPointer<QDoubleSpinBox> &spinbox)
{
    spinbox->setRange(-10000, 10000);
    spinbox->setSingleStep(0.1);
    spinbox->setAlignment(Qt::AlignRight);
}

static void UISetAngleSpinBoxRange(QScopedPointer<QDoubleSpinBox> &spinbox)
{
    spinbox->setRange(-360.0, 360.0);
    spinbox->setSingleStep(0.1);
    spinbox->setAlignment(Qt::AlignRight);
}

}

namespace vpvm
{

using namespace vpvl2;

BoneDialog::BoneDialog(BoneMotionModel *bmm, QSettings *settings, QWidget *parent)
    : QDialog(parent),
      m_positionGroup(new QGroupBox()),
      m_rotationGroup(new QGroupBox()),
      m_xPositionLabel(new QLabel()),
      m_yPositionLabel(new QLabel()),
      m_zPositionLabel(new QLabel()),
      m_stepPositionLabel(new QLabel()),
      m_xAngleLabel(new QLabel()),
      m_yAngleLabel(new QLabel()),
      m_zAngleLabel(new QLabel()),
      m_stepRotationLabel(new QLabel()),
      m_xPosition(createSpinBox()),
      m_yPosition(createSpinBox()),
      m_zPosition(createSpinBox()),
      m_stepPosition(createSpinBox()),
      m_xAngle(createSpinBox()),
      m_yAngle(createSpinBox()),
      m_zAngle(createSpinBox()),
      m_stepRotation(createSpinBox()),
      m_settingsRef(settings),
      m_boneMotionModelRef(bmm)
{
    IBone *bone = m_boneMotionModelRef->selectedBone();
    connect(m_xPosition.data(), SIGNAL(valueChanged(double)), SLOT(setXPosition(double)));
    UISetPositionSpinBoxRange(m_xPosition);
    connect(m_yPosition.data(), SIGNAL(valueChanged(double)), SLOT(setYPosition(double)));
    UISetPositionSpinBoxRange(m_yPosition);
    connect(m_zPosition.data(), SIGNAL(valueChanged(double)), SLOT(setZPosition(double)));
    UISetPositionSpinBoxRange(m_zPosition);
    connect(m_xAngle.data(), SIGNAL(valueChanged(double)), SLOT(setXAngle(double)));
    UISetAngleSpinBoxRange(m_xAngle);
    connect(m_yAngle.data(), SIGNAL(valueChanged(double)), SLOT(setYAngle(double)));
    UISetAngleSpinBoxRange(m_yAngle);
    connect(m_zAngle.data(), SIGNAL(valueChanged(double)), SLOT(setZAngle(double)));
    UISetAngleSpinBoxRange(m_zAngle);
    QScopedPointer<QHBoxLayout> subLayout(new QHBoxLayout());
    QScopedPointer<QGridLayout> gridLayout(new QGridLayout());
    gridLayout->addWidget(m_xPositionLabel.data(), 0, 0);
    gridLayout->addWidget(m_yPositionLabel.data(), 1, 0);
    gridLayout->addWidget(m_zPositionLabel.data(), 2, 0);
    gridLayout->addWidget(m_stepPositionLabel.data(), 3, 0);
    gridLayout->addWidget(m_xPosition.data(), 0, 1);
    gridLayout->addWidget(m_yPosition.data(), 1, 1);
    gridLayout->addWidget(m_zPosition.data(), 2, 1);
    gridLayout->addWidget(m_stepPosition.data(), 3, 1);
    m_positionGroup->setLayout(gridLayout.take());
    m_positionGroup->setEnabled(bone->isMovable());
    subLayout->addWidget(m_positionGroup.data());
    gridLayout.reset(new QGridLayout());
    gridLayout->addWidget(m_xAngleLabel.data(), 0, 0);
    gridLayout->addWidget(m_yAngleLabel.data(), 1, 0);
    gridLayout->addWidget(m_zAngleLabel.data(), 2, 0);
    gridLayout->addWidget(m_stepRotationLabel.data(), 3, 0);
    gridLayout->addWidget(m_xAngle.data(), 0, 1);
    gridLayout->addWidget(m_yAngle.data(), 1, 1);
    gridLayout->addWidget(m_zAngle.data(), 2, 1);
    gridLayout->addWidget(m_stepRotation.data(), 3, 1);
    m_rotationGroup->setLayout(gridLayout.take());
    m_rotationGroup->setEnabled(bone->isRotateable());
    subLayout->addWidget(m_rotationGroup.data());
    QScopedPointer<QVBoxLayout> mainLayout(new QVBoxLayout());
    mainLayout->addLayout(subLayout.take());
    QScopedPointer<QDialogButtonBox> buttons(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel));
    connect(m_stepPosition.data(), SIGNAL(valueChanged(double)), SLOT(setPositionStep(double)));
    connect(m_stepRotation.data(), SIGNAL(valueChanged(double)), SLOT(setRotationStep(double)));
    connect(buttons.data(), SIGNAL(accepted()), SLOT(dialogAccepted()));
    connect(buttons.data(), SIGNAL(rejected()), SLOT(dialogRejected()));
    mainLayout->addWidget(buttons.take());
    setLayout(mainLayout.take());
    setPosition(bone->localPosition());
    setRotation(bone->localRotation());
    retranslate();
    m_stepPosition->setSingleStep(0.001);
    m_stepRotation->setSingleStep(0.001);
    m_stepPosition->setValue(m_settingsRef->value("boneDialog/positionStep", 0.05).toDouble());
    m_stepRotation->setValue(m_settingsRef->value("boneDialog/rotationStep", 0.05).toDouble());
    m_boneMotionModelRef->mutableState()->save();
}

BoneDialog::~BoneDialog()
{
}

void BoneDialog::retranslate()
{
    /* lupdate cannot parse tr() syntax correctly */
    m_positionGroup->setTitle(vpvm::BoneDialog::tr("Position"));
    m_xPositionLabel->setText(vpvm::BoneDialog::tr("X"));
    m_yPositionLabel->setText(vpvm::BoneDialog::tr("Y"));
    m_zPositionLabel->setText(vpvm::BoneDialog::tr("Z"));
    m_stepPositionLabel->setText(vpvm::BoneDialog::tr("Step Degree"));
    m_rotationGroup->setTitle(vpvm::BoneDialog::tr("Rotation Axis"));
    m_xAngleLabel->setText(vpvm::BoneDialog::tr("X"));
    m_yAngleLabel->setText(vpvm::BoneDialog::tr("Y"));
    m_zAngleLabel->setText(vpvm::BoneDialog::tr("Z"));
    m_stepRotationLabel->setText(vpvm::BoneDialog::tr("Step Degree"));
    setWindowTitle(vpvm::BoneDialog::tr("Bone Dialog of %1")
                   .arg(toQStringFromBone(m_boneMotionModelRef->selectedBone())));
}

void BoneDialog::setPosition(const Vector3 &position)
{
    m_xPosition->setValue(position.x());
    m_yPosition->setValue(position.y());
    m_zPosition->setValue(position.z());
}

void BoneDialog::setRotation(const Quaternion &rotation)
{
    m_xAngle->setValue(degree(rotation.x()));
    m_yAngle->setValue(degree(rotation.y()));
    m_zAngle->setValue(degree(rotation.z()));
}

void BoneDialog::setXPosition(double value)
{
    m_boneMotionModelRef->setPosition('X', value);
}

void BoneDialog::setYPosition(double value)
{
    m_boneMotionModelRef->setPosition('Y', value);
}

void BoneDialog::setZPosition(double value)
{
    m_boneMotionModelRef->setPosition('Z', value);
}

void BoneDialog::setXAngle(double value)
{
    m_boneMotionModelRef->setRotation('X', radian(value));
}

void BoneDialog::setYAngle(double value)
{
    m_boneMotionModelRef->setRotation('Y', radian(value));
}

void BoneDialog::setZAngle(double value)
{
    m_boneMotionModelRef->setRotation('Z', radian(value));
}

void BoneDialog::setPositionStep(double value)
{
    m_xPosition->setSingleStep(value);
    m_yPosition->setSingleStep(value);
    m_zPosition->setSingleStep(value);
}

void BoneDialog::setRotationStep(double value)
{
    m_xAngle->setSingleStep(value);
    m_yAngle->setSingleStep(value);
    m_zAngle->setSingleStep(value);
}

void BoneDialog::dialogAccepted()
{
    m_boneMotionModelRef->mutableState()->discard();
    m_settingsRef->setValue("boneDialog/positionStep", m_stepPosition->value());
    m_settingsRef->setValue("boneDialog/rotationStep", m_stepRotation->value());
    close();
}

void BoneDialog::dialogRejected()
{
    int ret = warning(0,
                      vpvm::BoneDialog::tr("Confirm"),
                      vpvm::BoneDialog::tr("Do you want to discard your changes?"),
                      "",
                      QMessageBox::Yes | QMessageBox::No);
    switch (ret) {
    case QMessageBox::Yes:
        m_boneMotionModelRef->mutableState()->restore();
        close();
        break;
    case QMessageBox::No:
    default:
        break;
    }
}

QDoubleSpinBox *BoneDialog::createSpinBox()
{
    QScopedPointer<QDoubleSpinBox> spinBox(new QDoubleSpinBox());
    spinBox->setAlignment(Qt::AlignRight);
    spinBox->setDecimals(3);
    return spinBox.take();
}

} /* namespace vpvm */
