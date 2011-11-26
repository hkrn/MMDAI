/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn                                    */
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

#ifndef BONEDIALOG_H
#define BONEDIALOG_H

#include <QtGui/QDialog>
#include <vpvl/Common.h>

class BoneMotionModel;
class QDoubleSpinBox;
class QLabel;

class BoneDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BoneDialog(BoneMotionModel *bmm, QWidget *parent = 0);
    ~BoneDialog();

private slots:
    void retranslate();
    void setPosition(const vpvl::Vector3 &position);
    void setRotation(const vpvl::Quaternion &rotation);
    void setXPosition(double value);
    void setYPosition(double value);
    void setZPosition(double value);
    void setXAngle(double value);
    void setYAngle(double value);
    void setZAngle(double value);
    void dialogAccepted();
    void dialogRejected();

private:
    QLabel *m_xPositionLabel;
    QLabel *m_yPositionLabel;
    QLabel *m_zPositionLabel;
    QLabel *m_xAngleLabel;
    QLabel *m_yAngleLabel;
    QLabel *m_zAngleLabel;
    QDoubleSpinBox *m_xPosition;
    QDoubleSpinBox *m_yPosition;
    QDoubleSpinBox *m_zPosition;
    QDoubleSpinBox *m_xAngle;
    QDoubleSpinBox *m_yAngle;
    QDoubleSpinBox *m_zAngle;
    BoneMotionModel *m_boneMotionModel;

    Q_DISABLE_COPY(BoneDialog)
};

#endif // BONEDIALOG_H
