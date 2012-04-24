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

using namespace vpvl2;

ModelSettingWidget::ModelSettingWidget(QWidget *parent) :
    QWidget(parent),
    m_edgeColorDialog(0)
{
    /* エッジ幅 */
    m_edgeOffsetLabel = new QLabel();
    m_edgeOffsetSpinBox = new QDoubleSpinBox();
    connect(m_edgeOffsetSpinBox, SIGNAL(valueChanged(double)), SIGNAL(edgeOffsetDidChange(double)));
    m_edgeOffsetSpinBox->setEnabled(false);
    m_edgeOffsetSpinBox->setSingleStep(0.1);
    m_edgeOffsetSpinBox->setRange(0.0, 2.0);
    /* エッジ色 */
    m_edgeColorDialogOpenButton = new QPushButton();
    connect(m_edgeColorDialogOpenButton, SIGNAL(clicked()), SLOT(openEdgeColorDialog()));
    m_projectiveShadowCheckbox = new QCheckBox();
    connect(m_projectiveShadowCheckbox, SIGNAL(toggled(bool)), SIGNAL(projectiveShadowDidChange(bool)));
    m_projectiveShadowCheckbox->setEnabled(false);
    /* モデルの位置(X,Y,Z) */
    const Scalar &zfar = 10000;
    QFormLayout *formLayout = new QFormLayout();
    m_px = createSpinBox(SLOT(updatePosition()), -zfar, zfar);
    formLayout->addRow("X", m_px);
    m_py = createSpinBox(SLOT(updatePosition()), -zfar, zfar);
    formLayout->addRow("Y", m_py);
    m_pz = createSpinBox(SLOT(updatePosition()), -zfar, zfar);
    formLayout->addRow("Z", m_pz);
    m_positionGroup = new QGroupBox();
    m_positionGroup->setLayout(formLayout);
    /* モデルの回転(X,Y,Z) */
    formLayout = new QFormLayout();
    m_rx = createSpinBox(SLOT(updateRotation()), -180.0, 180.0);
    formLayout->addRow("X", m_rx);
    m_ry = createSpinBox(SLOT(updateRotation()), -180.0, 180.0);
    formLayout->addRow("Y", m_ry);
    m_rz = createSpinBox(SLOT(updateRotation()), -180.0, 180.0);
    formLayout->addRow("Z", m_rz);
    m_rotationGroup = new QGroupBox();
    m_rotationGroup->setLayout(formLayout);
    /* 構築 */
    QVBoxLayout *mainLayout = new QVBoxLayout();
    QHBoxLayout *subLayout = new QHBoxLayout();
    subLayout->setAlignment(Qt::AlignCenter);
    subLayout->addWidget(m_edgeOffsetLabel);
    subLayout->addWidget(m_edgeOffsetSpinBox);
    subLayout->addWidget(m_edgeColorDialogOpenButton);
    mainLayout->addLayout(subLayout);
    mainLayout->addWidget(m_projectiveShadowCheckbox, 0, Qt::AlignCenter);
    subLayout = new QHBoxLayout();
    subLayout->addWidget(m_positionGroup);
    subLayout->addWidget(m_rotationGroup);
    mainLayout->addLayout(subLayout);
    mainLayout->addStretch();
    setLayout(mainLayout);
    retranslate();
}

ModelSettingWidget::~ModelSettingWidget()
{
}

void ModelSettingWidget::retranslate()
{
    m_edgeOffsetLabel->setText(tr("Edge offset:"));
    m_edgeColorDialogOpenButton->setText(tr("Color"));
    m_projectiveShadowCheckbox->setText(tr("Enable projective shadow"));
    m_positionGroup->setTitle(tr("Position offset"));
    m_rotationGroup->setTitle(tr("Rotation offset"));
}

void ModelSettingWidget::openEdgeColorDialog()
{
    createEdgeColorDialog();
    m_edgeColorDialog->show();
}

void ModelSettingWidget::setModel(IModel *model, SceneLoader *loader)
{
    /* 予期せぬ値変更を伴うシグナル発行防止のため、一時的に無効にする */
    disableSignals();
    if (model) {
        createEdgeColorDialog();
        // FIXME: m_edgeOffsetSpinBox->setValue(model->edgeOffset());
        m_edgeOffsetSpinBox->setEnabled(true);
        m_projectiveShadowCheckbox->setEnabled(true);
        const Vector3 &color = kZeroC; // FIXME: model->edgeColor();
        QColor c;
        c.setRedF(color.x());
        c.setGreenF(color.y());
        c.setBlueF(color.z());
        c.setAlphaF(1.0);
        m_edgeColorDialog->setCurrentColor(c);
        if (loader) {
            m_projectiveShadowCheckbox->setChecked(loader->isProjectiveShadowEnabled(model));
        }
        else {
            m_projectiveShadowCheckbox->setChecked(false);
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
        m_projectiveShadowCheckbox->setEnabled(false);
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

void ModelSettingWidget::createEdgeColorDialog()
{
    if (!m_edgeColorDialog) {
        m_edgeColorDialog = new QColorDialog(this);
        connect(m_edgeColorDialog, SIGNAL(currentColorChanged(QColor)), SIGNAL(edgeColorDidChange(QColor)));
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

void ModelSettingWidget::disableSignals()
{
    disconnect(m_px, SIGNAL(valueChanged(double)), this, SLOT(updatePosition()));
    disconnect(m_py, SIGNAL(valueChanged(double)), this, SLOT(updatePosition()));
    disconnect(m_pz, SIGNAL(valueChanged(double)), this, SLOT(updatePosition()));
    disconnect(m_rx, SIGNAL(valueChanged(double)), this, SLOT(updateRotation()));
    disconnect(m_ry, SIGNAL(valueChanged(double)), this, SLOT(updateRotation()));
    disconnect(m_rz, SIGNAL(valueChanged(double)), this, SLOT(updateRotation()));
}

void ModelSettingWidget::enableSignals()
{
    connect(m_px, SIGNAL(valueChanged(double)), this, SLOT(updatePosition()));
    connect(m_py, SIGNAL(valueChanged(double)), this, SLOT(updatePosition()));
    connect(m_pz, SIGNAL(valueChanged(double)), this, SLOT(updatePosition()));
    connect(m_rx, SIGNAL(valueChanged(double)), this, SLOT(updateRotation()));
    connect(m_ry, SIGNAL(valueChanged(double)), this, SLOT(updateRotation()));
    connect(m_rz, SIGNAL(valueChanged(double)), this, SLOT(updateRotation()));
}

QDoubleSpinBox *ModelSettingWidget::createSpinBox(const char *slot, double min, double max, double step) const
{
    QDoubleSpinBox *spinBox = new QDoubleSpinBox();
    spinBox->setRange(min, max);
    spinBox->setSingleStep(step);
    connect(spinBox, SIGNAL(valueChanged(double)), slot);
    return spinBox;
}
