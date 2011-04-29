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
#include "QMAWidget.h"

QMAWidget::QMAWidget(QMAPreference *preference, QWidget *parent)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent),
      m_preference(preference),
      m_sceneUpdateTimer(this),
      m_controller(new MMDAI::SceneController(this, preference)),
      m_parser(m_controller, &m_factory),
      m_x(0),
      m_y(0),
      m_doubleClicked(false),
      m_showLog(true),
      m_displayBone(false),
      m_displayRigidBody(false),
      m_activeMotion(true)
{
    m_sceneUpdateTimer.setSingleShot(false);
    connect(&m_sceneUpdateTimer, SIGNAL(timeout()), this, SLOT(updateScene()));
    setAcceptDrops(true);
    setAutoFillBackground(false);
}

QMAWidget::~QMAWidget()
{
}

void QMAWidget::handleEventMessage(const char *eventType, int argc, ...)
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
    emit pluginEventPost(eventType, arguments);
}

bool QMAWidget::addModel(const QString &filename)
{
    QByteArray encodedPath = QFile::encodeName(filename);
    const char *path = encodedPath.constData();
    MMDAI::IModelLoader *modelLoader = m_factory.createModelLoader(path);
    MMDAI::ILipSyncLoader *lipSyncLoader = m_factory.createLipSyncLoader(path);
    bool ret = m_controller->addModel(NULL, modelLoader, lipSyncLoader, NULL, NULL, NULL, NULL);
    m_factory.releaseModelLoader(modelLoader);
    m_factory.releaseLipSyncLoader(lipSyncLoader);
    return ret;
}

bool QMAWidget::changeModel(const QString &filename, MMDAI::PMDObject *object)
{
    QByteArray encodedPath = QFile::encodeName(filename);
    const char *path = encodedPath.constData();
    MMDAI::IModelLoader *modelLoader = m_factory.createModelLoader(path);
    MMDAI::ILipSyncLoader *lipSyncLoader = m_factory.createLipSyncLoader(path);
    bool ret = m_controller->changeModel(object, modelLoader, lipSyncLoader);
    m_factory.releaseModelLoader(modelLoader);
    m_factory.releaseLipSyncLoader(lipSyncLoader);
    return ret;
}

bool QMAWidget::setStage(const QString &filename)
{
    QByteArray encodedPath = QFile::encodeName(filename);
    MMDAI::IModelLoader *loader = m_factory.createModelLoader(encodedPath.constData());
    bool ret = m_controller->loadStage(loader);
    m_factory.releaseModelLoader(loader);
    return ret;
}

bool QMAWidget::setFloor(const QString &filename)
{
    QByteArray encodedPath = QFile::encodeName(filename);
    MMDAI::IModelLoader *loader = m_factory.createModelLoader(encodedPath.constData());
    bool ret = m_controller->loadFloor(loader);
    m_factory.releaseModelLoader(loader);
    return ret;
}

bool QMAWidget::setBackground(const QString &filename)
{
    QByteArray encodedPath = QFile::encodeName(filename);
    const char *path = encodedPath.constData();
    MMDAI::IModelLoader *loader = m_factory.createModelLoader(path);
    bool ret = m_controller->loadBackground(loader);
    m_factory.releaseModelLoader(loader);
    return ret;
}

bool QMAWidget::insertMotionToAllModels(const QString &filename)
{
    QByteArray encodedPath = QFile::encodeName(filename);
    const char *path = encodedPath.constData();
    bool ret = true;
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
    return ret;
}

bool QMAWidget::insertMotionToModel(const QString &filename, MMDAI::PMDObject *object)
{
    QByteArray encodedPath = QFile::encodeName(filename);
    MMDAI::IMotionLoader *loader = m_factory.createMotionLoader(encodedPath.constData());
    bool ret = m_controller->addMotion(object, NULL, loader, false, true, true, true, 0.0f);
    m_factory.releaseMotionLoader(loader);
    return ret;
}

void QMAWidget::zoom(bool up, enum QMAWidgetZoomOption option)
{
    float delta = m_preference->getFloat(MMDAI::kPreferenceScaleStep);
    float scale = m_controller->getScale();
    if (option & Faster) /* faster */
        delta = (delta - 1.0f) * 5.0f + 1.0f;
    else if (option & Slower) /* slower */
        delta = (delta - 1.0f) * 0.2f + 1.0f;
    if (delta != 0) {
        if (up)
            scale *= delta;
        else
            scale /= delta;
    }
    m_controller->setScale(scale);
    update();
}

void QMAWidget::loadPlugins(QFile &file)
{
    foreach (QObject *instance, QPluginLoader::staticInstances()) {
        QMAPlugin *plugin = qobject_cast<QMAPlugin *>(instance);
        if (plugin != NULL) {
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
                if (plugin != NULL) {
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
    emit pluginLoaded(m_controller, QFileInfo(file).baseName());
}

void QMAWidget::addPlugin(QMAPlugin *plugin)
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

void QMAWidget::delegateCommand(const QString &command, const QList<QVariant> &arguments)
{
    qDebug().nospace() << "delegateCommand command=" << command << ", arguments="  << arguments;
    int argc = arguments.count();
    QByteArray cmd = command.toUtf8();
    char **argv = static_cast<char **>(calloc(sizeof(char *), argc));
    if (argv != NULL) {
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
            m_parser.parse(cmd.constData(), argv, argc);
        for (int i = 0; i < argc; i++) {
            MMDAIMemoryRelease(argv[i]);
        }
        MMDAIMemoryRelease(argv);
        if (!err)
            emit pluginCommandPost(command, arguments);
    }
}

void QMAWidget::delegateEvent(const QString &type, const QList<QVariant> &arguments)
{
    if (!QMAPlugin::isRenderEvent(type))
        qDebug().nospace() << "delegateEvent type=" << type << ", arguments=" << arguments;
    emit pluginEventPost(type, arguments);
}

void QMAWidget::updateScene()
{
    QList<QVariant> arguments;
    const QRect rectangle(geometry());
    const QPoint point = mapFromGlobal(QCursor::pos());
    double intervalFrame = m_sceneFrameTimer.getInterval();
    double stepMax = m_preference->getInt(MMDAI::kPreferenceBulletFPS);
    double stepFrame = 30.0 / stepMax;
    double restFrame = intervalFrame;
    double adjustFrame = 0.0, procFrame = 0.0;

    if (m_activeMotion) {
        for (int i = 0; i < stepMax; i++) {
            if (restFrame <= stepFrame) {
                procFrame = restFrame;
                i = stepMax;
            }
            else {
                procFrame = stepFrame;
                restFrame -= stepFrame;
            }
            adjustFrame = m_sceneFrameTimer.getAuxFrame(procFrame);
            m_controller->updateMotion(procFrame, adjustFrame);
            arguments.clear();
            arguments << rectangle << point << procFrame + adjustFrame;
            delegateEvent(QMAPlugin::getUpdateEvent(), arguments);
        }
    }
    else {
    }

    m_controller->updateSkin();
    m_controller->updateDepthTextureViewParam();

    update();
}

void QMAWidget::setBaseMotion(MMDAI::PMDObject *object, MMDAI::IMotionLoader *loader)
{
    MMDAI::MotionPlayer *player = object->getMotionManager()->getMotionPlayerList();
    for (; player != NULL; player = player->next) {
        if (player->active && MMDAIStringEqualsIn(player->name, "base", 4)) {
            m_controller->changeMotion(object, "base", loader);
            break;
        }
    }
    if (player == NULL) {
        m_controller->addMotion(object, "base", loader, true, false, true, true, MMDAI::MotionManager::kDefaultPriority);
    }
}

void QMAWidget::initializeGL()
{
}

void QMAWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    if (!m_sceneUpdateTimer.isActive()) {
        QStringList args = qApp->arguments();
        QFile file("MMDAIUserData:/MMDAI.mdf");
        if (args.count() > 1) {
            QString filename = args.at(1);
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
        }
        m_preference->load(file);
        m_controller->initialize(width(), height());
        m_controller->updateLight();
        loadPlugins(file);
        m_sceneFrameTimer.start();
        m_sceneUpdateTimer.start(10);
    }
}

void QMAWidget::resizeGL(int width, int height)
{
    m_controller->setRect(width, height);
}

void QMAWidget::paintGL()
{
    double fps = m_sceneFrameTimer.getFPS();
    glColor3f(1, 0, 0);
    m_controller->updateObject(fps);
    m_controller->updateModelView(0);
    m_controller->updateProjection(0);
    delegateEvent(QMAPlugin::getPreRenderEvent(), QMAPlugin::getEmptyArguments());
    m_controller->prerenderScene();
    m_controller->renderScene();
    if (m_displayBone)
        m_controller->renderModelBones();
    if (m_displayRigidBody)
        m_controller->renderModelRigidBodies();
    m_sceneFrameTimer.count();
    delegateEvent(QMAPlugin::getPostRenderEvent(), QMAPlugin::getEmptyArguments());
}

void QMAWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    m_controller->selectObject(event->x(), event->y());
    m_controller->setHighlightObject(m_controller->getSelectedObject());
    m_doubleClicked = true;
}

void QMAWidget::mouseMoveEvent(QMouseEvent *event)
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
        else if (modifiers & Qt::ControlModifier && selectedObject != NULL) {
            m_controller->setHighlightObject(selectedObject);
            btVector3 pos = selectedObject->getTargetPosition();
            float scale = m_controller->getScale();
            float step = m_preference->getFloat(MMDAI::kPreferenceTranslateStep);
            if (modifiers & Qt::ShiftModifier) {
                /* with Shift-key, move on XY (coronal) plane */
                pos.setX(pos.x() + x * 0.1f * step / scale);
                pos.setY(pos.y() - y * 0.1f * step / scale);
            } else {
                /* else, move on XZ (axial) plane */
                pos.setX(pos.x() + x * 0.1f * step / scale);
                pos.setZ(pos.z() + y * 0.1f * step / scale);
            }
            selectedObject->setPosition(pos);
            selectedObject->setMoveSpeed(-1.0f);
        }
        else if (modifiers & Qt::ShiftModifier) {
            m_controller->setModelViewPosition(x, y);
        }
        else {
            m_controller->setModelViewRotation(x, y);
        }
        m_x = event->x();
        m_y = event->y();
        update();
    }
}

void QMAWidget::mousePressEvent(QMouseEvent *event)
{
    m_x = event->x();
    m_y = event->y();
    m_doubleClicked = false;
    m_controller->selectObject(m_x, m_y);
}

void QMAWidget::mouseReleaseEvent(QMouseEvent * /* event */)
{
    if (!m_doubleClicked)
        m_controller->setHighlightObject(NULL);
}

void QMAWidget::wheelEvent(QWheelEvent *event)
{
    Qt::KeyboardModifiers modifiers = event->modifiers();
    QMAWidgetZoomOption option = Normal;
    if (modifiers & Qt::ControlModifier) /* faster */
        option = Faster;
    else if (modifiers & Qt::ShiftModifier) /* slower */
        option = Slower;
    zoom(event->delta() > 0, option);
}

void QMAWidget::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void QMAWidget::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void QMAWidget::dropEvent(QDropEvent *event)
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
                        if (selectedObject != NULL) {
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
                        if (selectedObject != NULL) {
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

void QMAWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}

void QMAWidget::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    m_sceneUpdateTimer.stop();
    emit pluginUnloaded();
}
