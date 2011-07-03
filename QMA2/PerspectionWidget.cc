#include "PerspectionWidget.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

PerspectionWidget::PerspectionWidget(QWidget *parent) :
    QWidget(parent)
{
    QHBoxLayout *hlayout = 0;
    QPushButton *button = 0;
    QVBoxLayout *layout = new QVBoxLayout();
    hlayout = new QHBoxLayout();
    button = new QPushButton(tr("Front"));
    connect(button, SIGNAL(clicked()), this, SLOT(setPerspectionFront()));
    hlayout->addWidget(button);
    button = new QPushButton(tr("Back"));
    connect(button, SIGNAL(clicked()), this, SLOT(setPerspectionBack()));
    hlayout->addWidget(button);
    button = new QPushButton(tr("Top"));
    connect(button, SIGNAL(clicked()), this, SLOT(setPerspectionTop()));
    hlayout->addWidget(button);
    layout->addLayout(hlayout);
    hlayout = new QHBoxLayout();
    button = new QPushButton(tr("Left"));
    connect(button, SIGNAL(clicked()), this, SLOT(setPerspectionLeft()));
    hlayout->addWidget(button);
    button = new QPushButton(tr("Right"));
    connect(button, SIGNAL(clicked()), this, SLOT(setPerspectionRight()));
    hlayout->addWidget(button);
    button = new QPushButton(tr("Camera"));
    hlayout->addWidget(button);
    layout->addLayout(hlayout);
    setLayout(layout);
}

void PerspectionWidget::setPerspectionFront()
{
    btVector3 angle(0, 0, 0);
    emit perspectionDidChange(0, &angle, 0, 0);
}

void PerspectionWidget::setPerspectionBack()
{
    btVector3 angle(0, 180, 0);
    emit perspectionDidChange(0, &angle, 0, 0);
}

void PerspectionWidget::setPerspectionTop()
{
    btVector3 angle(90, 0, 0);
    emit perspectionDidChange(0, &angle, 0, 0);
}

void PerspectionWidget::setPerspectionLeft()
{
    btVector3 angle(0, -90, 0);
    emit perspectionDidChange(0, &angle, 0, 0);
}

void PerspectionWidget::setPerspectionRight()
{
    btVector3 angle(0, 90, 0);
    emit perspectionDidChange(0, &angle, 0, 0);
}
