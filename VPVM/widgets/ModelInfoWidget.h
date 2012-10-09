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

#ifndef VPVM_MODELINFOWIDGET_H
#define VPVM_MODELINFOWIDGET_H

#include <QtGui/QWidget>

namespace vpvl2 {
class IModel;
}

class QLabel;
class QLineEdit;
class QTextEdit;

namespace vpvm
{

using namespace vpvl2;
class SceneLoader;

class ModelInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ModelInfoWidget(QWidget *parent = 0);
    ~ModelInfoWidget();

private slots:
    void retranslate();
    void setModel(IModel *model);

private:
    QLabel *m_nameLabel;
    QLineEdit *m_nameValueLabel;
    QLabel *m_commentLabel;
    QTextEdit *m_commentValueLabel;
    QLabel *m_verticesCountLabel;
    QLineEdit *m_verticesCountValueLabel;
    QLabel *m_indicesCountLabel;
    QLineEdit *m_indicesCountValueLabel;
    QLabel *m_materialsCountLabel;
    QLineEdit *m_materialsCountValueLabel;
    QLabel *m_bonesCountLabel;
    QLineEdit *m_bonesCountValueLabel;
    QLabel *m_IKsCountLabel;
    QLineEdit *m_IKsCountValueLabel;
    QLabel *m_morphsCountLabel;
    QLineEdit *m_morphsCountValueLabel;
    QLabel *m_rigidBodiesCountLabel;
    QLineEdit *m_rigidBodiesCountValueLabel;
    QLabel *m_constrantsCountLabel;
    QLineEdit *m_constrantsCountValueLabel;
};

} /* namespace vpvm */

#endif // MODELINFOWIDGET_H
