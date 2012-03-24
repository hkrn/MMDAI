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
#include <vpvl/vpvl.h>
#include <vpvl/gl2/Renderer.h>

ExportVideoDialog::ExportVideoDialog(SceneLoader *loader,
                                     const QSize &min,
                                     const QSize &max,
                                     QSettings *settings,
                                     MainWindow *parent)
    : QDialog(),
      m_loader(loader),
      m_settings(settings)
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    QHBoxLayout *subLayout = new QHBoxLayout();
    m_pathEdit = new QLineEdit();
    m_openFileButton = new QPushButton(tr("Open"));
    connect(m_openFileButton, SIGNAL(clicked()), SLOT(openFileDialog()));
    subLayout->addWidget(m_pathEdit);
    subLayout->addWidget(m_openFileButton);
    mainLayout->addLayout(subLayout);
    int maxFrameIndex = loader->renderEngine()->scene()->maxFrameIndex();
    m_widthBox = new QSpinBox();
    m_widthBox->setRange(min.width(), max.width());
    m_heightBox = new QSpinBox();
    m_heightBox->setRange(min.height(), max.height());
    m_fromIndexBox = new QSpinBox();
    m_fromIndexBox->setRange(0, maxFrameIndex);
    m_toIndexBox = new QSpinBox();
    m_toIndexBox->setRange(0, maxFrameIndex);
    m_videoBitrateBox = new QSpinBox();
    m_videoBitrateBox->setRange(1, 100000);
    /* 現在の実装は PNG の生形式で出力するため、ビットレート設定は反映されない */
    m_videoBitrateBox->setEnabled(false);
    m_videoBitrateBox->setValue(0);
    m_sceneFPSBox = new QComboBox();
    m_sceneFPSBox->addItem("30", 30);
    m_sceneFPSBox->addItem("60", 60);
    m_sceneFPSBox->addItem("120", 120);
    m_includeGridBox = new QCheckBox(tr("Include grid field"));
    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->addWidget(new QLabel(tr("Width (px): ")), 0, 0);
    gridLayout->addWidget(m_widthBox, 0, 1);
    gridLayout->addWidget(new QLabel(tr("Height (px): ")), 0, 2);
    gridLayout->addWidget(m_heightBox, 0, 3);
    gridLayout->addWidget(new QLabel(tr("Keyframe from: ")), 1, 0);
    gridLayout->addWidget(m_fromIndexBox, 1, 1);
    gridLayout->addWidget(new QLabel(tr("Keyframe to: ")), 1, 2);
    gridLayout->addWidget(m_toIndexBox, 1, 3);
    gridLayout->addWidget(new QLabel(tr("Video Bitrate (kbps): ")), 2, 0);
    gridLayout->addWidget(m_videoBitrateBox, 2, 1);
    gridLayout->addWidget(new QLabel(tr("Scene FPS: ")), 2, 2);
    gridLayout->addWidget(m_sceneFPSBox, 2, 3);
    mainLayout->addLayout(gridLayout);
    mainLayout->addWidget(m_includeGridBox, 0, Qt::AlignCenter);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    connect(this, SIGNAL(settingsDidSave()), parent, SLOT(invokeVideoEncoder()));
    connect(buttons, SIGNAL(accepted()), this, SLOT(saveSettings()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(close()));
    mainLayout->addWidget(buttons);
    setWindowTitle(tr("Exporting video setting"));
    setLayout(mainLayout);
}

ExportVideoDialog::~ExportVideoDialog()
{
}

void ExportVideoDialog::openFileDialog()
{
    const QString &filename = internal::openFileDialog("exportVideoDialog/lastAudioDirectory",
                                                       tr("Open audio file"),
                                                       tr("WAV file (*.wav)"),
                                                       m_settings);
    if (!filename.isEmpty())
        m_pathEdit->setText(filename);
}

void ExportVideoDialog::saveSettings()
{
    m_loader->setBackgroundAudioPath(backgroundAudio());
    m_loader->setSceneWidth(sceneWidth());
    m_loader->setSceneHeight(sceneHeight());
    m_loader->setFrameIndexEncodeVideoFrom(fromIndex());
    m_loader->setFrameIndexEncodeVideoTo(toIndex());
    m_loader->setSceneFPSForEncodeVideo(sceneFPS());
    m_loader->setGridIncluded(includesGrid());
    emit settingsDidSave();
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
    int maxFrameIndex = m_loader->renderEngine()->scene()->maxFrameIndex();
    m_pathEdit->setText(m_loader->backgroundAudio());
    m_widthBox->setValue(m_loader->sceneWidth());
    m_heightBox->setValue(m_loader->sceneHeight());
    m_fromIndexBox->setMaximum(maxFrameIndex);
    m_fromIndexBox->setValue(qBound(0, m_loader->frameIndexEncodeVideoFrom(), maxFrameIndex));
    m_toIndexBox->setMaximum(maxFrameIndex);
    m_toIndexBox->setValue(maxFrameIndex);
    switch (m_loader->sceneFPSForEncodeVideo()) {
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
    m_includeGridBox->setChecked(m_loader->isGridIncluded());
}
