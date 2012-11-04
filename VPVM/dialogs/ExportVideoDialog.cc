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

#include "common/SceneWidget.h"
#include "common/SceneLoader.h"
#include "common/util.h"
#include "dialogs/ExportVideoDialog.h"
#include "MainWindow.h"

#include <QtGui/QtGui>
#include <vpvl2/vpvl2.h>

namespace vpvm
{

/* lupdate cannot parse tr() syntax correctly */

ExportVideoDialog::ExportVideoDialog(SceneLoader *loader,
                                     const QSize &min,
                                     const QSize &max,
                                     QSettings *settings)
    : QDialog(),
      m_loaderRef(loader),
      m_settingsRef(settings),
      m_pathEdit(new QLineEdit()),
      m_openFileButton(new QPushButton(vpvm::ExportVideoDialog::tr("Open"))),
      m_widthBox(new QSpinBox()),
      m_heightBox(new QSpinBox()),
      m_fromIndexBox(new QSpinBox()),
      m_toIndexBox(new QSpinBox()),
      m_videoBitrateBox(new QSpinBox()),
      m_sceneFPSBox(new QComboBox()),
      m_includeGridBox(new QCheckBox(vpvm::ExportVideoDialog::tr("Include grid field")))
{
    QScopedPointer<QVBoxLayout> mainLayout(new QVBoxLayout());
    QScopedPointer<QHBoxLayout> subLayout(new QHBoxLayout());
    connect(m_openFileButton.data(), SIGNAL(clicked()), SLOT(openFileDialog()));
    subLayout->addWidget(m_pathEdit.data());
    subLayout->addWidget(m_openFileButton.data());
    mainLayout->addLayout(subLayout.take());
    int maxFrameIndex = loader->sceneRef()->maxFrameIndex();
    m_widthBox->setRange(min.width(), max.width());
    m_heightBox->setRange(min.height(), max.height());
    m_fromIndexBox->setRange(0, maxFrameIndex);
    m_toIndexBox->setRange(0, maxFrameIndex);
    m_videoBitrateBox->setRange(1, 100000);
    /* 現在の実装は PNG の生形式で出力するため、ビットレート設定は反映されない */
    m_videoBitrateBox->setEnabled(false);
    m_videoBitrateBox->setValue(0);
    m_sceneFPSBox->addItem("30", 30);
    m_sceneFPSBox->addItem("60", 60);
    m_sceneFPSBox->addItem("120", 120);
    QScopedPointer<QGridLayout> gridLayout(new QGridLayout());
    gridLayout->addWidget(new QLabel(vpvm::ExportVideoDialog::tr("Width (px): ")), 0, 0);
    gridLayout->addWidget(m_widthBox.data(), 0, 1);
    gridLayout->addWidget(new QLabel(vpvm::ExportVideoDialog::tr("Height (px): ")), 0, 2);
    gridLayout->addWidget(m_heightBox.data(), 0, 3);
    gridLayout->addWidget(new QLabel(vpvm::ExportVideoDialog::tr("Keyframe from: ")), 1, 0);
    gridLayout->addWidget(m_fromIndexBox.data(), 1, 1);
    gridLayout->addWidget(new QLabel(vpvm::ExportVideoDialog::tr("Keyframe to: ")), 1, 2);
    gridLayout->addWidget(m_toIndexBox.data(), 1, 3);
    gridLayout->addWidget(new QLabel(vpvm::ExportVideoDialog::tr("Video Bitrate (kbps): ")), 2, 0);
    gridLayout->addWidget(m_videoBitrateBox.data(), 2, 1);
    gridLayout->addWidget(new QLabel(vpvm::ExportVideoDialog::tr("Scene FPS: ")), 2, 2);
    gridLayout->addWidget(m_sceneFPSBox.data(), 2, 3);
    mainLayout->addLayout(gridLayout.take());
    mainLayout->addWidget(m_includeGridBox.data(), 0, Qt::AlignCenter);
    QScopedPointer<QDialogButtonBox> buttons(new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel));
    connect(buttons.data(), SIGNAL(accepted()), SLOT(saveSettings()));
    connect(buttons.data(), SIGNAL(rejected()), SLOT(close()));
    mainLayout->addWidget(buttons.take());
    setWindowTitle(vpvm::ExportVideoDialog::tr("Exporting video setting"));
    setLayout(mainLayout.take());
}

ExportVideoDialog::~ExportVideoDialog()
{
}

void ExportVideoDialog::openFileDialog()
{
    const QString &filename = vpvm::openFileDialog("exportVideoDialog/lastAudioDirectory",
                                                   vpvm::ExportVideoDialog::tr("Open audio file"),
                                                   vpvm::ExportVideoDialog::tr("WAV file (*.wav)"),
                                                   m_settingsRef);
    if (!filename.isEmpty())
        m_pathEdit->setText(filename);
}

void ExportVideoDialog::saveSettings()
{
    m_loaderRef->setBackgroundAudioPath(backgroundAudio());
    m_loaderRef->setSceneWidth(sceneWidth());
    m_loaderRef->setSceneHeight(sceneHeight());
    m_loaderRef->setFrameIndexEncodeVideoFrom(fromIndex());
    m_loaderRef->setFrameIndexEncodeVideoTo(toIndex());
    m_loaderRef->setSceneFPSForEncodeVideo(sceneFPS());
    m_loaderRef->setGridIncluded(includesGrid());
    emit settingsDidSave();
}

void ExportVideoDialog::setImageConfiguration(bool value)
{
    m_pathEdit->setEnabled(!value);
    m_fromIndexBox->setEnabled(!value);
    m_toIndexBox->setEnabled(!value);
    m_sceneFPSBox->setEnabled(!value);
}

const QString ExportVideoDialog::backgroundAudio() const
{
    return m_pathEdit->text();
}

int ExportVideoDialog::sceneWidth() const
{
    return m_widthBox->value();
}

int ExportVideoDialog::sceneHeight() const
{
    return m_heightBox->value();
}

int ExportVideoDialog::fromIndex() const
{
    return m_fromIndexBox->value();
}

int ExportVideoDialog::toIndex() const
{
    return m_toIndexBox->value();
}

int ExportVideoDialog::videoBitrate() const
{
    return m_videoBitrateBox->value() * 1000;
}

int ExportVideoDialog::sceneFPS() const
{
    return m_sceneFPSBox->itemData(m_sceneFPSBox->currentIndex()).toInt();
}

bool ExportVideoDialog::includesGrid() const
{
    return m_includeGridBox->isChecked();
}

void ExportVideoDialog::showEvent(QShowEvent * /* event */)
{
    int maxFrameIndex = m_loaderRef->sceneRef()->maxFrameIndex();
    m_pathEdit->setText(m_loaderRef->backgroundAudio());
    m_widthBox->setValue(m_loaderRef->sceneWidth());
    m_heightBox->setValue(m_loaderRef->sceneHeight());
    m_fromIndexBox->setMaximum(maxFrameIndex);
    m_fromIndexBox->setValue(qBound(0, m_loaderRef->frameIndexEncodeVideoFrom(), maxFrameIndex));
    m_toIndexBox->setMaximum(maxFrameIndex);
    m_toIndexBox->setValue(maxFrameIndex);
    switch (m_loaderRef->sceneFPSForEncodeVideo()) {
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
    m_includeGridBox->setChecked(m_loaderRef->isGridIncluded());
}

} /* namespace vpvm */
