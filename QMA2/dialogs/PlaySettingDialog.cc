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

#include "common/SceneLoader.h"
#include "common/SceneWidget.h"
#include "dialogs/PlaySettingDialog.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

PlaySettingDialog::PlaySettingDialog(MainWindow * /* parent */, SceneWidget *scene)
    : QDialog(),
      m_sceneLoader(scene->sceneLoader())
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    int maxFrameIndex = scene->scene()->maxFrameIndex();
    m_fromIndexBox = new QSpinBox();
    m_fromIndexBox->setRange(0, maxFrameIndex);
    m_toIndexBox = new QSpinBox();
    m_toIndexBox->setRange(0, maxFrameIndex);
    m_sceneFPSBox = new QComboBox();
    m_sceneFPSBox->addItem("30", 30);
    m_sceneFPSBox->addItem("60", 60);
    m_sceneFPSBox->addItem("120", 120);
    m_loopBox = new QCheckBox(tr("Loop"));
    QFormLayout *formLayout = new QFormLayout();
    formLayout->addRow(tr("Keyframe from: "), m_fromIndexBox);
    formLayout->addRow(tr("Keyframe to: "), m_toIndexBox);
    formLayout->addRow(tr("Scene FPS: "), m_sceneFPSBox);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(m_loopBox, 0, Qt::AlignCenter);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    connect(this, SIGNAL(settingsDidSave()), this, SLOT(close()));
    connect(buttons, SIGNAL(accepted()), this, SLOT(saveSettings()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(close()));
    mainLayout->addWidget(buttons);
    setWindowTitle(tr("Playing scene setting"));
    setLayout(mainLayout);
}

PlaySettingDialog::~PlaySettingDialog()
{
}

void PlaySettingDialog::saveSettings()
{
    m_sceneLoader->setFrameIndexPlayFrom(fromIndex());
    m_sceneLoader->setFrameIndexPlayTo(toIndex());
    m_sceneLoader->setSceneFPSForPlay(sceneFPS());
    m_sceneLoader->setLoop(isLoop());
    emit settingsDidSave();
}

int PlaySettingDialog::fromIndex() const
{
    return m_fromIndexBox->value();
}

int PlaySettingDialog::toIndex() const
{
    return m_toIndexBox->value();
}

int PlaySettingDialog::sceneFPS() const
{
    return m_sceneFPSBox->itemData(m_sceneFPSBox->currentIndex()).toInt();
}

bool PlaySettingDialog::isLoop() const
{
    return m_loopBox->isChecked();
}

void PlaySettingDialog::showEvent(QShowEvent * /* event */)
{
    SceneLoader *loader = m_sceneLoader;
    m_fromIndexBox->setValue(loader->frameIndexPlayFrom());
    m_toIndexBox->setValue(loader->frameIndexPlayTo());
    switch (m_sceneLoader->sceneFPSForPlay()) {
    case 120:
        m_sceneFPSBox->setCurrentIndex(2);
        break;
    case 60:
    default:
        m_sceneFPSBox->setCurrentIndex(1);
        break;
    case 30:
        m_sceneFPSBox->setCurrentIndex(0);
        break;
    }
    m_loopBox->setChecked(loader->isLoop());
}
