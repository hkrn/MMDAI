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

#ifndef VPVM_MODELSETTINGWIDGET_H
#define VPVM_MODELSETTINGWIDGET_H

#include <QtCore/QPointer>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtWidgets/QWidget>
#else
#include <QtGui/QWidget>
#endif
#include <vpvl2/Common.h>

namespace vpvl2 {
class IModel;
}

class QButtonGroup;
class QColorDialog;
class QDoubleSpinBox;
class QGroupBox;
class QLabel;
class QPushButton;
class QRadioButton;
class QSlider;
class QSpinBox;

namespace vpvm
{

using namespace vpvl2;
class SceneLoader;

class ModelSettingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ModelSettingWidget(QWidget *parent = 0);
    ~ModelSettingWidget();

signals:
    void edgeOffsetDidChange(double value);
    void opacityDidChange(const Scalar &value);
    void positionOffsetDidChange(const Vector3 &value);
    void rotationOffsetDidChange(const Vector3 &value);
    void projectiveShadowDidEnable(bool value);
    void selfShadowDidEnable(bool value);

private slots:
    void retranslate();
    void setModel(IModel *model, SceneLoader *loader);
    void setPositionOffset(const Vector3 &position);
    void updatePosition();
    void updateRotation();
    void emitOpacitySignal(int value);

private:
    void disableSignals();
    void enableSignals();
    QDoubleSpinBox *createSpinBox(const char *slot, double min, double max, double step = 0.1) const;

    QPointer<QGroupBox> m_edgeGroup;
    QPointer<QDoubleSpinBox> m_edgeOffsetSpinBox;
    QPointer<QGroupBox> m_opacityGroup;
    QPointer<QSlider> m_opacitySlider;
    QPointer<QSpinBox> m_opacitySpinBox;
    QPointer<QGroupBox> m_shadowGroup;
    QPointer<QButtonGroup> m_shadowButtonsGroup;
    QPointer<QRadioButton> m_noShadowCheckbox;
    QPointer<QRadioButton> m_projectiveShadowCheckbox;
    QPointer<QRadioButton> m_selfShadowCheckbox;
    QPointer<QDoubleSpinBox> m_px;
    QPointer<QDoubleSpinBox> m_py;
    QPointer<QDoubleSpinBox> m_pz;
    QPointer<QGroupBox> m_positionGroup;
    QPointer<QDoubleSpinBox> m_rx;
    QPointer<QDoubleSpinBox> m_ry;
    QPointer<QDoubleSpinBox> m_rz;
    QPointer<QGroupBox> m_rotationGroup;
};

} /* namespace vpvm */

#endif // MODELSETTINGWIDGET_H
