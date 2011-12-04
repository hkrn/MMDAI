/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn                                    */
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

#include "BoneDialog.h"
#include "BoneMotionModel.h"
#include "common/util.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

namespace {

static void UISetPositionSpinBoxRange(QDoubleSpinBox *spinbox)
{
    spinbox->setRange(-vpvl::Scene::kFrustumFar, vpvl::Scene::kFrustumFar);
    spinbox->setSingleStep(0.1);
}

static void UISetAngleSpinBoxRange(QDoubleSpinBox *spinbox)
{
    spinbox->setRange(-360.0, 360.0);
    spinbox->setSingleStep(0.1);
}

}

BoneDialog::BoneDialog(BoneMotionModel *bmm,
                       QWidget *parent) :
    QDialog(parent),
    m_boneMotionModel(bmm)
{
    vpvl::Bone *bone = m_boneMotionModel->selectedBone();
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
    setPosition(bone->position());
    setRotation(bone->rotation());
    retranslate();
    m_boneMotionModel->saveState();
}

BoneDialog::~BoneDialog()
{
}

void BoneDialog::retranslate()
{
    m_xPositionLabel->setText(tr("X Position"));
    m_yPositionLabel->setText(tr("Y Position"));
    m_zPositionLabel->setText(tr("Z Position"));
    m_xAngleLabel->setText(tr("X Axis"));
    m_yAngleLabel->setText(tr("Y Axis"));
    m_zAngleLabel->setText(tr("Z Axis"));
    setWindowTitle(tr("Bone dialog of %1").arg(internal::toQString(m_boneMotionModel->selectedBone())));
}

void BoneDialog::setPosition(const vpvl::Vector3 &position)
{
    m_xPosition->setValue(position.x());
    m_yPosition->setValue(position.y());
    m_zPosition->setValue(position.z());
}

void BoneDialog::setRotation(const vpvl::Quaternion &rotation)
{
    m_xAngle->setValue(vpvl::degree(rotation.x()));
    m_yAngle->setValue(vpvl::degree(rotation.y()));
    m_zAngle->setValue(vpvl::degree(rotation.z()));
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
    m_boneMotionModel->setRotation('X', vpvl::radian(value));
}

void BoneDialog::setYAngle(double value)
{
    m_boneMotionModel->setRotation('Y', vpvl::radian(value));
}

void BoneDialog::setZAngle(double value)
{
    m_boneMotionModel->setRotation('Z', vpvl::radian(value));
}

void BoneDialog::dialogAccepted()
{
    m_boneMotionModel->discardState();
    close();
}

void BoneDialog::dialogRejected()
{
    m_boneMotionModel->restoreState();
    close();
}
