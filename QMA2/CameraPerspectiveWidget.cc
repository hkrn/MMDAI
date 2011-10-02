#include "CameraPerspectiveWidget.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

CameraPerspectiveWidget::CameraPerspectiveWidget(QWidget *parent) :
    QWidget(parent)
{
    QHBoxLayout *hlayout = 0;
    QPushButton *button = 0;
    QVBoxLayout *layout = new QVBoxLayout();
    hlayout = new QHBoxLayout();
    button = new QPushButton(tr("Front"));
    connect(button, SIGNAL(clicked()), this, SLOT(setCameraPerspectiveFront()));
    hlayout->addWidget(button);
    button = new QPushButton(tr("Back"));
    connect(button, SIGNAL(clicked()), this, SLOT(setCameraPerspectiveBack()));
    hlayout->addWidget(button);
    button = new QPushButton(tr("Top"));
    connect(button, SIGNAL(clicked()), this, SLOT(setCameraPerspectiveTop()));
    hlayout->addWidget(button);
    layout->addLayout(hlayout);
    hlayout = new QHBoxLayout();
    button = new QPushButton(tr("Left"));
    connect(button, SIGNAL(clicked()), this, SLOT(setCameraPerspectiveLeft()));
    hlayout->addWidget(button);
    button = new QPushButton(tr("Right"));
    connect(button, SIGNAL(clicked()), this, SLOT(setCameraPerspectiveRight()));
    hlayout->addWidget(button);
    button = new QPushButton(tr("Camera"));
    hlayout->addWidget(button);
    layout->addLayout(hlayout);
    setLayout(layout);
}

void CameraPerspectiveWidget::setCameraPerspectiveFront()
{
    vpvl::Vector3 angle(0, 0, 0);
    emit cameraPerspectiveDidChange(0, &angle, 0, 0);
}

void CameraPerspectiveWidget::setCameraPerspectiveBack()
{
    vpvl::Vector3 angle(0, 180, 0);
    emit cameraPerspectiveDidChange(0, &angle, 0, 0);
}

void CameraPerspectiveWidget::setCameraPerspectiveTop()
{
    vpvl::Vector3 angle(90, 0, 0);
    emit cameraPerspectiveDidChange(0, &angle, 0, 0);
}

void CameraPerspectiveWidget::setCameraPerspectiveLeft()
{
    vpvl::Vector3 angle(0, -90, 0);
    emit cameraPerspectiveDidChange(0, &angle, 0, 0);
}

void CameraPerspectiveWidget::setCameraPerspectiveRight()
{
    vpvl::Vector3 angle(0, 90, 0);
    emit cameraPerspectiveDidChange(0, &angle, 0, 0);
}
