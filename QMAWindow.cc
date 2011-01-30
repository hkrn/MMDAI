/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn (libMMDAI)                         */
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

#include "QMAWindow.h"

QMAWindow::QMAWindow(QWidget *parent) :
    QMainWindow(parent),
    m_isFullScreen(false),
    m_enablePhysicsSimulation(true)
{
  m_settings = new QSettings(parent);
  m_widget = new QMAWidget(parent);
  setCentralWidget(m_widget);

  createActions();
  createMenu();
  setWindowTitle("QtMMDAI");

  readSetting();
  setUnifiedTitleAndToolBarOnMac(true);
  statusBar()->showMessage("Ready");
}

QMAWindow::~QMAWindow()
{
  delete m_settings;
  delete m_widget;
}

void QMAWindow::closeEvent(QCloseEvent *event)
{
  writeSetting();
  event->accept();
}

void QMAWindow::keyPressEvent(QKeyEvent *event)
{
  switch (event->key()) {
  case Qt::Key_D: /* Debug */
    break;
  case Qt::Key_S: /* info String */
    break;
  case Qt::Key_V: /* test command / Vsync */
    break;
  }
  m_widget->sendKeyEvent(event->text());
}

void QMAWindow::insertMotionToAllModels()
{
  QString path = m_settings->value("window/lastVMDDirectory").toString();
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open VMD file"), path, tr("VMD (*.vmd)"));
  if (!fileName.isEmpty()) {
    QDir dir(fileName);
    dir.cdUp();
    m_settings->value("window/lastVMDDirectory", dir.absolutePath());
    SceneController *controller = m_widget->getSceneController();
    const char *filename = fileName.toUtf8().constData();
    int count = controller->countPMDObjects();
    for (int i = 0; i < count; i++) {
      PMDObject *object = controller->getPMDObject(i);
      if (object->isEnable() && object->allowMotionFileDrop()) {
        VMDLoader *loader = m_widget->getModelLoaderFactory()->createMotionLoader(filename);
        controller->addMotion(object, loader);
      }
    }
  }
}

void QMAWindow::insertMotionToSelectedModel()
{
  QString path = m_settings->value("window/lastVMDDirectory").toString();
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open model PMD file"), path, tr("VMD (*.vmd)"));
  if (!fileName.isEmpty()) {
    QDir dir(fileName);
    dir.cdUp();
    m_settings->value("window/lastVMDDirectory", dir.absolutePath());
    SceneController *controller = m_widget->getSceneController();
    PMDObject *selectedObject = controller->getSelectedPMDObject();
    if (selectedObject != NULL) {
      VMDLoader *loader = m_widget->getModelLoaderFactory()->createMotionLoader(fileName.toUtf8().constData());
      controller->addMotion(selectedObject, loader);
    }
  }
}

void QMAWindow::addModel()
{
  QString path = m_settings->value("window/lastPMDDirectory").toString();
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open model PMD file"), path, tr("PMD (*.pmd)"));
  if (!fileName.isEmpty()) {
    QDir dir(fileName);
    dir.cdUp();
    m_settings->value("window/lastPMDDirectory", dir.absolutePath());
    PMDModelLoader *loader = m_widget->getModelLoaderFactory()->createModelLoader(fileName.toUtf8().constData());
    m_widget->getSceneController()->addModel(loader);
  }
}

void QMAWindow::setStage()
{
  QString path = m_settings->value("window/lastStageDirectory").toString();
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open stage PMD file"), path, tr("PMD (*.pmd *.xpmd)"));
  if (!fileName.isEmpty()) {
    QDir dir(fileName);
    dir.cdUp();
    m_settings->value("window/lastStageDirectory", dir.absolutePath());
    PMDModelLoader *loader = m_widget->getModelLoaderFactory()->createModelLoader(fileName.toUtf8().constData());
    m_widget->getSceneController()->loadStage(loader);
  }
}

void QMAWindow::setFloor()
{
  QString path = m_settings->value("window/lastFloorDirectory").toString();
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open floor image"), path, tr("Image (*.bmp *.png *.tga)"));
  if (!fileName.isEmpty()) {
    QDir dir(fileName);
    dir.cdUp();
    m_settings->value("window/lastFloorDirectory", dir.absolutePath());
    PMDModelLoader *loader = m_widget->getModelLoaderFactory()->createModelLoader(fileName.toUtf8().constData());
    m_widget->getSceneController()->loadFloor(loader);
  }
}

void QMAWindow::setBackground()
{
  QString path = m_settings->value("window/lastBackgroundDirectory").toString();
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open floor image"), path, tr("Image (*.bmp *.png *.tga)"));
  if (!fileName.isEmpty()) {
    QDir dir(fileName);
    dir.cdUp();
    m_settings->value("window/lastBackgroundDirectory", dir.absolutePath());
    PMDModelLoader *loader = m_widget->getModelLoaderFactory()->createModelLoader(fileName.toUtf8().constData());
    m_widget->getSceneController()->loadBackground(loader);
  }
}

void QMAWindow::rotateUp()
{
  SceneController *controller = m_widget->getSceneController();
  controller->rotate(0.0f, -controller->getOption()->getRotateStep(), 0.0f);
}

void QMAWindow::rotateDown()
{
  SceneController *controller = m_widget->getSceneController();
  controller->rotate(0.0f, controller->getOption()->getRotateStep(), 0.0f);
}

void QMAWindow::rotateLeft()
{
  SceneController *controller = m_widget->getSceneController();
  controller->rotate(-controller->getOption()->getRotateStep(), 0.0f, 0.0f);
}

void QMAWindow::rotateRight()
{
  SceneController *controller = m_widget->getSceneController();
  controller->rotate(controller->getOption()->getRotateStep(), 0.0f, 0.0f);
}

void QMAWindow::translateUp()
{
  SceneController *controller = m_widget->getSceneController();
  controller->translate(0.0f, -controller->getOption()->getTranslateStep(), 0.0f);
}

void QMAWindow::translateDown()
{
  SceneController *controller = m_widget->getSceneController();
  controller->translate(0.0f, controller->getOption()->getTranslateStep(), 0.0f);
}

void QMAWindow::translateLeft()
{
  SceneController *controller = m_widget->getSceneController();
  controller->translate(controller->getOption()->getTranslateStep(), 0.0f, 0.0f);
}

void QMAWindow::translateRight()
{
  SceneController *controller = m_widget->getSceneController();
  controller->translate(-controller->getOption()->getTranslateStep(), 0.0f, 0.0f);
}

void QMAWindow::toggleDisplayBone()
{
  m_widget->toggleDisplayBone();
}

void QMAWindow::toggleDisplayRigidBody()
{
  m_widget->toggleDisplayRigidBody();
}

void QMAWindow::increaseEdgeThin()
{
  setEdgeThin(1);
}

void QMAWindow::decreaseEdgeThin()
{
  setEdgeThin(-1);
}

void QMAWindow::togglePhysicSimulation()
{
  SceneController *controller = m_widget->getSceneController();
  int count = controller->countPMDObjects();
  m_enablePhysicsSimulation = !m_enablePhysicsSimulation;
  for (int i = 0; i < count; i++)
    controller->getPMDObject(i)->getPMDModel()->setPhysicsControl(m_enablePhysicsSimulation);
}

void QMAWindow::toggleShadowMapping()
{
  SceneController *controller = m_widget->getSceneController();
  Option *option = controller->getOption();
  if (option->getUseShadowMapping()) {
    option->setUseShadowMapping(false);
    controller->setShadowMapping(false);
  }
  else {
    option->setUseShadowMapping(true);
    controller->setShadowMapping(true);
  }
}

void QMAWindow::toggleShadowMappingLightFirst()
{
  Option *option = m_widget->getSceneController()->getOption();
  if (option->getShadowMappingLightFirst())
    option->setShadowMappingLightFirst(false);
  else
    option->setShadowMappingLightFirst(true);
}

void QMAWindow::toggleFullScreen()
{
  if (m_isFullScreen) {
    showNormal();
    statusBar()->show();
    m_isFullScreen = false;
  }
  else {
    statusBar()->hide();
    showFullScreen();
    m_isFullScreen = true;
  }
}

void QMAWindow::zoomIn()
{
  SceneController *controller = m_widget->getSceneController();
  controller->setScale(controller->getOption()->getScaleStep() * controller->getScale());
  m_widget->update();
}

void QMAWindow::zoomOut()
{
  SceneController *controller = m_widget->getSceneController();
  controller->setScale(controller->getOption()->getScaleStep() / controller->getScale());
  m_widget->update();
}

void QMAWindow::selectObject()
{
  QAction *action = qobject_cast<QAction *>(sender());
  if (action) {
    const char *name = action->text().toUtf8().constData();
    SceneController *controller = m_widget->getSceneController();
    PMDObject *object = controller->findPMDObject(name);
    if (object != NULL) {
      controller->selectPMDObject(object);
      controller->hightlightPMDObject();
    }
  }
}

void QMAWindow::changeSelectedObject()
{
  QString path = m_settings->value("window/lastPMDDirectory").toString();
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open model PMD file"), path, tr("PMD (*.pmd)"));
  if (!fileName.isEmpty()) {
    QDir dir(fileName);
    dir.cdUp();
    m_settings->value("window/lastPMDDirectory", dir.absolutePath());
    SceneController *controller = m_widget->getSceneController();
    PMDObject *selectedObject = controller->getSelectedPMDObject();
    if (selectedObject != NULL){
      PMDModelLoader *loader = m_widget->getModelLoaderFactory()->createModelLoader(fileName.toUtf8().constData());
      controller->changeModel(selectedObject, loader);
    }
  }
}

void QMAWindow::deleteSelectedObject()
{
  SceneController *controller = m_widget->getSceneController();
  PMDObject *selectedObject = controller->getSelectedPMDObject();
  if (selectedObject != NULL)
    controller->deleteModel(selectedObject);
}

void QMAWindow::about()
{
  QMessageBox::about(this, tr("About QtMMDAI"), tr("<h2>QtMMDAI 0.3 <small>(CodeName: 40mP)</small></h2>"
                                                   "<p>Copyright (C) 2010-2011<br>"
                                                   "Nagoya Institute of Technology Department of Computer Science, "
                                                   "hkrn (@hikarincl2)<br>"
                                                   "All rights reserved.</p>"
                                                   "<p>This application uses following libraries<ul>"
                                                   "<li><a href='http://github.com/hkrn/MMDAI/'>libMMDME</a></li>"
                                                   "<li><a href='http://github.com/hkrn/MMDAI/'>libMMDAI</a></li>"
                                                   "<li><a href='http://qt.nokia.com'>Qt (LGPL)</a></li>"
                                                   "<li><a href='http://bulletphysics.org'>Bullet Physic Library</a></li>"
                                                   "<li><a href='http://elf-stone.com/glee.php'>OpenGL Easy Extension Library</a></li>"
                                                   "</ul></p>"
                                                   "<p><a href='http://github.com/hkrn/MMDAI/'>MMDAI</a> is a fork project of "
                                                   "<a href='http://www.mmdagent.jp'>MMDAgent</a></p>"));
}

void QMAWindow::receiveEvent(const QString &type,
                             const QStringList &arguments)
{
  if (type == MMDAGENT_EVENT_MODEL_ADD) {
    QString name = arguments.at(0);
    QAction *action = new QAction(name, this);
    action->setStatusTip(tr("Select a model %1").arg(name));
    connect(action, SIGNAL(triggered()), this, SLOT(selectObject()));
    m_selectModelMenu->addAction(action);
  }
  else if (type == MMDAGENT_EVENT_MODEL_DELETE) {
    QString name = arguments.at(0);
    QAction *actionToRemove = NULL;
    foreach (QAction *action, m_selectModelMenu->actions()) {
      if (action->text() == name) {
        actionToRemove = action;
        break;
      }
    }
    if (actionToRemove != NULL)
      m_selectModelMenu->removeAction(actionToRemove);
  }
}

void QMAWindow::createActions()
{
  QAction *action = NULL;
  QList<QKeySequence> shortcuts;

  action = new QAction(tr("Insert to the all models"), this);
  action->setStatusTip(tr("Insert a motion to the all models."));
  action->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_V));
  connect(action, SIGNAL(triggered()), this, SLOT(insertMotionToAllModels()));
  m_insertMotionToAllAction = action;

  action = new QAction(tr("Insert to the selected model"), this);
  action->setStatusTip(tr("Insert a motion to the selected model. If an object is not selected, do nothing."));
  action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_V));
  connect(action, SIGNAL(triggered()), this, SLOT(insertMotionToSelectedModel()));
  m_insertMotionToSelectedAction = action;

  action = new QAction(tr("Add Model"), this);
  action->setStatusTip(tr("Add a PMD model to the scene (Maximum is 20)."));
  action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_M));
  connect(action, SIGNAL(triggered()), this, SLOT(addModel()));
  m_addModelAction = action;

  action = new QAction(tr("Set Stage"), this);
  action->setStatusTip(tr("Set or replace a stage to the scene."));
  action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
  connect(action, SIGNAL(triggered()), this, SLOT(setStage()));
  m_setStageAction = action;

  action = new QAction(tr("Set Floor"), this);
  action->setStatusTip(tr("Set or replace a floor to the scene."));
  action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F));
  connect(action, SIGNAL(triggered()), this, SLOT(setFloor()));
  m_setFloorAction = action;

  action = new QAction(tr("Set Background"), this);
  action->setStatusTip(tr("Set or replace a floor to the scene."));
  action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_B));
  connect(action, SIGNAL(triggered()), this, SLOT(setBackground()));
  m_setBackgroundAction = action;

  action = new QAction(tr("Increase edge thin"), this);
  action->setStatusTip(tr("Increase light edge thin."));
  action->setShortcut(Qt::Key_E);
  connect(action, SIGNAL(triggered()), this, SLOT(increaseEdgeThin()));
  m_increaseEdgeThinAction = action;

  action = new QAction(tr("Decrease edge thin"), this);
  action->setStatusTip(tr("Decrease light edge thin."));
  action->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_E));
  connect(action, SIGNAL(triggered()), this, SLOT(decreaseEdgeThin()));
  m_decreaseEdgeThinAction = action;

  action = new QAction(tr("Toggle display bone"), this);
  action->setStatusTip(tr("Enable / Disable displaying bones of the models."));
  action->setShortcut(Qt::Key_B);
  connect(action, SIGNAL(triggered()), this, SLOT(toggleDisplayBone()));
  m_toggleDisplayBoneAction = action;

  action = new QAction(tr("Toggle rigid body"), this);
  action->setStatusTip(tr("Enable / Disable displaying rigid body of the models."));
  action->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_W));
  connect(action, SIGNAL(triggered()), this, SLOT(toggleDisplayRigidBody()));
  m_toggleDisplayRigidBodyAction = action;

  action = new QAction(tr("Toggle physic simulation"), this);
  action->setStatusTip(tr("Enable / Disable physic simulation using Bullet."));
  action->setShortcut(Qt::Key_P);
  connect(action, SIGNAL(triggered()), this, SLOT(togglePhysicSimulation()));
  m_togglePhysicSimulationAction = action;

  action = new QAction(tr("Toggle shadow mapping"), this);
  action->setStatusTip(tr("Enable / Disable shadow mapping."));
  action->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_X));
  connect(action, SIGNAL(triggered()), this, SLOT(toggleShadowMapping()));
  m_toggleShadowMappingAction = action;

  action = new QAction(tr("Toggle shadow mapping light first"), this);
  action->setStatusTip(tr("Enable / Disable shadow mapping light first."));
  action->setShortcut(Qt::Key_X);
  connect(action, SIGNAL(triggered()), this, SLOT(toggleShadowMappingLightFirst()));
  m_toggleShadowMappingFirstAction = action;

  action = new QAction(tr("Toggle Fullscreen"), this);
  action->setStatusTip("Enable / Disable fullscreen");
  action->setShortcut(Qt::Key_F);
  connect(action, SIGNAL(triggered()), this, SLOT(toggleFullScreen()));
  m_toggleFullScreenAction = action;

  action = new QAction(tr("Zoom in"), this);
  action->setStatusTip(tr("Zoom in the scene."));
  action->setShortcut(Qt::Key_Plus);
  connect(action, SIGNAL(triggered()), this, SLOT(zoomIn()));
  m_zoomInAction = action;

  action = new QAction(tr("Zoom out"), this);
  action->setStatusTip(tr("Zoom out the scene."));
  action->setShortcut(Qt::Key_Minus);
  connect(action, SIGNAL(triggered()), this, SLOT(zoomOut()));
  m_zoomOutAction = action;

  action = new QAction(tr("Rotate up"), this);
  action->setStatusTip("Rotate a model up.");
  action->setShortcut(Qt::Key_Up);
  connect(action, SIGNAL(triggered()), this, SLOT(rotateUp()));
  m_rotateUpAction = action;

  action = new QAction(tr("Rotate down"), this);
  action->setStatusTip("Rotate a model down.");
  action->setShortcut(Qt::Key_Down);
  connect(action, SIGNAL(triggered()), this, SLOT(rotateDown()));
  m_rotateDownAction = action;

  action = new QAction(tr("Rotate Left"), this);
  action->setStatusTip("Rotate a model left.");
  action->setShortcut(Qt::Key_Left);
  connect(action, SIGNAL(triggered()), this, SLOT(rotateLeft()));
  m_rotateLeftAction = action;

  action = new QAction(tr("Rotate right"), this);
  action->setStatusTip("Rotate a model right.");
  action->setShortcut(Qt::Key_Right);
  connect(action, SIGNAL(triggered()), this, SLOT(rotateRight()));
  m_rotateRightAction = action;

  action = new QAction(tr("Translate up"), this);
  action->setStatusTip("Translate a model up.");
  action->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Up));
  connect(action, SIGNAL(triggered()), this, SLOT(translateUp()));
  m_translateUpAction = action;

  action = new QAction(tr("Translate down"), this);
  action->setStatusTip("Rotate a model down.");
  action->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Down));
  connect(action, SIGNAL(triggered()), this, SLOT(translateDown()));
  m_translateDownAction = action;

  action = new QAction(tr("Translate left"), this);
  action->setStatusTip("Rotate a model left.");
  action->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Left));
  connect(action, SIGNAL(triggered()), this, SLOT(translateLeft()));
  m_translateLeftAction = action;

  action = new QAction(tr("Translate right"), this);
  action->setStatusTip("Rotate a model right.");
  action->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Right));
  connect(action, SIGNAL(triggered()), this, SLOT(translateRight()));
  m_translateRightAction = action;

  action = new QAction(tr("Change selected model"), this);
  action->setStatusTip("Change selected model to the specified model. If an object is not selected, do nothing.");
  action->setShortcut(QKeySequence(Qt::ALT + Qt::Key_M));
  connect(action, SIGNAL(triggered()), this, SLOT(changeSelectedObject()));
  m_changeSelectedObjectAction = action;

  action = new QAction(tr("Delete selected model"), this);
  action->setStatusTip("Delete selected model from the scene. If an object is not selected, do nothing.");
  action->setShortcut(Qt::Key_Delete);
  connect(action, SIGNAL(triggered()), this, SLOT(deleteSelectedObject()));
  m_deleteSelectedObjectAction = action;

  action = new QAction(tr("E&xit"), this);
  action->setShortcuts(QKeySequence::Quit);
  action->setStatusTip(tr("Exit the application."));
  connect(action, SIGNAL(triggered()), this, SLOT(close()));
  m_exitAction = action;

  shortcuts.append(QKeySequence(Qt::ALT + Qt::Key_Question));
  shortcuts.append(QKeySequence(Qt::ALT + Qt::Key_Slash));
  action = new QAction(tr("&About"), this);
  action->setStatusTip(tr("Show the application's About box."));
  action->setShortcuts(shortcuts);
  connect(action, SIGNAL(triggered()), this, SLOT(about()));
  m_aboutAction = action;

  action = new QAction(tr("About &Qt"), this);
  action->setStatusTip(tr("Show the Qt library's About box."));
  connect(action, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
  m_aboutQtAction = action;

  connect(m_widget, SIGNAL(pluginEventPost(QString,QStringList)),
          this, SLOT(receiveEvent(QString,QStringList)));
}

void QMAWindow::setEdgeThin(int n)
{
  SceneController *controller = m_widget->getSceneController();
  Option *option = controller->getOption();
  int count = controller->countPMDObjects();
  float thin = option->getCartoonEdgeWidth() * option->getCartoonEdgeStep() * n;
  option->setCartoonEdgeWidth(thin);
  for (int i = 0; i < count; i++)
    controller->getPMDObject(i)->getPMDModel()->setEdgeThin(thin);
}

void QMAWindow::createMenu()
{
  m_fileMenu = menuBar()->addMenu(tr("&File"));
  m_motionMenu = m_fileMenu->addMenu(tr("Add motion"));
  m_motionMenu->addAction(m_insertMotionToAllAction);
  m_motionMenu->addAction(m_insertMotionToSelectedAction);
  m_fileMenu->addAction(m_addModelAction);
  m_fileMenu->addAction(m_setStageAction);
  m_fileMenu->addAction(m_setFloorAction);
  m_fileMenu->addAction(m_setBackgroundAction);
  m_fileMenu->addSeparator();
  m_fileMenu->addAction(m_exitAction);

  menuBar()->addSeparator();
  m_sceneMenu = menuBar()->addMenu("&Scene");
  m_sceneMenu->addAction(m_toggleFullScreenAction);
  m_sceneMenu->addAction(m_zoomInAction);
  m_sceneMenu->addAction(m_zoomOutAction);
  m_sceneMenu->addAction(m_rotateUpAction);
  m_sceneMenu->addAction(m_rotateDownAction);
  m_sceneMenu->addAction(m_rotateLeftAction);
  m_sceneMenu->addAction(m_rotateRightAction);
  m_sceneMenu->addAction(m_translateUpAction);
  m_sceneMenu->addAction(m_translateDownAction);
  m_sceneMenu->addAction(m_translateLeftAction);
  m_sceneMenu->addAction(m_translateRightAction);
  m_sceneMenu->addAction(m_increaseEdgeThinAction);
  m_sceneMenu->addAction(m_decreaseEdgeThinAction);
  m_sceneMenu->addAction(m_togglePhysicSimulationAction);
  m_sceneMenu->addAction(m_toggleShadowMappingAction);
  m_sceneMenu->addAction(m_toggleShadowMappingFirstAction);
  m_sceneMenu->addAction(m_toggleDisplayBoneAction);
  m_sceneMenu->addAction(m_toggleDisplayRigidBodyAction);

  menuBar()->addSeparator();
  m_modelMenu = menuBar()->addMenu("&Model");
  m_selectModelMenu = m_modelMenu->addMenu("Select model");
  m_modelMenu->addAction(m_changeSelectedObjectAction);
  m_modelMenu->addAction(m_deleteSelectedObjectAction);

  menuBar()->addSeparator();
  m_helpMenu = menuBar()->addMenu(tr("&Help"));
  m_helpMenu->addAction(m_aboutAction);
  m_helpMenu->addAction(m_aboutQtAction);
}

void QMAWindow::readSetting()
{
  QPoint point = m_settings->value("window/pos", QPoint(200, 200)).toPoint();
  QSize size = m_settings->value("window/size", QSize(800, 480)).toSize();
  resize(size);
  move(point);
}

void QMAWindow::writeSetting()
{
  m_settings->value("window/pos", pos());
  m_settings->value("window/size", size());
}
