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

#include "QMAWidget.h"
#include "QMAPlugin.h"

#include "btBulletDynamicsCommon.h"

QMAWidget::QMAWidget(QWidget *parent)
  : QGLWidget(parent),
  m_controller(new SceneController(this)),
  m_parser(m_controller, &m_factory),
  m_x(0),
  m_y(0),
  m_doubleClicked(false),
  m_showLog(true),
  m_displayBone(false),
  m_displayRigidBody(false),
  m_frameAdjust(0.0),
  m_frameCue(0.0)
{
  memset(m_movings, 0, sizeof(double) * MAX_MODEL);
  setAcceptDrops(true);
  setWindowTitle("QtMMDAI");
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

void QMAWidget::sendKeyEvent(const QString &text)
{
  QStringList arguments;
  arguments << text;
  emit pluginEventPost(QString(MMDAGENT_EVENT_KEY), arguments);
}

QMAModelLoaderFactory *QMAWidget::getModelLoaderFactory()
{
  return &m_factory;
}

SceneController *QMAWidget::getSceneController()
{
  return m_controller;
}

void QMAWidget::loadPlugins()
{
  QDir appDir = QDir(qApp->applicationDirPath());
#if defined(Q_OS_WIN)
  QString dirName = appDir.dirName().toLower();
  if (dirName == "debug" || dirName == "release")
    appDir.cdUp();
#elif defined(Q_OS_MAC)
  if (appDir.dirName() == "MacOS") {
    appDir.cdUp();
    appDir.cdUp();
    appDir.cdUp();
  }
#endif
  QDir pluginsDir = appDir;
  pluginsDir.cd("Plugins");
  foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
    QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
    QObject *instance = loader.instance();
    QMAPlugin *plugin = qobject_cast<QMAPlugin *>(instance);
    if (plugin != NULL) {
      connect(this, SIGNAL(pluginInitialized(SceneController *)),
              plugin, SLOT(initialize(SceneController *)));
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
      connect(this, SIGNAL(pluginRendered()),
              plugin, SLOT(render()));
      connect(plugin, SIGNAL(commandPost(const QString&, const QStringList&)),
              this, SLOT(delegateCommand(const QString&, const QStringList&)));
      connect(plugin, SIGNAL(eventPost(const QString&, const QStringList&)),
              this, SLOT(delegateEvent(const QString&, const QStringList&)));
    }
    else {
      qWarning() << fileName << "was not loaded by an error:" << loader.errorString();
    }
  }
  QDir::setSearchPaths("mmdai", QStringList(appDir.absolutePath()));
  int size[2];
  size[0] = width();
  size[1] = height();
  m_controller->init(size, appDir.absoluteFilePath("AppData").toUtf8().constData());
  m_controller->getOption()->load(appDir.absoluteFilePath("MMDAI.mdf").toUtf8().constData());
  emit pluginInitialized(m_controller);
}

void QMAWidget::delegateCommand(const QString &command, const QStringList &arguments)
{
  qDebug().nospace() << "delegateCommand command=" << command << ", arguments="  << arguments;
#if 0
  if (command == "MMDAI_EXIT") {
    QApplication::closeAllWindows();
    return;
  }
#endif
  int argc = arguments.count();
  const char *cmd = strdup(command.toUtf8().constData());
  const char **argv = static_cast<const char **>(calloc(sizeof(char *), argc));
  if (cmd != NULL) {
    if (argv != NULL) {
      bool err = false;
      for (int i = 0; i < argc; i++) {
        if ((argv[i] = strdup(arguments.at(i).toUtf8().constData())) == NULL) {
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
      free(argv);
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
  Option *option = m_controller->getOption();
  double intervalFrame = m_timer.getInterval();
  double stepMax = option->getBulletFps();
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
    adjustFrame = m_timer.getAuxFrame(procFrame);
    if (adjustFrame != 0.0)
      m_frameCue = 90.0;
    m_controller->updateMotion(procFrame, adjustFrame);
    emit pluginUpdated(rectangle, point, procFrame + adjustFrame);
  }

  m_controller->updateAfterSimulation();

  /* decrement each indicator */
  if (m_frameAdjust > 0.0)
    m_frameAdjust -= intervalFrame;
  if (m_frameCue > 0.0)
    m_frameCue -= intervalFrame;
  int size = m_controller->countPMDObjects();
  for (int i = 0; i < size; i++) {
    PMDObject *object = m_controller->getPMDObject(i);
    if (object->isEnable()) {
      if (object->isMoving()) {
        m_movings[i] = 15.0;
      }
      else if (m_movings[i] > 0.0) {
        m_movings[i] -= intervalFrame;
      }
    }
  }

  update();
}

void QMAWidget::changeBaseMotion(PMDObject *object, VMDLoader *loader)
{
  MotionPlayer *player = object->getMotionManager()->getMotionPlayerList();
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
  loadPlugins();
  m_controller->updateLight();
  emit pluginStarted();
  m_timer.start();
  startTimer(10);
}

void QMAWidget::resizeGL(int width, int height)
{
  m_controller->setRect(width, height);
}

void QMAWidget::paintGL()
{
  double fps = m_timer.getFPS();
  m_controller->updateModelPositionAndRotation(fps);
  m_controller->renderScene();
  if (m_displayBone)
    m_controller->renderPMDObjectsForDebug();
  if (m_displayRigidBody)
    m_controller->renderBulletForDebug();
  m_controller->renderLogger();
  m_timer.count();
  emit pluginRendered();
}

void QMAWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
  m_controller->selectPMDObject(event->x(), event->y());
  m_controller->hightlightPMDObject();
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
      float *f = m_controller->getOption()->getLightDirection();
      btVector3 v = btVector3(f[0], f[1], f[2]);
      btMatrix3x3 matrix = btMatrix3x3(
          btQuaternion(0, y * 0.007f, 0.0) *
          btQuaternion(x * 0.007f, 0, 0)
          );
      v = v * matrix;
      m_controller->changeLightDirection(v.x(), v.y(), v.z());
    }
    else if (modifiers & Qt::ShiftModifier) {
      float fx = 0.0f, fy = 0.0f, fz = 20.0f, scale = m_controller->getScale();
      fx = x / (float) m_controller->getWidth();
      fy = -y / (float) m_controller->getHeight();
      fx = (float)(fx * (fz - RENDER_VIEWPOINT_CAMERA_Z) / RENDER_VIEWPOINT_FRUSTUM_NEAR);
      fy = (float)(fy * (fz - RENDER_VIEWPOINT_CAMERA_Z) / RENDER_VIEWPOINT_FRUSTUM_NEAR);
      fx /= scale;
      fy /= scale;
      fz = 0.0f;
      m_controller->translate(fx, fy, fz);
    }
    else if (modifiers & Qt::ControlModifier) {
      PMDObject *selectedObject = m_controller->getSelectedPMDObject();
      if (selectedObject != NULL) {
        btVector3 pos;
        m_controller->hightlightPMDObject();
        selectedObject->getPosition(pos);
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
    m_controller->hightlightPMDObject();
}

void QMAWidget::wheelEvent(QWheelEvent *event)
{
  Qt::KeyboardModifiers modifiers = event->modifiers();
  float delta = m_controller->getOption()->getScaleStep();
  float scale = m_controller->getScale();
  if (modifiers & Qt::ControlModifier) /* faster */
    delta = (delta - 1.0f) * 5.0f + 1.0f;
  else if (modifiers & Qt::ShiftModifier) /* slower */
    delta = (delta - 1.0f) * 0.2f + 1.0f;
  if (event->delta() > 0)
    scale *= delta;
  else
    scale /= delta;
  m_controller->setScale(scale);
  update();
}

void QMAWidget::timerEvent(QTimerEvent * /* event */)
{
  updateScene();
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
    qDebug() << "modifiers: " << modifiers;
    foreach (QUrl url, mimeData->urls()) {
      /* load only a local file */
      if (url.scheme() == "file") {
        const QString path = url.toLocalFile();
        const char *filename = path.toUtf8().constData();
        if (path.endsWith(".vmd")) {
          /* motion */
          if (modifiers & Qt::ControlModifier) {
            /* select all objects */
            int count = m_controller->countPMDObjects();
            if (modifiers & Qt::ShiftModifier) {
              /* insert a motion to the all objects */
              for (int i = 0; i < count; i++) {
                PMDObject *object = m_controller->getPMDObject(i);
                if (object->isEnable() && object->allowMotionFileDrop()) {
                  VMDLoader *loader = m_factory.createMotionLoader(filename);
                  m_controller->addMotion(object, loader);
                }
              }
            }
            else {
              /* change base motion to the all objects */
              for (int i = 0; i < count; i++) {
                PMDObject *object = m_controller->getPMDObject(i);
                if (object->isEnable() && object->allowMotionFileDrop()) {
                  VMDLoader *loader = m_factory.createMotionLoader(filename);
                  changeBaseMotion(object, loader);
                }
              }
            }
          }
          else {
            PMDObject *selectedObject = m_controller->getSelectedPMDObject();
            if (!m_doubleClicked || selectedObject == NULL || !selectedObject->allowMotionFileDrop()) {
              const QPoint pos = event->pos();
              PMDObject *dropAllowed = NULL;
              m_controller->selectPMDObject(pos.x(), pos.y(), &dropAllowed);
              selectedObject = m_controller->getSelectedPMDObject();
              if (selectedObject == NULL)
                selectedObject = dropAllowed;
            }
            if (selectedObject != NULL) {
              if (modifiers & Qt::ShiftModifier) {
                /* insert a motion to the model */
                VMDLoader *loader = m_factory.createMotionLoader(filename);
                m_controller->addMotion(selectedObject, loader);
              }
              else {
                /* change base motion to the model */
                VMDLoader *loader = m_factory.createMotionLoader(filename);
                changeBaseMotion(selectedObject, loader);
              }
            }
          }
          /* timer resume */
        }
        else if (path.endsWith(".xpmd")) {
          /* stage */
          PMDModelLoader *loader = m_factory.createModelLoader(filename);
          m_controller->loadStage(loader);
        }
        else if (path.endsWith(".pmd")) {
          /* model */
          if (modifiers & Qt::ControlModifier) {
            PMDModelLoader *loader = m_factory.createModelLoader(filename);
            m_controller->addModel(loader);
          }
          else {
            PMDObject *selectedObject = m_controller->getSelectedPMDObject();
            if (!m_doubleClicked || selectedObject == NULL) {
              const QPoint pos = event->pos();
              m_controller->selectPMDObject(pos.x(), pos.y());
              selectedObject = m_controller->getSelectedPMDObject();
            }
            if (selectedObject != NULL) {
              PMDModelLoader *loader = m_factory.createModelLoader(filename);
              m_controller->changeModel(selectedObject, loader);
            }
            else
              g_logger.log("Warning: pmd file dropped but no model at the point");
          }
        }
        else if (path.endsWith(".bmp") || path.endsWith(".tga") || path.endsWith(".png")) {
          /* floor or background */
          PMDModelLoader *loader = m_factory.createModelLoader(filename);
          if (modifiers & Qt::ControlModifier)
            m_controller->loadFloor(loader);
          else
            m_controller->loadBackground(loader);
        }
        else {
          g_logger.log("Warning: dropped file is not supported: %s", filename);
        }
      }
      else {
        g_logger.log("Warning: doesn't support except file scheme");
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
  emit pluginStopped();
}
