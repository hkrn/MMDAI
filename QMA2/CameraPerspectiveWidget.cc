#include "CameraPerspectiveWidget.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

CameraPerspectiveWidget::CameraPerspectiveWidget(QWidget *parent) :
    QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    QHBoxLayout *subLayout = 0;
    subLayout = new QHBoxLayout();
    m_frontLabel = new QPushButton();
    connect(m_frontLabel, SIGNAL(clicked()), this, SLOT(setCameraPerspectiveFront()));
    subLayout->addWidget(m_frontLabel);
    m_backLabel = new QPushButton();
    connect(m_backLabel, SIGNAL(clicked()), this, SLOT(setCameraPerspectiveBack()));
    subLayout->addWidget(m_backLabel);
    m_topLabel = new QPushButton();
    connect(m_topLabel, SIGNAL(clicked()), this, SLOT(setCameraPerspectiveTop()));
    subLayout->addWidget(m_topLabel);
    mainLayout->addLayout(subLayout);
    subLayout = new QHBoxLayout();
    m_leftLabel = new QPushButton();
    connect(m_leftLabel, SIGNAL(clicked()), this, SLOT(setCameraPerspectiveLeft()));
    subLayout->addWidget(m_leftLabel);
    m_rightLabel = new QPushButton();
    connect(m_rightLabel, SIGNAL(clicked()), this, SLOT(setCameraPerspectiveRight()));
    subLayout->addWidget(m_rightLabel);
    m_cameraLabel = new QPushButton();
    subLayout->addWidget(m_cameraLabel);
    mainLayout->addLayout(subLayout);
    retranslate();
    setLayout(mainLayout);
}

void CameraPerspectiveWidget::retranslate()
{
    m_frontLabel->setText(tr("Front"));
    m_backLabel->setText(tr("Back"));
    m_topLabel->setText(tr("Top"));
    m_leftLabel->setText(tr("Left"));
    m_rightLabel->setText(tr("Right"));
    m_cameraLabel->setText(tr("Camera"));
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
