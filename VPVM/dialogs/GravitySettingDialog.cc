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

#include "GravitySettingDialog.h"
#include "common/SceneLoader.h"

#include <QtGui/QtGui>
#include <vpvl2/vpvl2.h>

namespace vpvm
{

using namespace vpvl2;

GravitySettingDialog::GravitySettingDialog(SceneLoader *loader, QWidget *parent)
    : QDialog(parent),
      m_axisGroup(new QGroupBox()),
      m_axisXLabel(new QLabel()),
      m_axisX(createSpinBox(loader->worldGravity().x())),
      m_axisYLabel(new QLabel()),
      m_axisY(createSpinBox(loader->worldGravity().y())),
      m_axisZLabel(new QLabel()),
      m_axisZ(createSpinBox(loader->worldGravity().z())),
      m_randSeedLabel(new QLabel()),
      m_randSeedSpinBox(new QSpinBox())
{
    QScopedPointer<QGridLayout> axisGroupLayout(new QGridLayout());
    axisGroupLayout->addWidget(m_axisXLabel.data(), 0, 0, Qt::AlignCenter);
    axisGroupLayout->addWidget(m_axisX.data(), 0, 1);
    axisGroupLayout->addWidget(m_axisYLabel.data(), 1, 0, Qt::AlignCenter);
    axisGroupLayout->addWidget(m_axisY.data(), 1, 1);
    axisGroupLayout->addWidget(m_axisZLabel.data(), 2, 0, Qt::AlignCenter);
    axisGroupLayout->addWidget(m_axisZ.data(), 2, 1);
    m_axisGroup->setLayout(axisGroupLayout.take());
    QScopedPointer<QVBoxLayout> mainLayout(new QVBoxLayout());
    mainLayout->addWidget(m_axisGroup.data());
    m_randSeedSpinBox->setValue(loader->worldRandSeed());
    QScopedPointer<QHBoxLayout> subLayout(new QHBoxLayout());
    subLayout->addWidget(m_randSeedLabel.data(), 0, Qt::AlignRight);
    subLayout->addWidget(m_randSeedSpinBox.data(), 0, Qt::AlignLeft);
    mainLayout->addLayout(subLayout.take());
    QScopedPointer<QDialogButtonBox> button(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel));
    connect(button.data(), SIGNAL(accepted()), SLOT(accept()));
    connect(button.data(), SIGNAL(rejected()), SLOT(reject()));
    connect(this, SIGNAL(accepted()), SLOT(emitSignal()));
    connect(this, SIGNAL(worldGravityDidSet(Vector3)), loader, SLOT(setWorldGravity(Vector3)));
    connect(this, SIGNAL(worldRandSeedDidSet(ulong)), loader, SLOT(setRandSeed(ulong)));
    mainLayout->addWidget(button.take());
    setWindowTitle(tr("Gravity Setting"));
    setLayout(mainLayout.take());
    retranslate();
}

GravitySettingDialog::~GravitySettingDialog()
{
}

void GravitySettingDialog::retranslate()
{
    m_axisGroup->setTitle(tr("Axis and Power"));
    m_randSeedLabel->setText(tr("Rand Seed"));
    m_axisXLabel->setText(tr("X"));
    m_axisYLabel->setText(tr("Y"));
    m_axisZLabel->setText(tr("Z"));
}

void GravitySettingDialog::emitSignal()
{
    const Vector3 value(m_axisX->value(), m_axisY->value(), m_axisZ->value());
    emit worldGravityDidSet(value);
    emit worldRandSeedDidSet(m_randSeedSpinBox->value());
}

QDoubleSpinBox *GravitySettingDialog::createSpinBox(double value) const
{
    QScopedPointer<QDoubleSpinBox> spinBox(new QDoubleSpinBox());
    spinBox->setAlignment(Qt::AlignRight);
    spinBox->setRange(-10000, 10000);
    spinBox->setSingleStep(0.1);
    spinBox->setValue(value);
    return spinBox.take();
}

} /* namespace vpvm */
