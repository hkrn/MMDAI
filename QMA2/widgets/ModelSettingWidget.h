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

#ifndef MODELSETTINGWIDGET_H
#define MODELSETTINGWIDGET_H

#include <QtGui/QWidget>
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
class SceneLoader;

class ModelSettingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ModelSettingWidget(QWidget *parent = 0);
    ~ModelSettingWidget();

signals:
    void edgeOffsetDidChange(double value);
    void edgeColorDidChange(const QColor &color);
    void opacityDidChange(const vpvl2::Scalar &value);
    void positionOffsetDidChange(const vpvl2::Vector3 &value);
    void rotationOffsetDidChange(const vpvl2::Vector3 &value);
    void projectiveShadowDidEnable(bool value);
    void selfShadowDidEnable(bool value);

private slots:
    void retranslate();
    void openEdgeColorDialog();
    void setModel(vpvl2::IModel *model, SceneLoader *loader);
    void setPositionOffset(const vpvl2::Vector3 &position);
    void updatePosition();
    void updateRotation();
    void emitOpacitySignal(int value);

private:
    void disableSignals();
    void enableSignals();
    void createEdgeColorDialog();
    QDoubleSpinBox *createSpinBox(const char *slot, double min, double max, double step = 0.1) const;

    QLabel *m_edgeOffsetLabel;
    QDoubleSpinBox *m_edgeOffsetSpinBox;
    QPushButton *m_edgeColorDialogOpenButton;
    QColorDialog *m_edgeColorDialog;
    QLabel *m_opacityLabel;
    QSlider *m_opacitySlider;
    QSpinBox *m_opacitySpinBox;
    QButtonGroup *m_radioButtonsGroup;
    QRadioButton *m_noShadowCheckbox;
    QRadioButton *m_projectiveShadowCheckbox;
    QRadioButton *m_selfShadowCheckbox;
    QDoubleSpinBox *m_px;
    QDoubleSpinBox *m_py;
    QDoubleSpinBox *m_pz;
    QGroupBox *m_positionGroup;
    QDoubleSpinBox *m_rx;
    QDoubleSpinBox *m_ry;
    QDoubleSpinBox *m_rz;
    QGroupBox *m_rotationGroup;
};

#endif // MODELSETTINGWIDGET_H
