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

#include "widgets/AssetWidget.h"
#include "widgets/CameraPerspectiveWidget.h"
#include "widgets/InterpolationWidget.h"
#include "widgets/MorphWidget.h"
#include "widgets/TabWidget.h"

#include <QtGui/QtGui>

TabWidget::TabWidget(QSettings *settings,
                     QWidget *parent) :
    QWidget(parent),
    m_settings(settings),
    m_asset(0),
    m_camera(0)
{
    m_asset = new AssetWidget();
    m_camera = new CameraPerspectiveWidget();
    m_tabWidget = new QTabWidget();
    m_tabWidget->addTab(m_asset, "");
    m_tabWidget->addTab(m_camera, "");
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(m_tabWidget);
    retranslate();
    setMinimumWidth(350);
    setLayout(layout);
    restoreGeometry(m_settings->value("tabWidget/geometry").toByteArray());
}

TabWidget::~TabWidget()
{
}

void TabWidget::retranslate()
{
    m_tabWidget->setTabText(0, tr("Asset"));
    m_tabWidget->setTabText(1, tr("Camera"));
    setWindowTitle(tr("Scene Tabs"));
}

void TabWidget::closeEvent(QCloseEvent *event)
{
    m_settings->setValue("tabWidget/geometry", saveGeometry());
    event->accept();
}
