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

FrameWeightDialog::FrameWeightDialog(QWidget *parent) :
    QDialog(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    m_weightBox = new QDoubleSpinBox();
    m_weightBox->setMinimum(0.01);
    m_weightBox->setSingleStep(0.01);
    QFormLayout *formLayout = new QFormLayout();
    formLayout->addRow(tr("Keyframe weight"), m_weightBox);
    mainLayout->addLayout(formLayout);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    mainLayout->addWidget(buttons);
    connect(buttons, SIGNAL(accepted()), this, SLOT(emitKeyframeWeight()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(close()));
    connect(this, SIGNAL(keyframeWeightDidSet(float)), this, SLOT(close()));
    resetValue();
    setWindowTitle(tr("Keyframe weight dialog"));
    setLayout(mainLayout);
}

FrameWeightDialog::~FrameWeightDialog()
{
}

void FrameWeightDialog::resetValue()
{
    m_weightBox->setValue(1.0);
}

void FrameWeightDialog::emitKeyframeWeight()
{
    emit keyframeWeightDidSet(m_weightBox->value());
}
