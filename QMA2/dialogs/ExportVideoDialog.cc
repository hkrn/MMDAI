#include "common/SceneWidget.h"
#include "common/SceneLoader.h"
#include "dialogs/ExportVideoDialog.h"
#include "MainWindow.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

ExportVideoDialog::ExportVideoDialog(MainWindow *parent, SceneWidget *sceneWidget)
    : QDialog(parent),
      m_sceneWidget(sceneWidget)
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    int maxFrameIndex = sceneWidget->scene()->maxFrameIndex();
    m_widthBox = new QSpinBox();
    m_widthBox->setRange(sceneWidget->minimumWidth(), sceneWidget->maximumWidth());
    m_heightBox = new QSpinBox();
    m_heightBox->setRange(sceneWidget->minimumHeight(), sceneWidget->maximumHeight());
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
    SceneLoader *loader = m_sceneWidget->sceneLoader();
    loader->setSceneWidth(sceneWidth());
    loader->setSceneHeight(sceneHeight());
    loader->setFrameIndexEncodeVideoFrom(fromIndex());
    loader->setFrameIndexEncodeVideoTo(toIndex());
    loader->setSceneFPSForEncodeVideo(sceneFPS());
    loader->setGridIncluded(includesGrid());
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

void ExportVideoDialog::showEvent(QShowEvent * /* event */)
{
    SceneLoader *loader = m_sceneWidget->sceneLoader();
    int maxFrameIndex = m_sceneWidget->scene()->maxFrameIndex();
    m_widthBox->setValue(loader->sceneWidth());
    m_heightBox->setValue(loader->sceneHeight());
    m_fromIndexBox->setValue(qBound(0, loader->frameIndexEncodeVideoFrom(), maxFrameIndex));
    m_toIndexBox->setValue(maxFrameIndex);
    switch (loader->sceneFPSForEncodeVideo()) {
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
    m_includeGridBox->setChecked(loader->isGridIncluded());
}
