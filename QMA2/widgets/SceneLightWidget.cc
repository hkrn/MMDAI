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

#include "SceneLightWidget.h"

#include <QtGui/QtGui>

SceneLightWidget::SceneLightWidget(QWidget *parent) :
    QWidget(parent)
{
    QHBoxLayout *subLayout = new QHBoxLayout();
    /* 色(R,G,B) */
    QFormLayout *formLayout = new QFormLayout();
    m_r = createSpinBox(SLOT(updateColor()), 0.0, 1.0, 0.005, 3);
    formLayout->addRow("R", m_r);
    m_g = createSpinBox(SLOT(updateColor()), 0.0, 1.0, 0.005, 3);
    formLayout->addRow("G", m_g);
    m_b = createSpinBox(SLOT(updateColor()), 0.0, 1.0, 0.005, 3);
    formLayout->addRow("B", m_b);
    m_colorGroup = new QGroupBox();
    m_colorGroup->setLayout(formLayout);
    subLayout->addWidget(m_colorGroup);
    /* 位置(X,Y,Z) */
    formLayout = new QFormLayout();
    m_x = createSpinBox(SLOT(updatePosition()), -1.0, 1.0, 0.01, 3);
    formLayout->addRow("X", m_x);
    m_y = createSpinBox(SLOT(updatePosition()), -1.0, 1.0, 0.01, 3);
    formLayout->addRow("Y", m_y);
    m_z = createSpinBox(SLOT(updatePosition()), -1.0, 1.0, 0.01, 3);
    formLayout->addRow("Z", m_z);
    m_directionGroup = new QGroupBox();
    m_directionGroup->setLayout(formLayout);
    subLayout->addWidget(m_directionGroup);
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addLayout(subLayout);
    m_openColorDialog = new QPushButton();
    connect(m_openColorDialog, SIGNAL(clicked()), SLOT(openColorDialog()));
    mainLayout->addWidget(m_openColorDialog, 0, Qt::AlignCenter);
    mainLayout->addStretch();
    setLayout(mainLayout);
    retranslate();
}

SceneLightWidget::~SceneLightWidget()
{
}

void SceneLightWidget::setColor(const vpvl::Color &value)
{
    disconnect(m_r, SIGNAL(valueChanged(double)), this, SLOT(updateColor()));
    disconnect(m_g, SIGNAL(valueChanged(double)), this, SLOT(updateColor()));
    disconnect(m_b, SIGNAL(valueChanged(double)), this, SLOT(updateColor()));
    m_r->setValue(value.x());
    m_g->setValue(value.y());
    m_b->setValue(value.z());
    connect(m_r, SIGNAL(valueChanged(double)), this, SLOT(updateColor()));
    connect(m_g, SIGNAL(valueChanged(double)), this, SLOT(updateColor()));
    connect(m_b, SIGNAL(valueChanged(double)), this, SLOT(updateColor()));
}

void SceneLightWidget::setPosition(const vpvl::Vector3 &value)
{
    disconnect(m_x, SIGNAL(valueChanged(double)), this, SLOT(updatePosition()));
    disconnect(m_y, SIGNAL(valueChanged(double)), this, SLOT(updatePosition()));
    disconnect(m_z, SIGNAL(valueChanged(double)), this, SLOT(updatePosition()));
    m_x->setValue(value.x());
    m_y->setValue(value.y());
    m_z->setValue(value.z());
    connect(m_x, SIGNAL(valueChanged(double)), this, SLOT(updatePosition()));
    connect(m_y, SIGNAL(valueChanged(double)), this, SLOT(updatePosition()));
    connect(m_z, SIGNAL(valueChanged(double)), this, SLOT(updatePosition()));
}

void SceneLightWidget::retranslate()
{
    m_colorGroup->setTitle(tr("Color"));
    m_directionGroup->setTitle(tr("Direction"));
    m_openColorDialog->setText(tr("Open color dialog"));
}

void SceneLightWidget::updateColor()
{
    const vpvl::Color color(m_r->value(), m_g->value(), m_b->value(), 1.0);
    emit lightColorDidSet(color);
}

void SceneLightWidget::updatePosition()
{
    const vpvl::Vector3 position(m_x->value(), m_y->value(), m_z->value());
    emit lightPositionDidSet(position);
}

void SceneLightWidget::openColorDialog()
{
    QColorDialog dialog;
    vpvl::Color colorToRestore(m_r->value(), m_g->value(), m_b->value(), 1.0);
    connect(&dialog, SIGNAL(currentColorChanged(QColor)), SLOT(setColor(QColor)));
    if (dialog.exec() == QColorDialog::Rejected) {
        setColor(colorToRestore);
        updateColor();
    }
}

void SceneLightWidget::setColor(const QColor &value)
{
    setColor(vpvl::Color(value.redF(), value.blueF(), value.greenF(), 1.0));
    updateColor();
}

QDoubleSpinBox *SceneLightWidget::createSpinBox(const char *slot, double min, double max, double step, int prec) const
{
    QDoubleSpinBox *spinBox = new QDoubleSpinBox();
    spinBox->setRange(min, max);
    spinBox->setSingleStep(step);
    spinBox->setDecimals(prec);
    connect(spinBox, SIGNAL(valueChanged(double)), slot);
    return spinBox;
}
