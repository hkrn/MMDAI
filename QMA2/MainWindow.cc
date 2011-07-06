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

#include "FaceWidget.h"
#include "HandleWidget.h"
#include "PerspectionWidget.h"
#include "SceneWidget.h"
#include "TimelineWidget.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_settings(QSettings::IniFormat, QSettings::UserScope, "MMDAI", "QMA2"),
      m_model(0),
      m_bone(0),
      m_position(0.0f, 0.0f, 0.0f),
      m_angle(0.0f, 0.0f, 0.0f),
      m_fovy(0.0f),
      m_distance(0.0f),
      m_currentFPS(0)
{
    QMenuBar *menuBar;
#ifdef Q_OS_MAC
    menuBar = new QMenuBar(0);
#else
    menuBar = this->menuBar();
#endif

    m_left = new QSplitter(Qt::Vertical, this);
    m_main = new QSplitter(Qt::Horizontal, this);
    m_settings.setIniCodec("UTF-8");
    m_timeline = new TimelineWidget();
    m_left->addWidget(m_timeline);
    QTabWidget *tab = new QTabWidget();
    m_face = new FaceWidget();
    tab->addTab(m_face, tr("Face"));
    m_handle = new HandleWidget();
    tab->addTab(m_handle, tr("Handle"));
    m_perspection = new PerspectionWidget();
    tab->addTab(m_perspection, tr("Perspection"));
    m_left->addWidget(tab);
    m_main->addWidget(m_left);
    QVBoxLayout *layout = new QVBoxLayout();
    m_scene = new SceneWidget(&m_settings);
    layout->addWidget(m_scene);
    m_info = new QLabel();
    layout->addWidget(m_info);
    layout->setContentsMargins(QMargins());
    layout->setStretch(0, 1);
    m_widget = new QWidget;
    m_widget->setLayout(layout);
    m_main->addWidget(m_widget);

    setCentralWidget(m_main);
    setWindowTitle(qAppName());
    createActions();
    createMenus(menuBar);
    updateInformation();

    connect(m_scene, SIGNAL(modelDidAdd(vpvl::PMDModel*)),
            this, SLOT(addModel(vpvl::PMDModel*)));
    connect(m_scene, SIGNAL(modelDidDelete(vpvl::PMDModel*)),
            this, SLOT(deleteModel(vpvl::PMDModel*)));
    connect(m_scene, SIGNAL(modelDidSelect(vpvl::PMDModel*)),
            m_timeline, SLOT(setModel(vpvl::PMDModel*)));
    connect(m_scene, SIGNAL(modelDidSelect(vpvl::PMDModel*)),
            m_face, SLOT(setModel(vpvl::PMDModel*)));
    connect(m_perspection, SIGNAL(perspectionDidChange(btVector3*,btVector3*,float*,float*)),
            m_scene, SLOT(setCameraPerspective(btVector3*,btVector3*,float*,float*)));
    connect(m_timeline, SIGNAL(boneDidSelect(vpvl::Bone*)),
            m_handle, SLOT(setBone(vpvl::Bone*)));
    connect(m_scene, SIGNAL(fpsDidUpdate(int)),
            this, SLOT(setCurrentFPS(int)));
    connect(m_scene, SIGNAL(modelDidSelect(vpvl::PMDModel*)),
            this, SLOT(setModel(vpvl::PMDModel*)));
    connect(m_timeline, SIGNAL(boneDidSelect(vpvl::Bone*)),
            this, SLOT(setBone(vpvl::Bone*)));
    connect(m_scene, SIGNAL(cameraPerspectionDidSet(btVector3,btVector3,float,float)),
            this, SLOT(setCameraPerspective(btVector3,btVector3,float,float)));

    statusBar()->show();
    restoreGeometry(m_settings.value("mainWindow/geometry").toByteArray());
    restoreState(m_settings.value("mainWindow/state").toByteArray());
    m_main->restoreGeometry(m_settings.value("mainSplitter/geometry").toByteArray());
    m_main->restoreState(m_settings.value("mainSplitter/state").toByteArray());
    m_left->restoreGeometry(m_settings.value("leftSplitter/geometry").toByteArray());
    m_left->restoreState(m_settings.value("leftSplitter/state").toByteArray());
}

MainWindow::~MainWindow()
{
    delete m_face;
    delete m_handle;
    delete m_perspection;
    delete m_scene;
    delete m_timeline;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    m_settings.setValue("mainWindow/geometry", saveGeometry());
    m_settings.setValue("mainWindow/state", saveState());
    m_settings.setValue("mainSplitter/geometry", m_main->saveGeometry());
    m_settings.setValue("mainSplitter/state", m_main->saveState());
    m_settings.setValue("leftSplitter/geometry", m_left->saveGeometry());
    m_settings.setValue("leftSplitter/state", m_left->saveState());
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

void MainWindow::addModel(vpvl::PMDModel *model)
{
    QString name = SceneWidget::toUnicodeModelName(model);
    QAction *action = new QAction(name, this);
    action->setStatusTip(tr("Select a model %1").arg(name));
    connect(action, SIGNAL(triggered()), this, SLOT(selectModel()));
    m_selectModelMenu->addAction(action);
}

void MainWindow::deleteModel(vpvl::PMDModel *model)
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

void MainWindow::setCurrentFPS(int value)
{
    m_currentFPS = value;
    updateInformation();
}

void MainWindow::setModel(vpvl::PMDModel *value)
{
    m_model = value;
    updateInformation();
}

void MainWindow::setBone(vpvl::Bone *value)
{
    m_bone = value;
    updateInformation();
}

void MainWindow::setCameraPerspective(const btVector3 &pos, const btVector3 &angle, float fovy, float distance)
{
    m_position = pos;
    m_angle = angle;
    m_fovy = fovy;
    m_distance = distance;
    updateInformation();
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
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Up));
    connect(action, SIGNAL(triggered()), m_scene, SLOT(rotateUp()));
    m_rotateUpAction = action;

    action = new QAction(tr("Rotate down"), this);
    action->setStatusTip(tr("Rotate a model down."));
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Down));
    connect(action, SIGNAL(triggered()), m_scene, SLOT(rotateDown()));
    m_rotateDownAction = action;

    action = new QAction(tr("Rotate Left"), this);
    action->setStatusTip(tr("Rotate a model left."));
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Left));
    connect(action, SIGNAL(triggered()), m_scene, SLOT(rotateLeft()));
    m_rotateLeftAction = action;

    action = new QAction(tr("Rotate right"), this);
    action->setStatusTip(tr("Rotate a model right."));
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Right));
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

void MainWindow::updateInformation()
{
    QString modelName = tr("N/A"), boneName = tr("N/A");
    QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
    if (m_model)
        modelName = codec->toUnicode(reinterpret_cast<const char *>(m_model->name()));
    if (m_bone)
        boneName = codec->toUnicode(reinterpret_cast<const char *>(m_bone->name()));
    m_info->setText(tr("<font color=blue><b>Model</b></font>: <font color=red><b>%1</b></font> "
                       "<font color=blue><b>Bone</b></font>: <font color=red><b>%2</b></font> "
                       "<font color=blue>%3fps</font><br>"
                       "Fovy: %4 Distance: %5 Position: (%6,%7,%8) Angle: (%9,%10,%11)")
                    .arg(modelName).arg(boneName).arg(m_currentFPS)
                    .arg(m_fovy).arg(m_distance)
                    .arg(m_position.x()).arg(m_position.y()).arg(m_position.z())
                    .arg(m_angle.x()).arg(m_angle.y()).arg(m_angle.z()));
}

void MainWindow::about()
{
}
