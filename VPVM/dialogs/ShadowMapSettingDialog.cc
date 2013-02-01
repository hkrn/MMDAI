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
      m_centerLabel(new QLabel()),
      m_x(createSpinBox(-loader->sceneRef()->camera()->zfar(), loader->sceneRef()->camera()->zfar())),
      m_y(createSpinBox(-loader->sceneRef()->camera()->zfar(), loader->sceneRef()->camera()->zfar())),
      m_z(createSpinBox(-loader->sceneRef()->camera()->zfar(), loader->sceneRef()->camera()->zfar())),
      m_distanceLabel(new QLabel()),
      m_distance(createSpinBox(0.0, loader->sceneRef()->camera()->zfar())),
      m_rateLabel(new QLabel()),
      m_rate(createSpinBox(0.0, 1.0))
{
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
    formLayout->addRow(m_distanceLabel.data(), m_distance.data());
    formLayout->addRow(m_rateLabel.data(), m_rate.data());
    mainLayout->addLayout(formLayout.take());
    QScopedPointer<QDialogButtonBox> button(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel));
    connect(button.data(), SIGNAL(accepted()), SLOT(accept()));
    connect(button.data(), SIGNAL(rejected()), SLOT(reject()));
    connect(this, SIGNAL(accepted()), SLOT(emitSignals()));
    connect(this, SIGNAL(sizeDidChange(QSize)), loader, SLOT(setShadowMapSize(QSize)));
    connect(this, SIGNAL(centerDidChange(Vector3)), loader, SLOT(setShadowCenter(Vector3)));
    connect(this, SIGNAL(distanceDidChange(Scalar)), loader, SLOT(setShadowDistance(Scalar)));
    connect(this, SIGNAL(rateDidChange(Scalar)), loader, SLOT(setShadowRate(Scalar)));
    const Vector3 &value = loader->shadowCenter();
    m_x->setValue(value.x());
    m_y->setValue(value.y());
    m_z->setValue(value.z());
    m_distance->setValue(loader->shadowDistance());
    m_rate->setValue(loader->shadowRate());
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
    m_distanceLabel->setText(tr("Distance"));
    m_rateLabel->setText(tr("Rate"));
    setWindowTitle(tr("Shadow Map Setting"));
}

void ShadowMapSettingDialog::emitSignals()
{
    Vector3 boundingSphere(m_x->value(), m_y->value(), m_z->value());
    int size = m_sizeComboBox->itemData(m_sizeComboBox->currentIndex()).toInt();
    emit sizeDidChange(QSize(size, size));
    emit centerDidChange(boundingSphere);
    emit distanceDidChange(m_distance->value());
    emit rateDidChange(m_rate->value());
}

QDoubleSpinBox *ShadowMapSettingDialog::createSpinBox(double min, double max)
{
    QScopedPointer<QDoubleSpinBox> spinBox(new QDoubleSpinBox());
    spinBox->setRange(min, max);
    spinBox->setAlignment(Qt::AlignLeft);
    return spinBox.take();
}

} /* namespace vpvm */
