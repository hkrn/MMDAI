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

#include "GravitySettingDialog.h"
#include "common/SceneLoader.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

using namespace vpvl;

GravitySettingDialog::GravitySettingDialog(SceneLoader *loader, QWidget *parent) :
    QDialog(parent),
    m_sceneLoader(loader)
{
    const Vector3 &gravity = loader->worldGravity();
    QHBoxLayout *subLayout = new QHBoxLayout();
    subLayout->addWidget(new QLabel("X"));
    m_axisX = createSpinBox(gravity.x());
    subLayout->addWidget(m_axisX);
    subLayout->addWidget(new QLabel("Y"));
    m_axisY = createSpinBox(gravity.y());
    subLayout->addWidget(m_axisY);
    subLayout->addWidget(new QLabel("Z"));
    m_axisZ = createSpinBox(gravity.z());
    subLayout->addWidget(m_axisZ);
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addLayout(subLayout);
    QDialogButtonBox *button = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(button, SIGNAL(accepted()), SLOT(accept()));
    connect(button, SIGNAL(rejected()), SLOT(reject()));
    mainLayout->addWidget(button);
    setWindowTitle(tr("Gravity setting"));
    setLayout(mainLayout);
}

GravitySettingDialog::~GravitySettingDialog()
{
}

const Vector3 GravitySettingDialog::value() const
{
    return Vector3(m_axisX->value(), m_axisY->value(), m_axisZ->value());
}

void GravitySettingDialog::showEvent(QShowEvent * /* event */)
{
    const Vector3 &gravity = m_sceneLoader->worldGravity();
    m_axisX->setValue(gravity.x());
    m_axisY->setValue(gravity.y());
    m_axisZ->setValue(gravity.z());
}

QDoubleSpinBox *GravitySettingDialog::createSpinBox(double value) const
{
    QDoubleSpinBox *spinBox = new QDoubleSpinBox();
    spinBox->setAlignment(Qt::AlignRight);
    spinBox->setRange(-10000, 10000);
    spinBox->setSingleStep(0.1);
    spinBox->setValue(value);
    return spinBox;
}
