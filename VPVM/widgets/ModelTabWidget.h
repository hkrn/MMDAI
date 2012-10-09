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

#ifndef VPVM_MODELTABWIDGET_H
#define VPVM_MODELTABWIDGET_H

#include <QtGui/QWidget>

class QCloseEvent;
class QTabWidget;
class QSettings;

namespace vpvm
{

class BoneMotionModel;
class MorphMotionModel;
class MorphWidget;
class ModelInfoWidget;
class ModelSettingWidget;
class SceneMotionModel;

class ModelTabWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ModelTabWidget(QSettings *settings,
                            MorphMotionModel *mmm,
                            QWidget *parent);
    ~ModelTabWidget();

    MorphWidget *morphWidget() const { return m_morphWidget; }
    ModelInfoWidget *modelInfoWidget() const { return m_modelInfoWidget; }
    ModelSettingWidget *modelSettingWidget() const { return m_modelSettingWidget; }

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void retranslate();

private:
    QTabWidget *m_tabWidget;
    QSettings *m_settings;
    MorphWidget *m_morphWidget;
    ModelInfoWidget *m_modelInfoWidget;
    ModelSettingWidget *m_modelSettingWidget;

    Q_DISABLE_COPY(ModelTabWidget)
};

} /* namespace vpvm */

#endif // MODELWIDGET_H
