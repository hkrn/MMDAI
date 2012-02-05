#include "common/SceneWidget.h"
#include "dialogs/PlaySettingDialog.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

PlaySettingDialog::PlaySettingDialog(MainWindow * /* parent */, QSettings *settings, SceneWidget *scene)
    : QDialog(),
      m_settings(settings)
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    int maxFrameIndex = scene->scene()->maxFrameIndex();
    m_fromIndexBox = new QSpinBox();
    m_fromIndexBox->setRange(0, maxFrameIndex);
    int fromIndex = settings->value("playSettingDialog/fromIndex", 0).toInt();
    m_fromIndexBox->setValue(qBound(0, fromIndex, maxFrameIndex));
    m_toIndexBox = new QSpinBox();
    m_toIndexBox->setRange(0, maxFrameIndex);
    m_toIndexBox->setValue(maxFrameIndex);
    m_sceneFPSBox = new QComboBox();
    m_sceneFPSBox->addItem("30", 30);
    m_sceneFPSBox->addItem("60", 60);
    m_sceneFPSBox->addItem("120", 120);
    int sceneFPSIndex = settings->value("playSettingDialog/sceneFPSIndex", 1).toInt();
    m_sceneFPSBox->setCurrentIndex(sceneFPSIndex);
    m_loopBox = new QCheckBox(tr("Loop"));
    m_loopBox->setChecked(settings->value("playSettingDialog/loop", false).toBool());
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
    m_settings->setValue("playSettingDialog/fromIndex", m_fromIndexBox->value());
    m_settings->setValue("playSettingDialog/toIndex", m_toIndexBox->value());
    m_settings->setValue("playSettingDialog/sceneFPSIndex", m_sceneFPSBox->currentIndex());
    m_settings->setValue("playSettingDialog/loop", m_loopBox->isChecked());
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
