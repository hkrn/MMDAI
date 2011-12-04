/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn                                    */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#include "FaceWidget.h"
#include "FaceMotionModel.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>
#include "common/util.h"

FaceWidget::FaceWidget(FaceMotionModel *fmm, QWidget *parent) :
    QWidget(parent),
    m_eyes(0),
    m_lips(0),
    m_eyeblows(0),
    m_others(0),
    m_faceMotionModel(fmm)
{
    QSlider *slider = 0;

    QVBoxLayout *eyeVBoxLayout = new QVBoxLayout();
    QHBoxLayout *eyeHBoxLayout = new QHBoxLayout();
    m_eyeLabel = new QLabel();
    m_eyeRegistButton = new QPushButton();
    m_eyes = new QComboBox();
    slider = createSlider();
    connect(m_eyeRegistButton, SIGNAL(clicked()), this, SLOT(registerEye()));
    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(setEyeWeight(int)));
    eyeHBoxLayout->addWidget(m_eyeLabel);
    eyeHBoxLayout->addWidget(m_eyeRegistButton);
    eyeVBoxLayout->addLayout(eyeHBoxLayout);
    eyeVBoxLayout->addWidget(m_eyes);
    eyeVBoxLayout->addWidget(slider);

    QVBoxLayout *lipVBoxLayout = new QVBoxLayout();
    QHBoxLayout *lipHBoxLayout = new QHBoxLayout();
    m_lipLabel = new QLabel();
    m_lipRegistButton = new QPushButton();
    m_lips = new QComboBox();
    slider = createSlider();
    connect(m_lipRegistButton, SIGNAL(clicked()), this, SLOT(registerLip()));
    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(setLipWeight(int)));
    lipHBoxLayout->addWidget(m_lipLabel);
    lipHBoxLayout->addWidget(m_lipRegistButton);
    lipVBoxLayout->addLayout(lipHBoxLayout);
    lipVBoxLayout->addWidget(m_lips);
    lipVBoxLayout->addWidget(slider);

    QVBoxLayout *eyeblowVBoxLayout = new QVBoxLayout();
    QHBoxLayout *eyeblowHBoxLayout = new QHBoxLayout();
    m_eyeblowLabel = new QLabel();
    m_eyeblowRegistButton = new QPushButton();
    m_eyeblows = new QComboBox();
    slider = createSlider();
    connect(m_eyeblowRegistButton, SIGNAL(clicked()), this, SLOT(registerEyeblow()));
    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(setEyeblowWeight(int)));
    eyeblowHBoxLayout->addWidget(m_eyeblowLabel);
    eyeblowHBoxLayout->addWidget(m_eyeblowRegistButton);
    eyeblowVBoxLayout->addLayout(eyeblowHBoxLayout);
    eyeblowVBoxLayout->addWidget(m_eyeblows);
    eyeblowVBoxLayout->addWidget(slider);

    QVBoxLayout *otherVBoxLayout = new QVBoxLayout();
    QHBoxLayout *otherHBoxLayout = new QHBoxLayout();
    m_otherLabel = new QLabel();
    m_otherRegistButton = new QPushButton();
    m_others = new QComboBox();
    slider = createSlider();
    connect(m_otherRegistButton, SIGNAL(clicked()), this, SLOT(registerOther()));
    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(setOtherWeight(int)));
    otherHBoxLayout->addWidget(m_otherLabel);
    otherHBoxLayout->addWidget(m_otherRegistButton);
    otherVBoxLayout->addLayout(otherHBoxLayout);
    otherVBoxLayout->addWidget(m_others);
    otherVBoxLayout->addWidget(slider);

    QGridLayout *layout = new QGridLayout();
    layout->addLayout(eyeVBoxLayout, 0, 0);
    layout->addLayout(lipVBoxLayout, 0, 1);
    layout->addLayout(eyeblowVBoxLayout, 1, 0);
    layout->addLayout(otherVBoxLayout, 1, 1);
    retranslate();
    setLayout(layout);

    connect(m_faceMotionModel, SIGNAL(modelDidChange(vpvl::PMDModel*)),
            this, SLOT(setPMDModel(vpvl::PMDModel*)));
}

void FaceWidget::retranslate()
{
    QString buttonText = tr("Regist");
    m_eyeRegistButton->setText(buttonText);
    m_lipRegistButton->setText(buttonText);
    m_eyeblowRegistButton->setText(buttonText);
    m_otherRegistButton->setText(buttonText);
    m_eyeLabel->setText(tr("Eye"));
    m_lipLabel->setText(tr("Lip"));
    m_eyeblowLabel->setText(tr("Eyeblow"));
    m_otherLabel->setText(tr("Other"));
}

void FaceWidget::setPMDModel(vpvl::PMDModel *model)
{
    m_eyes->clear();
    m_lips->clear();
    m_eyeblows->clear();
    m_others->clear();
    if (model) {
        const vpvl::FaceList &faces = m_faceMotionModel->selectedModel()->facesForUI();
        const int nfaces = faces.count();
        for (int i = 0; i < nfaces; i++) {
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
    setFaceWeight(m_eyes, value);
}

void FaceWidget::setLipWeight(int value)
{
    setFaceWeight(m_lips, value);
}

void FaceWidget::setEyeblowWeight(int value)
{
    setFaceWeight(m_eyeblows, value);
}

void FaceWidget::setOtherWeight(int value)
{
    setFaceWeight(m_others, value);
}

void FaceWidget::registerEye()
{
    registerBase(m_eyes);
}

void FaceWidget::registerLip()
{
    registerBase(m_lips);
}

void FaceWidget::registerEyeblow()
{
    registerBase(m_eyeblows);
}

void FaceWidget::registerOther()
{
    registerBase(m_others);
}

void FaceWidget::registerBase(const QComboBox *comboBox)
{
    int index = comboBox->currentIndex();
    if (index >= 0) {
        vpvl::Face *face = m_faceMotionModel->findFace(comboBox->itemText(index));
        if (face)
            emit faceDidRegister(face);
    }
}

void FaceWidget::setFaceWeight(const QComboBox *comboBox, int value)
{
    int index = comboBox->currentIndex();
    if (index >= 0) {
        vpvl::Face *face = m_faceMotionModel->findFace(comboBox->itemText(index));
        if (face) {
            float weight = value / static_cast<float>(kSliderMaximumValue);
            m_faceMotionModel->setWeight(weight, face);
        }
    }
}

QSlider *FaceWidget::createSlider()
{
    QSlider *slider = new QSlider(Qt::Horizontal);
    slider->setTickInterval(20);
    slider->setMinimum(0);
    slider->setMaximum(kSliderMaximumValue);
    return slider;
}
