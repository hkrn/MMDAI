/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
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

#include "common/util.h"
#include "models/FaceMotionModel.h"
#include "widgets/FaceWidget.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

FaceWidget::FaceWidget(FaceMotionModel *fmm, QWidget *parent) :
    QWidget(parent),
    m_faceMotionModel(fmm)
{
    /* 目(左上) */
    QVBoxLayout *eyeVBoxLayout = new QVBoxLayout();
    m_eyeRegistButton = new QPushButton();
    m_eyes = new QComboBox();
    m_eyeSlider = createSlider();
    connect(m_eyeRegistButton, SIGNAL(clicked()), this, SLOT(registerEye()));
    connect(m_eyeSlider, SIGNAL(valueChanged(int)), this, SLOT(setEyeWeight(int)));
    eyeVBoxLayout->addWidget(m_eyes);
    eyeVBoxLayout->addWidget(m_eyeSlider);
    eyeVBoxLayout->addWidget(m_eyeRegistButton);
    m_eyeGroup = new QGroupBox();
    m_eyeGroup->setLayout(eyeVBoxLayout);
    /* 口唇(右上) */
    QVBoxLayout *lipVBoxLayout = new QVBoxLayout();
    m_lipRegistButton = new QPushButton();
    m_lips = new QComboBox();
    m_lipSlider = createSlider();
    connect(m_lipRegistButton, SIGNAL(clicked()), this, SLOT(registerLip()));
    connect(m_lipSlider, SIGNAL(valueChanged(int)), this, SLOT(setLipWeight(int)));
    lipVBoxLayout->addWidget(m_lips);
    lipVBoxLayout->addWidget(m_lipSlider);
    lipVBoxLayout->addWidget(m_lipRegistButton);
    m_lipGroup = new QGroupBox();
    m_lipGroup->setLayout(lipVBoxLayout);
    /* まゆ(左下) */
    QVBoxLayout *eyeblowVBoxLayout = new QVBoxLayout();
    m_eyeblowRegistButton = new QPushButton();
    m_eyeblows = new QComboBox();
    m_eyeblowSlider = createSlider();
    connect(m_eyeblowRegistButton, SIGNAL(clicked()), this, SLOT(registerEyeblow()));
    connect(m_eyeblowSlider, SIGNAL(valueChanged(int)), this, SLOT(setEyeblowWeight(int)));
    eyeblowVBoxLayout->addWidget(m_eyeblows);
    eyeblowVBoxLayout->addWidget(m_eyeblowSlider);
    eyeblowVBoxLayout->addWidget(m_eyeblowRegistButton);
    m_eyeblowGroup = new QGroupBox();
    m_eyeblowGroup->setLayout(eyeblowVBoxLayout);
    /* その他(右下) */
    QVBoxLayout *otherVBoxLayout = new QVBoxLayout();
    m_otherRegistButton = new QPushButton();
    m_others = new QComboBox();
    m_otherSlider = createSlider();
    connect(m_otherRegistButton, SIGNAL(clicked()), this, SLOT(registerOther()));
    connect(m_otherSlider, SIGNAL(valueChanged(int)), this, SLOT(setOtherWeight(int)));
    otherVBoxLayout->addWidget(m_others);
    otherVBoxLayout->addWidget(m_otherSlider);
    otherVBoxLayout->addWidget(m_otherRegistButton);
    m_otherGroup = new QGroupBox();
    m_otherGroup->setLayout(otherVBoxLayout);
    /* レイアウト結合 */
    QVBoxLayout *mainLayout = new QVBoxLayout();
    QHBoxLayout *subLayout = new QHBoxLayout();
    subLayout->addWidget(m_eyeGroup);
    subLayout->addWidget(m_lipGroup);
    mainLayout->addLayout(subLayout);
    subLayout = new QHBoxLayout();
    subLayout->addWidget(m_eyeblowGroup);
    subLayout->addWidget(m_otherGroup);
    mainLayout->addLayout(subLayout);
    mainLayout->addStretch();
    retranslate();
    setLayout(mainLayout);
    setEnabled(false);
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
    m_eyeGroup->setTitle(tr("Eye"));
    m_lipGroup->setTitle(tr("Lip"));
    m_eyeblowGroup->setTitle(tr("Eyeblow"));
    m_otherGroup->setTitle(tr("Other"));
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
            const QString &utf8Name = internal::toQString(face);
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
        setEnabled(true);
    }
    else {
        setEnabled(false);
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

void FaceWidget::updateFaceWeightValues()
{
    /* SceneWidget#seekMotion でモデルのモーフ値が変更済みなので、その値を取り出してスライダーに反映させる */
    updateFaceWeight(m_eyes, m_eyeSlider);
    updateFaceWeight(m_lips, m_lipSlider);
    updateFaceWeight(m_eyeblows, m_eyeblowSlider);
    updateFaceWeight(m_others, m_otherSlider);
}

void FaceWidget::updateFaceWeight(const QComboBox *comboBox, QSlider *slider)
{
    int index = comboBox->currentIndex();
    if (index >= 0) {
        vpvl::Face *face = m_faceMotionModel->findFace(comboBox->itemText(index));
        if (face)
            slider->setValue(face->weight() * 100.0f);
    }
}

void FaceWidget::setFaceWeight(const QComboBox *comboBox, int value)
{
    int index = comboBox->currentIndex();
    if (index >= 0) {
        vpvl::Face *face = m_faceMotionModel->findFace(comboBox->itemText(index));
        if (face) {
            /* モデルのモーフの変更だけ行う。キーフレームの登録は行わない */
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
