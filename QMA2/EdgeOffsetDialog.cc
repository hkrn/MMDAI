#include "EdgeOffsetDialog.h"
#include "MainWindow.h"
#include "SceneWidget.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

EdgeOffsetDialog::EdgeOffsetDialog(MainWindow *parent, SceneWidget *scene)
    : QDialog(parent),
      m_spinBox(0),
      m_sceneWidget(scene),
      m_selected(scene->selectedModel()),
      m_edgeOffset(scene->modelEdgeOffset())
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    QLabel *label = new QLabel(tr("Model edge offset value"));
    m_spinBox = new QDoubleSpinBox();
    connect(m_spinBox, SIGNAL(valueChanged(double)), this, SLOT(setEdgeOffset(double)));
    m_spinBox->setValue(m_edgeOffset);
    m_spinBox->setSingleStep(0.1f);
    m_spinBox->setRange(0.0f, 2.0f);
    QHBoxLayout *subLayout = new QHBoxLayout();
    subLayout->addWidget(label);
    subLayout->addWidget(m_spinBox);
    mainLayout->addLayout(subLayout);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    connect(buttons, SIGNAL(accepted()), this, SLOT(commit()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(rollback()));
    mainLayout->addWidget(buttons);
    setWindowTitle(tr("Edge offset dialog"));
    setLayout(mainLayout);
    m_sceneWidget->hideSelectedModelEdge();
}

EdgeOffsetDialog::~EdgeOffsetDialog()
{
}

void EdgeOffsetDialog::closeEvent(QCloseEvent * /* event */)
{
    m_sceneWidget->showSelectedModelEdge();
}

void EdgeOffsetDialog::setEdgeOffset(double value)
{
    m_selected->setEdgeOffset(value);
    m_sceneWidget->setModelEdgeOffset(m_edgeOffset);
    m_sceneWidget->updateMotion();
}

void EdgeOffsetDialog::commit()
{
    m_selected->setEdgeOffset(m_spinBox->value());
    close();
}

void EdgeOffsetDialog::rollback()
{
    m_selected->setEdgeOffset(m_edgeOffset);
    close();
}
