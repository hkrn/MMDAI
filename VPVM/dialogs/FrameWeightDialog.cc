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

#include "FrameWeightDialog.h"
#include "common/SceneWidget.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

FrameWeightDialog::FrameWeightDialog(TimelineTabWidget::Type type, QWidget *parent) :
    QDialog(parent),
    m_position(1.0, 1.0, 1.0),
    m_rotation(1.0, 1.0, 1.0),
    m_morphWeight(1.0)
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    if (type == TimelineTabWidget::kBone) {
        QHBoxLayout *subLayout = new QHBoxLayout();
        QFormLayout *formLayout = new QFormLayout();
        formLayout->addRow(tr("X"), createSpinBox(SLOT(setPositionXWeight(double))));
        formLayout->addRow(tr("Y"), createSpinBox(SLOT(setPositionYWeight(double))));
        formLayout->addRow(tr("Z"), createSpinBox(SLOT(setPositionZWeight(double))));
        QGroupBox *groupBox = new QGroupBox(tr("Position"));
        groupBox->setLayout(formLayout);
        subLayout->addWidget(groupBox);
        formLayout = new QFormLayout();
        formLayout->addRow(tr("X"), createSpinBox(SLOT(setRotationXWeight(double))));
        formLayout->addRow(tr("Y"), createSpinBox(SLOT(setRotationYWeight(double))));
        formLayout->addRow(tr("Z"), createSpinBox(SLOT(setRotationZWeight(double))));
        groupBox = new QGroupBox(tr("Rotation"));
        groupBox->setLayout(formLayout);
        subLayout->addWidget(groupBox);
        mainLayout->addLayout(subLayout);
        connect(this, SIGNAL(accepted()), SLOT(emitBoneWeightSignal()));
    }
    else if (type == TimelineTabWidget::kMorph) {
        QFormLayout *subLayout = new QFormLayout();
        subLayout->addRow(tr("Keyframe weight"), createSpinBox(SLOT(setMorphWeight(double))));
        mainLayout->addLayout(subLayout);
        connect(this, SIGNAL(accepted()), SLOT(emitMorphWeightSignal()));
    }
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mainLayout->addWidget(buttons);
    connect(buttons, SIGNAL(accepted()), SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), SLOT(reject()));
    setLayout(mainLayout);
    setWindowTitle(tr("Keyframe weight dialog"));
}

FrameWeightDialog::~FrameWeightDialog()
{
}

void FrameWeightDialog::setPositionXWeight(double value)
{
    m_position.setX(value);
}

void FrameWeightDialog::setPositionYWeight(double value)
{
    m_position.setY(value);
}

void FrameWeightDialog::setPositionZWeight(double value)
{
    m_position.setZ(value);
}

void FrameWeightDialog::setRotationXWeight(double value)
{
    m_rotation.setX(value);
}

void FrameWeightDialog::setRotationYWeight(double value)
{
    m_rotation.setY(value);
}

void FrameWeightDialog::setRotationZWeight(double value)
{
    m_rotation.setZ(value);
}

void FrameWeightDialog::setMorphWeight(double value)
{
    m_morphWeight = value;
}

void FrameWeightDialog::emitBoneWeightSignal()
{
    emit boneWeightDidSet(m_position, m_rotation);
}

void FrameWeightDialog::emitMorphWeightSignal()
{
    emit morphKeyframeWeightDidSet(m_morphWeight);
}

QDoubleSpinBox *FrameWeightDialog::createSpinBox(const char *slot)
{
    QDoubleSpinBox *weightBox = new QDoubleSpinBox();
    connect(weightBox, SIGNAL(valueChanged(double)), slot);
    weightBox->setMinimum(0.01);
    weightBox->setSingleStep(0.01);
    weightBox->setValue(1.0);
    return weightBox;
}
