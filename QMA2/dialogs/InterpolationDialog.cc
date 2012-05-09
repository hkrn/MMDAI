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
#include "models/SceneMotionModel.h"
#include "dialogs/InterpolationDialog.h"
#include "widgets/InterpolationGraphWidget.h"
#include "widgets/TimelineTabWidget.h"

#include <QtGui/QtGui>
#include <vpvl2/vpvl2.h>

using namespace vpvl2;

InterpolationDialog::InterpolationDialog(BoneMotionModel *bmm, SceneMotionModel *smm, QWidget *parent)
    : QWidget(parent)
{

    m_comboBox = new QComboBox();
    m_graphWidget = new InterpolationGraphWidget(bmm, smm);
    connect(m_comboBox, SIGNAL(currentIndexChanged(int)), m_graphWidget, SLOT(setIndex(int)));
    QHBoxLayout *c = new QHBoxLayout();
    QPushButton *button = new QPushButton(tr("Reset"));
    connect(button, SIGNAL(clicked()), this, SLOT(resetInterpolation()));
    c->addWidget(m_comboBox);
    c->addWidget(button);
    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->addWidget(new QLabel("X1"), 0, 0);
    gridLayout->addWidget(createSpinBox(20, SIGNAL(x1ValueDidChange(int)), SLOT(setX1(int))), 0, 1);
    gridLayout->addWidget(new QLabel("X2"), 1, 0);
    gridLayout->addWidget(createSpinBox(107, SIGNAL(x2ValueDidChange(int)), SLOT(setX2(int))), 1, 1);
    gridLayout->addWidget(new QLabel("Y1"), 2, 0);
    gridLayout->addWidget(createSpinBox(20, SIGNAL(y1ValueDidChange(int)), SLOT(setY1(int))), 2, 1);
    gridLayout->addWidget(new QLabel("Y2"), 3, 0);
    gridLayout->addWidget(createSpinBox(107, SIGNAL(y2ValueDidChange(int)), SLOT(setY2(int))), 3, 1);
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addLayout(c);
    QHBoxLayout *subLayout = new QHBoxLayout();
    subLayout->addWidget(m_graphWidget);
    subLayout->addLayout(gridLayout);
    subLayout->setAlignment(gridLayout, Qt::AlignCenter);
    mainLayout->addLayout(subLayout);
    //mainLayout->addWidget(m_graphWidget, Qt::AlignVCenter);
    //mainLayout->setAlignment(gridLayout, Qt::AlignCenter);
    mainLayout->addStretch();
    setLayout(mainLayout);
    setEnabled(false);
}

InterpolationDialog::~InterpolationDialog()
{
}

void InterpolationDialog::setMode(int mode)
{
    bool enabled = true;
    m_comboBox->clear();
    if (mode == TimelineTabWidget::kBoneTabIndex) {
        m_comboBox->addItem(tr("X axis"));
        m_comboBox->addItem(tr("Y axis"));
        m_comboBox->addItem(tr("Z axis"));
        m_comboBox->addItem(tr("Rotation"));
        m_graphWidget->setType(InterpolationGraphWidget::kBone);
    }
    else if (mode == TimelineTabWidget::kSceneTabIndex) {
        m_comboBox->addItem(tr("X axis"));
        m_comboBox->addItem(tr("Y axis"));
        m_comboBox->addItem(tr("Z axis"));
        m_comboBox->addItem(tr("Rotation"));
        m_comboBox->addItem(tr("Fovy"));
        m_comboBox->addItem(tr("Distance"));
        m_graphWidget->setType(InterpolationGraphWidget::kCamera);
    }
    else {
        enabled = false;
    }
    setEnabled(enabled);
}

void InterpolationDialog::setModelIndices(const QModelIndexList &indices)
{
    m_graphWidget->setModelIndices(indices);
}

void InterpolationDialog::disable()
{
    resetInterpolation();
    setEnabled(false);
}

void InterpolationDialog::resetInterpolation()
{
    m_graphWidget->setX1(20);
    m_graphWidget->setY1(20);
    m_graphWidget->setX2(107);
    m_graphWidget->setY2(107);
}

QSpinBox *InterpolationDialog::createSpinBox(int defaultValue,
                                             const char *signal,
                                             const char *slot)
{
    QSpinBox *spinBox = new QSpinBox();
    spinBox->setRange(0, 127);
    connect(spinBox, SIGNAL(valueChanged(int)), m_graphWidget, slot);
    connect(m_graphWidget, signal, spinBox, SLOT(setValue(int)));
    spinBox->setValue(defaultValue);
    return spinBox;
}
