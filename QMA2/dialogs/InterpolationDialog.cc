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

    m_graphWidget = new InterpolationGraphWidget(bmm, smm);
    QVBoxLayout *mainLayout = new QVBoxLayout();
    QHBoxLayout *subLayout = new QHBoxLayout();
    m_parameterTypeLabel = new QLabel();
    m_parameterTypeComboBox = new QComboBox();
    connect(m_parameterTypeComboBox, SIGNAL(currentIndexChanged(int)), m_graphWidget, SLOT(selectParameterType(int)));
    subLayout->addWidget(m_parameterTypeLabel);
    subLayout->addWidget(m_parameterTypeComboBox);
    mainLayout->addLayout(subLayout);
    m_presetLabel = new QLabel();
    m_presetComboBox = new QComboBox();
    connect(m_presetComboBox, SIGNAL(currentIndexChanged(int)), SLOT(selectPreset(int)));
    subLayout = new QHBoxLayout();
    subLayout->addWidget(m_presetLabel);
    subLayout->addWidget(m_presetComboBox);
    mainLayout->addLayout(subLayout);
    m_applyAllButton = new QPushButton();
    connect(m_applyAllButton, SIGNAL(clicked()), m_graphWidget, SLOT(applyAll()));
    mainLayout->addWidget(m_applyAllButton);
    mainLayout->setAlignment(m_applyAllButton, Qt::AlignCenter);
    QFormLayout *parameterLayout = new QFormLayout();
    parameterLayout->addRow("X1", createSpinBox(20, SIGNAL(x1ValueDidChange(int)), SLOT(setX1(int))));
    parameterLayout->addRow("X2", createSpinBox(107, SIGNAL(x2ValueDidChange(int)), SLOT(setX2(int))));
    parameterLayout->addRow("Y1", createSpinBox(20, SIGNAL(y1ValueDidChange(int)), SLOT(setY1(int))));
    parameterLayout->addRow("Y2", createSpinBox(107, SIGNAL(y2ValueDidChange(int)), SLOT(setY2(int))));
    subLayout = new QHBoxLayout();
    subLayout->addWidget(m_graphWidget);
    subLayout->addLayout(parameterLayout);
    subLayout->setAlignment(m_graphWidget, Qt::AlignRight);
    subLayout->setAlignment(parameterLayout, Qt::AlignLeft);
    mainLayout->addLayout(subLayout);
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Discard|QDialogButtonBox::Reset);
    connect(m_buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(clickButton(QAbstractButton*)));
    mainLayout->addWidget(m_buttonBox);
    mainLayout->addStretch();
    setLayout(mainLayout);
    retranslate();
    setEnabled(false);
    setWindowTitle(tr("Keyframe interpolation setting"));
}

InterpolationDialog::~InterpolationDialog()
{
}

void InterpolationDialog::setMode(int mode)
{
    bool enabled = true;
    m_parameterTypeComboBox->clear();
    if (mode == TimelineTabWidget::kBoneTabIndex) {
        m_parameterTypeComboBox->addItem(tr("X axis"));
        m_parameterTypeComboBox->addItem(tr("Y axis"));
        m_parameterTypeComboBox->addItem(tr("Z axis"));
        m_parameterTypeComboBox->addItem(tr("Rotation"));
        m_graphWidget->setType(InterpolationGraphWidget::kBone);
    }
    else if (mode == TimelineTabWidget::kSceneTabIndex) {
        m_parameterTypeComboBox->addItem(tr("X axis"));
        m_parameterTypeComboBox->addItem(tr("Y axis"));
        m_parameterTypeComboBox->addItem(tr("Z axis"));
        m_parameterTypeComboBox->addItem(tr("Rotation"));
        m_parameterTypeComboBox->addItem(tr("Fovy"));
        m_parameterTypeComboBox->addItem(tr("Distance"));
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

void InterpolationDialog::retranslate()
{
    m_parameterTypeLabel->setText(tr("Parameter type"));
    m_presetLabel->setText(tr("Preset"));
    m_presetComboBox->addItem(tr("None"), QVector4D());
    m_presetComboBox->addItem(tr("Linear default"), QVector4D(20, 107, 20, 107));
    m_presetComboBox->addItem(tr("S-curve"), QVector4D(127, 0, 0, 127));
    m_presetComboBox->addItem(tr("Reversed S-curve"), QVector4D(0, 127, 127, 0));
    m_presetComboBox->addItem(tr("Half S-curve"), QVector4D(64, 64, 0, 127));
    m_presetComboBox->addItem(tr("Half reversed S-curve"), QVector4D(0, 127, 64, 64));
    m_applyAllButton->setText(tr("Apply all"));
}

void InterpolationDialog::disable()
{
    m_graphWidget->setLinearInterpolation();
    setEnabled(false);
}

void InterpolationDialog::clickButton(QAbstractButton *button)
{
    QDialogButtonBox::ButtonRole role = m_buttonBox->buttonRole(button);
    switch (role) {
    case QDialogButtonBox::AcceptRole:
        m_graphWidget->save();
        close();
        break;
    case QDialogButtonBox::DestructiveRole:
        close();
        break;
    case QDialogButtonBox::ResetRole:
        m_graphWidget->setLinearInterpolation();
        break;
    case QDialogButtonBox::InvalidRole:
    case QDialogButtonBox::ActionRole:
    case QDialogButtonBox::ApplyRole:
    case QDialogButtonBox::RejectRole:
    case QDialogButtonBox::HelpRole:
    case QDialogButtonBox::YesRole:
    case QDialogButtonBox::NoRole:
    default:
        break;
    }
}

void InterpolationDialog::selectPreset(int value)
{
    const QVariant &variant = m_presetComboBox->itemData(value);
    const QVector4D &v4 = variant.value<QVector4D>();
    if (!v4.isNull()) {
        m_graphWidget->setX1(v4.x());
        m_graphWidget->setX2(v4.y());
        m_graphWidget->setY1(v4.z());
        m_graphWidget->setY2(v4.w());
    }
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
