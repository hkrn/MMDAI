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

#include "ExtendedSceneWidget.h"
#include "LicenseWidget.h"
#include "LoggerWidget.h"
#include "SceneLoader.h"
#include "Script.h"
#include "util.h"

#include <QtGui/QtGui>
#include <vpvl2/vpvl2.h>

using namespace vpvl2;

namespace
{

void ConstructScriptArguments(const QString &input, QString &command, QList<QVariant> &arguments)
{
    QStringList strings = QString(input).split("|");
    command = strings.first();
    strings.pop_front();
    foreach (QString string, strings)
        arguments << string;
}

static int FindIndexOfActions(IModel *model, const QList<QAction *> &actions)
{
    const QString &name = internal::toQStringFromModel(model);
    int i = 0, found = -1;
    foreach (QAction *action, actions) {
        if (action->text() == name) {
            found = i;
            break;
        }
        i++;
    }
    return found;
}

}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_encoding(0),
    m_factory(0),
    m_settings(QSettings::IniFormat, QSettings::UserScope, qApp->organizationName(), qAppName()),
    m_licenseWidget(0),
    m_loggerWidget(0),
    m_sceneWidget(0),
    m_model(0),
    m_currentFPS(0)
{
    m_encoding = new internal::Encoding();
    m_factory = new vpvl2::Factory(m_encoding);
    m_licenseWidget = new LicenseWidget();
    m_loggerWidget = LoggerWidget::createInstance(&m_settings);
    m_sceneWidget = new ExtendedSceneWidget(m_encoding, m_factory, &m_settings);
    resize(600, 600);
    setMinimumSize(QSize(640, 480));
    setCentralWidget(m_sceneWidget);
    buildMenuBar();
    restoreGeometry(m_settings.value("mainWindow/geometry").toByteArray());
    restoreState(m_settings.value("mainWindow/state").toByteArray());
    setWindowTitle(buildWindowTitle());
    statusBar()->show();
}

MainWindow::~MainWindow()
{
    delete m_menuBar;
    delete m_licenseWidget;
}

void MainWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
        m_sceneWidget->loadFile(action->data().toString());
}

void MainWindow::addRecentFile(const QString &filename)
{
    QStringList files = m_settings.value("mainWindow/recentFiles").toStringList();
    files.removeAll(filename);
    files.prepend(filename);
    while (files.size() > kMaxRecentFiles)
        files.removeLast();
    m_settings.setValue("mainWindow/recentFiles", files);
    updateRecentFiles();
}

void MainWindow::updateRecentFiles()
{
    QStringList files = m_settings.value("mainWindow/recentFiles").toStringList();
    int maxFiles = kMaxRecentFiles, nRecentFiles = qMin(files.size(), maxFiles);
    QFileInfo fileInfo;
    for (int i = 0; i < nRecentFiles; i++) {
        QAction *action = m_actionRecentFiles[i];
        fileInfo.setFile(files[i]);
        action->setText(tr("&%1 %2").arg(i + 1).arg(fileInfo.fileName()));
        action->setData(files[i]);
        action->setVisible(true);
    }
    for (int i = nRecentFiles; i < kMaxRecentFiles; i++)
        m_actionRecentFiles[i]->setVisible(false);
}

void MainWindow::clearRecentFiles()
{
    m_settings.setValue("mainWindow/recentFiles", QStringList());
    updateRecentFiles();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    m_settings.setValue("mainWindow/geometry", saveGeometry());
    m_settings.setValue("mainWindow/state", saveState());
    qApp->sendEvent(m_sceneWidget, event);
    event->accept();
}

void MainWindow::selectCurrentModel()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        const QUuid uuid(action->data().toString());
        IModel *model = m_sceneWidget->sceneLoader()->findModel(uuid);
        m_sceneWidget->setSelectedModel(model);
    }
}

void MainWindow::setCurrentModel(IModel *value)
{
    m_model = value;
    updateInformation();
}

void MainWindow::addModel(IModel *model, const QUuid &uuid)
{
    /* 追加されたモデルをモデル選択のメニューに追加する */
    QString name = internal::toQStringFromModel(model);
    QAction *action = new QAction(name, this);
    action->setData(uuid.toString());
    action->setStatusTip(tr("Select a model %1").arg(name));
    connect(action, SIGNAL(triggered()), this, SLOT(selectCurrentModel()));
    m_menuRetainModels->addAction(action);
    m_sceneWidget->setSelectedModel(model);
}

void MainWindow::deleteModel(IModel *model, const QUuid &uuid)
{
    SceneLoader *loader = m_sceneWidget->sceneLoader();
    /* 削除されるモデルが選択中のモデルと同じなら選択状態を解除しておく(残すと不正アクセスの原因になるので) */
    if (model == loader->selectedModel())
        loader->setSelectedModel(0);
    /* 削除されるモデルをモデル選択のメニューから削除する */
    QAction *actionToRemove = 0;
    const QString &uuidString = uuid.toString();
    foreach (QAction *action, m_menuRetainModels->actions()) {
        if (action->data() == uuidString) {
            actionToRemove = action;
            break;
        }
    }
    if (actionToRemove)
        m_menuRetainModels->removeAction(actionToRemove);
}

void MainWindow::addAsset(IModel *asset, const QUuid &uuid)
{
    /* 追加されたアクセサリをアクセサリ選択のメニューに追加する */
    QString name = internal::toQStringFromModel(asset);
    QAction *action = new QAction(name, this);
    action->setData(uuid.toString());
    action->setStatusTip(tr("Select an asset %1").arg(name));
    //connect(action, SIGNAL(triggered()), this, SLOT(selectCurrentModel()));
    m_menuRetainAssets->addAction(action);
}

void MainWindow::deleteAsset(IModel * /* asset */, const QUuid &uuid)
{
    /* 削除されたアクセサリをアクセサリ選択のメニューから削除する */
    QAction *actionToRemove = 0;
    const QString &uuidString = uuid.toString();
    foreach (QAction *action, m_menuRetainAssets->actions()) {
        if (action->data() == uuidString) {
            actionToRemove = action;
            break;
        }
    }
    if (actionToRemove)
        m_menuRetainAssets->removeAction(actionToRemove);
}


void MainWindow::updateInformation()
{
    setWindowTitle(buildWindowTitle());
}

void MainWindow::connectWidgets()
{
}

void MainWindow::startSceneUpdate()
{
    setWindowTitle(buildWindowTitle(0));
}

void MainWindow::stopSceneUpdate()
{
    setWindowTitle(buildWindowTitle());
}

void MainWindow::updateFPS(int fps)
{
    setWindowTitle(buildWindowTitle(fps));
}

void MainWindow::executeCommand()
{
    QString input = QInputDialog::getText(this, tr("Execute command"), tr("Execute command")), command;
    QList<QVariant> arguments;
    ConstructScriptArguments(input, command, arguments);
    Script *script = m_sceneWidget->script();
    if (script)
        script->handleCommand(command, arguments);
}

void MainWindow::executeEvent()
{
    QString input = QInputDialog::getText(this, tr("Execute event"), tr("Execute event")), event;
    QList<QVariant> arguments;
    ConstructScriptArguments(input, event, arguments);
    Script *script = m_sceneWidget->script();
    if (script)
        script->handleEvent(event, arguments);
}

void MainWindow::selectNextModel()
{
    const QList<QAction *> &actions = m_menuRetainModels->actions();
    if (!actions.isEmpty()) {
        const SceneLoader *loader = m_sceneWidget->sceneLoader();
        int index = FindIndexOfActions(loader->selectedModel(), actions);
        if (index == -1 || index == actions.length() - 1)
            m_sceneWidget->setSelectedModel(loader->findModel(actions.first()->text()));
        else
            m_sceneWidget->setSelectedModel(loader->findModel(actions.at(index + 1)->text()));
    }
}

void MainWindow::selectPreviousModel()
{
    const QList<QAction *> &actions = m_menuRetainModels->actions();
    if (!actions.isEmpty()) {
        const SceneLoader *loader = m_sceneWidget->sceneLoader();
        int index = FindIndexOfActions(loader->selectedModel(), actions);
        if (index == -1 || index == 0)
            m_sceneWidget->setSelectedModel(loader->findModel(actions.last()->text()));
        else
            m_sceneWidget->setSelectedModel(loader->findModel(actions.at(index - 1)->text()));
    }
}

const QString MainWindow::buildWindowTitle()
{
    QString title = qAppName();
    if (m_model)
        title += " - " + internal::toQStringFromModel(m_model);
    return title;
}

const QString MainWindow::buildWindowTitle(int fps)
{
    return buildWindowTitle() + tr(" (FPS: %1)").arg(fps);
}

void MainWindow::buildMenuBar()
{
    connect(m_sceneWidget, SIGNAL(initailizeGLContextDidDone()), this, SLOT(connectSceneLoader()));
    m_actionAddModel = new QAction(this);
    connect(m_actionAddModel, SIGNAL(triggered()), m_sceneWidget, SLOT(addModel()));
    m_actionAddAsset = new QAction(this);
    connect(m_actionAddAsset, SIGNAL(triggered()), m_sceneWidget, SLOT(addAsset()));
    m_actionInsertToAllModels = new QAction(this);
    connect(m_actionInsertToAllModels, SIGNAL(triggered()), m_sceneWidget, SLOT(insertMotionToAllModels()));
    m_actionInsertToSelectedModel = new QAction(this);
    connect(m_actionInsertToSelectedModel, SIGNAL(triggered()), m_sceneWidget, SLOT(insertMotionToSelectedModel()));
    m_actionSetCamera = new QAction(this);
    connect(m_actionSetCamera, SIGNAL(triggered()), m_sceneWidget, SLOT(setCamera()));
    m_actionExit = new QAction(this);
    m_actionExit->setMenuRole(QAction::QuitRole);
    connect(m_actionExit, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

    m_actionLoadScript = new QAction(this);
    connect(m_actionLoadScript, SIGNAL(triggered()), m_sceneWidget, SLOT(loadScript()));
    m_actionPlay = new QAction(this);
    connect(m_actionPlay, SIGNAL(triggered()), m_sceneWidget, SLOT(play()));
    m_actionPause = new QAction(this);
    connect(m_actionPause, SIGNAL(triggered()), m_sceneWidget, SLOT(pause()));
    m_actionStop = new QAction(this);
    connect(m_actionStop, SIGNAL(triggered()), m_sceneWidget, SLOT(stop()));
    m_actionEnableAcceleration = new QAction(this);
    m_actionEnableAcceleration->setCheckable(true);
    m_actionEnableAcceleration->setEnabled(Scene::isAcceleratorSupported());
    m_actionShowModelDialog = new QAction(this);
    m_actionShowModelDialog->setCheckable(true);
    m_actionShowModelDialog->setChecked(m_sceneWidget->showModelDialog());
    connect(m_actionShowModelDialog, SIGNAL(triggered(bool)), m_sceneWidget, SLOT(setShowModelDialog(bool)));
    m_actionExecuteCommand = new QAction(this);
    connect(m_actionExecuteCommand, SIGNAL(triggered()), this, SLOT(executeCommand()));
    m_actionExecuteEvent = new QAction(this);
    connect(m_actionExecuteEvent, SIGNAL(triggered()), this, SLOT(executeEvent()));

    m_actionZoomIn = new QAction(this);
    connect(m_actionZoomIn, SIGNAL(triggered()), m_sceneWidget, SLOT(zoomIn()));
    m_actionZoomOut = new QAction(this);
    connect(m_actionZoomOut, SIGNAL(triggered()), m_sceneWidget, SLOT(zoomOut()));
    m_actionRotateUp = new QAction(this);
    connect(m_actionRotateUp, SIGNAL(triggered()), m_sceneWidget, SLOT(rotateUp()));
    m_actionRotateDown = new QAction(this);
    connect(m_actionRotateDown, SIGNAL(triggered()), m_sceneWidget, SLOT(rotateDown()));
    m_actionRotateLeft = new QAction(this);
    connect(m_actionRotateLeft, SIGNAL(triggered()), m_sceneWidget, SLOT(rotateLeft()));
    m_actionRotateRight = new QAction(this);
    connect(m_actionRotateRight, SIGNAL(triggered()), m_sceneWidget, SLOT(rotateRight()));
    m_actionTranslateUp = new QAction(this);
    connect(m_actionTranslateUp, SIGNAL(triggered()), m_sceneWidget, SLOT(translateUp()));
    m_actionTranslateDown = new QAction(this);
    connect(m_actionTranslateDown, SIGNAL(triggered()), m_sceneWidget, SLOT(translateDown()));
    m_actionTranslateLeft = new QAction(this);
    connect(m_actionTranslateLeft, SIGNAL(triggered()), m_sceneWidget, SLOT(translateLeft()));
    m_actionTranslateRight = new QAction(this);
    connect(m_actionTranslateRight, SIGNAL(triggered()), m_sceneWidget, SLOT(translateRight()));
    m_actionResetCamera = new QAction(this);
    connect(m_actionResetCamera, SIGNAL(triggered()), m_sceneWidget, SLOT(resetCamera()));
    m_actionShowBones = new QAction(this);

    m_actionSelectNextModel = new QAction(this);
    connect(m_actionSelectNextModel, SIGNAL(triggered()), this, SLOT(selectNextModel()));
    m_actionSelectPreviousModel = new QAction(this);
    connect(m_actionSelectPreviousModel, SIGNAL(triggered()), this, SLOT(selectPreviousModel()));
    m_actionRevertSelectedModel = new QAction(this);
    connect(m_actionRevertSelectedModel, SIGNAL(triggered()), m_sceneWidget, SLOT(revertSelectedModel()));
    m_actionDeleteSelectedModel = new QAction(this);
    connect(m_actionDeleteSelectedModel, SIGNAL(triggered()), m_sceneWidget, SLOT(deleteSelectedModel()));

    m_actionShowLogMessage = new QAction(this);
    connect(m_actionShowLogMessage, SIGNAL(triggered()), m_loggerWidget, SLOT(show()));
    m_actionEnableTransparent = new QAction(this);
    m_actionEnableTransparent->setCheckable(true);
    m_actionEnableTransparent->setChecked(m_sceneWidget->isTransparentEnabled());
    connect(m_actionEnableTransparent, SIGNAL(triggered(bool)), this, SLOT(toggleTransparentWindow(bool)));
    m_actionEnableMoveGesture = new QAction(this);
    m_actionEnableMoveGesture->setCheckable(true);
    m_actionEnableMoveGesture->setChecked(m_sceneWidget->isMoveGestureEnabled());
    connect(m_actionEnableMoveGesture, SIGNAL(triggered(bool)), m_sceneWidget, SLOT(setMoveGestureEnable(bool)));
    m_actionEnableRotateGesture = new QAction(this);
    m_actionEnableRotateGesture->setCheckable(true);
    m_actionEnableRotateGesture->setChecked(m_sceneWidget->isRotateGestureEnabled());
    connect(m_actionEnableRotateGesture, SIGNAL(triggered(bool)), m_sceneWidget, SLOT(setRotateGestureEnable(bool)));
    m_actionEnableScaleGesture = new QAction(this);
    m_actionEnableScaleGesture->setCheckable(true);
    m_actionEnableScaleGesture->setChecked(m_sceneWidget->isRotateGestureEnabled());
    connect(m_actionEnableScaleGesture, SIGNAL(triggered(bool)), m_sceneWidget, SLOT(setScaleGestureEnable(bool)));

    m_actionClearRecentFiles = new QAction(this);
    connect(m_actionClearRecentFiles, SIGNAL(triggered()), this, SLOT(clearRecentFiles()));
    m_actionAbout = new QAction(this);
    connect(m_actionAbout, SIGNAL(triggered()), m_licenseWidget, SLOT(show()));
    m_actionAbout->setMenuRole(QAction::AboutRole);
    m_actionAboutQt = new QAction(this);
    connect(m_actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    m_actionAboutQt->setMenuRole(QAction::AboutQtRole);

#ifdef Q_OS_MACX
    m_menuBar = new QMenuBar(0);
#else
    m_menuBar = menuBar();
#endif
    m_menuFile = new QMenu(this);
    m_menuFile->addAction(m_actionLoadScript);
    m_menuFile->addSeparator();
    m_menuFile->addAction(m_actionAddModel);
    m_menuFile->addAction(m_actionAddAsset);
    m_menuFile->addSeparator();
    m_menuFile->addAction(m_actionInsertToAllModels);
    m_menuFile->addAction(m_actionInsertToSelectedModel);
    m_menuFile->addAction(m_actionSetCamera);
    m_menuRecentFiles = new QMenu(this);
    for (int i = 0; i < kMaxRecentFiles; i++) {
        QAction *action = new QAction(this);
        connect(action, SIGNAL(triggered()), this, SLOT(openRecentFile()));
        action->setVisible(false);
        m_actionRecentFiles[i] = action;
        m_menuRecentFiles->addAction(action);
    }
    m_menuRecentFiles->addSeparator();
    m_menuRecentFiles->addAction(m_actionClearRecentFiles);
    m_menuFile->addSeparator();
    m_menuFile->addMenu(m_menuRecentFiles);
    m_menuFile->addSeparator();
    m_menuFile->addAction(m_actionExit);
    m_menuBar->addMenu(m_menuFile);
    m_menuScript = new QMenu(this);
    m_menuScript->addAction(m_actionPlay);
    m_menuScript->addAction(m_actionPause);
    m_menuScript->addAction(m_actionStop);
    m_menuScript->addSeparator();
    m_menuScript->addAction(m_actionEnableAcceleration);
    m_menuScript->addAction(m_actionShowModelDialog);
    m_menuScript->addSeparator();
    m_menuScript->addAction(m_actionExecuteCommand);
    m_menuScript->addAction(m_actionExecuteEvent);
    m_menuBar->addMenu(m_menuScript);
    m_menuScene = new QMenu(this);
    m_menuScene->addAction(m_actionZoomIn);
    m_menuScene->addAction(m_actionZoomOut);
    m_menuScene->addSeparator();
    m_menuScene->addAction(m_actionRotateUp);
    m_menuScene->addAction(m_actionRotateDown);
    m_menuScene->addAction(m_actionRotateLeft);
    m_menuScene->addAction(m_actionRotateRight);
    m_menuScene->addSeparator();
    m_menuScene->addAction(m_actionTranslateUp);
    m_menuScene->addAction(m_actionTranslateDown);
    m_menuScene->addAction(m_actionTranslateLeft);
    m_menuScene->addAction(m_actionTranslateRight);
    m_menuScene->addSeparator();
    m_menuScene->addAction(m_actionResetCamera);
    m_menuBar->addMenu(m_menuScene);
    m_menuModel = new QMenu(this);
    m_menuRetainModels = new QMenu(this);
    m_menuModel->addMenu(m_menuRetainModels);
    m_menuRetainAssets = new QMenu(this);
    // if (Asset::isSupported())
        m_menuScene->addMenu(m_menuRetainAssets);
    m_menuModel->addAction(m_actionSelectNextModel);
    m_menuModel->addAction(m_actionSelectPreviousModel);
    m_menuModel->addSeparator();
    m_menuModel->addAction(m_actionRevertSelectedModel);
    m_menuModel->addAction(m_actionDeleteSelectedModel);
    m_menuBar->addMenu(m_menuModel);
    m_menuView = new QMenu(this);
    m_menuView->addAction(m_actionShowLogMessage);
    //m_menuView->addAction(m_actionEnableTransparent);
    m_menuView->addSeparator();
    m_menuView->addAction(m_actionEnableMoveGesture);
    m_menuView->addAction(m_actionEnableRotateGesture);
    m_menuView->addAction(m_actionEnableScaleGesture);
    m_menuBar->addMenu(m_menuView);
    m_menuHelp = new QMenu(this);
    m_menuHelp->addAction(m_actionAbout);
    m_menuHelp->addAction(m_actionAboutQt);
    m_menuBar->addMenu(m_menuHelp);

    connect(m_sceneWidget, SIGNAL(fileDidLoad(QString)), this, SLOT(addRecentFile(QString)));
    connect(m_sceneWidget, SIGNAL(fpsDidUpdate(int)), this, SLOT(updateFPS(int)));
    connect(m_sceneWidget, SIGNAL(scriptDidLoaded(QString)), this, SLOT(disableAcceleration()));

    retranslate();
}

void MainWindow::connectSceneLoader()
{
    SceneLoader *loader = m_sceneWidget->sceneLoader();
    connect(loader, SIGNAL(modelDidAdd(vpvl2::IModel*,QUuid)), this, SLOT(addModel(vpvl2::IModel*,QUuid)));
    connect(loader, SIGNAL(modelWillDelete(vpvl2::IModel*,QUuid)), this, SLOT(deleteModel(vpvl2::IModel*,QUuid)));
    connect(loader, SIGNAL(assetDidAdd(vpvl2::IModel*,QUuid)), this, SLOT(addAsset(vpvl2::IModel*,QUuid)));
    connect(loader, SIGNAL(assetWillDelete(vpvl2::IModel*,QUuid)), this, SLOT(deleteAsset(vpvl2::IModel*,QUuid)));
    connect(loader, SIGNAL(modelDidSelect(vpvl2::IModel*,SceneLoader*)), this, SLOT(setCurrentModel(vpvl2::IModel*)));
    connect(m_actionEnableAcceleration, SIGNAL(triggered(bool)), loader, SLOT(setAccelerationEnabled(bool)));
}

void MainWindow::disableAcceleration()
{
    m_actionEnableAcceleration->setEnabled(false);
}

void MainWindow::toggleTransparentWindow(bool value)
{
    Qt::WindowFlags flags = windowFlags();
    if (value) {
        statusBar()->hide();
        flags |= Qt::FramelessWindowHint;
    }
    else {
        statusBar()->show();
        flags ^= Qt::FramelessWindowHint;
    }
    //setWindowFlags(flags);
    setAttribute(Qt::WA_TranslucentBackground, value);
    m_sceneWidget->setTransparentEnable(value);
}

void MainWindow::retranslate()
{
    m_actionLoadScript->setText(tr("Load script"));
    m_actionLoadScript->setToolTip(tr("Load a script"));
    m_actionLoadScript->setShortcut(tr("Ctrl+Shift+S"));
    m_actionAddModel->setText(tr("Add model"));
    m_actionAddModel->setToolTip(tr("Add a model to the scene."));
    m_actionAddModel->setShortcut(tr("Ctrl+Shift+M"));
    m_actionAddAsset->setText(tr("Add asset"));
    m_actionAddAsset->setToolTip(tr("Add an asset to the scene."));
    m_actionAddAsset->setShortcut(tr("Ctrl+Shift+A"));
    // m_actionAddAsset->setEnabled(Asset::isSupported());
    m_actionInsertToAllModels->setText(tr("Insert to all models"));
    m_actionInsertToAllModels->setToolTip(tr("Insert a motion to the all models."));
    m_actionInsertToAllModels->setShortcut(tr("Ctrl+Shift+V"));
    m_actionInsertToSelectedModel->setText(tr("Insert to selected model"));
    m_actionInsertToSelectedModel->setToolTip(tr("Insert a motion to the selected model."));
    m_actionInsertToSelectedModel->setShortcut(tr("Ctrl+Alt+Shift+V"));
    m_actionSetCamera->setText(tr("Set camera motion"));
    m_actionSetCamera->setToolTip(tr("Set a camera motion to the scene."));
    m_actionSetCamera->setShortcut(tr("Ctrl+Shift+C"));
    m_actionExit->setText(tr("Exit"));
    m_actionExit->setToolTip(tr("Exit this application."));
    m_actionExit->setShortcut(tr("Ctrl+Q"));
    m_actionPlay->setText(tr("Play"));
    m_actionPlay->setStatusTip(tr("Play current scene."));
    m_actionPause->setText(tr("Pause"));
    m_actionPause->setStatusTip(tr("Pause current scene."));
    m_actionStop->setText(tr("Stop"));
    m_actionStop->setStatusTip(tr("Stop current scene."));
    m_actionEnableAcceleration->setText(tr("Enable acceleration"));
    m_actionEnableAcceleration->setStatusTip(tr("Enable or disable acceleration using OpenCL if supported."));
    m_actionShowModelDialog->setText(tr("Show model dialog"));
    m_actionShowModelDialog->setStatusTip(tr("Show or hide model dialog when the model is loaded."));
    m_actionExecuteCommand->setText(tr("Execute command"));
    m_actionExecuteCommand->setStatusTip(tr("Execute command to the script."));
    m_actionExecuteEvent->setText(tr("Execute event"));
    m_actionExecuteEvent->setStatusTip(tr("Execute event to the script."));
    m_actionZoomIn->setText(tr("Zoom in"));
    m_actionZoomIn->setStatusTip(tr("Zoom in the scene."));
    m_actionZoomIn->setShortcut(tr("+"));
    m_actionZoomOut->setText(tr("Zoom out"));
    m_actionZoomOut->setStatusTip(tr("Zoom out the scene."));
    m_actionZoomOut->setShortcut(tr("-"));
    m_actionRotateUp->setText(tr("Rotate up"));
    m_actionRotateUp->setStatusTip(tr("Rotate up the scene."));
    m_actionRotateUp->setShortcut(tr("Ctrl+Up"));
    m_actionRotateDown->setText(tr("Rotate down"));
    m_actionRotateDown->setStatusTip(tr("Rotate down the scene."));
    m_actionRotateDown->setShortcut(tr("Ctrl+Down"));
    m_actionRotateLeft->setText(tr("Rotate left"));
    m_actionRotateLeft->setStatusTip(tr("Rotate left the scene."));
    m_actionRotateLeft->setShortcut(tr("Ctrl+Left"));
    m_actionRotateRight->setText(tr("Rotate right"));
    m_actionRotateRight->setStatusTip(tr("Rotate right the scene."));
    m_actionRotateRight->setShortcut(tr("Ctrl+Right"));
    m_actionTranslateUp->setText(tr("Translate up"));
    m_actionTranslateUp->setStatusTip(tr("Translate up the scene."));
    m_actionTranslateUp->setShortcut(tr("Shift+Up"));
    m_actionTranslateDown->setText(tr("Translate down"));
    m_actionTranslateDown->setStatusTip(tr("Translate down the scene."));
    m_actionTranslateDown->setShortcut(tr("Shift+Down"));
    m_actionTranslateLeft->setText(tr("Translate left"));
    m_actionTranslateLeft->setStatusTip(tr("Translate left the scene."));
    m_actionTranslateLeft->setShortcut(tr("Shift+Left"));
    m_actionTranslateRight->setText(tr("Translate right"));
    m_actionTranslateRight->setStatusTip(tr("Translate right the scene."));
    m_actionTranslateRight->setShortcut(tr("Shift+Right"));
    m_actionResetCamera->setText(tr("Reset camera"));
    m_actionResetCamera->setStatusTip(tr("Reset camera perspective."));
    m_actionSelectNextModel->setText(tr("Select next model"));
    m_actionSelectNextModel->setStatusTip(tr("Select the next model."));
    m_actionSelectNextModel->setShortcut(QKeySequence::SelectNextPage);
    m_actionSelectPreviousModel->setText(tr("Select previous model"));
    m_actionSelectPreviousModel->setStatusTip(tr("Select the previous model."));
    m_actionSelectPreviousModel->setShortcut(QKeySequence::SelectPreviousPage);
    m_actionRevertSelectedModel->setText(tr("Revert selected model"));
    m_actionRevertSelectedModel->setStatusTip(tr("Revert the selected model."));
    m_actionDeleteSelectedModel->setText(tr("Delete selected model"));
    m_actionDeleteSelectedModel->setStatusTip(tr("Delete the selected model from the scene."));
    m_actionDeleteSelectedModel->setShortcut(tr("Ctrl+Shift+Backspace"));
    m_actionShowLogMessage->setText(tr("Open log message window"));
    m_actionShowLogMessage->setToolTip(tr("Open a window of log messages such as script."));
    m_actionEnableTransparent->setText(tr("Enable transparent window"));
    m_actionEnableTransparent->setToolTip(tr("Enable making a window transparent."));
    m_actionEnableMoveGesture->setText(tr("Enable move gesture"));
    m_actionEnableMoveGesture->setStatusTip(tr("Enable moving scene/model/bone by pan gesture."));
    m_actionEnableRotateGesture->setText(tr("Enable rotate gesture"));
    m_actionEnableRotateGesture->setStatusTip(tr("Enable rotate scene/model/bone by pinch gesture."));
    m_actionEnableScaleGesture->setText(tr("Enable scale gesture"));
    m_actionEnableScaleGesture->setStatusTip(tr("Enable scale scene by pinch gesture."));
    m_actionAbout->setText(tr("About"));
    m_actionAbout->setStatusTip(tr("About this application."));
    m_actionAbout->setShortcut(tr("Alt+Q, Alt+/"));
    m_actionAboutQt->setText(tr("About Qt"));
    m_actionAboutQt->setStatusTip(tr("About Qt."));
    m_actionClearRecentFiles->setText(tr("Clear recent files history"));
    m_actionClearRecentFiles->setStatusTip(tr("Clear the history of recently opened files."));
    m_menuFile->setTitle(tr("&File"));
    m_menuScript->setTitle(tr("Script"));
    m_menuScene->setTitle(tr("&Scene"));
    m_menuModel->setTitle(tr("&Model"));
    m_menuView->setTitle(tr("&View"));
    m_menuRetainAssets->setTitle(tr("Select asset"));
    m_menuRetainModels->setTitle(tr("Select model"));
    m_menuRecentFiles->setTitle(tr("Open recent files"));
    m_menuHelp->setTitle(tr("&Help"));
}
