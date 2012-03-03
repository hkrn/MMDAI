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

#ifndef FRAMEWEIGHTDIALOG_H
#define FRAMEWEIGHTDIALOG_H

#include "widgets/TimelineTabWidget.h" /* for TimelineTabWidget::Type */

#include <QtGui/QDialog>

class QSettings;
class QDoubleSpinBox;
class SceneWidget;

class FrameWeightDialog : public QDialog
{
    Q_OBJECT

public:
    FrameWeightDialog(TimelineTabWidget::Type type, QWidget *parent = 0);
    ~FrameWeightDialog();

private slots:
    void setPositionXWeight(double value);
    void setPositionYWeight(double value);
    void setPositionZWeight(double value);
    void setRotationXWeight(double value);
    void setRotationYWeight(double value);
    void setRotationZWeight(double value);
    void setMorphWeight(double value);
    void emitBoneWeightSignal();
    void emitMorphWeightSignal();

signals:
    void boneWeightDidSet(const vpvl::Vector3 &position, const vpvl::Vector3 &rotation);
    void morphKeyframeWeightDidSet(float weight);

private:
    QDoubleSpinBox *createSpinBox(const char *slot);

    vpvl::Vector3 m_position;
    vpvl::Vector3 m_rotation;
    float m_morphWeight;

    Q_DISABLE_COPY(FrameWeightDialog)
};

#endif // FRAMESELECTIONDIALOG_H
