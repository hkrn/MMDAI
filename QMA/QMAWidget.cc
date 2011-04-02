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

#include <MMDAI/MMDAI.h>
#include "QMAWidget.h"

QMAWidget::QMAWidget(MMDAI::Preference *preference, QWidget *parent)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent),
    m_sceneUpdateTimer(this),
    m_preference(preference),
    m_controller(new MMDAI::SceneController(this, preference)),
    m_parser(m_controller, &m_factory),
    m_x(0),
    m_y(0),
    m_doubleClicked(false),
    m_showLog(true),
    m_displayBone(false),
    m_displayRigidBody(false)
{
    m_sceneUpdateTimer.setSingleShot(false);
    connect(&m_sceneUpdateTimer, SIGNAL(timeout()), this, SLOT(updateScene()));
    setAcceptDrops(true);
    setAutoFillBackground(false);
}

QMAWidget::~QMAWidget()
{
    delete m_controller;
}

void QMAWidget::handleEventMessage(const char *eventType, int argc, ...)
{
    QStringList arguments;
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

void QMAWidget::toggleDisplayBone()
{
    m_displayBone = !m_displayBone;
}

void QMAWidget::toggleDisplayRigidBody()
{
    m_displayRigidBody = !m_displayRigidBody;
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

QMAModelLoaderFactory *QMAWidget::getModelLoaderFactory()
{
    return &m_factory;
}

QMATimer *QMAWidget::getSceneFrameTimer()
{
    return &m_sceneFrameTimer;
}

MMDAI::SceneController *QMAWidget::getSceneController() const
{
    return m_controller;
}

void QMAWidget::loadPlugins()
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
    QDir pluginsDir = QDir::searchPaths("mmdai").at(0);
    if (pluginsDir.exists("Plugins")) {
        pluginsDir.cd("Plugins");
        foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
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
    emit pluginInitialized(m_controller);
}

void QMAWidget::addPlugin(QMAPlugin *plugin)
{
    connect(this, SIGNAL(pluginInitialized(MMDAI::SceneController*)),
            plugin, SLOT(initialize(MMDAI::SceneController*)));
    connect(this, SIGNAL(pluginStarted()),
            plugin, SLOT(start()));
    connect(this, SIGNAL(pluginStopped()),
            plugin, SLOT(stop()));
    connect(this, SIGNAL(pluginCommandPost(const QString&, const QStringList&)),
            plugin, SLOT(receiveCommand(const QString&, const QStringList&)));
    connect(this, SIGNAL(pluginEventPost(const QString&, const QStringList&)),
            plugin, SLOT(receiveEvent(const QString&, const QStringList&)));
    connect(this, SIGNAL(pluginUpdated(const QRect&, const QPoint&, double)),
            plugin, SLOT(update(const QRect&, const QPoint&, double)));
    connect(this, SIGNAL(pluginPreRendered()),
            plugin, SLOT(prerender()));
    connect(this, SIGNAL(pluginPostRendered()),
            plugin, SLOT(postrender()));
    connect(plugin, SIGNAL(commandPost(const QString&, const QStringList&)),
            this, SLOT(delegateCommand(const QString&, const QStringList&)));
    connect(plugin, SIGNAL(eventPost(const QString&, const QStringList&)),
            this, SLOT(delegateEvent(const QString&, const QStringList&)));
    MMDAILogInfo("%s was loaded successfully", plugin->metaObject()->className());
}

void QMAWidget::delegateCommand(const QString &command, const QStringList &arguments)
{
    qDebug().nospace() << "delegateCommand command=" << command << ", arguments="  << arguments;
    int argc = arguments.count();
    const char *cmd = MMDAIStringClone(command.toUtf8().constData());
    const char **argv = static_cast<const char **>(calloc(sizeof(char *), argc));
    if (cmd != NULL) {
        if (argv != NULL) {
            bool err = false;
            for (int i = 0; i < argc; i++) {
                QString arg = arguments.at(i);
                if ((argv[i] = MMDAIStringClone(arg.toUtf8().constData())) == NULL) {
                    err = true;
                    break;
                }
            }
            if (!err)
                m_parser.parse(cmd, argv, argc);
            for (int i = 0; i < argc; i++) {
                if (argv[i] != NULL)
                    free(const_cast<char *>(argv[i]));
            }
            MMDAIMemoryRelease(argv);
            if (!err)
                emit pluginCommandPost(command, arguments);
        }
        free(const_cast<char *>(cmd));
    }
}

void QMAWidget::delegateEvent(const QString &type, const QStringList &arguments)
{
    qDebug().nospace() << "delegateEvent type=" << type << ", arguments=" << arguments;
    emit pluginEventPost(type, arguments);
}

void QMAWidget::updateScene()
{
    const QRect rectangle(geometry());
    const QPoint point = mapFromGlobal(QCursor::pos());
    double intervalFrame = m_sceneFrameTimer.getInterval();
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
        adjustFrame = m_sceneFrameTimer.getAuxFrame(procFrame);
        m_controller->updateMotion(procFrame, adjustFrame);
        emit pluginUpdated(rectangle, point, procFrame + adjustFrame);
    }

    m_controller->updateAfterSimulation();
    m_controller->updateDepthTextureViewParam();

    update();
}

void QMAWidget::changeBaseMotion(MMDAI::PMDObject *object, MMDAI::VMDLoader *loader)
{
    MMDAI::MotionPlayer *player = object->getMotionManager()->getMotionPlayerList();
    for (; player != NULL; player = player->next) {
        if (player->active && strncmp(player->name, "base", 4) == 0) {
            m_controller->changeMotion(object, "base", loader);
            break;
        }
    }
    if (player == NULL) {
        m_controller->addMotion(object, "base", loader, true, false, true, true);
    }
}

void QMAWidget::initializeGL()
{
}

void QMAWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    if (!m_sceneUpdateTimer.isActive()) {
        m_controller->initializeScreen(width(), height());
        m_controller->updateLight();
        loadPlugins();
        emit pluginStarted();
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
    m_controller->updateModelPositionAndRotation(fps);
    m_controller->prerenderScene();
    emit pluginPreRendered();
    m_controller->updateModelViewProjectionMatrix();
    m_controller->renderScene();
    if (m_displayBone)
        m_controller->renderPMDObjectsForDebug();
    if (m_displayRigidBody)
        m_controller->renderBulletForDebug();
    m_sceneFrameTimer.count();
    emit pluginPostRendered();
}

void QMAWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    m_controller->selectPMDObject(event->x(), event->y());
    m_controller->setHighlightPMDObject(m_controller->getSelectedPMDObject());
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
        if (modifiers & Qt::ControlModifier && modifiers & Qt::ShiftModifier) {
            float direction[4];
            m_preference->getFloat4(MMDAI::kPreferenceLightDirection, direction);
            btVector3 v = btVector3(direction[0], direction[1], direction[2]);
            btMatrix3x3 matrix = btMatrix3x3(btQuaternion(0, y * 0.007f, 0.0) * btQuaternion(x * 0.007f, 0, 0));
            v = v * matrix;
            m_controller->changeLightDirection(v.x(), v.y(), v.z());
        }
        else if (modifiers & Qt::ShiftModifier) {
            float fx = 0.0f, fy = 0.0f, fz = 20.0f, scale = m_controller->getScale();
            fx = x / (float) m_controller->getWidth();
            fy = -y / (float) m_controller->getHeight();
            fx = (float)(fx * (fz - RENDER_VIEWPOINT_CAMERA_Z) / RENDER_VIEWPOINT_FRUSTUM_NEAR);
            fy = (float)(fy * (fz - RENDER_VIEWPOINT_CAMERA_Z) / RENDER_VIEWPOINT_FRUSTUM_NEAR);
            if (scale != 0) {
                fx /= scale;
                fy /= scale;
            }
            fz = 0.0f;
            m_controller->translate(fx, fy, fz);
        }
        else if (modifiers & Qt::ControlModifier) {
            MMDAI::PMDObject *selectedObject = m_controller->getSelectedPMDObject();
            if (selectedObject != NULL) {
                btVector3 pos;
                m_controller->setHighlightPMDObject(selectedObject);
                selectedObject->getTargetPosition(pos);
                pos.setX(pos.x() + x / 20.0f);
                pos.setZ(pos.z() + y / 20.0f);
                selectedObject->setPosition(pos);
                selectedObject->setMoveSpeed(-1.0f);
            }
        }
        else {
            m_controller->rotate(x * 0.007f, y * 0.007f, 0.0f);
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
    m_controller->selectPMDObject(m_x, m_y);
}

void QMAWidget::mouseReleaseEvent(QMouseEvent * /* event */)
{
    if (!m_doubleClicked)
        m_controller->setHighlightPMDObject(NULL);
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
                if (path.endsWith(".vmd", Qt::CaseInsensitive)) {
                    /* motion */
                    if (modifiers & Qt::ControlModifier) {
                        /* select all objects */
                        int count = m_controller->countPMDObjects();
                        if (modifiers & Qt::ShiftModifier) {
                            /* insert a motion to the all objects */
                            for (int i = 0; i < count; i++) {
                                MMDAI::PMDObject *object = m_controller->getPMDObject(i);
                                if (object->isEnable() && object->allowMotionFileDrop()) {
                                    MMDAI::VMDLoader *loader = m_factory.createMotionLoader(filename);
                                    m_controller->addMotion(object, loader);
                                }
                            }
                        }
                        else {
                            /* change base motion to the all objects */
                            for (int i = 0; i < count; i++) {
                                MMDAI::PMDObject *object = m_controller->getPMDObject(i);
                                if (object->isEnable() && object->allowMotionFileDrop()) {
                                    MMDAI::VMDLoader *loader = m_factory.createMotionLoader(filename);
                                    changeBaseMotion(object, loader);
                                }
                            }
                        }
                    }
                    else {
                        MMDAI::PMDObject *selectedObject = m_controller->getSelectedPMDObject();
                        if (!m_doubleClicked || selectedObject == NULL || !selectedObject->allowMotionFileDrop()) {
                            const QPoint pos = event->pos();
                            MMDAI::PMDObject *dropAllowed = NULL;
                            m_controller->selectPMDObject(pos.x(), pos.y(), &dropAllowed);
                            selectedObject = m_controller->getSelectedPMDObject();
                            if (selectedObject == NULL)
                                selectedObject = dropAllowed;
                        }
                        if (selectedObject != NULL) {
                            if (modifiers & Qt::ShiftModifier) {
                                /* insert a motion to the model */
                                MMDAI::VMDLoader *loader = m_factory.createMotionLoader(filename);
                                m_controller->addMotion(selectedObject, loader);
                            }
                            else {
                                /* change base motion to the model */
                                MMDAI::VMDLoader *loader = m_factory.createMotionLoader(filename);
                                changeBaseMotion(selectedObject, loader);
                            }
                        }
                    }
                    /* timer resume */
                }
                else if (path.endsWith(".xpmd", Qt::CaseInsensitive)) {
                    /* stage */
                    MMDAI::PMDModelLoader *loader = m_factory.createModelLoader(filename);
                    m_controller->loadStage(loader);
                }
                else if (path.endsWith(".pmd", Qt::CaseInsensitive)) {
                    /* model */
                    if (modifiers & Qt::ControlModifier) {
                        MMDAI::PMDModelLoader *modelLoader = m_factory.createModelLoader(filename);
                        MMDAI::LipSyncLoader *lipSyncLoader = m_factory.createLipSyncLoader(filename);
                        m_controller->addModel(modelLoader, lipSyncLoader);
                        m_factory.releaseModelLoader(modelLoader);
                        m_factory.releaseLipSyncLoader(lipSyncLoader);
                    }
                    else {
                        MMDAI::PMDObject *selectedObject = m_controller->getSelectedPMDObject();
                        if (!m_doubleClicked || selectedObject == NULL) {
                            const QPoint pos = event->pos();
                            m_controller->selectPMDObject(pos.x(), pos.y());
                            selectedObject = m_controller->getSelectedPMDObject();
                        }
                        if (selectedObject != NULL) {
                            MMDAI::PMDModelLoader *modelLoader = m_factory.createModelLoader(filename);
                            MMDAI::LipSyncLoader *lipSyncLoader = m_factory.createLipSyncLoader(filename);
                            m_controller->changeModel(selectedObject, modelLoader, lipSyncLoader);
                            m_factory.releaseModelLoader(modelLoader);
                            m_factory.releaseLipSyncLoader(lipSyncLoader);
                        }
                        else {
                            MMDAILogWarnString("pmd file dropped but no model at the point");
                        }
                    }
                }
                else if (path.endsWith(".bmp", Qt::CaseInsensitive)
                    || path.endsWith(".tga", Qt::CaseInsensitive)
                    || path.endsWith(".png", Qt::CaseInsensitive)) {
                    /* floor or background */
                    MMDAI::PMDModelLoader *loader = m_factory.createModelLoader(filename);
                    if (modifiers & Qt::ControlModifier)
                        m_controller->loadFloor(loader);
                    else
                        m_controller->loadBackground(loader);
                }
                else {
                    MMDAILogInfo("dropped file is not supported: %s", filename);
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

void QMAWidget::closeEvent(QCloseEvent * /* event */)
{
    m_sceneUpdateTimer.stop();
    emit pluginStopped();
}
