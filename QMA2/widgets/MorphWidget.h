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

#ifndef MORPHWIDGET_H
#define MORPHWIDGET_H

#include <QtGui/QWidget>

namespace vpvl2 {
class IModel;
class IMorph;
}

class MorphMotionModel;
class QComboBox;
class QGroupBox;
class QPushButton;
class QSlider;

class MorphWidget : public QWidget
{
    Q_OBJECT

public:
    static const int kSliderMaximumValue = 100;

    explicit MorphWidget(MorphMotionModel *fmm, QWidget *parent = 0);

signals:
    void morphDidRegister(vpvl2::IMorph *face);

private slots:
    void retranslate();
    void setPMDModel(vpvl2::IModel *model);
    void setEyeWeight(int value);
    void setLipWeight(int value);
    void setEyeblowWeight(int value);
    void setOtherWeight(int value);
    void registerEye();
    void registerLip();
    void registerEyeblow();
    void registerOther();
    void updateMorphWeightValues();

private:
    void setMorphWeight(const QComboBox *comboBox, int value);
    void registerBase(const QComboBox *comboBox);
    void updateMorphWeight(const QComboBox *comboBox, QSlider *slider);
    vpvl2::IMorph *findMorph(const QString &name);
    QSlider *createSlider();

    QGroupBox *m_eyeGroup;
    QGroupBox *m_lipGroup;
    QGroupBox *m_eyeblowGroup;
    QGroupBox *m_otherGroup;
    QComboBox *m_eyes;
    QComboBox *m_lips;
    QComboBox *m_eyeblows;
    QComboBox *m_others;
    QSlider *m_eyeSlider;
    QSlider *m_lipSlider;
    QSlider *m_eyeblowSlider;
    QSlider *m_otherSlider;
    QPushButton *m_eyeRegistButton;
    QPushButton *m_lipRegistButton;
    QPushButton *m_eyeblowRegistButton;
    QPushButton *m_otherRegistButton;
    MorphMotionModel *m_morphMotionModel;

    Q_DISABLE_COPY(MorphWidget)
};

#endif // MORPHWIDGET_H
