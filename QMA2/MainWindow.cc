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

#include "MainWindow.h"

#include "SceneWidget.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_settings(QSettings::IniFormat, QSettings::UserScope, "MMDAI", "QMA2")
{
    m_settings.setIniCodec("UTF-8");
    m_scene = new SceneWidget(&m_settings, this);
    connect(m_scene, SIGNAL(modelDidAdd(const vpvl::PMDModel*)),
            this, SLOT(addModel(const vpvl::PMDModel*)));
    connect(m_scene, SIGNAL(modelDidDelete(const vpvl::PMDModel*)),
            this, SLOT(deleteModel(const vpvl::PMDModel*)));

    QMenuBar *menuBar;
#ifdef Q_OS_MAC
    menuBar = new QMenuBar(0);
#else
    menuBar = this->menuBar();
#endif
    createActions();
    createMenus(menuBar);

    setCentralWidget(m_scene);
    setWindowTitle(qAppName());
    setMinimumSize(640, 480);

    restoreGeometry(m_settings.value("geometry").toByteArray());
    restoreState(m_settings.value("state").toByteArray());
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    m_settings.setValue("geometry", saveGeometry());
    m_settings.setValue("state", saveState());
    event->accept();
}

void MainWindow::selectModel()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        vpvl::PMDModel *model = m_scene->models().value(action->text());
        m_scene->setSelectedModel(model);
    }
}

void MainWindow::revertSelectedModel()
{
    m_scene->setSelectedModel(0);
}

void MainWindow::addModel(const vpvl::PMDModel *model)
{
    QString name = SceneWidget::toUnicodeModelName(model);
    QAction *action = new QAction(name, this);
    action->setStatusTip(tr("Select a model %1").arg(name));
    connect(action, SIGNAL(triggered()), this, SLOT(selectModel()));
    m_selectModelMenu->addAction(action);
}

void MainWindow::deleteModel(const vpvl::PMDModel *model)
{
    QAction *actionToRemove = 0;
    QString name = SceneWidget::toUnicodeModelName(model);
    foreach (QAction *action, m_selectModelMenu->actions()) {
        if (action->text() == name) {
            actionToRemove = action;
            break;
        }
    }
    if (actionToRemove)
        m_selectModelMenu->removeAction(actionToRemove);
}

void MainWindow::createActions()
{
    QAction *action = NULL;

    action = new QAction(tr("Add model"), this);
    action->setStatusTip(tr("Add a PMD model to the scene."));
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_M));
    connect(action, SIGNAL(triggered()), m_scene, SLOT(addModel()));
    m_addModelAction = action;

    action = new QAction(tr("Add asset"), this);
    action->setStatusTip(tr("Add an asset to the scene."));
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_S));
    connect(action, SIGNAL(triggered()), m_scene, SLOT(addAsset()));
    m_addAssetAction = action;

    action = new QAction(tr("Insert to the all models"), this);
    action->setStatusTip(tr("Insert a motion to the all models."));
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_V));
    connect(action, SIGNAL(triggered()), m_scene, SLOT(insertMotionToAllModels()));
    m_insertMotionToAllAction = action;

    action = new QAction(tr("Insert to the selected model"), this);
    action->setStatusTip(tr("Insert a motion to the selected model. If an object is not selected, do nothing."));
    action->setShortcut(QKeySequence(Qt::ALT + Qt::CTRL + Qt::SHIFT + Qt::Key_V));
    connect(action, SIGNAL(triggered()), m_scene, SLOT(insertMotionToSelectedModel()));
    m_insertMotionToSelectedAction = action;

    action = new QAction(tr("Set camera"), this);
    action->setStatusTip(tr("Set or replace a camera motion to the scene."));
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_C));
    connect(action, SIGNAL(triggered()), m_scene, SLOT(setCamera()));
    m_setCameraAction = action;

    action = new QAction(tr("Zoom in"), this);
    action->setStatusTip(tr("Zoom in the scene."));
    action->setShortcut(Qt::Key_Plus);
    connect(action, SIGNAL(triggered()), m_scene, SLOT(zoomIn()));
    m_zoomInAction = action;

    action = new QAction(tr("Zoom out"), this);
    action->setStatusTip(tr("Zoom out the scene."));
    action->setShortcut(Qt::Key_Minus);
    connect(action, SIGNAL(triggered()), m_scene, SLOT(zoomOut()));
    m_zoomOutAction = action;

    action = new QAction(tr("Rotate up"), this);
    action->setStatusTip(tr("Rotate a model up."));
    action->setShortcut(Qt::Key_Up);
    connect(action, SIGNAL(triggered()), m_scene, SLOT(rotateUp()));
    m_rotateUpAction = action;

    action = new QAction(tr("Rotate down"), this);
    action->setStatusTip(tr("Rotate a model down."));
    action->setShortcut(Qt::Key_Down);
    connect(action, SIGNAL(triggered()), m_scene, SLOT(rotateDown()));
    m_rotateDownAction = action;

    action = new QAction(tr("Rotate Left"), this);
    action->setStatusTip(tr("Rotate a model left."));
    action->setShortcut(Qt::Key_Left);
    connect(action, SIGNAL(triggered()), m_scene, SLOT(rotateLeft()));
    m_rotateLeftAction = action;

    action = new QAction(tr("Rotate right"), this);
    action->setStatusTip(tr("Rotate a model right."));
    action->setShortcut(Qt::Key_Right);
    connect(action, SIGNAL(triggered()), m_scene, SLOT(rotateRight()));
    m_rotateRightAction = action;

    action = new QAction(tr("Translate up"), this);
    action->setStatusTip(tr("Translate a model up."));
    action->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Up));
    connect(action, SIGNAL(triggered()), m_scene, SLOT(translateUp()));
    m_translateUpAction = action;

    action = new QAction(tr("Translate down"), this);
    action->setStatusTip(tr("Translate a model down."));
    action->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Down));
    connect(action, SIGNAL(triggered()), m_scene, SLOT(translateDown()));
    m_translateDownAction = action;

    action = new QAction(tr("Translate left"), this);
    action->setStatusTip(tr("Translate a model left."));
    action->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Left));
    connect(action, SIGNAL(triggered()), m_scene, SLOT(translateLeft()));
    m_translateLeftAction = action;

    action = new QAction(tr("Translate right"), this);
    action->setStatusTip(tr("Translate a model right."));
    action->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Right));
    connect(action, SIGNAL(triggered()), m_scene, SLOT(translateRight()));
    m_translateRightAction = action;

    action = new QAction(tr("Reset camera"), this);
    action->setStatusTip(tr("Reset camera perspective."));
    connect(action, SIGNAL(triggered()), m_scene, SLOT(resetCamera()));
    m_resetCameraAction = action;

    action = new QAction(tr("Revert selected model"), this);
    action->setStatusTip(tr("Revert the selected model."));
    connect(action, SIGNAL(triggered()), this, SLOT(revertSelectedModel()));
    m_revertSelectedModelAction = action;

    action = new QAction(tr("Delete selected model"), this);
    action->setStatusTip(tr("Delete the selected model from the scene. If an object is not selected, do nothing."));
    action->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Delete);
    connect(action, SIGNAL(triggered()), m_scene, SLOT(deleteSelectedModel()));
    m_deleteSelectedModelAction = action;

    action = new QAction(tr("E&xit"), this);
    action->setMenuRole(QAction::QuitRole);
    action->setShortcuts(QKeySequence::Quit);
    action->setStatusTip(tr("Exit the application."));
    connect(action, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));
    m_exitAction = action;

    QList<QKeySequence> shortcuts;
    shortcuts.append(QKeySequence(Qt::ALT + Qt::Key_Question));
    shortcuts.append(QKeySequence(Qt::ALT + Qt::Key_Slash));
    action = new QAction(tr("&About"), this);
    action->setMenuRole(QAction::AboutRole);
    action->setStatusTip(tr("Show the application's About box."));
    action->setShortcuts(shortcuts);
    connect(action, SIGNAL(triggered()), this, SLOT(about()));
    m_aboutAction = action;

    action = new QAction(tr("About &Qt"), this);
    action->setMenuRole(QAction::AboutQtRole);
    action->setStatusTip(tr("Show the Qt library's About box."));
    connect(action, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    m_aboutQtAction = action;
}

void MainWindow::createMenus(QMenuBar *menuBar)
{
    m_fileMenu = menuBar->addMenu(tr("&File"));
    m_fileMenu->addAction(m_addModelAction);
    m_fileMenu->addAction(m_addAssetAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_insertMotionToAllAction);
    m_fileMenu->addAction(m_insertMotionToSelectedAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_setCameraAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_exitAction);
    m_sceneMenu = menuBar->addMenu(tr("&Scene"));
    m_sceneMenu->addAction(m_zoomInAction);
    m_sceneMenu->addAction(m_zoomOutAction);
    m_sceneMenu->addSeparator();
    m_sceneMenu->addAction(m_rotateUpAction);
    m_sceneMenu->addAction(m_rotateDownAction);
    m_sceneMenu->addAction(m_rotateLeftAction);
    m_sceneMenu->addAction(m_rotateRightAction);
    m_sceneMenu->addSeparator();
    m_sceneMenu->addAction(m_translateUpAction);
    m_sceneMenu->addAction(m_translateDownAction);
    m_sceneMenu->addAction(m_translateLeftAction);
    m_sceneMenu->addAction(m_translateRightAction);
    m_sceneMenu->addSeparator();
    m_sceneMenu->addAction(m_resetCameraAction);
    m_modelMenu = menuBar->addMenu(tr("&Model"));
    m_selectModelMenu = m_modelMenu->addMenu(tr("Select model"));
    m_modelMenu->addAction(m_revertSelectedModelAction);
    m_modelMenu->addAction(m_deleteSelectedModelAction);
    m_helpMenu = menuBar->addMenu(tr("&Help"));
    m_helpMenu->addAction(m_aboutAction);
    m_helpMenu->addAction(m_aboutQtAction);
}

void MainWindow::about()
{
}
