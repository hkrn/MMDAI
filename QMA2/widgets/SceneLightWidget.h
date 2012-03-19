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

#ifndef SCENELIGHTWIDGET_H
#define SCENELIGHTWIDGET_H

#include <QtGui/QWidget>
#include <vpvl/Common.h>

class QDoubleSpinBox;
class QGroupBox;
class QPushButton;
class QSpinBox;

class SceneLightWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SceneLightWidget(QWidget *parent = 0);
    ~SceneLightWidget();

public slots:
    void setColor(const vpvl::Color &value);
    void setPosition(const vpvl::Vector3 &value);

signals:
    void lightColorDidSet(const vpvl::Color &value);
    void lightPositionDidSet(const vpvl::Vector3 &value);

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

    QGroupBox *m_colorGroup;
    QGroupBox *m_directionGroup;
    QSpinBox *m_r;
    QSpinBox *m_g;
    QSpinBox *m_b;
    QDoubleSpinBox *m_x;
    QDoubleSpinBox *m_y;
    QDoubleSpinBox *m_z;
    QPushButton *m_openColorDialog;
};

#endif // SCENELIGHTWIDGET_H
