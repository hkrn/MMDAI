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

#ifndef VPVM_SCENELIGHTWIDGET_H
#define VPVM_SCENELIGHTWIDGET_H

#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtWidgets/QWidget>
#else
#include <QtGui/QWidget>
#endif
#include <vpvl2/Common.h>

class QDoubleSpinBox;
class QGroupBox;
class QPushButton;
class QSpinBox;

namespace vpvm
{

using namespace vpvl2;

class SceneLightWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SceneLightWidget(QWidget *parent = 0);
    ~SceneLightWidget();

public slots:
    void setColor(const Vector3 &value);
    void setDirection(const Vector3 &value);

signals:
    void lightColorDidSet(const Vector3 &value);
    void lightDirectionDidSet(const Vector3 &value);

private slots:
    void retranslate();
    void updateColor();
    void updatePosition();
    void openColorDialog();
    void setQColor(const QColor &value);

private:
    void emitColorDidChange();
    void emitPositionDidChange();
    QSpinBox *createSpinBox(const char *slot) const;
    QDoubleSpinBox *createDoubleSpinBox(const char *slot) const;

    QScopedPointer<QGroupBox> m_colorGroup;
    QScopedPointer<QGroupBox> m_directionGroup;
    QScopedPointer<QSpinBox> m_r;
    QScopedPointer<QSpinBox> m_g;
    QScopedPointer<QSpinBox> m_b;
    QScopedPointer<QDoubleSpinBox> m_x;
    QScopedPointer<QDoubleSpinBox> m_y;
    QScopedPointer<QDoubleSpinBox> m_z;
    QScopedPointer<QPushButton> m_openColorDialog;
};

} /* namespace vpvm */

#endif // SCENELIGHTWIDGET_H
