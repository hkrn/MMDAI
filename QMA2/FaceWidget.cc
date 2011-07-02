#include "FaceWidget.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

FaceWidget::FaceWidget(QWidget *parent) :
    QWidget(parent)
{
    QLabel *label = 0;
    QPushButton *button = 0;
    QSlider *slider = 0;
    QString buttonLabel = tr("Regist");

    QVBoxLayout *eyeVBoxLayout = new QVBoxLayout();
    QHBoxLayout *eyeHBoxLayout = new QHBoxLayout();
    label = new QLabel(tr("Eye"));
    button = new QPushButton(buttonLabel);
    m_eyes = new QComboBox();
    slider = new QSlider(Qt::Horizontal);
    eyeHBoxLayout->addWidget(label);
    eyeHBoxLayout->addWidget(button);
    eyeVBoxLayout->addLayout(eyeHBoxLayout);
    eyeVBoxLayout->addWidget(m_eyes);
    eyeVBoxLayout->addWidget(slider);

    QVBoxLayout *lipVBoxLayout = new QVBoxLayout();
    QHBoxLayout *lipHBoxLayout = new QHBoxLayout();
    label = new QLabel(tr("Lip"));
    button = new QPushButton(buttonLabel);
    m_lips = new QComboBox();
    slider = new QSlider(Qt::Horizontal);
    lipHBoxLayout->addWidget(label);
    lipHBoxLayout->addWidget(button);
    lipVBoxLayout->addLayout(lipHBoxLayout);
    lipVBoxLayout->addWidget(m_lips);
    lipVBoxLayout->addWidget(slider);

    QVBoxLayout *eyeblowVBoxLayout = new QVBoxLayout();
    QHBoxLayout *eyeblowHBoxLayout = new QHBoxLayout();
    label = new QLabel(tr("Eyeblow"));
    button = new QPushButton(buttonLabel);
    m_eyeblows = new QComboBox();
    slider = new QSlider(Qt::Horizontal);
    eyeblowHBoxLayout->addWidget(label);
    eyeblowHBoxLayout->addWidget(button);
    eyeblowVBoxLayout->addLayout(eyeblowHBoxLayout);
    eyeblowVBoxLayout->addWidget(m_eyeblows);
    eyeblowVBoxLayout->addWidget(slider);

    QVBoxLayout *otherVBoxLayout = new QVBoxLayout();
    QHBoxLayout *otherHBoxLayout = new QHBoxLayout();
    label = new QLabel(tr("Other"));
    button = new QPushButton(buttonLabel);
    m_others = new QComboBox();
    slider = new QSlider(Qt::Horizontal);
    otherHBoxLayout->addWidget(label);
    otherHBoxLayout->addWidget(button);
    otherVBoxLayout->addLayout(otherHBoxLayout);
    otherVBoxLayout->addWidget(m_others);
    otherVBoxLayout->addWidget(slider);

    QGridLayout *layout = new QGridLayout();
    layout->addLayout(eyeVBoxLayout, 0, 0);
    layout->addLayout(lipVBoxLayout, 0, 1);
    layout->addLayout(eyeblowVBoxLayout, 1, 0);
    layout->addLayout(otherVBoxLayout, 1, 1);
    setLayout(layout);
}

void FaceWidget::setModel(vpvl::PMDModel *model)
{
    if (model) {
        m_model = model;
    }
    else {
        m_eyes->clear();
        m_lips->clear();
        m_eyeblows->clear();
        m_others->clear();
    }
}
