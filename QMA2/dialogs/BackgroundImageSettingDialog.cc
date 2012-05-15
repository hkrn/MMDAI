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

#include <QtGui/QtGui>

#include "common/SceneLoader.h"
#include "BackgroundImageSettingDialog.h"

BackgroundImageSettingDialog::BackgroundImageSettingDialog(SceneLoader *loader, QWidget *parent) :
    QDialog(parent),
    m_position(loader->backgroundImagePosition()),
    m_scaled(loader->isBackgroundImageScaled())
{
    QFormLayout *subLayout = new QFormLayout();
    m_x = new QSpinBox();
    m_x->setRange(-maximumWidth(), maximumWidth());
    m_x->setValue(m_position.x());
    m_x->setDisabled(m_scaled);
    connect(m_x, SIGNAL(valueChanged(int)), SLOT(setPositionX(int)));
    subLayout->addRow("X", m_x);
    m_y = new QSpinBox();
    m_y->setRange(-maximumHeight(), maximumHeight());
    m_y->setValue(m_position.y());
    m_y->setDisabled(m_scaled);
    connect(m_y, SIGNAL(valueChanged(int)), SLOT(setPositionY(int)));
    subLayout->addRow("Y", m_y);
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addLayout(subLayout);
    m_checkbox = new QCheckBox();
    connect(m_checkbox, SIGNAL(clicked(bool)), SIGNAL(scaleDidEnable(bool)));
    connect(m_checkbox, SIGNAL(clicked(bool)), m_x, SLOT(setDisabled(bool)));
    connect(m_checkbox, SIGNAL(clicked(bool)), m_y, SLOT(setDisabled(bool)));
    m_checkbox->setText(tr("Scale background image"));
    m_checkbox->setChecked(m_scaled);
    mainLayout->addWidget(m_checkbox);
    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(box, SIGNAL(accepted()), SLOT(close()));
    connect(box, SIGNAL(rejected()), SLOT(restoreAndClose()));
    mainLayout->addWidget(box);
    mainLayout->addStretch();
    setLayout(mainLayout);
    setWindowTitle(tr("Background image setting"));
}

BackgroundImageSettingDialog::~BackgroundImageSettingDialog()
{
}

void BackgroundImageSettingDialog::setPositionX(int value)
{
    emit positionDidChange(QPoint(value, m_y->value()));
}

void BackgroundImageSettingDialog::setPositionY(int value)
{
    emit positionDidChange(QPoint(m_x->value(), value));
}

void BackgroundImageSettingDialog::restoreAndClose()
{
    emit positionDidChange(m_position);
    emit scaleDidEnable(m_scaled);
    close();
}
