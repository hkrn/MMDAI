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

#include "ShadowMapSettingDialog.h"
#include "common/SceneLoader.h"

#include <QtGui/QtGui>
#include <QtOpenGL/QtOpenGL>
#include <vpvl2/vpvl2.h>

namespace vpvm
{

using namespace vpvl2;

ShadowMapSettingDialog::ShadowMapSettingDialog(SceneLoader *loader, QWidget *parent)
    : QDialog(parent),
      m_sizeLabel(new QLabel()),
      m_sizeComboBox(new QComboBox()),
      m_enableSoftShadow(new QCheckBox()),
      m_enableAutoLightView(new QCheckBox()),
      m_centerLabel(new QLabel()),
      m_x(createSpinBox(-loader->sceneRef()->camera()->zfar(), loader->sceneRef()->camera()->zfar())),
      m_y(createSpinBox(-loader->sceneRef()->camera()->zfar(), loader->sceneRef()->camera()->zfar())),
      m_z(createSpinBox(-loader->sceneRef()->camera()->zfar(), loader->sceneRef()->camera()->zfar())),
      m_radiusLabel(new QLabel()),
      m_radius(createSpinBox(0.0, loader->sceneRef()->camera()->zfar()))
{
    m_enableSoftShadow->setChecked(loader->isSoftShadowEnabled());
    m_boundingSphere = loader->shadowBoundingSphere();
    bool autoLightView = m_boundingSphere.isZero() && btFuzzyZero(m_boundingSphere.w());
    if (autoLightView) {
        Vector3 center;
        Scalar radius;
        loader->getBoundingSphere(center, radius);
        m_boundingSphere.setValue(center.x(), center.y(), center.z(), radius);
    }
    toggleLightViewParameter(autoLightView);
    connect(m_enableAutoLightView.data(), SIGNAL(toggled(bool)), SLOT(toggleLightViewParameter(bool)));
    int i = 8, size = 128, max;
    const qreal swidth = loader->shadowMapSize().width();
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
    while (size <= max) {
        m_sizeComboBox->addItem(QString("%1x%1").arg(size), size);
        if (swidth == size)
            m_sizeComboBox->setCurrentIndex(m_sizeComboBox->count() - 1);
        size = powl(2, i);
        i++;
    }
    QScopedPointer<QVBoxLayout> mainLayout(new QVBoxLayout());
    QScopedPointer<QFormLayout> formLayout(new QFormLayout());
    formLayout->addRow(m_sizeLabel.data(), m_sizeComboBox.data());
    mainLayout->addLayout(formLayout.take());
    formLayout.reset(new QFormLayout());
    QScopedPointer<QFormLayout> subLayout(new QFormLayout());
    subLayout->addRow("X", m_x.data());
    subLayout->addRow("Y", m_y.data());
    subLayout->addRow("Z", m_z.data());
    formLayout->addRow(m_centerLabel.data(), subLayout.take());
    formLayout->addRow(m_radiusLabel.data(), m_radius.data());
    mainLayout->addWidget(m_enableSoftShadow.data());
    mainLayout->addWidget(m_enableAutoLightView.data());
    mainLayout->addLayout(formLayout.take());
    QScopedPointer<QDialogButtonBox> button(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel));
    connect(button.data(), SIGNAL(accepted()), SLOT(accept()));
    connect(button.data(), SIGNAL(rejected()), SLOT(reject()));
    connect(this, SIGNAL(accepted()), SLOT(emitSignals()));
    connect(this, SIGNAL(sizeDidChange(QSize)), loader, SLOT(setShadowMapSize(QSize)));
    connect(this, SIGNAL(softShadowDidEnable(bool)), loader, SLOT(setSoftShadowEnable(bool)));
    connect(this, SIGNAL(boundingSphereDidChange(Vector4)), loader, SLOT(setShadowBoundingSphere(Vector4)));
    mainLayout->addWidget(button.take());
    setLayout(mainLayout.take());
    retranslate();
}

ShadowMapSettingDialog::~ShadowMapSettingDialog()
{
}

void ShadowMapSettingDialog::retranslate()
{
    m_sizeLabel->setText(tr("Size"));
    m_centerLabel->setText(tr("Center"));
    m_radiusLabel->setText(tr("Radius"));
    m_enableSoftShadow->setText(tr("Enable Soft Shadow"));
    m_enableAutoLightView->setText(tr("Calculate Light View Automatically"));
    setWindowTitle(tr("Shadow Map Setting"));
}

void ShadowMapSettingDialog::emitSignals()
{
    int size = m_sizeComboBox->itemData(m_sizeComboBox->currentIndex()).toInt();
    bool enableSoftShadow = m_enableSoftShadow->isChecked();
    Vector4 boundingSphere(m_x->value(), m_y->value(), m_z->value(), m_radius->value());
    emit sizeDidChange(QSize(size, size));
    emit softShadowDidEnable(enableSoftShadow);
    emit boundingSphereDidChange(boundingSphere);
}

void ShadowMapSettingDialog::toggleLightViewParameter(bool value)
{
    if (value) {
        m_x->setValue(0);
        m_y->setValue(0);
        m_z->setValue(0);
        m_radius->setValue(0);
        m_x->setEnabled(false);
        m_y->setEnabled(false);
        m_z->setEnabled(false);
        m_radius->setEnabled(false);
        m_enableAutoLightView->setChecked(true);
    }
    else {
        m_x->setValue(m_boundingSphere.x());
        m_y->setValue(m_boundingSphere.y());
        m_z->setValue(m_boundingSphere.z());
        m_radius->setValue(m_boundingSphere.w());
        m_x->setEnabled(true);
        m_y->setEnabled(true);
        m_z->setEnabled(true);
        m_radius->setEnabled(true);
        m_enableAutoLightView->setChecked(false);
    }
}

QDoubleSpinBox *ShadowMapSettingDialog::createSpinBox(double min, double max)
{
    QScopedPointer<QDoubleSpinBox> spinBox(new QDoubleSpinBox());
    spinBox->setRange(min, max);
    spinBox->setAlignment(Qt::AlignLeft);
    return spinBox.take();
}

} /* namespace vpvm */
