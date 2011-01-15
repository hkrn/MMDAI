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
/* - Neither the name of the MMDAgent project team nor the names of  */
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
    QMainWindow(parent)
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
      if (object->isEnable() && object->allowMotionFileDrop())
        controller->addMotion(object, filename);
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
    if (selectedObject != NULL)
      controller->addMotion(selectedObject, fileName.toUtf8().constData());
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
    m_widget->getSceneController()->addModel(fileName.toUtf8().constData());
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
    m_widget->getSceneController()->loadStage(fileName.toUtf8().constData());
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
    m_widget->getSceneController()->loadFloor(fileName.toUtf8().constData());
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
    m_widget->getSceneController()->loadBackground(fileName.toUtf8().constData());
  }
}

void QMAWindow::about()
{
  QMessageBox::about(this, tr("About Application"), tr("<strong>QtMMDAI</strong> 0.1 (CodeName: 40mP)<br>"
        "<small><br>"
        "Copyright 2010-2011<br><br>"
        "Nagoya Institute of Technology Department of Computer Science<br>"
        "hkrn (@hikarincl2)<br>"
        "<br>"
        "All rights reserved"
        "</small>"));
}

void QMAWindow::receiveEvent(SceneController */*controller*/,
                             const QString &/*type*/,
                             const QString &/*arguments*/)
{
}

void QMAWindow::createActions()
{
  QAction *action = NULL;

  action = new QAction(tr("Insert to the all models"), this);
  action->setStatusTip(tr("Insert the motion to the all models"));
  connect(action, SIGNAL(triggered()), this, SLOT(insertMotionToAllModels()));
  m_insertMotionToAllAction = action;

  action = new QAction(tr("Insert to the selected model"), this);
  action->setStatusTip(tr("Insert the motion to the selected model"));
  connect(action, SIGNAL(triggered()), this, SLOT(insertMotionToSelectedModel()));
  m_insertMotionToSelectedAction = action;

  action = new QAction(tr("Add &Model"), this);
  action->setStatusTip(tr("Add a PMD model to the scene"));
  connect(action, SIGNAL(triggered()), this, SLOT(addModel()));
  m_addModelAction = action;

  action = new QAction(tr("Set &Stage"), this);
  action->setStatusTip(tr("Set a stage to the scene"));
  connect(action, SIGNAL(triggered()), this, SLOT(setStage()));
  m_setStageAction = action;

  action = new QAction(tr("Set &Floor"), this);
  action->setStatusTip(tr("Set a floor to the scene"));
  connect(action, SIGNAL(triggered()), this, SLOT(setFloor()));
  m_setFloorAction = action;

  action = new QAction(tr("Set &Background"), this);
  action->setStatusTip(tr("Set a floor to the scene"));
  connect(action, SIGNAL(triggered()), this, SLOT(setBackground()));
  m_setBackgroundAction = action;

  action = new QAction(tr("E&xit"), this);
  action->setShortcuts(QKeySequence::Quit);
  action->setStatusTip(tr("Exit the application"));
  connect(action, SIGNAL(triggered()), this, SLOT(close()));
  m_exitAction = action;

  action = new QAction(tr("&About"), this);
  action->setStatusTip(tr("Show the application's About box"));
  connect(action, SIGNAL(triggered()), this, SLOT(about()));
  m_aboutAction = action;

  action = new QAction(tr("About &Qt"), this);
  action->setStatusTip(tr("Show the Qt library's About box"));
  connect(action, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
  m_aboutQtAction = action;

  connect(m_widget, SIGNAL(pluginEventPost(SceneController*,QString,QString)),
          this, SLOT(receiveEvent(SceneController*,QString,QString)));
}

void QMAWindow::createMenu()
{
  m_fileMenu = menuBar()->addMenu(tr("&File"));
  m_motionMenu = m_fileMenu->addMenu(tr("Add Motion"));
  m_motionMenu->addAction(m_insertMotionToAllAction);
  m_motionMenu->addAction(m_insertMotionToSelectedAction);
  m_fileMenu->addAction(m_addModelAction);
  m_fileMenu->addAction(m_setStageAction);
  m_fileMenu->addAction(m_setFloorAction);
  m_fileMenu->addAction(m_setBackgroundAction);
  m_fileMenu->addSeparator();
  m_fileMenu->addAction(m_exitAction);

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
