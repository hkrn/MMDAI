#include "FaceWidget.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>
#include "util.h"

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
    slider = createSlider();
    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(setEyeWeight(int)));
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
    slider = createSlider();
    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(setLipWeight(int)));
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
    slider = createSlider();
    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(setEyeblowWeight(int)));
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
    slider = createSlider();
    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(setOtherWeight(int)));
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
    m_eyes->clear();
    m_lips->clear();
    m_eyeblows->clear();
    m_others->clear();
    if (model) {
        m_model = model;
        const vpvl::FaceList &faces = model->faces();
        const uint32_t nFaces = faces.size();
        for (uint32_t i = 0; i < nFaces; i++) {
            vpvl::Face *face = faces[i];
            const uint8_t *name = face->name();
            const QString utf8Name = internal::toQString(face);
            switch (face->type()) {
            case vpvl::Face::kEye:
                m_eyes->addItem(utf8Name, name);
                break;
            case vpvl::Face::kLip:
                m_lips->addItem(utf8Name, name);
                break;
            case vpvl::Face::kEyeblow:
                m_eyeblows->addItem(utf8Name, name);
                break;
            case vpvl::Face::kOther:
                m_others->addItem(utf8Name, name);
                break;
            default:
                break;
            }
        }
    }
}

void FaceWidget::setEyeWeight(int value)
{
    int index = m_eyes->currentIndex();
    if (index >= 0)
        setFaceWeight(m_eyes->itemText(index), value);
}

void FaceWidget::setLipWeight(int value)
{
    int index = m_lips->currentIndex();
    if (index >= 0)
        setFaceWeight(m_lips->itemText(index), value);
}

void FaceWidget::setEyeblowWeight(int value)
{
    int index = m_eyeblows->currentIndex();
    if (index >= 0)
        setFaceWeight(m_eyeblows->itemText(index), value);
}

void FaceWidget::setOtherWeight(int value)
{
    int index = m_others->currentIndex();
    if (index >= 0)
        setFaceWeight(m_others->itemText(index), value);
}

void FaceWidget::setFaceWeight(const QString &name, int value)
{
    QByteArray bytes = internal::getTextCodec()->fromUnicode(name);
    vpvl::Face *face = m_model->findFace(reinterpret_cast<const uint8_t *>(bytes.constData()));
    if (face)
        face->setWeight(value / static_cast<float>(kSliderMaximumValue));
}

QSlider *FaceWidget::createSlider()
{
    QSlider *slider = new QSlider(Qt::Horizontal);
    slider->setTickInterval(20);
    slider->setMinimum(0);
    slider->setMaximum(kSliderMaximumValue);
    return slider;
}
