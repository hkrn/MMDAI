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

#ifndef VPVM_BACKGROUNDIMAGESETTINGDIALOG_H
#define VPVM_BACKGROUNDIMAGESETTINGDIALOG_H

#include <QtCore/QPoint>
#include <QtGui/QDialog>

class QCheckBox;
class QSpinBox;

namespace vpvm
{

class SceneLoader;

class BackgroundImageSettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BackgroundImageSettingDialog(SceneLoader *loader, QWidget *parent = 0);
    ~BackgroundImageSettingDialog();

signals:
    void positionDidChange(const QPoint &value);
    void uniformDidEnable(bool value);

private slots:
    void setPositionX(int value);
    void setPositionY(int value);
    void restoreAndClose();

private:
    QScopedPointer<QSpinBox> m_x;
    QScopedPointer<QSpinBox> m_y;
    QScopedPointer<QCheckBox> m_checkbox;
    QPoint m_position;
    bool m_scaled;
};

} /* namespace vpvm */

#endif // BACKGROUNDIMAGESETTINGDIALOG_H
