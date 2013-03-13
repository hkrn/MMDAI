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

#include "SceneLightWidget.h"

#include <QtGui/QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtWidgets/QtWidgets>
#endif

namespace vpvm
{

using namespace vpvl2;

SceneLightWidget::SceneLightWidget(QWidget *parent) :
    QWidget(parent),
    m_colorGroup(new QGroupBox()),
    m_directionGroup(new QGroupBox()),
    m_r(createSpinBox(SLOT(updateColor()))),
    m_g(createSpinBox(SLOT(updateColor()))),
    m_b(createSpinBox(SLOT(updateColor()))),
    m_x(createDoubleSpinBox(SLOT(updatePosition()))),
    m_y(createDoubleSpinBox(SLOT(updatePosition()))),
    m_z(createDoubleSpinBox(SLOT(updatePosition()))),
    m_openColorDialog(new QPushButton())
{
    QScopedPointer<QHBoxLayout> subLayout(new QHBoxLayout());
    /* 色(R,G,B) */
    QScopedPointer<QFormLayout> formLayout(new QFormLayout());
    formLayout->addRow("R", m_r.data());
    formLayout->addRow("G", m_g.data());
    formLayout->addRow("B", m_b.data());
    m_colorGroup->setLayout(formLayout.take());
    subLayout->addWidget(m_colorGroup.data());
    /* 位置(X,Y,Z) */
    formLayout.reset(new QFormLayout());
    formLayout->addRow("X", m_x.data());
    formLayout->addRow("Y", m_y.data());
    formLayout->addRow("Z", m_z.data());
    m_directionGroup->setLayout(formLayout.take());
    subLayout->addWidget(m_directionGroup.data());

    QScopedPointer<QVBoxLayout> mainLayout(new QVBoxLayout());
    mainLayout->addLayout(subLayout.take());
    connect(m_openColorDialog.data(), SIGNAL(clicked()), SLOT(openColorDialog()));
    mainLayout->addWidget(m_openColorDialog.data(), 0, Qt::AlignCenter);
    mainLayout->addStretch();
    setLayout(mainLayout.take());
    retranslate();
}

SceneLightWidget::~SceneLightWidget()
{
}

void SceneLightWidget::setColor(const Vector3 &value)
{
    disconnect(m_r.data(), SIGNAL(valueChanged(int)), this, SLOT(updateColor()));
    disconnect(m_g.data(), SIGNAL(valueChanged(int)), this, SLOT(updateColor()));
    disconnect(m_b.data(), SIGNAL(valueChanged(int)), this, SLOT(updateColor()));
    m_r->setValue(value.x() * m_r->maximum());
    m_g->setValue(value.y() * m_g->maximum());
    m_b->setValue(value.z() * m_b->maximum());
    connect(m_r.data(), SIGNAL(valueChanged(int)), this, SLOT(updateColor()));
    connect(m_g.data(), SIGNAL(valueChanged(int)), this, SLOT(updateColor()));
    connect(m_b.data(), SIGNAL(valueChanged(int)), this, SLOT(updateColor()));
}

void SceneLightWidget::setDirection(const Vector3 &value)
{
    disconnect(m_x.data(), SIGNAL(valueChanged(double)), this, SLOT(updatePosition()));
    disconnect(m_y.data(), SIGNAL(valueChanged(double)), this, SLOT(updatePosition()));
    disconnect(m_z.data(), SIGNAL(valueChanged(double)), this, SLOT(updatePosition()));
    m_x->setValue(value.x());
    m_y->setValue(value.y());
    m_z->setValue(value.z());
    connect(m_x.data(), SIGNAL(valueChanged(double)), this, SLOT(updatePosition()));
    connect(m_y.data(), SIGNAL(valueChanged(double)), this, SLOT(updatePosition()));
    connect(m_z.data(), SIGNAL(valueChanged(double)), this, SLOT(updatePosition()));
}

void SceneLightWidget::retranslate()
{
    m_colorGroup->setTitle(tr("Color"));
    m_directionGroup->setTitle(tr("Direction"));
    m_openColorDialog->setText(tr("Open Color Dialog"));
}

void SceneLightWidget::updateColor()
{
    Color color;
    color.setX(m_r->value() / float(m_r->maximum()));
    color.setY(m_g->value() / float(m_g->maximum()));
    color.setZ(m_b->value() / float(m_b->maximum()));
    color.setW(1.0);
    emit lightColorDidSet(color);
}

void SceneLightWidget::updatePosition()
{
    const Vector3 position(m_x->value(), m_y->value(), m_z->value());
    emit lightDirectionDidSet(position);
}

void SceneLightWidget::openColorDialog()
{
    const QColor colorToRestore(m_r->value(), m_g->value(), m_b->value());
    QColorDialog dialog(colorToRestore);
    connect(&dialog, SIGNAL(currentColorChanged(QColor)), SLOT(setQColor(QColor)));
    if (dialog.exec() == QColorDialog::Rejected)
        setQColor(colorToRestore);
}

void SceneLightWidget::setQColor(const QColor &value)
{
    setColor(Color(value.redF(), value.greenF(), value.blueF(), 1.0));
    updateColor();
}

QSpinBox *SceneLightWidget::createSpinBox(const char *slot) const
{
    QScopedPointer<QSpinBox> spinBox(new QSpinBox());
    spinBox->setAlignment(Qt::AlignRight);
    spinBox->setRange(0, 255);
    spinBox->setSingleStep(1);
    connect(spinBox.data(), SIGNAL(valueChanged(int)), slot);
    return spinBox.take();
}

QDoubleSpinBox *SceneLightWidget::createDoubleSpinBox(const char *slot) const
{
    QScopedPointer<QDoubleSpinBox> spinBox(new QDoubleSpinBox());
    spinBox->setAlignment(Qt::AlignRight);
    spinBox->setRange(-1.0, 1.0);
    spinBox->setSingleStep(0.01);
    spinBox->setDecimals(3);
    connect(spinBox.data(), SIGNAL(valueChanged(double)), slot);
    return spinBox.take();
}

} /* namespace vpvm */
