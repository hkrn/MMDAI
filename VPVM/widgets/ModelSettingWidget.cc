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

#include "ModelSettingWidget.h"
#include "common/SceneLoader.h"

#include <QtGui/QtGui>
#include <vpvl2/vpvl2.h>

namespace vpvm
{

static const Scalar kZfar = 10000;

using namespace vpvl2;

ModelSettingWidget::ModelSettingWidget(QWidget *parent)
    : QWidget(parent),
      m_edgeOffsetLabel(new QLabel()),
      m_edgeOffsetSpinBox(new QDoubleSpinBox()),
      m_edgeColorDialogOpenButton(new QPushButton()),
      m_opacityLabel(new QLabel()),
      m_opacitySlider(new QSlider(Qt::Horizontal)),
      m_opacitySpinBox(new QSpinBox()),
      m_shadowGroup(new QGroupBox()),
      m_shadowButtonsGroup(new QButtonGroup()),
      m_noShadowCheckbox(new QRadioButton()),
      m_projectiveShadowCheckbox(new QRadioButton()),
      m_selfShadowCheckbox(new QRadioButton()),
      m_px(createSpinBox(SLOT(updatePosition()), -kZfar, kZfar)),
      m_py(createSpinBox(SLOT(updatePosition()), -kZfar, kZfar)),
      m_pz(createSpinBox(SLOT(updatePosition()), -kZfar, kZfar)),
      m_positionGroup(new QGroupBox()),
      m_rx(createSpinBox(SLOT(updateRotation()), -180.0, 180.0)),
      m_ry(createSpinBox(SLOT(updateRotation()), -180.0, 180.0)),
      m_rz(createSpinBox(SLOT(updateRotation()), -180.0, 180.0)),
      m_rotationGroup(new QGroupBox())
{
    /* エッジ幅 */
    connect(m_edgeOffsetSpinBox.data(), SIGNAL(valueChanged(double)), SIGNAL(edgeOffsetDidChange(double)));
    m_edgeOffsetSpinBox->setEnabled(false);
    m_edgeOffsetSpinBox->setSingleStep(0.1);
    m_edgeOffsetSpinBox->setRange(0.0, 2.0);
    /* エッジ色 */
    connect(m_edgeColorDialogOpenButton.data(), SIGNAL(clicked()), SLOT(openEdgeColorDialog()));
    /* 不透明度 */
    m_opacitySlider->setRange(0, 100);
    m_opacitySpinBox->setRange(0, 100);
    connect(m_opacitySlider.data(), SIGNAL(valueChanged(int)), m_opacitySpinBox.data(), SLOT(setValue(int)));
    connect(m_opacitySpinBox.data(), SIGNAL(valueChanged(int)), m_opacitySlider.data(), SLOT(setValue(int)));
    connect(m_opacitySpinBox.data(), SIGNAL(valueChanged(int)), SLOT(emitOpacitySignal(int)));
    /* 影の種類の選択 */
    m_shadowGroup->setEnabled(false);
    m_noShadowCheckbox->setChecked(true);
    connect(m_projectiveShadowCheckbox.data(), SIGNAL(toggled(bool)), SIGNAL(projectiveShadowDidEnable(bool)));
    connect(m_selfShadowCheckbox.data(), SIGNAL(toggled(bool)), SIGNAL(selfShadowDidEnable(bool)));
    m_shadowButtonsGroup->addButton(m_noShadowCheckbox.data());
    m_shadowButtonsGroup->addButton(m_projectiveShadowCheckbox.data());
    m_shadowButtonsGroup->addButton(m_selfShadowCheckbox.data());
    /* モデルの位置(X,Y,Z) */
    QScopedPointer<QFormLayout> formLayout(new QFormLayout());
    formLayout->addRow("X", m_px.data());
    formLayout->addRow("Y", m_py.data());
    formLayout->addRow("Z", m_pz.data());
    m_positionGroup->setLayout(formLayout.take());
    /* モデルの回転(X,Y,Z) */
    formLayout.reset(new QFormLayout());
    formLayout->addRow("X", m_rx.data());
    formLayout->addRow("Y", m_ry.data());
    formLayout->addRow("Z", m_rz.data());
    m_rotationGroup->setLayout(formLayout.take());
    /* 構築 */
    QScopedPointer<QVBoxLayout> mainLayout(new QVBoxLayout());
    QScopedPointer<QLayout> subLayout(new QHBoxLayout());
    subLayout->setAlignment(Qt::AlignCenter);
    subLayout->addWidget(m_opacityLabel.data());
    subLayout->addWidget(m_opacitySlider.data());
    subLayout->addWidget(m_opacitySpinBox.data());
    mainLayout->addLayout(subLayout.take());
    subLayout.reset(new QHBoxLayout());
    subLayout->setAlignment(Qt::AlignCenter);
    subLayout->addWidget(m_edgeOffsetLabel.data());
    subLayout->addWidget(m_edgeOffsetSpinBox.data());
    subLayout->addWidget(m_edgeColorDialogOpenButton.data());
    mainLayout->addLayout(subLayout.take());
    subLayout.reset(new QVBoxLayout());
    subLayout->addWidget(m_noShadowCheckbox.data());
    subLayout->addWidget(m_projectiveShadowCheckbox.data());
    subLayout->addWidget(m_selfShadowCheckbox.data());
    m_shadowGroup->setLayout(subLayout.take());
    mainLayout->addWidget(m_shadowGroup.data());
    subLayout.reset(new QHBoxLayout());
    subLayout->addWidget(m_positionGroup.data());
    subLayout->addWidget(m_rotationGroup.data());
    mainLayout->addLayout(subLayout.take());
    mainLayout->addStretch();
    setLayout(mainLayout.take());
    retranslate();
}

ModelSettingWidget::~ModelSettingWidget()
{
}

void ModelSettingWidget::retranslate()
{
    m_edgeOffsetLabel->setText(tr("Edge offset:"));
    m_edgeColorDialogOpenButton->setText(tr("Color"));
    m_opacityLabel->setText(tr("Opacity:"));
    m_shadowGroup->setTitle(tr("Shadow"));
    m_noShadowCheckbox->setText(tr("Disable shadow"));
    m_projectiveShadowCheckbox->setText(tr("Enable projective shadow"));
    m_selfShadowCheckbox->setText(tr("Enable self shadow"));
    m_positionGroup->setTitle(tr("Position offset"));
    m_rotationGroup->setTitle(tr("Rotation offset"));
}

void ModelSettingWidget::openEdgeColorDialog()
{
    createEdgeColorDialog(QColor(Qt::black));
    m_edgeColorDialog->show();
}

void ModelSettingWidget::setModel(IModel *model, SceneLoader *loader)
{
    /* 予期せぬ値変更を伴うシグナル発行防止のため、一時的に無効にする */
    disableSignals();
    if (model) {
        const Vector3 &color = model->edgeColor();
        const QColor &c = QColor::fromRgbF(color.x(), color.y(), color.z());
        createEdgeColorDialog(c);
        m_edgeOffsetSpinBox->setValue(model->edgeWidth());
        m_edgeOffsetSpinBox->setEnabled(true);
        m_opacitySlider->setEnabled(true);
        m_opacitySpinBox->setEnabled(true);
        m_opacitySpinBox->setValue(model->opacity() * m_opacitySpinBox->maximum());
        m_shadowGroup->setEnabled(true);
        m_noShadowCheckbox->setChecked(true);
        m_edgeColorDialog->setCurrentColor(c);
        if (loader) {
            m_projectiveShadowCheckbox->setChecked(loader->isProjectiveShadowEnabled(model));
            m_selfShadowCheckbox->setChecked(loader->isSelfShadowEnabled(model));
        }
        else {
            m_projectiveShadowCheckbox->setChecked(false);
            m_selfShadowCheckbox->setChecked(false);
        }
        const Vector3 &position = model->position();
        m_px->setValue(position.x());
        m_py->setValue(position.y());
        m_pz->setValue(position.z());
        if (loader) {
            const Vector3 &angle = loader->modelRotation(model);
            m_rx->setValue(angle.x());
            m_ry->setValue(angle.y());
            m_rz->setValue(angle.z());
        }
        else {
            m_rx->setValue(0.0);
            m_ry->setValue(0.0);
            m_rz->setValue(0.0);
        }
    }
    else {
        m_edgeOffsetSpinBox->setValue(0.0f);
        m_edgeOffsetSpinBox->setEnabled(false);
        m_shadowGroup->setEnabled(true);
        m_opacitySlider->setEnabled(false);
        m_opacitySpinBox->setEnabled(false);
        m_opacitySpinBox->setValue(0);
        m_px->setValue(0.0);
        m_py->setValue(0.0);
        m_pz->setValue(0.0);
        m_rx->setValue(0.0);
        m_ry->setValue(0.0);
        m_rz->setValue(0.0);
    }
    enableSignals();
}

void ModelSettingWidget::setPositionOffset(const Vector3 &position)
{
    disableSignals();
    m_px->setValue(position.x());
    m_py->setValue(position.y());
    m_pz->setValue(position.z());
    enableSignals();
}

void ModelSettingWidget::createEdgeColorDialog(const QColor &color)
{
    if (!m_edgeColorDialog) {
        m_edgeColorDialog.reset(new QColorDialog(color, this));
        connect(m_edgeColorDialog.data(), SIGNAL(currentColorChanged(QColor)), SIGNAL(edgeColorDidChange(QColor)));
        m_edgeColorDialog->setOption(QColorDialog::NoButtons);
    }
}

void ModelSettingWidget::updatePosition()
{
    const Vector3 position(m_px->value(), m_py->value(), m_pz->value());
    emit positionOffsetDidChange(position);
}

void ModelSettingWidget::updateRotation()
{
    const Vector3 rotation(m_rx->value(), m_ry->value(), m_rz->value());
    emit rotationOffsetDidChange(rotation);
}

void ModelSettingWidget::emitOpacitySignal(int value)
{
    emit opacityDidChange(value / Scalar(m_opacitySlider->maximum()));
}

void ModelSettingWidget::disableSignals()
{
    disconnect(m_px.data(), SIGNAL(valueChanged(double)), this, SLOT(updatePosition()));
    disconnect(m_py.data(), SIGNAL(valueChanged(double)), this, SLOT(updatePosition()));
    disconnect(m_pz.data(), SIGNAL(valueChanged(double)), this, SLOT(updatePosition()));
    disconnect(m_rx.data(), SIGNAL(valueChanged(double)), this, SLOT(updateRotation()));
    disconnect(m_ry.data(), SIGNAL(valueChanged(double)), this, SLOT(updateRotation()));
    disconnect(m_rz.data(), SIGNAL(valueChanged(double)), this, SLOT(updateRotation()));
}

void ModelSettingWidget::enableSignals()
{
    connect(m_px.data(), SIGNAL(valueChanged(double)), this, SLOT(updatePosition()));
    connect(m_py.data(), SIGNAL(valueChanged(double)), this, SLOT(updatePosition()));
    connect(m_pz.data(), SIGNAL(valueChanged(double)), this, SLOT(updatePosition()));
    connect(m_rx.data(), SIGNAL(valueChanged(double)), this, SLOT(updateRotation()));
    connect(m_ry.data(), SIGNAL(valueChanged(double)), this, SLOT(updateRotation()));
    connect(m_rz.data(), SIGNAL(valueChanged(double)), this, SLOT(updateRotation()));
}

QDoubleSpinBox *ModelSettingWidget::createSpinBox(const char *slot, double min, double max, double step) const
{
    QScopedPointer<QDoubleSpinBox> spinBox(new QDoubleSpinBox());
    spinBox->setRange(min, max);
    spinBox->setSingleStep(step);
    connect(spinBox.data(), SIGNAL(valueChanged(double)), slot);
    return spinBox.take();
}

} /* namespace vpvm */
