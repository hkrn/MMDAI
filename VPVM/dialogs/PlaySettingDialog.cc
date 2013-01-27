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

#include "common/SceneLoader.h"
#include "common/SceneWidget.h"
#include "dialogs/PlaySettingDialog.h"

#include <QtGui/QtGui>
#include <vpvl2/vpvl2.h>
#include <vpvl2/qt/Util.h>

namespace vpvm
{

using namespace vpvl2::qt;

/* lupdate cannot parse tr() syntax correctly */

PlaySettingDialog::PlaySettingDialog(SceneLoader *loader, QSettings *settings, QWidget *parent)
    : QDialog(parent),
      m_loaderRef(loader),
      m_settingsRef(settings),
      m_audioGroup(new QGroupBox()),
      m_pathEdit(new QLineEdit()),
      m_openFileButton(new QPushButton()),
      m_timeIndexGroup(new QGroupBox()),
      m_fromIndexLabel(new QLabel()),
      m_toIndexLabel(new QLabel()),
      m_sceneFPSLabel(new QLabel()),
      m_fromIndexBox(new QSpinBox()),
      m_toIndexBox(new QSpinBox()),
      m_sceneFPSBox(new QComboBox()),
      m_toggleSettingGroup(new QGroupBox()),
      m_loopBox(new QCheckBox()),
      m_selectModelBox(new QCheckBox()),
      m_boneWireFramesBox(new QCheckBox()),
      m_playButton(new QPushButton())
{
    int maxTimeIndex = m_loaderRef->sceneRef()->maxTimeIndex();
    connect(m_openFileButton.data(), SIGNAL(clicked()), SLOT(openFileDialog()));
    m_fromIndexBox->setRange(0, maxTimeIndex);
    m_toIndexBox->setRange(0, maxTimeIndex);
    m_sceneFPSBox->addItem("30", 30);
    m_sceneFPSBox->addItem("60", 60);
    m_sceneFPSBox->addItem("120", 120);
    QScopedPointer<QVBoxLayout> mainLayout(new QVBoxLayout());
    QScopedPointer<QLayout> subLayout(new QHBoxLayout());
    subLayout->addWidget(m_pathEdit.data());
    subLayout->addWidget(m_openFileButton.data());
    m_audioGroup->setLayout(subLayout.take());
    mainLayout->addWidget(m_audioGroup.data());
    QScopedPointer<QVBoxLayout> timeIndexLayout(new QVBoxLayout());
    subLayout.reset(new QHBoxLayout());
    subLayout->addWidget(m_fromIndexLabel.data());
    subLayout->addWidget(m_fromIndexBox.data());
    subLayout->addWidget(m_toIndexLabel.data());
    subLayout->addWidget(m_toIndexBox.data());
    timeIndexLayout->addLayout(subLayout.take());
    subLayout.reset(new QHBoxLayout());
    subLayout->setAlignment(Qt::AlignCenter);
    subLayout->addWidget(m_sceneFPSLabel.data());
    subLayout->addWidget(m_sceneFPSBox.data());
    timeIndexLayout->addLayout(subLayout.take());
    m_timeIndexGroup->setLayout(timeIndexLayout.take());
    mainLayout->addWidget(m_timeIndexGroup.data());
    subLayout.reset(new QVBoxLayout());
    subLayout->setAlignment(Qt::AlignCenter);
    subLayout->addWidget(m_loopBox.data());
    subLayout->addWidget(m_selectModelBox.data());
    subLayout->addWidget(m_boneWireFramesBox.data());
    m_toggleSettingGroup->setLayout(subLayout.take());
    mainLayout->addWidget(m_toggleSettingGroup.data());
    QScopedPointer<QDialogButtonBox> buttons(new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel));
    connect(m_playButton.data(), SIGNAL(clicked()), SIGNAL(playingDidStart()));
    buttons->addButton(m_playButton.data(), QDialogButtonBox::ActionRole);
    connect(this, SIGNAL(settingsDidSave()), this, SLOT(close()));
    connect(buttons.data(), SIGNAL(accepted()), SLOT(saveSettings()));
    connect(buttons.data(), SIGNAL(rejected()), SLOT(close()));
    mainLayout->addWidget(buttons.take());
    retranslate();
    setLayout(mainLayout.take());
    /* 開いてから設定を行う関係で、ダイアログを開かずにそのまま再生して即終了を防ぐため、空のイベントを発行する */
    showEvent(0);
}

PlaySettingDialog::~PlaySettingDialog()
{
}

void PlaySettingDialog::openFileDialog()
{
    const QString &filename = Util::openFileDialog("playSettingDialog/lastAudioDirectory",
                                                   vpvm::PlaySettingDialog::tr("Open audio file"),
                                                   vpvm::PlaySettingDialog::tr("WAV file (*.wav)"),
                                                   m_settingsRef);
    if (!filename.isEmpty())
        m_pathEdit->setText(filename);
}

void PlaySettingDialog::saveSettings()
{
    m_loaderRef->setBackgroundAudioPath(backgroundAudio());
    m_loaderRef->setTimeIndexPlayFrom(fromIndex());
    m_loaderRef->setTimeIndexPlayTo(toIndex());
    m_loaderRef->setSceneFPSForPlay(sceneFPS());
    m_loaderRef->setLoop(isLoopEnabled());
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
    int maxIndex = m_loaderRef->sceneRef()->maxTimeIndex();
    m_pathEdit->setText(m_loaderRef->backgroundAudio());
    m_fromIndexBox->setMaximum(maxIndex);
    m_fromIndexBox->setValue(m_loaderRef->timeIndexPlayFrom());
    m_toIndexBox->setMaximum(maxIndex);
    m_toIndexBox->setValue(m_loaderRef->timeIndexPlayTo());
    switch (m_loaderRef->sceneFPSForPlay()) {
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
    m_loopBox->setChecked(m_loaderRef->isLoop());
    /* 現時点でシークの実装が無いので、開始位置指定とループは無効にする */
    const QString &audio = m_loaderRef->backgroundAudio();
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
    m_fromIndexLabel->setText(vpvm::PlaySettingDialog::tr("Keyframe From: "));
    m_toIndexLabel->setText(vpvm::PlaySettingDialog::tr("Keyframe To: "));
    m_sceneFPSLabel->setText(vpvm::PlaySettingDialog::tr("Scene FPS: "));
    m_loopBox->setText(vpvm::PlaySettingDialog::tr("Loop"));
    m_selectModelBox->setText(vpvm::PlaySettingDialog::tr("Be Model Selected"));
    m_boneWireFramesBox->setText(vpvm::PlaySettingDialog::tr("Draw Bone Wireframes"));
    m_playButton->setText(vpvm::PlaySettingDialog::tr("Play"));
    m_audioGroup->setTitle(vpvm::PlaySettingDialog::tr("Audio File Setting"));
    m_timeIndexGroup->setTitle(vpvm::PlaySettingDialog::tr("Scene Range to Play and FPS Setting"));
    m_toggleSettingGroup->setTitle(vpvm::PlaySettingDialog::tr("Enable/Disable Setting"));
    setWindowTitle(vpvm::PlaySettingDialog::tr("Playing Scene Setting"));
}

} /* namespace vpvm */
