/**

 Copyright (c) 2010-2013  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#include "FrameSelectionDialog.h"
#include "SceneWidget.h"

#include <vpvl2/vpvl2.h>

#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtWidgets>
#endif

namespace vpvm
{

FrameSelectionDialog::FrameSelectionDialog(QWidget *parent)
    : QDialog(parent),
      m_fromIndexBox(new QSpinBox()),
      m_toIndexBox(new QSpinBox())
{
    QScopedPointer<QVBoxLayout> mainLayout(new QVBoxLayout());
    QScopedPointer<QFormLayout> formLayout(new QFormLayout());
    m_fromIndexBox->setAlignment(Qt::AlignRight);
    m_toIndexBox->setAlignment(Qt::AlignRight);
    formLayout->addRow(tr("Keyframe From"), m_fromIndexBox.data());
    formLayout->addRow(tr("Keyframe To"), m_toIndexBox.data());
    mainLayout->addLayout(formLayout.take());
    QScopedPointer<QDialogButtonBox> buttons(new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel));
    connect(buttons.data(), SIGNAL(accepted()), SLOT(emitFrameIndices()));
    connect(buttons.data(), SIGNAL(rejected()), SLOT(close()));
    mainLayout->addWidget(buttons.take());
    connect(this, SIGNAL(frameIndicesDidSelect(int,int)), SLOT(close()));
    setWindowTitle(tr("Keyframe Range Selection Setting"));
    setLayout(mainLayout.take());
}

FrameSelectionDialog::~FrameSelectionDialog()
{
}

void FrameSelectionDialog::setMaxTimeIndex(int value)
{
    m_fromIndexBox->setRange(0, value);
    m_toIndexBox->setRange(0, value);
    m_toIndexBox->setValue(value);
}

void FrameSelectionDialog::emitFrameIndices()
{
    emit frameIndicesDidSelect(m_fromIndexBox->value(), m_toIndexBox->value());
}

} /* namespace vpvm */
