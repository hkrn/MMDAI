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

#ifndef VPVM_INTERPOLATIONDIALOG_H
#define VPVM_INTERPOLATIONDIALOG_H

#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtWidgets/QWidget>
#include <QtWidgets/QAbstractItemView>
#else
#include <QtGui/QWidget>
#include <QtGui/QAbstractItemView>
#endif
#include <vpvl2/IBoneKeyframe.h>
#include <vpvl2/ICameraKeyframe.h>

class QAbstractButton;
class QComboBox;
class QGroupBox;
class QSpinBox;
class QLabel;
class QPushButton;
class QDialogButtonBox;

namespace vpvm
{

class BoneMotionModel;
class InterpolationGraphWidget;
class SceneMotionModel;

class InterpolationDialog : public QWidget
{
    Q_OBJECT

public:
    explicit InterpolationDialog(BoneMotionModel *bmm, SceneMotionModel *smm, QWidget *parent = 0);
    ~InterpolationDialog();

    void setMode(int mode);
    void setModelIndices(const QModelIndexList &indices);
    bool hasValidKeyframes() const;

private slots:
    void retranslate();
    void disable();
    void clickButton(QAbstractButton *button);
    void selectPreset(int value);

private:
    QSpinBox *createSpinBox(int defaultValue,
                            const char *signal,
                            const char *slot);

    QScopedPointer<QLabel> m_parameterTypeLabel;
    QScopedPointer<QComboBox> m_parameterTypeComboBox;
    QScopedPointer<QLabel> m_presetLabel;
    QScopedPointer<QComboBox> m_presetComboBox;
    QScopedPointer<QGroupBox> m_parameterGroup;
    QScopedPointer<QPushButton> m_applyAllButton;
    QScopedPointer<QDialogButtonBox> m_buttonBox;
    QScopedPointer<InterpolationGraphWidget> m_graphWidget;

    Q_DISABLE_COPY(InterpolationDialog)
};

} /* namespace vpvm */

#endif // INTERPOLATIONWIDGET_H
