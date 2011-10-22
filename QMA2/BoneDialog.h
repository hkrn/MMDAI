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
#include <vpvl/PMDModel.h>

namespace Ui {
    class BoneDialog;
}

class BoneMotionModel;

class BoneDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BoneDialog(BoneMotionModel *bmm, QWidget *parent = 0);
    ~BoneDialog();

private slots:
    void setPosition(const vpvl::Vector3 &pos);
    void setRotation(const vpvl::Quaternion &rot);
    void on_XPositionSpinBox_valueChanged(double value);
    void on_YPositionSpinBox_valueChanged(double value);
    void on_ZPositionSpinBox_valueChanged(double value);
    void on_XAxisSpinBox_valueChanged(double value);
    void on_YAxisSpinBox_valueChanged(double value);
    void on_ZAxisSpinBox_valueChanged(double value);
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::BoneDialog *ui;
    BoneMotionModel *m_boneMotionModel;
};

#endif // BONEDIALOG_H
