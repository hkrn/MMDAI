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

#ifndef VPVM_MORPHWIDGET_H
#define VPVM_MORPHWIDGET_H

#include <QtCore/QPointer>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtCore/QStringListModel>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QCompleter>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QWidget>
#else
#include <QtGui/QComboBox>
#include <QtGui/QCompleter>
#include <QtGui/QGroupBox>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QStringListModel>
#include <QtGui/QWidget>
#endif
#include <vpvl2/Common.h>

namespace vpvl2 {
class IModel;
class IMorph;
}

namespace vpvm
{

using namespace vpvl2;
class MorphMotionModel;

class MorphWidget : public QWidget
{
    Q_OBJECT

public:
    static const int kSliderMaximumValue = 100;

    explicit MorphWidget(MorphMotionModel *fmm, QWidget *parent = 0);
    ~MorphWidget();

signals:
    void morphWillChange();
    void morphDidChange();
    void morphDidRegister(IMorph *morph);

private slots:
    void retranslate();
    void setPMDModel(IModel *model);
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
    IMorph *findMorph(const QString &name);
    QSlider *createSlider() const;

    QPointer<QComboBox> m_eyes;
    QPointer<QSlider> m_eyeSlider;
    QPointer<QStringListModel> m_eyesCompleterModel;
    QPointer<QGroupBox> m_eyeGroup;
    QPointer<QComboBox> m_lips;
    QPointer<QSlider> m_lipSlider;
    QPointer<QStringListModel> m_lipsCompleterModel;
    QPointer<QGroupBox> m_lipGroup;
    QPointer<QComboBox> m_eyeblows;
    QPointer<QSlider> m_eyeblowSlider;
    QPointer<QStringListModel> m_eyeblowsCompleterModel;
    QPointer<QGroupBox> m_eyeblowGroup;
    QPointer<QComboBox> m_others;
    QPointer<QSlider> m_otherSlider;
    QPointer<QStringListModel> m_othersCompleterModel;
    QPointer<QGroupBox> m_otherGroup;
    QPointer<QPushButton> m_eyeRegisterButton;
    QPointer<QPushButton> m_lipRegisterButton;
    QPointer<QPushButton> m_eyeblowRegisterButton;
    QPointer<QPushButton> m_otherRegisterButton;
    QPointer<QPushButton> m_resetAllButton;
    MorphMotionModel *m_morphMotionModelRef;

    Q_DISABLE_COPY(MorphWidget)
};

} /* namespace vpvm */

#endif // MORPHWIDGET_H
