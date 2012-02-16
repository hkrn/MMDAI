#include "common/SceneWidget.h"
#include "common/SceneLoader.h"
#include "dialogs/ExportVideoDialog.h"
#include "MainWindow.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

ExportVideoDialog::ExportVideoDialog(MainWindow *parent, SceneWidget *scene)
    : QDialog(parent),
      m_sceneLoader(scene->sceneLoader())
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    int maxFrameIndex = scene->scene()->maxFrameIndex();
    m_widthBox = new QSpinBox();
    m_widthBox->setRange(scene->minimumWidth(), scene->maximumWidth());
    m_widthBox->setValue(m_sceneLoader->sceneWidth());
    m_heightBox = new QSpinBox();
    m_heightBox->setRange(scene->minimumHeight(), scene->maximumHeight());
    m_heightBox->setValue(m_sceneLoader->sceneHeight());
    m_fromIndexBox = new QSpinBox();
    m_fromIndexBox->setRange(0, maxFrameIndex);
    m_fromIndexBox->setValue(qBound(0, m_sceneLoader->frameIndexEncodeVideoFrom(), maxFrameIndex));
    m_toIndexBox = new QSpinBox();
    m_toIndexBox->setRange(0, maxFrameIndex);
    m_toIndexBox->setValue(maxFrameIndex);
    m_videoBitrateBox = new QSpinBox();
    m_videoBitrateBox->setRange(1, 100000);
    /* 現在の実装は PNG の生形式で出力するため、ビットレート設定は反映されない */
    m_videoBitrateBox->setEnabled(false);
    m_videoBitrateBox->setValue(0);
    m_sceneFPSBox = new QComboBox();
    m_sceneFPSBox->addItem("30", 30);
    m_sceneFPSBox->addItem("60", 60);
    m_sceneFPSBox->addItem("120", 120);
    switch (m_sceneLoader->sceneFPSForEncodeVideo()) {
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
    m_includeGridBox = new QCheckBox(tr("Include grid field"));
    m_includeGridBox->setChecked(m_sceneLoader->isGridIncluded());
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
    connect(this, SIGNAL(settingsDidSave()), parent, SLOT(startExportingVideo()));
    connect(buttons, SIGNAL(accepted()), this, SLOT(saveSettings()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(close()));
    mainLayout->addWidget(buttons);
    setWindowTitle(tr("Exporting video setting"));
    setLayout(mainLayout);
}

ExportVideoDialog::~ExportVideoDialog()
{
}

void ExportVideoDialog::saveSettings()
{
    m_sceneLoader->setSceneWidth(sceneWidth());
    m_sceneLoader->setSceneHeight(sceneHeight());
    m_sceneLoader->setFrameIndexEncodeVideoFrom(fromIndex());
    m_sceneLoader->setFrameIndexEncodeVideoTo(toIndex());
    m_sceneLoader->setSceneFPSForEncodeVideo(sceneFPS());
    m_sceneLoader->setGridIncluded(includesGrid());
    emit settingsDidSave();
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
