#include "ExportVideoDialog.h"
#include "MainWindow.h"
#include "common/SceneWidget.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

ExportVideoDialog::ExportVideoDialog(MainWindow *parent, QSettings *settings, SceneWidget *scene)
    : QDialog(parent),
      m_settings(settings)
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    int maxFrameIndex = scene->scene()->maxFrameIndex();
    m_widthBox = new QSpinBox();
    m_widthBox->setRange(scene->minimumWidth(), scene->maximumWidth());
    int width = settings->value("exportVideoDialog/width", scene->minimumWidth()).toInt();
    m_widthBox->setValue(width);
    m_heightBox = new QSpinBox();
    m_heightBox->setRange(scene->minimumHeight(), scene->maximumHeight());
    int height = settings->value("exportVideoDialog/height", scene->minimumHeight()).toInt();
    m_heightBox->setValue(height);
    m_fromIndexBox = new QSpinBox();
    m_fromIndexBox->setRange(0, maxFrameIndex);
    int fromIndex = settings->value("exportVideoDialog/fromIndex", 0).toInt();
    m_fromIndexBox->setValue(qBound(0, fromIndex, maxFrameIndex));
    m_toIndexBox = new QSpinBox();
    m_toIndexBox->setRange(0, maxFrameIndex);
    int toIndex = settings->value("exportVideoDialog/toIndex", maxFrameIndex).toInt();
    m_toIndexBox->setValue(qBound(0, toIndex, maxFrameIndex));
    m_includeGridBox = new QCheckBox(tr("Include grid field"));
    m_includeGridBox->setChecked(settings->value("exportVideoDialog/includeGridBox", false).toBool());
    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->addWidget(new QLabel(tr("Width")), 0, 0);
    gridLayout->addWidget(m_widthBox, 0, 1);
    gridLayout->addWidget(new QLabel(tr("Height")), 0, 2);
    gridLayout->addWidget(m_heightBox, 0, 3);
    gridLayout->addWidget(new QLabel(tr("Keyframe from")), 1, 0);
    gridLayout->addWidget(m_fromIndexBox, 1, 1);
    gridLayout->addWidget(new QLabel(tr("Keyframe to")), 1, 2);
    gridLayout->addWidget(m_toIndexBox, 1, 3);
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
    m_settings->setValue("exportVideoDialog/width", m_widthBox->value());
    m_settings->setValue("exportVideoDialog/height", m_heightBox->value());
    m_settings->setValue("exportVideoDialog/fromIndex", m_fromIndexBox->value());
    m_settings->setValue("exportVideoDialog/toIndex", m_toIndexBox->value());
    m_settings->setValue("exportVideoDialog/includeGridBox", m_includeGridBox->isChecked());
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

bool ExportVideoDialog::includesGrid() const
{
    return m_includeGridBox->isChecked();
}
