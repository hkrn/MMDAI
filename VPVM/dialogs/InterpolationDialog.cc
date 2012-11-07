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

namespace vpvm
{

/* lupdate cannot parse tr() syntax correctly */

using namespace vpvl2;

InterpolationDialog::InterpolationDialog(BoneMotionModel *bmm, SceneMotionModel *smm, QWidget *parent)
    : QWidget(parent),
      m_parameterTypeLabel(new QLabel()),
      m_parameterTypeComboBox(new QComboBox()),
      m_presetLabel(new QLabel()),
      m_presetComboBox(new QComboBox()),
      m_parameterGroup(new QGroupBox()),
      m_applyAllButton(new QPushButton()),
      m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Discard|QDialogButtonBox::Reset)),
      m_graphWidget(new InterpolationGraphWidget(bmm, smm))
{
    QScopedPointer<QVBoxLayout> mainLayout(new QVBoxLayout());
    QScopedPointer<QHBoxLayout> subLayout(new QHBoxLayout());
    connect(m_parameterTypeComboBox.data(), SIGNAL(currentIndexChanged(int)),
            m_graphWidget.data(), SLOT(selectParameterType(int)));
    subLayout->addWidget(m_parameterTypeLabel.data());
    subLayout->addWidget(m_parameterTypeComboBox.data());
    mainLayout->addLayout(subLayout.take());
    connect(m_presetComboBox.data(), SIGNAL(currentIndexChanged(int)), SLOT(selectPreset(int)));
    subLayout.reset(new QHBoxLayout());
    subLayout->addWidget(m_presetLabel.data());
    subLayout->addWidget(m_presetComboBox.data());
    mainLayout->addLayout(subLayout.take());
    QScopedPointer<QFormLayout> parameterLayout(new QFormLayout());
    parameterLayout->addRow("X1", createSpinBox(20, SIGNAL(x1ValueDidChange(int)), SLOT(setX1(int))));
    parameterLayout->addRow("X2", createSpinBox(107, SIGNAL(x2ValueDidChange(int)), SLOT(setX2(int))));
    parameterLayout->addRow("Y1", createSpinBox(20, SIGNAL(y1ValueDidChange(int)), SLOT(setY1(int))));
    parameterLayout->addRow("Y2", createSpinBox(107, SIGNAL(y2ValueDidChange(int)), SLOT(setY2(int))));
    QScopedPointer<QVBoxLayout> groupLayout(new QVBoxLayout());
    subLayout.reset(new QHBoxLayout());
    subLayout->addWidget(m_graphWidget.data());
    subLayout->addLayout(parameterLayout.data());
    subLayout->setAlignment(m_graphWidget.data(), Qt::AlignRight);
    subLayout->setAlignment(parameterLayout.take(), Qt::AlignLeft);
    groupLayout->addLayout(subLayout.take());
    connect(m_applyAllButton.data(), SIGNAL(clicked()), m_graphWidget.data(), SLOT(applyAll()));
    groupLayout->addWidget(m_applyAllButton.data());
    groupLayout->setAlignment(m_applyAllButton.data(), Qt::AlignCenter);
    m_parameterGroup->setLayout(groupLayout.take());
    mainLayout->addWidget(m_parameterGroup.data());
    connect(m_buttonBox.data(), SIGNAL(clicked(QAbstractButton*)), SLOT(clickButton(QAbstractButton*)));
    mainLayout->addWidget(m_buttonBox.data());
    mainLayout->addStretch();
    setLayout(mainLayout.take());
    retranslate();
    setEnabled(false);
    setWindowTitle(vpvm::InterpolationDialog::tr("Keyframe interpolation setting"));
}

InterpolationDialog::~InterpolationDialog()
{
}

void InterpolationDialog::setMode(int mode)
{
    bool enabled = true;
    m_parameterTypeComboBox->clear();
    m_presetComboBox->setCurrentIndex(0);
    if (mode == TimelineTabWidget::kBoneTabIndex) {
        m_parameterTypeComboBox->addItem(vpvm::InterpolationDialog::tr("X axis"));
        m_parameterTypeComboBox->addItem(vpvm::InterpolationDialog::tr("Y axis"));
        m_parameterTypeComboBox->addItem(vpvm::InterpolationDialog::tr("Z axis"));
        m_parameterTypeComboBox->addItem(vpvm::InterpolationDialog::tr("Rotation"));
        m_graphWidget->setType(InterpolationGraphWidget::kBone);
    }
    else if (mode == TimelineTabWidget::kSceneTabIndex) {
        m_parameterTypeComboBox->addItem(vpvm::InterpolationDialog::tr("X axis"));
        m_parameterTypeComboBox->addItem(vpvm::InterpolationDialog::tr("Y axis"));
        m_parameterTypeComboBox->addItem(vpvm::InterpolationDialog::tr("Z axis"));
        m_parameterTypeComboBox->addItem(vpvm::InterpolationDialog::tr("Rotation"));
        m_parameterTypeComboBox->addItem(vpvm::InterpolationDialog::tr("Fovy"));
        m_parameterTypeComboBox->addItem(vpvm::InterpolationDialog::tr("Distance"));
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

bool InterpolationDialog::hasValidKeyframes() const
{
    return m_graphWidget->isEnabled();
}

void InterpolationDialog::retranslate()
{
    m_parameterTypeLabel->setText(vpvm::InterpolationDialog::tr("Parameter type"));
    m_presetLabel->setText(vpvm::InterpolationDialog::tr("Preset"));
    m_presetComboBox->clear();
    m_presetComboBox->addItem(vpvm::InterpolationDialog::tr("None"), QVector4D());
    m_presetComboBox->addItem(vpvm::InterpolationDialog::tr("Linear default"), QVector4D(20, 107, 20, 107));
    m_presetComboBox->addItem(vpvm::InterpolationDialog::tr("S-curve"), QVector4D(127, 0, 0, 127));
    m_presetComboBox->addItem(vpvm::InterpolationDialog::tr("Reversed S-curve"), QVector4D(0, 127, 127, 0));
    m_presetComboBox->addItem(vpvm::InterpolationDialog::tr("Half S-curve"), QVector4D(64, 64, 0, 127));
    m_presetComboBox->addItem(vpvm::InterpolationDialog::tr("Half reversed S-curve"), QVector4D(0, 127, 64, 64));
    m_applyAllButton->setText(vpvm::InterpolationDialog::tr("Apply all"));
    m_parameterGroup->setTitle(vpvm::InterpolationDialog::tr("Interpolation parameter"));
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
        m_graphWidget->reset();
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
    QScopedPointer<QSpinBox> spinBox(new QSpinBox());
    spinBox->setAlignment(Qt::AlignRight);
    spinBox->setRange(0, 127);
    connect(spinBox.data(), SIGNAL(valueChanged(int)), m_graphWidget.data(), slot);
    connect(m_graphWidget.data(), signal, spinBox.data(), SLOT(setValue(int)));
    spinBox->setValue(defaultValue);
    return spinBox.take();
}

} /* namespace vpvm */
