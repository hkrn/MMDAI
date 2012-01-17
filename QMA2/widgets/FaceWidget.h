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

#ifndef FACEWIDGET_H
#define FACEWIDGET_H

#include <QtGui/QWidget>

namespace vpvl {
class PMDModel;
class Face;
}

class FaceMotionModel;
class QComboBox;
class QGroupBox;
class QPushButton;
class QSlider;

class FaceWidget : public QWidget
{
    Q_OBJECT

public:
    static const int kSliderMaximumValue = 100;

    explicit FaceWidget(FaceMotionModel *fmm, QWidget *parent = 0);

signals:
    void faceDidRegister(vpvl::Face *face);

private slots:
    void retranslate();
    void setPMDModel(vpvl::PMDModel *model);
    void setEyeWeight(int value);
    void setLipWeight(int value);
    void setEyeblowWeight(int value);
    void setOtherWeight(int value);
    void registerEye();
    void registerLip();
    void registerEyeblow();
    void registerOther();
    void updateFaceWeightValues();

private:
    void setFaceWeight(const QComboBox *comboBox, int value);
    void registerBase(const QComboBox *comboBox);
    void updateFaceWeight(const QComboBox *comboBox, QSlider *slider);
    vpvl::Face *findFace(const QString &name);
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
    FaceMotionModel *m_faceMotionModel;

    Q_DISABLE_COPY(FaceWidget)
};

#endif // FACEWIDGET_H
