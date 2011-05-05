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

#include <MMDAI/MMDAI.h>
#include "QMAPreference.h"
#include "QMAScenePlayer.h"

QMAScenePlayer::QMAScenePlayer(QMAPreference *preference, QWidget *parent)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent),
      m_controller(0),
      m_debug(0),
      m_preference(preference),
      m_interval(static_cast<int>(1000.0 / 120.0)),
      m_text(font()),
      m_sceneUpdateTimer(this),
      m_parser(0),
      m_x(0),
      m_y(0),
      m_doubleClicked(false),
      m_showLog(true),
      m_enablePhysicsSimulation(true)
{
    m_controller = new MMDAI::SceneController(this, preference);
    m_parser = new MMDAI::CommandParser(m_controller, &m_factory);
    m_debug = new QMADebugRenderEngine(m_controller);
    createActions();
    setAcceptDrops(true);
    setAutoFillBackground(false);
}

QMAScenePlayer::~QMAScenePlayer()
{
    delete m_debug;
    m_debug = 0;
    delete m_parser;
    m_parser = 0;
    delete m_controller;
    m_controller = 0;
}

void QMAScenePlayer::initialize()
{
    m_sceneFrameTimer.initialize();
    m_sceneFrameTimer.startAdjustment();
    m_sceneUpdateTimer.setSingleShot(false);
    connect(&m_sceneUpdateTimer, SIGNAL(timeout()), this, SLOT(updateScene()));
    QStringList args = qApp->arguments();
    if (args.count() > 1) {
        QString filename = args.at(1);
        loadUserPreference(filename);
    }
}

void QMAScenePlayer::handleEventMessage(const char *eventType, int argc...)
{
    QList<QVariant> arguments;
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    va_list ap;
    va_start(ap, argc);
    for (int i = 0; i < argc; i++) {
        char *argv = va_arg(ap, char*);
        arguments << codec->toUnicode(argv, strlen(argv));
    }
    va_end(ap);
    qDebug().nospace() << "handleEventMessage event=" << eventType << ", arguments=" << arguments;
    if (!handleEvent(eventType, arguments))
        emit pluginEventPost(eventType, arguments);
}

bool QMAScenePlayer::addModel(const QString &filename)
{
    bool ret = false;
    if (!filename.isEmpty()) {
        QByteArray encodedPath = QFile::encodeName(filename);
        const char *path = encodedPath.constData();
        MMDAI::IModelLoader *modelLoader = m_factory.createModelLoader(path);
        MMDAI::ILipSyncLoader *lipSyncLoader = m_factory.createLipSyncLoader(path);
        ret = m_controller->addModel(NULL, modelLoader, lipSyncLoader, NULL, NULL, NULL, NULL);
        m_factory.releaseModelLoader(modelLoader);
        m_factory.releaseLipSyncLoader(lipSyncLoader);
    }
    return ret;
}

bool QMAScenePlayer::changeModel(const QString &filename)
{
    return changeModel(filename, m_controller->getSelectedObject());
}

bool QMAScenePlayer::changeModel(const QString &filename, MMDAI::PMDObject *object)
{
    bool ret = false;
    if (!filename.isEmpty() && object) {
        QByteArray encodedPath = QFile::encodeName(filename);
        const char *path = encodedPath.constData();
        MMDAI::IModelLoader *modelLoader = m_factory.createModelLoader(path);
        MMDAI::ILipSyncLoader *lipSyncLoader = m_factory.createLipSyncLoader(path);
        ret = m_controller->changeModel(object, modelLoader, lipSyncLoader);
        m_factory.releaseModelLoader(modelLoader);
        m_factory.releaseLipSyncLoader(lipSyncLoader);
        return ret;
    }
    return ret;
}

bool QMAScenePlayer::deleteModel()
{
    return deleteModel(m_controller->getSelectedObject());
}

bool QMAScenePlayer::deleteModel(MMDAI::PMDObject *object)
{
    if (object) {
        m_controller->deleteModel(object);
        m_controller->deselectObject();
    }
    return false;
}

bool QMAScenePlayer::setStage(const QString &filename)
{
    bool ret = false;
    if (!filename.isEmpty()) {
        QByteArray encodedPath = QFile::encodeName(filename);
        MMDAI::IModelLoader *loader = m_factory.createModelLoader(encodedPath.constData());
        ret = m_controller->loadStage(loader);
        m_factory.releaseModelLoader(loader);
    }
    return ret;
}

bool QMAScenePlayer::setFloor(const QString &filename)
{
    bool ret = false;
    if (!filename.isEmpty()) {
        QByteArray encodedPath = QFile::encodeName(filename);
        MMDAI::IModelLoader *loader = m_factory.createModelLoader(encodedPath.constData());
        ret = m_controller->loadFloor(loader);
        m_factory.releaseModelLoader(loader);
    }
    return ret;
}

bool QMAScenePlayer::setBackground(const QString &filename)
{
    bool ret = false;
    if (!filename.isEmpty()) {
        QByteArray encodedPath = QFile::encodeName(filename);
        const char *path = encodedPath.constData();
        MMDAI::IModelLoader *loader = m_factory.createModelLoader(path);
        ret = m_controller->loadBackground(loader);
        m_factory.releaseModelLoader(loader);
    }
    return ret;
}

bool QMAScenePlayer::insertMotionToAllModels(const QString &filename)
{
    bool ret = false;
    if (!filename.isEmpty()) {
        QByteArray encodedPath = QFile::encodeName(filename);
        const char *path = encodedPath.constData();
        ret = true;
        int max = m_controller->getMaxObjects();
        for (int i = 0; i < max; i++) {
            MMDAI::PMDObject *object = m_controller->getObjectAt(i);
            if (object && object->isEnable() && object->allowMotionFileDrop()) {
                MMDAI::IMotionLoader *loader = m_factory.createMotionLoader(path);
                ret = m_controller->addMotion(object, NULL, loader, false, true, true, true, 0.0f);
                m_factory.releaseMotionLoader(loader);
                if (!ret)
                    break;
            }
        }
    }
    return ret;
}

bool QMAScenePlayer::insertMotionToSelectedModel(const QString &filename)
{
    return insertMotionToModel(filename, m_controller->getSelectedObject());
}

bool QMAScenePlayer::insertMotionToModel(const QString &filename, MMDAI::PMDObject *object)
{
    bool ret = false;
    if (!filename.isEmpty() && object) {
        QByteArray encodedPath = QFile::encodeName(filename);
        MMDAI::IMotionLoader *loader = m_factory.createMotionLoader(encodedPath.constData());
        ret = m_controller->addMotion(object, NULL, loader, false, true, true, true, 0.0f);
        m_factory.releaseMotionLoader(loader);
        return ret;
    }
    return ret;
}

void QMAScenePlayer::setBaseMotion(MMDAI::PMDObject *object, MMDAI::IMotionLoader *loader)
{
    MMDAI::MotionPlayer *player = object->getMotionManager()->getMotionPlayerList();
    for (; player; player = player->next) {
        if (player->active && MMDAIStringEqualsIn(player->name, "base", 4)) {
            m_controller->changeMotion(object, "base", loader);
            break;
        }
    }
    if (player == NULL) {
        m_controller->addMotion(object, "base", loader, true, false, true, true, MMDAI::MotionManager::kDefaultPriority);
    }
}

void QMAScenePlayer::zoom(bool up, Qt::KeyboardModifiers modifiers)
{
    if (modifiers & Qt::ControlModifier && modifiers & Qt::ShiftModifier) {
        float step = m_preference->getFloat(MMDAI::kPreferenceFovyStep);
        float value = m_controller->getFovy();
        if (step != 0.0f) {
            value = up ? value - step : value + step;
            m_controller->setFovy(value);
        }
    }
    else {
        float step = m_preference->getFloat(MMDAI::kPreferenceDistanceStep);
        float value = m_controller->getDistance();
        if (modifiers & Qt::ControlModifier)
            step *= 5.0f;
        else if (modifiers & Qt::ShiftModifier)
            step *= 0.2f;
        if (step != 0.0f) {
            value = up ? value - step : value + step;
            m_controller->setDistance(value);
        }
    }
    update();
}

void QMAScenePlayer::rotate(float x, float y)
{
    m_controller->setModelViewRotation(y, x);
}

void QMAScenePlayer::translate(float x, float y)
{
    m_controller->translate(btVector3(x, y, 0.0f) * m_preference->getFloat(MMDAI::kPreferenceTranslateStep));
}

void QMAScenePlayer::setEdgeThin(float value)
{
    value = qMax(qMin(value, 2.0f), 0.0f);
    m_preference->setFloat(MMDAI::kPreferenceCartoonEdgeWidth, value);
    int max = m_controller->getMaxObjects();
    for (int i = 0; i < max; i++) {
        MMDAI::PMDObject *object = m_controller->getObjectAt(i);
        if (object && object->isEnable())
            object->getModel()->setEdgeThin(value);
    }
}

void QMAScenePlayer::setEnablePhysicalEngine(bool value)
{
    int max = m_controller->getMaxObjects();
    for (int i = 0; i < max; i++) {
        MMDAI::PMDObject *object = m_controller->getObjectAt(i);
        if (object && object->isEnable()) {
            object->getModel()->setPhysicsControl(value);
        }
    }
}

void QMAScenePlayer::updateShadowMapping()
{
    m_controller->setShadowMapping();
}

void QMAScenePlayer::selectModel(const QString &name)
{
    QByteArray bytes = name.toUtf8();
    const char *alias = bytes.constData();
    MMDAI::PMDObject *object = m_controller->findObject(alias);
    if (object) {
        m_controller->selectObject(object);
        m_controller->setHighlightObject(object);
    }
}

void QMAScenePlayer::start()
{
    m_sceneFrameTimer.start();
    m_sceneUpdateTimer.start(m_interval);
}

void QMAScenePlayer::loadUserPreference(const QString &filename)
{
    QFile file("MMDAIUserData:/MMDAI.mdf");
    if (filename.endsWith(".fst") || filename.endsWith(".mdf")) {
        QDir dir(filename);
        QString path = dir.absolutePath();
        if (QFile::exists(path)) {
            if (!dir.cdUp())
                dir = QDir::currentPath();
            file.setFileName(path.replace(QRegExp("\\.(fst|mdf)$"), ".mdf"));
            QStringList searchPaths = QDir::searchPaths("MMDAIUserData");
            QString searchPath = dir.absolutePath();
            searchPaths.prepend(searchPath);
            QDir::setSearchPaths("MMDAIUserData", searchPaths);
            MMDAILogInfo("added %s to MMDAIUserData schema", searchPath.toUtf8().constData());
        }
    }
    m_preference->load(file);
}

void QMAScenePlayer::loadPlugins()
{
    foreach (QObject *instance, QPluginLoader::staticInstances()) {
        QMAPlugin *plugin = qobject_cast<QMAPlugin *>(instance);
        if (plugin) {
            addPlugin(plugin);
        }
        else {
            qWarning() << plugin->metaObject()->className() << "was not loaded";
        }
    }
    QDir pluginsDir("MMDAIPlugins:/");
    if (pluginsDir.exists()) {
        foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
            if (QLibrary::isLibrary(fileName)) {
                QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
                QObject *instance = loader.instance();
                QMAPlugin *plugin = qobject_cast<QMAPlugin *>(instance);
                if (plugin) {
                    addPlugin(plugin);
                }
                else {
                    MMDAILogWarn("%s was not loaded by an error: %s",
                                 fileName.toUtf8().constData(),
                                 loader.errorString().toUtf8().constData());
                }
            }
        }
    }
    emit pluginLoaded(m_controller, m_preference->getBaseName());
}

void QMAScenePlayer::delegateCommand(const QString &command, const QList<QVariant> &arguments)
{
    qDebug().nospace() << "delegateCommand command=" << command << ", arguments="  << arguments;
    if (!handleCommand(command, arguments)) {
        int argc = arguments.count();
        QByteArray cmd = command.toUtf8();
        char **argv = static_cast<char **>(calloc(sizeof(char *), argc));
        if (argv) {
            bool err = false;
            QRegExp regexp(".*/.+\\.\\w+$");
            for (int i = 0; i < argc; i++) {
                QString arg = arguments.at(i).toString();
                QByteArray bytes = regexp.indexIn(arg) != -1 ? QFile::encodeName(arg) : arg.toUtf8();
                if ((argv[i] = MMDAIStringClone(bytes.constData())) == NULL) {
                    err = true;
                    break;
                }
            }
            if (!err)
                m_parser->parse(cmd.constData(), argv, argc);
            for (int i = 0; i < argc; i++) {
                MMDAIMemoryRelease(argv[i]);
            }
            MMDAIMemoryRelease(argv);
            if (!err)
                emit pluginCommandPost(command, arguments);
        }
    }
}

void QMAScenePlayer::delegateEvent(const QString &type, const QList<QVariant> &arguments)
{
    if (!QMAPlugin::isRenderEvent(type))
        qDebug().nospace() << "delegateEvent type=" << type << ", arguments=" << arguments;
    emit pluginEventPost(type, arguments);
}

/* events */
void QMAScenePlayer::initializeGL()
{
    m_controller->initialize(width(), height());
    m_controller->updateLight();
    m_debug->initialize();
}

void QMAScenePlayer::resizeGL(int width, int height)
{
    m_controller->setRect(width, height);
}

void QMAScenePlayer::updateScene()
{
    QList<QVariant> arguments;
    const QRect rectangle(geometry());
    const QPoint point = mapFromGlobal(QCursor::pos());
    double intervalFrame = m_sceneFrameTimer.getFrameInterval();
    double stepMax = m_preference->getInt(MMDAI::kPreferenceBulletFPS);
    double stepFrame = 30.0 / stepMax;
    double restFrame = intervalFrame;
    double adjustFrame = 0.0, procFrame = 0.0;

    for (int i = 0; i < stepMax; i++) {
        if (restFrame <= stepFrame) {
            procFrame = restFrame;
            i = stepMax;
        }
        else {
            procFrame = stepFrame;
            restFrame -= stepFrame;
        }
        adjustFrame = m_sceneFrameTimer.getAdjustmentFrame(procFrame);
        m_controller->updateMotion(procFrame, adjustFrame);
        arguments.clear();
        arguments << rectangle << point << procFrame + adjustFrame;
        delegateEvent(QMAPlugin::getUpdateEvent(), arguments);
    }

    m_controller->updateSkin();
    m_controller->updateDepthTextureViewParam();

    update();
}

void QMAScenePlayer::paintGL()
{
    double fps = m_sceneFrameTimer.getFramePerSecond();
    int ellapsed = m_controller->isViewMoving() ? m_sceneFrameTimer.ellapsed() : 0;
    m_controller->updateObject(fps);
    m_controller->updateModelView(ellapsed);
    m_controller->updateProjection(ellapsed);
    delegateEvent(QMAPlugin::getPreRenderEvent(), QMAPlugin::getEmptyArguments());
    m_controller->prerenderScene();
    m_controller->renderScene();
    m_debug->render();
    m_sceneFrameTimer.countFrame();
    m_text.render();
    delegateEvent(QMAPlugin::getPostRenderEvent(), QMAPlugin::getEmptyArguments());
}

void QMAScenePlayer::mouseDoubleClickEvent(QMouseEvent *event)
{
    m_controller->selectObject(event->x(), event->y());
    m_controller->setHighlightObject(m_controller->getSelectedObject());
    m_doubleClicked = true;
}

void QMAScenePlayer::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        int x = event->x();
        int y = event->y();
        x -= m_x;
        y -= m_y;
        if (x > SHRT_MAX)
            x -= (USHRT_MAX + 1);
        if (y > SHRT_MAX)
            y -= (USHRT_MAX + 1);
        if (x < SHRT_MIN)
            x += (USHRT_MAX + 1);
        if (y < SHRT_MIN)
            y += (USHRT_MAX + 1);
        Qt::KeyboardModifiers modifiers = event->modifiers();
        MMDAI::PMDObject *selectedObject = m_controller->getSelectedObject();
        if (modifiers & Qt::ControlModifier && modifiers & Qt::ShiftModifier && selectedObject == NULL) {
            m_controller->setLightDirection(x, y);
        }
        else if (modifiers & Qt::ControlModifier && selectedObject) {
            m_controller->setHighlightObject(selectedObject);
            btVector3 pos = selectedObject->getTargetPosition();
            float distance = m_controller->getDistance();
            float step = m_preference->getFloat(MMDAI::kPreferenceTranslateStep);
            if (modifiers & Qt::ShiftModifier) {
                /* with Shift-key, move on XY (coronal) plane */
                pos.setX(pos.x() + x * 0.001f * step / distance);
                pos.setY(pos.y() - y * 0.001f * step / distance);
            } else {
                /* else, move on XZ (axial) plane */
                pos.setX(pos.x() + x * 0.001f * step / distance);
                pos.setZ(pos.z() + y * 0.001f * step / distance);
            }
            selectedObject->setPosition(pos);
            selectedObject->setMoveSpeed(-1.0f);
        }
        else if (modifiers & Qt::ShiftModifier) {
            m_controller->setModelViewPosition(y, x);
        }
        else {
            rotate(x, y);
        }
        m_x = event->x();
        m_y = event->y();
        update();
    }
}

void QMAScenePlayer::mousePressEvent(QMouseEvent *event)
{
    m_x = event->x();
    m_y = event->y();
    m_doubleClicked = false;
    m_controller->selectObject(m_x, m_y);
    if (event->modifiers() & Qt::ControlModifier)
        m_controller->setHighlightObject(m_controller->getSelectedObject());
}

void QMAScenePlayer::mouseReleaseEvent(QMouseEvent * /* event */)
{
    if (!m_doubleClicked) {
        m_controller->setHighlightObject(NULL);
        m_controller->deselectObject();
    }
}

void QMAScenePlayer::wheelEvent(QWheelEvent *event)
{
    zoom(event->delta() > 0, event->modifiers());
}

void QMAScenePlayer::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void QMAScenePlayer::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void QMAScenePlayer::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        Qt::KeyboardModifiers modifiers = event->keyboardModifiers();
        foreach (QUrl url, mimeData->urls()) {
            /* load only a local file */
            if (url.scheme() == "file") {
                const QString path = url.toLocalFile();
                const QByteArray encodedPath = QFile::encodeName(path);
                const char *filename = encodedPath.constData();
                bool ok = true;
                if (path.endsWith(".vmd", Qt::CaseInsensitive)) {
                    /* motion */
                    if (modifiers & Qt::ControlModifier) {
                        /* select all objects */
                        int max = m_controller->getMaxObjects();
                        if (modifiers & Qt::ShiftModifier) {
                            ok = insertMotionToAllModels(path);
                        }
                        else {
                            /* change base motion to the all objects */
                            for (int i = 0; i < max; i++) {
                                MMDAI::PMDObject *object = m_controller->getObjectAt(i);
                                if (object && object->isEnable() && object->allowMotionFileDrop()) {
                                    MMDAI::IMotionLoader *loader = m_factory.createMotionLoader(filename);
                                    setBaseMotion(object, loader);
                                    m_factory.releaseMotionLoader(loader);
                                }
                            }
                        }
                    }
                    else {
                        MMDAI::PMDObject *selectedObject = m_controller->getSelectedObject();
                        if (!m_doubleClicked || selectedObject == NULL || !selectedObject->allowMotionFileDrop()) {
                            const QPoint pos = event->pos();
                            MMDAI::PMDObject *dropAllowed = NULL;
                            m_controller->selectObject(pos.x(), pos.y(), &dropAllowed);
                            selectedObject = m_controller->getSelectedObject();
                            if (selectedObject == NULL)
                                selectedObject = dropAllowed;
                        }
                        if (selectedObject) {
                            if (modifiers & Qt::ShiftModifier) {
                                /* insert a motion to the model */
                                insertMotionToModel(path, selectedObject);
                            }
                            else {
                                /* change base motion to the model */
                                MMDAI::IMotionLoader *loader = m_factory.createMotionLoader(filename);
                                setBaseMotion(selectedObject, loader);
                                m_factory.releaseMotionLoader(loader);
                            }
                        }
                    }
                    /* timer resume */
                }
                else if (path.endsWith(".xpmd", Qt::CaseInsensitive)) {
                    ok = setStage(path);
                }
                else if (path.endsWith(".pmd", Qt::CaseInsensitive)) {
                    /* model */
                    if (modifiers & Qt::ControlModifier) {
                        addModel(path);
                    }
                    else {
                        MMDAI::PMDObject *selectedObject = m_controller->getSelectedObject();
                        if (!m_doubleClicked || selectedObject == NULL) {
                            const QPoint pos = event->pos();
                            m_controller->selectObject(pos.x(), pos.y());
                            selectedObject = m_controller->getSelectedObject();
                        }
                        if (selectedObject) {
                            ok = changeModel(path, selectedObject);
                        }
                        else {
                            MMDAILogWarnString("pmd file dropped but no model at the point");
                            ok = false;
                        }
                    }
                }
                else if (path.endsWith(".bmp", Qt::CaseInsensitive)
                         || path.endsWith(".tga", Qt::CaseInsensitive)
                         || path.endsWith(".png", Qt::CaseInsensitive)) {
                    /* floor or background */
                    if (modifiers & Qt::ControlModifier)
                        ok = setFloor(path);
                    else
                        ok = setBackground(path);
                }
                else {
                    MMDAILogInfo("dropped file is not supported: %s", filename);
                    ok = false;
                }
                if (ok) {
                    QList<QVariant> arguments;
                    arguments << path;
                    delegateEvent("DRAGANDDROP", arguments);
                }
            }
            else {
                MMDAILogWarnString("doesn't support except file scheme");
            }
        }
    }
    event->acceptProposedAction();
}

void QMAScenePlayer::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}

void QMAScenePlayer::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    m_sceneUpdateTimer.stop();
    emit pluginUnloaded();
}

bool QMAScenePlayer::handleCommand(const QString &command, const QList<QVariant> &arguments)
{
    bool ret = true;
    int argc = arguments.count();
    if (command == "MMDAI_SHOW_TEXT" && argc >= 1) {
        const QStringList text = arguments[0].toString().split("<br>");
        m_text.setText(text);
        if (argc >= 2) {
            int ms = arguments[1].toInt();
            if (ms > 0)
                QTimer::singleShot(ms, this, SLOT(hideText()));
        }
        m_text.setEnable(true);
    }
    else {
        ret = false;
    }
    return ret;
}

bool QMAScenePlayer::handleEvent(const QString &type, const QList<QVariant> &arguments)
{
    if (type == MMDAI::ISceneEventHandler::kModelAddEvent) {
        QString name = arguments.at(0).toString();
        QAction *action = new QAction(name, this);
        action->setStatusTip(tr("Select a model %1").arg(name));
        connect(action, SIGNAL(triggered()), this, SLOT(selectObject()));
        m_selectModelMenu->addAction(action);
    }
    else if (type == MMDAI::ISceneEventHandler::kModelDeleteEvent) {
        QString name = arguments.at(0).toString();
        QAction *actionToRemove = NULL;
        foreach (QAction *action, m_selectModelMenu->actions()) {
            if (action->text() == name) {
                actionToRemove = action;
                break;
            }
        }
        if (actionToRemove)
            m_selectModelMenu->removeAction(actionToRemove);
    }
    return false;
}

void QMAScenePlayer::hideText()
{
    m_text.setEnable(false);
}

/* menu actions */
void QMAScenePlayer::insertMotionToAllModels()
{
    QString fileName = openFileDialog("lastVMDDirectory", tr("Open VMD file"), tr("VMD (*.vmd)"));
    insertMotionToAllModels(fileName);
    m_sceneUpdateTimer.start();
}

void QMAScenePlayer::insertMotionToSelectedModel()
{
    QString fileName = openFileDialog("lastVMDDirectory", tr("Open model PMD file"), tr("VMD (*.vmd)"));
    insertMotionToSelectedModel(fileName);
    m_sceneUpdateTimer.start();
}

void QMAScenePlayer::addModel()
{
    QString fileName = openFileDialog("lastPMDDirectory", tr("Open model PMD file"), tr("PMD (*.pmd)"));
    addModel(fileName);
    m_sceneUpdateTimer.start();
}

void QMAScenePlayer::setStage()
{
    QString fileName = openFileDialog("lastStageDirectory", tr("Open stage PMD file"), tr("PMD (*.pmd *.xpmd)"));
    setStage(fileName);
    m_sceneUpdateTimer.start();
}

void QMAScenePlayer::setFloor()
{
    QString fileName = openFileDialog("lastFloorDirectory", tr("Open floor image"), tr("Image (*.bmp *.png *.tga)"));
    setFloor(fileName);
    m_sceneUpdateTimer.start();
}

void QMAScenePlayer::setBackground()
{
    QString fileName = openFileDialog("lastBackgroundDirectory", tr("Open background image"), tr("Image (*.bmp *.png *.tga)"));
    setBackground(fileName);
    m_sceneUpdateTimer.start();
}

void QMAScenePlayer::rotateUp()
{
    rotate(0.0f, 1.0f);
}

void QMAScenePlayer::rotateDown()
{
    rotate(0.0f, -1.0f);
}

void QMAScenePlayer::rotateLeft()
{
    rotate(1.0f, 0.0f);
}

void QMAScenePlayer::rotateRight()
{
    rotate(-1.0f, 0.0f);
}

void QMAScenePlayer::translateUp()
{
    translate(0.0f, 1.0f);
}

void QMAScenePlayer::translateDown()
{
    translate(0.0f, -1.0f);
}

void QMAScenePlayer::translateLeft()
{
    translate(-1.0f, 0.0f);
}

void QMAScenePlayer::translateRight()
{
    translate(1.0f, 0.0f);
}

void QMAScenePlayer::increaseEdgeThin()
{
    setEdgeThin(m_preference->getFloat(MMDAI::kPreferenceCartoonEdgeWidth) * m_preference->getFloat(MMDAI::kPreferenceCartoonEdgeStep));
}

void QMAScenePlayer::decreaseEdgeThin()
{
    setEdgeThin(m_preference->getFloat(MMDAI::kPreferenceCartoonEdgeWidth) / m_preference->getFloat(MMDAI::kPreferenceCartoonEdgeStep));
}

void QMAScenePlayer::togglePhysicSimulation()
{
    m_enablePhysicsSimulation = !m_enablePhysicsSimulation;
    setEnablePhysicalEngine(m_enablePhysicsSimulation);
}

void QMAScenePlayer::toggleShadowMapping()
{
    bool value = !m_preference->getBool(MMDAI::kPreferenceUseShadowMapping);
    m_preference->setBool(MMDAI::kPreferenceUseShadowMapping, value);
    updateShadowMapping();
}

void QMAScenePlayer::toggleShadowMappingLightFirst()
{
    bool value = !m_preference->getBool(MMDAI::kPreferenceShadowMappingLightFirst);
    m_preference->setBool(MMDAI::kPreferenceShadowMappingLightFirst, value);
}

void QMAScenePlayer::speak()
{
    bool ok = false;
    QString text = QInputDialog::getText(this, "", tr("Text to speak"), QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty()) {
        QList<QVariant> arguments;
        arguments << text;
        delegateEvent(QString("RECOG_EVENT_STOP"), arguments);
    }
}

void QMAScenePlayer::zoomIn()
{
    zoom(true, Qt::NoModifier);
}

void QMAScenePlayer::zoomOut()
{
    zoom(false, Qt::NoModifier);
}

void QMAScenePlayer::selectObject()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
        selectModel(action->text());
}

void QMAScenePlayer::changeSelectedObject()
{
    QString fileName = openFileDialog("lastPMDDirectory", tr("Open model PMD file"), tr("PMD (*.pmd)"));
    changeModel(fileName);
    m_sceneUpdateTimer.start();
}

void QMAScenePlayer::deleteSelectedObject()
{
    deleteModel();
}

void QMAScenePlayer::saveScene()
{
    const QString settingName("window/lastSaveSceneDirectory");
    QSettings *settings = m_preference->getSettings();
    const QString path = settings->value(settingName).toString();
    m_sceneUpdateTimer.stop();
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save image"), path, tr("Image (*.jpg, *.png, *.bmp)"));
    if (!fileName.isEmpty()) {
        QDir dir(fileName);
        dir.cdUp();
        settings->setValue(settingName, dir.absolutePath());
        QImage image = grabFrameBuffer();
        image.save(fileName);
    }
    m_sceneUpdateTimer.start();
}

void QMAScenePlayer::createActions()
{
    QAction *action = NULL;

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

    action = new QAction(tr("Add model"), this);
    action->setStatusTip(tr("Add a PMD model to the scene (Maximum is 20)."));
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_M));
    connect(action, SIGNAL(triggered()), this, SLOT(addModel()));
    m_addModelAction = action;

    action = new QAction(tr("Set stage"), this);
    action->setStatusTip(tr("Set or replace a stage to the scene."));
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
    connect(action, SIGNAL(triggered()), this, SLOT(setStage()));
    m_setStageAction = action;

    action = new QAction(tr("Set floor"), this);
    action->setStatusTip(tr("Set or replace a floor to the scene."));
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F));
    connect(action, SIGNAL(triggered()), this, SLOT(setFloor()));
    m_setFloorAction = action;

    action = new QAction(tr("Set background"), this);
    action->setStatusTip(tr("Set or replace a background to the scene."));
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_B));
    connect(action, SIGNAL(triggered()), this, SLOT(setBackground()));
    m_setBackgroundAction = action;

    action = new QAction(tr("Save screen as image"), this);
    action->setStatusTip(tr("Save the current scene as a image (BMP/JPEG/PNG are supported)."));
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
    connect(action, SIGNAL(triggered()), this, SLOT(saveScene()));
    m_saveSceneAction = action;

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

    action = new QAction(tr("Emulate speaking"), this);
    action->setStatusTip(tr("Emulates speaking using input dialog."));
    connect(action, SIGNAL(triggered()), this, SLOT(speak()));
    m_speakAction = action;

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
    action->setStatusTip(tr("Rotate a model up."));
    action->setShortcut(Qt::Key_Up);
    connect(action, SIGNAL(triggered()), this, SLOT(rotateUp()));
    m_rotateUpAction = action;

    action = new QAction(tr("Rotate down"), this);
    action->setStatusTip(tr("Rotate a model down."));
    action->setShortcut(Qt::Key_Down);
    connect(action, SIGNAL(triggered()), this, SLOT(rotateDown()));
    m_rotateDownAction = action;

    action = new QAction(tr("Rotate Left"), this);
    action->setStatusTip(tr("Rotate a model left."));
    action->setShortcut(Qt::Key_Left);
    connect(action, SIGNAL(triggered()), this, SLOT(rotateLeft()));
    m_rotateLeftAction = action;

    action = new QAction(tr("Rotate right"), this);
    action->setStatusTip(tr("Rotate a model right."));
    action->setShortcut(Qt::Key_Right);
    connect(action, SIGNAL(triggered()), this, SLOT(rotateRight()));
    m_rotateRightAction = action;

    action = new QAction(tr("Translate up"), this);
    action->setStatusTip(tr("Translate a model up."));
    action->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Up));
    connect(action, SIGNAL(triggered()), this, SLOT(translateUp()));
    m_translateUpAction = action;

    action = new QAction(tr("Translate down"), this);
    action->setStatusTip(tr("Translate a model down."));
    action->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Down));
    connect(action, SIGNAL(triggered()), this, SLOT(translateDown()));
    m_translateDownAction = action;

    action = new QAction(tr("Translate left"), this);
    action->setStatusTip(tr("Translate a model left."));
    action->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Left));
    connect(action, SIGNAL(triggered()), this, SLOT(translateLeft()));
    m_translateLeftAction = action;

    action = new QAction(tr("Translate right"), this);
    action->setStatusTip(tr("Translate a model right."));
    action->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Right));
    connect(action, SIGNAL(triggered()), this, SLOT(translateRight()));
    m_translateRightAction = action;

    action = new QAction(tr("Change selected model"), this);
    action->setStatusTip(tr("Change selected model to the specified model. If an object is not selected, do nothing."));
    action->setShortcut(QKeySequence(Qt::ALT + Qt::Key_M));
    connect(action, SIGNAL(triggered()), this, SLOT(changeSelectedObject()));
    m_changeSelectedObjectAction = action;

    action = new QAction(tr("Delete selected model"), this);
    action->setStatusTip(tr("Delete selected model from the scene. If an object is not selected, do nothing."));
    action->setShortcut(Qt::Key_Delete);
    connect(action, SIGNAL(triggered()), this, SLOT(deleteSelectedObject()));
    m_deleteSelectedObjectAction = action;
}

void QMAScenePlayer::createMenu(const QHash<QString, QMenu*> &menuBar)
{
    QMenu *fileMenu = menuBar["File"];
    QMenu *motionMenu = fileMenu->addMenu(tr("Add motion"));
    motionMenu->addAction(m_insertMotionToAllAction);
    motionMenu->addAction(m_insertMotionToSelectedAction);
    fileMenu->addAction(m_addModelAction);
    fileMenu->addAction(m_setStageAction);
    fileMenu->addAction(m_setFloorAction);
    fileMenu->addAction(m_setBackgroundAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_saveSceneAction);

    QMenu *sceneMenu = menuBar["Scene"];
    sceneMenu->addAction(m_zoomInAction);
    sceneMenu->addAction(m_zoomOutAction);
    sceneMenu->addSeparator();
    sceneMenu->addAction(m_rotateUpAction);
    sceneMenu->addAction(m_rotateDownAction);
    sceneMenu->addAction(m_rotateLeftAction);
    sceneMenu->addAction(m_rotateRightAction);
    sceneMenu->addSeparator();
    sceneMenu->addAction(m_translateUpAction);
    sceneMenu->addAction(m_translateDownAction);
    sceneMenu->addAction(m_translateLeftAction);
    sceneMenu->addAction(m_translateRightAction);
    sceneMenu->addSeparator();
    sceneMenu->addAction(m_increaseEdgeThinAction);
    sceneMenu->addAction(m_decreaseEdgeThinAction);
    sceneMenu->addSeparator();
    sceneMenu->addAction(m_togglePhysicSimulationAction);
    sceneMenu->addAction(m_toggleShadowMappingAction);
    sceneMenu->addAction(m_toggleShadowMappingFirstAction);
    sceneMenu->addAction(m_toggleDisplayBoneAction);
    sceneMenu->addAction(m_toggleDisplayRigidBodyAction);
    sceneMenu->addSeparator();
    sceneMenu->addAction(m_speakAction);

    QMenu *modelMenu = menuBar["Model"];
    m_selectModelMenu = modelMenu->addMenu(tr("Select model"));
    modelMenu->addAction(m_changeSelectedObjectAction);
    modelMenu->addAction(m_deleteSelectedObjectAction);
}

void QMAScenePlayer::addPlugin(QMAPlugin *plugin)
{
    connect(this, SIGNAL(pluginLoaded(MMDAI::SceneController*,QString)),
            plugin, SLOT(load(MMDAI::SceneController*,QString)));
    connect(this, SIGNAL(pluginUnloaded()),
            plugin, SLOT(unload()));
    connect(this, SIGNAL(pluginCommandPost(QString,QList<QVariant>)),
            plugin, SLOT(receiveCommand(QString,QList<QVariant>)));
    connect(this, SIGNAL(pluginEventPost(QString,QList<QVariant>)),
            plugin, SLOT(receiveEvent(QString,QList<QVariant>)));
    connect(plugin, SIGNAL(commandPost(QString,QList<QVariant>)),
            this, SLOT(delegateCommand(QString,QList<QVariant>)));
    connect(plugin, SIGNAL(eventPost(QString,QList<QVariant>)),
            this, SLOT(delegateEvent(QString,QList<QVariant>)));
    MMDAILogInfo("%s was loaded successfully", plugin->metaObject()->className());
}

const QString QMAScenePlayer::openFileDialog(const QString &name, const QString &description, const QString &extensions)
{
    const QString settingName(QString("window/%1").arg(name));
    QSettings *settings = m_preference->getSettings();
    const QString path = settings->value(settingName).toString();
    m_sceneUpdateTimer.stop();
    const QString fileName = QFileDialog::getOpenFileName(this, description, path, extensions);
    if (!fileName.isEmpty()) {
        QDir dir(fileName);
        dir.cdUp();
        settings->setValue(settingName, dir.absolutePath());
    }
    return fileName;
}
