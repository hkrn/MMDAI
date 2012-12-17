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
#include "models/MorphMotionModel.h"
#include "widgets/MorphWidget.h"

#include <QtGui/QtGui>
#include <vpvl2/vpvl2.h>
#include <vpvl2/qt/CString.h>

/* lupdate cannot parse tr() syntax correctly */

namespace vpvm
{

using namespace vpvl2;
using namespace vpvl2::qt;

MorphWidget::MorphWidget(MorphMotionModel *mmm, QWidget *parent) :
    QWidget(parent),
    m_eyes(new QComboBox()),
    m_eyeSlider(createSlider()),
    m_eyesCompleterModel(new QStringListModel()),
    m_eyeGroup(new QGroupBox()),
    m_lips(new QComboBox()),
    m_lipSlider(createSlider()),
    m_lipsCompleterModel(new QStringListModel()),
    m_lipGroup(new QGroupBox()),
    m_eyeblows(new QComboBox()),
    m_eyeblowSlider(createSlider()),
    m_eyeblowsCompleterModel(new QStringListModel()),
    m_eyeblowGroup(new QGroupBox()),
    m_others(new QComboBox()),
    m_otherSlider(createSlider()),
    m_othersCompleterModel(new QStringListModel()),
    m_otherGroup(new QGroupBox()),
    m_eyeRegisterButton(new QPushButton()),
    m_lipRegisterButton(new QPushButton()),
    m_eyeblowRegisterButton(new QPushButton()),
    m_otherRegisterButton(new QPushButton()),
    m_resetAllButton(new QPushButton()),
    m_morphMotionModelRef(mmm)
{
    /* 目(左上) */
    QScopedPointer<QVBoxLayout> eyeVBoxLayout(new QVBoxLayout());
    QScopedPointer<QCompleter> completer(new QCompleter());
    QScopedPointer<QLineEdit> lineEdit(new QLineEdit());
    completer->setModel(m_eyesCompleterModel.data());
    lineEdit->setCompleter(completer.take());
    m_eyes->setLineEdit(lineEdit.take());
    connect(m_eyes.data(), SIGNAL(currentIndexChanged(int)), SLOT(updateMorphWeightValues()));
    connect(m_eyeRegisterButton.data(), SIGNAL(clicked()), SLOT(registerEye()));
    connect(m_eyeSlider.data(), SIGNAL(valueChanged(int)), SLOT(setEyeWeight(int)));
    eyeVBoxLayout->addWidget(m_eyes.data());
    eyeVBoxLayout->addWidget(m_eyeSlider.data());
    eyeVBoxLayout->addWidget(m_eyeRegisterButton.data());
    m_eyeGroup->setLayout(eyeVBoxLayout.take());
    /* 口唇(右上) */
    QScopedPointer<QVBoxLayout> lipVBoxLayout(new QVBoxLayout());
    completer.reset(new QCompleter());
    lineEdit.reset(new QLineEdit());
    completer->setModel(m_lipsCompleterModel.data());
    lineEdit->setCompleter(completer.take());
    m_lips->setLineEdit(lineEdit.take());
    connect(m_lips.data(), SIGNAL(currentIndexChanged(int)), SLOT(updateMorphWeightValues()));
    connect(m_lipRegisterButton.data(), SIGNAL(clicked()), SLOT(registerLip()));
    connect(m_lipSlider.data(), SIGNAL(valueChanged(int)), SLOT(setLipWeight(int)));
    lipVBoxLayout->addWidget(m_lips.data());
    lipVBoxLayout->addWidget(m_lipSlider.data());
    lipVBoxLayout->addWidget(m_lipRegisterButton.data());
    m_lipGroup->setLayout(lipVBoxLayout.take());
    /* まゆ(左下) */
    QScopedPointer<QVBoxLayout> eyeblowVBoxLayout(new QVBoxLayout());
    completer.reset(new QCompleter());
    lineEdit.reset(new QLineEdit());
    completer->setModel(m_eyeblowsCompleterModel.data());
    lineEdit->setCompleter(completer.take());
    m_eyeblows->setLineEdit(lineEdit.take());
    connect(m_eyeblows.data(), SIGNAL(currentIndexChanged(int)), SLOT(updateMorphWeightValues()));
    connect(m_eyeblowRegisterButton.data(), SIGNAL(clicked()), SLOT(registerEyeblow()));
    connect(m_eyeblowSlider.data(), SIGNAL(valueChanged(int)), SLOT(setEyeblowWeight(int)));
    eyeblowVBoxLayout->addWidget(m_eyeblows.data());
    eyeblowVBoxLayout->addWidget(m_eyeblowSlider.data());
    eyeblowVBoxLayout->addWidget(m_eyeblowRegisterButton.data());
    m_eyeblowGroup->setLayout(eyeblowVBoxLayout.take());
    /* その他(右下) */
    QScopedPointer<QVBoxLayout> otherVBoxLayout(new QVBoxLayout());
    completer.reset(new QCompleter());
    lineEdit.reset(new QLineEdit());
    completer->setModel(m_othersCompleterModel.data());
    lineEdit->setCompleter(completer.take());
    m_others->setLineEdit(lineEdit.take());
    connect(m_others.data(), SIGNAL(currentIndexChanged(int)), SLOT(updateMorphWeightValues()));
    connect(m_otherRegisterButton.data(), SIGNAL(clicked()), SLOT(registerOther()));
    connect(m_otherSlider.data(), SIGNAL(valueChanged(int)), SLOT(setOtherWeight(int)));
    otherVBoxLayout->addWidget(m_others.data());
    otherVBoxLayout->addWidget(m_otherSlider.data());
    otherVBoxLayout->addWidget(m_otherRegisterButton.data());
    m_otherGroup->setLayout(otherVBoxLayout.take());
    /* 「全てのモーフをリセット」ボタン */
    connect(m_resetAllButton.data(), SIGNAL(clicked()), m_morphMotionModelRef, SLOT(resetAllMorphs()));
    /* レイアウト結合 */
    QScopedPointer<QVBoxLayout> mainLayout(new QVBoxLayout());
    QScopedPointer<QLayout> subLayout(new QHBoxLayout());
    subLayout->addWidget(m_eyeGroup.data());
    subLayout->addWidget(m_lipGroup.data());
    mainLayout->addLayout(subLayout.take());
    subLayout.reset(new QHBoxLayout());
    subLayout->addWidget(m_eyeblowGroup.data());
    subLayout->addWidget(m_otherGroup.data());
    mainLayout->addLayout(subLayout.take());
    mainLayout->addWidget(m_resetAllButton.data());
    mainLayout->addStretch();
    retranslate();
    setLayout(mainLayout.take());
    setEnabled(false);
    connect(m_morphMotionModelRef, SIGNAL(modelDidChange(IModelSharedPtr)), SLOT(setPMDModel(IModelSharedPtr)));
}

MorphWidget::~MorphWidget()
{
}

void MorphWidget::retranslate()
{
    const QString &buttonText = vpvm::MorphWidget::tr("Register");
    m_eyeRegisterButton->setText(buttonText);
    m_lipRegisterButton->setText(buttonText);
    m_eyeblowRegisterButton->setText(buttonText);
    m_otherRegisterButton->setText(buttonText);
    m_eyeGroup->setTitle(vpvm::MorphWidget::tr("Eye"));
    m_lipGroup->setTitle(vpvm::MorphWidget::tr("Lip"));
    m_eyeblowGroup->setTitle(vpvm::MorphWidget::tr("Eyeblow"));
    m_otherGroup->setTitle(vpvm::MorphWidget::tr("Other"));
    m_resetAllButton->setText(vpvm::MorphWidget::tr("Reset All Morphs"));
}

void MorphWidget::setPMDModel(const IModelSharedPtr model)
{
    QStringList eyes, lips, eyeblows, others;
    m_eyes->clear();
    m_lips->clear();
    m_eyeblows->clear();
    m_others->clear();
    if (model) {
        Array<IMorph *> morphs;
        m_morphMotionModelRef->selectedModel()->getMorphRefs(morphs);
        const int nmorphs = morphs.count();
        for (int i = 0; i < nmorphs; i++) {
            IMorph *morph = morphs[i];
            const QString &name = toQStringFromMorph(morph);
            switch (morph->category()) {
            case IMorph::kEye:
                m_eyes->addItem(name, name);
                eyes.append(name);
                break;
            case IMorph::kLip:
                m_lips->addItem(name, name);
                lips.append(name);
                break;
            case IMorph::kEyeblow:
                m_eyeblows->addItem(name, name);
                eyeblows.append(name);
                break;
            case IMorph::kOther:
                m_others->addItem(name, name);
                others.append(name);
                break;
            default:
                break;
            }
        }
        setEnabled(true);
        qDebug("Set a model to MorphWidget: %s", qPrintable(toQStringFromModel(model.data())));
    }
    else {
        setEnabled(false);
        qDebug("Reset MorphWidget");
    }
    m_eyesCompleterModel->setStringList(eyes);
    m_lipsCompleterModel->setStringList(lips);
    m_eyeblowsCompleterModel->setStringList(eyeblows);
    m_othersCompleterModel->setStringList(others);
}

void MorphWidget::setEyeWeight(int value)
{
    setMorphWeight(m_eyes.data(), value);
}

void MorphWidget::setLipWeight(int value)
{
    setMorphWeight(m_lips.data(), value);
}

void MorphWidget::setEyeblowWeight(int value)
{
    setMorphWeight(m_eyeblows.data(), value);
}

void MorphWidget::setOtherWeight(int value)
{
    setMorphWeight(m_others.data(), value);
}

void MorphWidget::registerEye()
{
    registerBase(m_eyes.data());
}

void MorphWidget::registerLip()
{
    registerBase(m_lips.data());
}

void MorphWidget::registerEyeblow()
{
    registerBase(m_eyeblows.data());
}

void MorphWidget::registerOther()
{
    registerBase(m_others.data());
}

void MorphWidget::registerBase(const QComboBox *comboBox)
{
    IModelSharedPtr model = m_morphMotionModelRef->selectedModel();
    int index = comboBox->currentIndex();
    if (model && index >= 0) {
        const CString s(comboBox->itemText(index));
        IMorph *morph = model->findMorph(&s);
        if (morph)
            emit morphDidRegister(morph);
    }
}

void MorphWidget::updateMorphWeightValues()
{
    updateMorphWeight(m_eyes.data(), m_eyeSlider.data());
    updateMorphWeight(m_lips.data(), m_lipSlider.data());
    updateMorphWeight(m_eyeblows.data(), m_eyeblowSlider.data());
    updateMorphWeight(m_others.data(), m_otherSlider.data());
}

void MorphWidget::updateMorphWeight(const QComboBox *comboBox, QSlider *slider)
{
    IModelSharedPtr model = m_morphMotionModelRef->selectedModel();
    int index = comboBox->currentIndex();
    if (model && index >= 0) {
        const CString s(comboBox->itemText(index));
        IMorph *morph = model->findMorph(&s);
        if (morph)
            slider->setValue(morph->weight() * kSliderMaximumValue);
    }
}

void MorphWidget::setMorphWeight(const QComboBox *comboBox, int value)
{
    IModelSharedPtr model = m_morphMotionModelRef->selectedModel();
    int index = comboBox->currentIndex();
    if (model && index >= 0) {
        const CString s(comboBox->itemText(index));
        IMorph *morph = model->findMorph(&s);
        if (morph) {
            /* モデルのモーフの変更だけ行う。キーフレームの登録は行わない */
            const Scalar &newWeight = value / static_cast<Scalar>(kSliderMaximumValue);
            m_morphMotionModelRef->setWeight(newWeight, morph);
            /* MorphWidget ではシークする必要がなく、むしろスライダーが動かない問題を抱えてしまうので無効化する */
            m_morphMotionModelRef->updateModel(m_morphMotionModelRef->selectedModel(), false);
        }
    }
}

QSlider *MorphWidget::createSlider() const
{
    QScopedPointer<QSlider> slider(new QSlider(Qt::Horizontal));
    connect(slider.data(), SIGNAL(sliderPressed()), SIGNAL(morphWillChange()));
    connect(slider.data(), SIGNAL(sliderReleased()), SIGNAL(morphDidChange()));
    slider->setTickInterval(20);
    slider->setMinimum(0);
    slider->setMaximum(kSliderMaximumValue);
    return slider.take();
}

} /* namespace vpvm */
