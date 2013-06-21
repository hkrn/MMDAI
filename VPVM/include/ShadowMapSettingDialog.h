/**

 Copyright (c) 2010-2013  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef VPVM_SHADOWMAPSETTINGDIALOG_H
#define VPVM_SHADOWMAPSETTINGDIALOG_H

#include <vpvl2/Common.h>
#include <vpvl2/qt/ApplicationContext.h> /* for using moc generate workaround */

#include <QDialog>

class QCheckBox;
class QDoubleSpinBox;
class QLabel;
class QComboBox;

namespace vpvm
{

using namespace vpvl2;
class SceneLoader;

class ShadowMapSettingDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ShadowMapSettingDialog(SceneLoader *loader, QWidget *parent = 0);
    ~ShadowMapSettingDialog();

signals:
    void sizeDidChange(const QSize &value);
    void centerDidChange(const Vector3 &value);
    void distanceDidChange(const Scalar &value);

private slots:
    void retranslate();
    void emitSignals();

private:
    static QDoubleSpinBox *createSpinBox(double min, double max);

    QScopedPointer<QLabel> m_sizeLabel;
    QScopedPointer<QComboBox> m_sizeComboBox;
    QScopedPointer<QLabel> m_centerLabel;
    QScopedPointer<QDoubleSpinBox> m_x;
    QScopedPointer<QDoubleSpinBox> m_y;
    QScopedPointer<QDoubleSpinBox> m_z;
    QScopedPointer<QLabel> m_distanceLabel;
    QScopedPointer<QDoubleSpinBox> m_distance;
};

} /* namespace vpvm */

#endif // SHADOWMAPSETTINGDIALOG_H
