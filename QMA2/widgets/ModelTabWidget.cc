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

#include "ModelTabWidget.h"

#include "MorphWidget.h"
#include "InterpolationWidget.h"
#include "ModelInfoWidget.h"
#include "ModelSettingWidget.h"
#include "models/BoneMotionModel.h"
#include "models/MorphMotionModel.h"
#include "models/SceneMotionModel.h"

#include <QtGui/QtGui>

ModelTabWidget::ModelTabWidget(QSettings *settings,
                               BoneMotionModel *bmm,
                               MorphMotionModel *mmm,
                               SceneMotionModel *smm,
                               QWidget *parent) :
    QWidget(parent),
    m_tabWidget(0),
    m_settings(settings),
    m_morphWidget(0),
    m_interpolationWidget(0),
    m_modelInfoWidget(0),
    m_modelSettingWidget(0)
{
    m_morphWidget = new MorphWidget(mmm);
    m_interpolationWidget = new InterpolationWidget(bmm, smm);
    m_modelInfoWidget = new ModelInfoWidget();
    m_modelSettingWidget = new ModelSettingWidget();
    m_tabWidget = new QTabWidget();
    m_tabWidget->addTab(m_modelInfoWidget, "");
    m_tabWidget->addTab(m_modelSettingWidget, "");
    m_tabWidget->addTab(m_morphWidget, "");
    m_tabWidget->addTab(m_interpolationWidget, "");
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(m_tabWidget);
    retranslate();
    setMinimumWidth(350);
    setLayout(layout);
    restoreGeometry(m_settings->value("modelTabWidget/geometry").toByteArray());
}

ModelTabWidget::~ModelTabWidget()
{
}

void ModelTabWidget::retranslate()
{
    m_tabWidget->setTabText(0, tr("Info"));
    m_tabWidget->setTabText(1, tr("Setting"));
    m_tabWidget->setTabText(2, tr("Morph"));
    m_tabWidget->setTabText(3, tr("Interpolation"));
    setWindowTitle(tr("Model Tabs"));
}

void ModelTabWidget::closeEvent(QCloseEvent *event)
{
    m_settings->setValue("modelTabWidget/geometry", saveGeometry());
    event->accept();
}
