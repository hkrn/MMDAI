/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
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
#include "widgets/MorphWidget.h"
#include "widgets/SceneLightWidget.h"
#include "widgets/TabWidget.h"

#include <QtGui/QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtWidgets/QtWidgets>
#endif

/* lupdate cannot parse tr() syntax correctly */

namespace vpvm
{

using namespace vpvl2;

TabWidget::TabWidget(SceneLoader *sceneLoaderRef, QSettings *settingsRef, QWidget *parent)
    : QWidget(parent),
      m_tabWidget(new QTabWidget()),
      m_asset(new AssetWidget(sceneLoaderRef)),
      m_camera(new CameraPerspectiveWidget(sceneLoaderRef)),
      m_light(new SceneLightWidget()),
      m_settingsRef(settingsRef)
{
    m_tabWidget->addTab(m_asset.data(), "");
    m_tabWidget->addTab(m_camera.data(), "");
    m_tabWidget->addTab(m_light.data(), "");
    QScopedPointer<QVBoxLayout> mainLayout(new QVBoxLayout());
    mainLayout->addWidget(m_tabWidget.data());
    retranslate();
    setMinimumWidth(350);
    setLayout(mainLayout.take());
    restoreGeometry(m_settingsRef->value("tabWidget/geometry").toByteArray());
}

TabWidget::~TabWidget()
{
}

void TabWidget::retranslate()
{
    m_tabWidget->setTabText(0, vpvm::TabWidget::tr("Asset"));
    m_tabWidget->setTabText(1, vpvm::TabWidget::tr("Camera"));
    m_tabWidget->setTabText(2, vpvm::TabWidget::tr("Light"));
    setWindowTitle(vpvm::TabWidget::tr("Scene Tabs"));
}

void TabWidget::closeEvent(QCloseEvent *event)
{
    m_settingsRef->setValue("tabWidget/geometry", saveGeometry());
    event->accept();
}

} /* namespace vpvm */
