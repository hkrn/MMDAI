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
#include "common/util.h"
#include "dialogs/PlaySettingDialog.h"

#include <QtGui/QtGui>
#include <vpvl2/vpvl2.h>

namespace vpvm
{

/* lupdate cannot parse tr() syntax correctly */

PlaySettingDialog::PlaySettingDialog(SceneLoader *loader, QSettings *settings, QWidget *parent)
    : QDialog(parent),
      m_loader(loader),
      m_settings(settings)
{
    int maxFrameIndex = m_loader->sceneRef()->maxFrameIndex();
    m_pathEdit = new QLineEdit();
    m_openFileButton = new QPushButton();
    connect(m_openFileButton, SIGNAL(clicked()), SLOT(openFileDialog()));
    m_fromIndexLabel = new QLabel();
    m_fromIndexBox = new QSpinBox();
    m_fromIndexBox->setRange(0, maxFrameIndex);
    m_toIndexLabel = new QLabel();
    m_toIndexBox = new QSpinBox();
    m_toIndexBox->setRange(0, maxFrameIndex);
    m_sceneFPSLabel = new QLabel();
    m_sceneFPSBox = new QComboBox();
    m_sceneFPSBox->addItem("30", 30);
    m_sceneFPSBox->addItem("60", 60);
    m_sceneFPSBox->addItem("120", 120);
    m_loopBox = new QCheckBox();
    m_selectModelBox = new QCheckBox();
    m_boneWireFramesBox = new QCheckBox();
    QVBoxLayout *mainLayout = new QVBoxLayout();
    QLayout *subLayout = new QHBoxLayout();
    subLayout->addWidget(m_pathEdit);
    subLayout->addWidget(m_openFileButton);
    mainLayout->addLayout(subLayout);
    subLayout = new QHBoxLayout();
    subLayout->addWidget(m_fromIndexLabel);
    subLayout->addWidget(m_fromIndexBox);
    subLayout->addWidget(m_toIndexLabel);
    subLayout->addWidget(m_toIndexBox);
    mainLayout->addLayout(subLayout);
    subLayout = new QHBoxLayout();
    subLayout->setAlignment(Qt::AlignCenter);
    subLayout->addWidget(m_sceneFPSLabel);
    subLayout->addWidget(m_sceneFPSBox);
    mainLayout->addLayout(subLayout);
    subLayout = new QVBoxLayout();
    subLayout->setAlignment(Qt::AlignCenter);
    subLayout->addWidget(m_loopBox);
    subLayout->addWidget(m_selectModelBox);
    subLayout->addWidget(m_boneWireFramesBox);
    mainLayout->addLayout(subLayout);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    m_playButton = new QPushButton(vpvm::PlaySettingDialog::tr("Play"));
    connect(m_playButton, SIGNAL(clicked()), SIGNAL(playingDidStart()));
    buttons->addButton(m_playButton, QDialogButtonBox::ActionRole);
    connect(this, SIGNAL(settingsDidSave()), this, SLOT(close()));
    connect(buttons, SIGNAL(accepted()), this, SLOT(saveSettings()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(close()));
    mainLayout->addWidget(buttons);
    retranslate();
    setLayout(mainLayout);
    /* 開いてから設定を行う関係で、ダイアログを開かずにそのまま再生して即終了を防ぐため、空のイベントを発行する */
    showEvent(0);
}

PlaySettingDialog::~PlaySettingDialog()
{
}

void PlaySettingDialog::openFileDialog()
{
    const QString &filename = vpvm::openFileDialog("playSettingDialog/lastAudioDirectory",
                                                   vpvm::PlaySettingDialog::tr("Open audio file"),
                                                   vpvm::PlaySettingDialog::tr("WAV file (*.wav)"),
                                                   m_settings);
    if (!filename.isEmpty())
        m_pathEdit->setText(filename);
}

void PlaySettingDialog::saveSettings()
{
    m_loader->setBackgroundAudioPath(backgroundAudio());
    m_loader->setFrameIndexPlayFrom(fromIndex());
    m_loader->setFrameIndexPlayTo(toIndex());
    m_loader->setSceneFPSForPlay(sceneFPS());
    m_loader->setLoop(isLoopEnabled());
    emit settingsDidSave();
}

const QString PlaySettingDialog::backgroundAudio() const
{
    return m_pathEdit->text();
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

bool PlaySettingDialog::isLoopEnabled() const
{
    return m_loopBox->isChecked();
}

bool PlaySettingDialog::isModelSelected() const
{
    return m_selectModelBox->isChecked();
}

bool PlaySettingDialog::isBoneWireframesVisible() const
{
    return m_boneWireFramesBox->isChecked();
}

void PlaySettingDialog::showEvent(QShowEvent * /* event */)
{
    int maxIndex = m_loader->sceneRef()->maxFrameIndex();
    m_pathEdit->setText(m_loader->backgroundAudio());
    m_fromIndexBox->setMaximum(maxIndex);
    m_fromIndexBox->setValue(m_loader->frameIndexPlayFrom());
    m_toIndexBox->setMaximum(maxIndex);
    m_toIndexBox->setValue(m_loader->frameIndexPlayTo());
    switch (m_loader->sceneFPSForPlay()) {
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
    m_loopBox->setChecked(m_loader->isLoop());
    /* 現時点でシークの実装が無いので、開始位置指定とループは無効にする */
    const QString &audio = m_loader->backgroundAudio();
    if (!audio.isEmpty()) {
        m_fromIndexBox->setValue(0);
        m_fromIndexBox->setDisabled(true);
        m_loopBox->setChecked(false);
        m_loopBox->setDisabled(true);
        m_selectModelBox->setChecked(false);
        m_selectModelBox->setDisabled(true);
    }
}

void PlaySettingDialog::retranslate()
{
    m_openFileButton->setText(vpvm::PlaySettingDialog::tr("Open"));
    m_fromIndexLabel->setText(vpvm::PlaySettingDialog::tr("Keyframe from: "));
    m_toIndexLabel->setText(vpvm::PlaySettingDialog::tr("Keyframe to: "));
    m_sceneFPSLabel->setText(vpvm::PlaySettingDialog::tr("Scene FPS: "));
    m_loopBox->setText(vpvm::PlaySettingDialog::tr("Loop"));
    m_selectModelBox->setText(vpvm::PlaySettingDialog::tr("Be model selected"));
    m_boneWireFramesBox->setText(vpvm::PlaySettingDialog::tr("Draw bone wireframes"));
    setWindowTitle(vpvm::PlaySettingDialog::tr("Playing scene setting"));
}

} /* namespace vpvm */
