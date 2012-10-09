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

static void UISetPositionSpinBoxRange(QDoubleSpinBox *spinbox)
{
    spinbox->setRange(-10000, 10000);
    spinbox->setSingleStep(0.1);
}

static void UISetAngleSpinBoxRange(QDoubleSpinBox *spinbox)
{
    spinbox->setRange(-360.0, 360.0);
    spinbox->setSingleStep(0.1);
}

}

namespace vpvm
{

using namespace vpvl2;

BoneDialog::BoneDialog(BoneMotionModel *bmm, QWidget *parent) :
    QDialog(parent),
    m_boneMotionModel(bmm)
{
    IBone *bone = m_boneMotionModel->selectedBone();
    m_xPositionLabel = new QLabel();
    m_yPositionLabel = new QLabel();
    m_zPositionLabel = new QLabel();
    m_xAngleLabel = new QLabel();
    m_yAngleLabel = new QLabel();
    m_zAngleLabel = new QLabel();
    m_xPosition = new QDoubleSpinBox();
    connect(m_xPosition, SIGNAL(valueChanged(double)), this, SLOT(setXPosition(double)));
    UISetPositionSpinBoxRange(m_xPosition);
    m_yPosition = new QDoubleSpinBox();
    connect(m_yPosition, SIGNAL(valueChanged(double)), this, SLOT(setYPosition(double)));
    UISetPositionSpinBoxRange(m_yPosition);
    m_zPosition = new QDoubleSpinBox();
    connect(m_zPosition, SIGNAL(valueChanged(double)), this, SLOT(setZPosition(double)));
    UISetPositionSpinBoxRange(m_zPosition);
    m_xAngle = new QDoubleSpinBox();
    connect(m_xAngle, SIGNAL(valueChanged(double)), this, SLOT(setXAngle(double)));
    UISetAngleSpinBoxRange(m_xAngle);
    m_yAngle = new QDoubleSpinBox();
    connect(m_yAngle, SIGNAL(valueChanged(double)), this, SLOT(setYAngle(double)));
    UISetAngleSpinBoxRange(m_yAngle);
    m_zAngle = new QDoubleSpinBox();
    connect(m_zAngle, SIGNAL(valueChanged(double)), this, SLOT(setZAngle(double)));
    UISetAngleSpinBoxRange(m_zAngle);
    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->addWidget(m_xPositionLabel, 0, 0);
    gridLayout->addWidget(m_yPositionLabel, 1, 0);
    gridLayout->addWidget(m_zPositionLabel, 2, 0);
    gridLayout->addWidget(m_xPosition, 0, 1);
    gridLayout->addWidget(m_yPosition, 1, 1);
    gridLayout->addWidget(m_zPosition, 2, 1);
    gridLayout->addWidget(m_xAngleLabel, 0, 2);
    gridLayout->addWidget(m_yAngleLabel, 1, 2);
    gridLayout->addWidget(m_zAngleLabel, 2, 2);
    gridLayout->addWidget(m_xAngle, 0, 3);
    gridLayout->addWidget(m_yAngle, 1, 3);
    gridLayout->addWidget(m_zAngle, 2, 3);
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addLayout(gridLayout);
    QVBoxLayout *subLayout = new QVBoxLayout();
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, SIGNAL(accepted()), this, SLOT(dialogAccepted()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(dialogRejected()));
    subLayout->addWidget(buttons);
    mainLayout->addLayout(subLayout);
    setLayout(mainLayout);
    setPosition(bone->localPosition());
    setRotation(bone->rotation());
    retranslate();
    m_boneMotionModel->mutableState()->save();
}

BoneDialog::~BoneDialog()
{
}

void BoneDialog::retranslate()
{
    /* lupdate cannot parse tr() syntax correctly */
    m_xPositionLabel->setText(vpvm::BoneDialog::tr("X Position"));
    m_yPositionLabel->setText(vpvm::BoneDialog::tr("Y Position"));
    m_zPositionLabel->setText(vpvm::BoneDialog::tr("Z Position"));
    m_xAngleLabel->setText(vpvm::BoneDialog::tr("X Axis"));
    m_yAngleLabel->setText(vpvm::BoneDialog::tr("Y Axis"));
    m_zAngleLabel->setText(vpvm::BoneDialog::tr("Z Axis"));
    setWindowTitle(vpvm::BoneDialog::tr("Bone dialog of %1")
                   .arg(toQStringFromBone(m_boneMotionModel->selectedBone())));
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
    m_boneMotionModel->setPosition('X', value);
}

void BoneDialog::setYPosition(double value)
{
    m_boneMotionModel->setPosition('Y', value);
}

void BoneDialog::setZPosition(double value)
{
    m_boneMotionModel->setPosition('Z', value);
}

void BoneDialog::setXAngle(double value)
{
    m_boneMotionModel->setRotation('X', radian(value));
}

void BoneDialog::setYAngle(double value)
{
    m_boneMotionModel->setRotation('Y', radian(value));
}

void BoneDialog::setZAngle(double value)
{
    m_boneMotionModel->setRotation('Z', radian(value));
}

void BoneDialog::dialogAccepted()
{
    m_boneMotionModel->mutableState()->discard();
    close();
}

void BoneDialog::dialogRejected()
{
    m_boneMotionModel->mutableState()->restore();
    close();
}

} /* namespace vpvm */
