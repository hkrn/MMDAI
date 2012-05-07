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

#include "ShadowMapSettingDialog.h"
#include "common/SceneLoader.h"

#include <QtGui/QtGui>
#include <QtOpenGL/QtOpenGL>
#include <vpvl2/vpvl2.h>

using namespace vpvl2;

ShadowMapSettingDialog::ShadowMapSettingDialog(SceneLoader *loader, QWidget *parent)
    : QDialog(parent)
{
    const Scene::ICamera *camera = loader->scene()->camera();
    const Scalar zfar = camera->zfar();
    m_sizeLabel = new QLabel();
    m_sizeComboBox = new QComboBox();
    m_centerLabel = new QLabel();
    m_x = new QDoubleSpinBox();
    m_x->setRange(-zfar, zfar);
    m_y = new QDoubleSpinBox();
    m_y->setRange(-zfar, zfar);
    m_z = new QDoubleSpinBox();
    m_z->setRange(-zfar, zfar);
    m_radiusLabel = new QLabel();
    m_radius = new QDoubleSpinBox();
    m_radius->setRange(0.0, zfar);
    m_enableSoftShadow = new QCheckBox();
    m_enableSoftShadow->setChecked(loader->isSoftShadowEnabled());
    const Vector4 &boundingSphere = loader->shadowBoundingSphere();
    if (!boundingSphere.isZero() && btFuzzyZero(boundingSphere.w())) {
        m_x->setValue(boundingSphere.x());
        m_y->setValue(boundingSphere.y());
        m_z->setValue(boundingSphere.z());
        m_radius->setValue(boundingSphere.w());
    }
    else {
        Vector3 center;
        Scalar radius;
        loader->getBoundingSphere(center, radius);
        m_x->setValue(center.x());
        m_y->setValue(center.y());
        m_z->setValue(center.z());
        m_radius->setValue(radius);
    }
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
    QFormLayout *formLayout = new QFormLayout();
    formLayout->addRow(m_sizeLabel, m_sizeComboBox);
    QFormLayout *subLayout = new QFormLayout();
    subLayout->addRow("X", m_x);
    subLayout->addRow("Y", m_y);
    subLayout->addRow("Z", m_z);
    formLayout->addRow(m_centerLabel, subLayout);
    formLayout->addRow(m_radiusLabel, m_radius);
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(m_enableSoftShadow);
    QDialogButtonBox *button = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(button, SIGNAL(accepted()), SLOT(accept()));
    connect(button, SIGNAL(rejected()), SLOT(reject()));
    connect(this, SIGNAL(accepted()), SLOT(emitSignals()));
    connect(this, SIGNAL(sizeDidChange(QSize)), loader, SLOT(setShadowMapSize(QSize)));
    connect(this, SIGNAL(softShadowDidEnable(bool)), loader, SLOT(setSoftShadowEnable(bool)));
    connect(this, SIGNAL(boundingSphereDidChange(vpvl2::Vector4)), loader, SLOT(setShadowBoundingSphere(vpvl2::Vector4)));
    mainLayout->addWidget(button);
    setLayout(mainLayout);
    setWindowTitle("Shadow map setting");
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
    m_enableSoftShadow->setText(tr("Enable soft shadow"));
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
