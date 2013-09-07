/**

 Copyright (c) 2010-2013  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#include <vpvl2/vpvl2.h>
#include <vpvl2/extensions/BaseApplicationContext.h>
#include <vpvl2/extensions/XMLProject.h>

#include "Grid.h"

#include <QtCore>
#include <QFileInfo>
#include <QQuickWindow>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QProcess>
#include <QSaveFile>
#include <QTemporaryFile>
#include <IGizmo.h>

#include "BoneRefObject.h"
#include "CameraRefObject.h"
#include "GraphicsDevice.h"
#include "RenderTarget.h"
#include "ModelProxy.h"
#include "ProjectProxy.h"
#include "Util.h"
#include "WorldProxy.h"

using namespace vpvl2;
using namespace vpvl2::extensions;
using namespace vpvl2::extensions::icu4c;

class RenderTarget::ApplicationContext : public BaseApplicationContext {
public:
    ApplicationContext(const ProjectProxy *proxy, const StringMap *stringMap)
        : BaseApplicationContext(proxy->projectInstanceRef(), proxy->encodingInstanceRef(), stringMap)
    {
    }
    ~ApplicationContext() {
    }

    void *findProcedureAddress(const void **candidatesPtr) const {
        const char **names = reinterpret_cast<const char **>(candidatesPtr);
        QOpenGLContext *context = QOpenGLContext::currentContext();
        while (const char *nameRef = *names) {
            QFunctionPointer function = context->getProcAddress(nameRef);
            if (function) {
                return reinterpret_cast<void *>(function);
            }
            names++;
        }
        return 0;
    }
    void getToonColor(const IString * /* name */, Color & /* value */, void * /* userData */) {
    }
    void getTime(float32 & /* value */, bool /* sync */) const {
    }
    void getElapsed(float32 & /* value */, bool /* sync */) const {
    }
    void uploadAnimatedTexture(float32 /* offset */, float32 /* speed */, float32 /* seek */, void * /* texture */) {
    }
    bool mapFile(const UnicodeString &path, MapBuffer *buffer) const {
        QScopedPointer<QFile> file(new QFile(Util::toQString(path)));
        if (file->open(QFile::ReadOnly | QFile::Unbuffered)) {
            bool ok = true;
            size_t size = 0;
#ifdef VPVL2_USE_MMAP
            size = file->size();
            buffer->address = file->map(0, size);
            ok = buffer->address != 0;
#else
            const QByteArray &bytes = file->readAll();
            size = bytes.size();
            buffer->address = new uint8_t[size];
            memcpy(buffer->address, bytes.constData(), size);
#endif
            buffer->size = size;
            buffer->opaque = reinterpret_cast<intptr_t>(file.take());
            return ok;
        }
        VPVL2_LOG(WARNING, "Cannot load " << qPrintable(file->fileName()) << ": " << qPrintable(file->errorString()));
        return false;
    }
    bool unmapFile(MapBuffer *buffer) const {
        if (QFile *file = reinterpret_cast<QFile *>(buffer->opaque)) {
#ifdef VPVL2_USE_MMAP
            file->unmap(buffer->address);
#else
            delete[] buffer->address;
#endif
            file->close();
            delete file;
            return true;
        }
        return false;
    }
    bool existsFile(const UnicodeString &path) const {
        return QFile::exists(Util::toQString(path));
    }
    bool uploadTextureOpaque(const uint8 *data, vsize size, const UnicodeString &key, ModelContext *context, TextureDataBridge &bridge) {
        QImage image;
        image.loadFromData(data, size);
        return uploadTextureQt(image, key, context, bridge);
    }
    bool uploadTextureOpaque(const UnicodeString &path, ModelContext *context, TextureDataBridge &bridge) {
        QImage image(Util::toQString(path));
        return uploadTextureQt(image, path, context, bridge);
    }

    bool uploadTextureQt(const QImage &image, const UnicodeString &key, ModelContext *modelContext, TextureDataBridge &bridge) {
        /* use Qt's pluggable image loader (jpg/png is loaded with libjpeg/libpng) */
        gl::BaseSurface::Format format(GL_BGRA, GL_RGBA8, GL_UNSIGNED_INT_8_8_8_8_REV, GL_TEXTURE_2D);
        const Vector3 size(image.width(), image.height(), 1);
        ITexture *texturePtr = modelContext->uploadTexture(image.constBits(), format, size, (bridge.flags & kGenerateTextureMipmap) != 0, false);
        return modelContext->cacheTexture(key, texturePtr, bridge);
    }
    QList<ModelProxy *> uploadEnqueuedModelProxies(ProjectProxy *projectProxy) {
        QList<ModelProxy *> uploadedModelProxies;
        XMLProject *projectRef = projectProxy->projectInstanceRef();
        while (!m_uploadingModels.empty()) {
            ModelProxy *modelProxy = m_uploadingModels.dequeue();
            const QFileInfo fileInfo(modelProxy->fileUrl().toLocalFile());
            const String dir(Util::fromQString(fileInfo.absoluteDir().absolutePath()));
            ModelContext context(this, 0, &dir);
            IModel *modelRef = modelProxy->data();
            IRenderEngineSmartPtr engine(projectRef->createRenderEngine(this, modelRef, Scene::kEffectCapable));
            engine->setUpdateOptions(IRenderEngine::kParallelUpdate);
            IEffect *effectRef = 0;
            engine->setEffect(effectRef, IEffect::kAutoDetection, &context);
            if (engine->upload(&context)) {
                parseOffscreenSemantic(effectRef, &dir);
                modelRef->setEdgeWidth(1.0f);
                const XMLProject::UUID &uuid = modelProxy->uuid().toString().toStdString();
                /* remove model reference from project first to add model/engine correctly after loading project */
                projectRef->removeModel(modelRef);
                projectRef->addModel(modelRef, engine.release(), uuid, 0);
                projectRef->setModelSetting(modelRef, XMLProject::kSettingNameKey, modelProxy->name().toStdString());
                projectRef->setModelSetting(modelRef, XMLProject::kSettingURIKey, modelProxy->fileUrl().toLocalFile().toStdString());
                projectRef->setModelSetting(modelRef, "selected", "false");
                addModelPath(modelRef, Util::fromQString(fileInfo.absoluteFilePath()));
                setEffectOwner(effectRef, modelRef);
                uploadedModelProxies.append(modelProxy);
            }
        }
        return uploadedModelProxies;
    }
    QList<ModelProxy *> deleteEnqueuedModelProxies(ProjectProxy *projectProxy) {
        QList<ModelProxy *> deletedModelProxies;
        XMLProject *projectRef = projectProxy->projectInstanceRef();
        while (!m_deletingModels.empty()) {
            ModelProxy *modelProxy = m_deletingModels.dequeue();
            IModel *modelRef = modelProxy->data();
            IRenderEngine *engine = projectRef->findRenderEngine(modelRef);
            projectRef->removeModel(modelRef);
            delete engine;
            deletedModelProxies.append(modelProxy);
        }
        return deletedModelProxies;
    }
    void enqueueModelProxyToUpload(ModelProxy *model) {
        m_uploadingModels.enqueue(model);
    }
    void enqueueModelProxyToDelete(ModelProxy *model) {
        m_deletingModels.enqueue(model);
    }

    static QOpenGLFramebufferObjectFormat framebufferObjectFormat(const QQuickWindow *win) {
        QOpenGLFramebufferObjectFormat format;
        format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        format.setSamples(win->format().samples());
        return format;
    }

private:
    QQueue<ModelProxy *> m_uploadingModels;
    QQueue<ModelProxy *> m_deletingModels;
};

class RenderTarget::EncodingTask : public QObject, public QRunnable {
    Q_OBJECT

public:
    EncodingTask()
        : QObject(),
          m_estimatedFrameCount(0)
    {
        setAutoDelete(false);
    }
    ~EncodingTask() {
        stop();
    }

    void setSize(const QSize &value) {
        m_size = value;
    }
    void setTitle(const QString &value) {
        m_title = value;
    }
    void setInputImageFormat(const QString &value) {
        m_inputImageFormat = value;
    }
    void setOutputPath(const QString &value) {
        m_outputPath = value;
    }
    void setOutputFormat(const QString &value) {
        m_outputFormat = value;
    }
    void setEstimatedFrameCount(const qint64 value) {
        m_estimatedFrameCount = value;
    }

    void reset() {
        m_workerId = QUuid::createUuid().toByteArray().toHex();
        m_workerDir.reset(new QTemporaryDir());
        m_workerDirPath = m_workerDir->path();
        m_inputImageFormat = "bmp";
        m_outputFormat = "png";
        m_fbo.reset();
    }
    QOpenGLFramebufferObject *generateFramebufferObject(QQuickWindow *win) {
        if (!m_fbo) {
            m_fbo.reset(new QOpenGLFramebufferObject(m_size, ApplicationContext::framebufferObjectFormat(win)));
        }
        return m_fbo.data();
    }
    QString generateFilename(const qreal &timeIndex) {
        const QString &filename = QStringLiteral("%1-%2.%3")
                .arg(m_workerId)
                .arg(qRound64(timeIndex), 9, 10, QLatin1Char('0'))
                .arg(m_inputImageFormat);
        const QString &path = m_workerDirPath.absoluteFilePath(filename);
        return path;
    }

    void stop() {
        if (m_process && m_process->state() == QProcess::Running) {
            m_process->kill();
            VPVL2_LOG(INFO, "Tried killing encode process " << m_process->pid());
            m_process->waitForFinished(5000);
            if (m_process->isOpen()) {
                m_process->terminate();
                VPVL2_LOG(INFO, "Tried terminating encode process " << m_process->pid());
                m_process->waitForFinished(5000);
            }
            if (m_process->state() == QProcess::Running) {
                VPVL2_LOG(WARNING, "error=" << m_process->error());
            }
            m_process.reset();
        }
    }

signals:
    void encodeDidBegin();
    void encodeDidProceed(quint64 proceed, quint64 estimated);
    void encodeDidFinish(bool isNormalExit);

private:
    void run() {
        stop();
        QStringList arguments;
#ifndef QT_NO_DEBUG
        arguments.append("-v");
        arguments.append("debug");
#endif
        arguments.append("-r");
        arguments.append(QStringLiteral("%1").arg(30));
        arguments.append("-s");
        arguments.append(QStringLiteral("%1x%2").arg(m_size.width()).arg(m_size.height()));
        arguments.append("-qscale");
        arguments.append("1");
        arguments.append("-vcodec");
        arguments.append(m_inputImageFormat);
        arguments.append("-metadata");
        arguments.append(QStringLiteral("title=\"%1\"").arg(m_title));
        arguments.append("-i");
        arguments.append(m_workerDirPath.absoluteFilePath(QStringLiteral("%1-%09d.%2").arg(m_workerId).arg(m_inputImageFormat)));
        arguments.append("-map");
        arguments.append("0");
        arguments.append("-c:v");
        arguments.append(m_outputFormat);
        arguments.append("-y");
        arguments.append(m_outputPath);
        QScopedPointer<QTemporaryFile> executable(QTemporaryFile::createLocalFile(":libav/avconv"));
        const QString &executablePath = executable->fileName();
        QFile::setPermissions(executablePath, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
        m_process.reset(new QProcess(this));
        m_process->setArguments(arguments);
        m_process->setProgram(executablePath);
        m_process->setProcessChannelMode(QProcess::MergedChannels);
        /* disable color output from standard output */
        QStringList environments = m_process->environment();
        environments << "AV_LOG_FORCE_NOCOLOR" << "1";
        m_process->setEnvironment(environments);
        m_process->start();
        VPVL2_VLOG(1, "executable=" << m_process->program().toStdString() << " arguments=" << arguments.join(" ").toStdString());
        VPVL2_VLOG(2, "Waiting for starting encoding task");
        m_process->waitForStarted();
        emit encodeDidBegin();
        VPVL2_VLOG(1, "Started encoding task");
        QRegExp regexp("^frame\\s*=\\s*(\\d+)");
        while (m_process->waitForReadyRead()) {
            const QByteArray &output = m_process->readAllStandardOutput();
            VPVL2_VLOG(2, output.constData());
            if (regexp.indexIn(output) >= 0) {
                quint64 proceeded = regexp.cap(1).toLongLong();
                emit encodeDidProceed(proceeded, m_estimatedFrameCount);
            }
        }
        VPVL2_VLOG(2, "Waiting for finishing encoding task");
        m_process->waitForFinished();
        QProcess::ExitStatus status = m_process->exitStatus();
        VPVL2_VLOG(1, "Finished encoding task: code=" << m_process->exitCode() << " status=" << status);
        emit encodeDidFinish(status == QProcess::NormalExit);
        m_process.reset();
        m_workerDir.reset();
        m_fbo.reset();
        m_estimatedFrameCount = 0;
    }

    QScopedPointer<QProcess> m_process;
    QScopedPointer<QOpenGLFramebufferObject> m_fbo;
    QScopedPointer<QTemporaryDir> m_workerDir;
    QDir m_workerDirPath;
    QString m_workerId;
    QSize m_size;
    QString m_title;
    QString m_inputPath;
    QString m_outputPath;
    QString m_inputImageFormat;
    QString m_outputFormat;
    quint64 m_estimatedFrameCount;
};

RenderTarget::RenderTarget(QQuickItem *parent)
    : QQuickItem(parent),
      m_translationGizmo(CreateMoveGizmo()),
      m_orientationGizmo(CreateRotateGizmo()),
      m_grid(new Grid()),
      m_encodingTask(new EncodingTask()),
      m_editMode(SelectMode),
      m_projectProxyRef(0),
      m_currentGizmoRef(0),
      m_screenColor(Qt::white),
      m_lastTimeIndex(0),
      m_currentTimeIndex(0),
      m_snapStepSize(5, 5, 5),
      m_visibleGizmoMasks(AxisX | AxisY | AxisZ | AxisScreen),
      m_grabbingGizmo(false),
      m_playing(false),
      m_dirty(false)
{
    m_translationGizmo->SetSnap(m_snapStepSize.x(), m_snapStepSize.y(), m_snapStepSize.z());
    m_translationGizmo->SetEditMatrix(m_editMatrix.data());
    m_orientationGizmo->SetEditMatrix(m_editMatrix.data());
    m_orientationGizmo->SetAxisMask(m_visibleGizmoMasks);
    connect(this, &RenderTarget::windowChanged, this, &RenderTarget::handleWindowChange);
    connect(m_encodingTask.data(), &EncodingTask::encodeDidBegin, this, &RenderTarget::encodeDidBegin);
    connect(m_encodingTask.data(), &EncodingTask::encodeDidProceed, this, &RenderTarget::encodeDidProceed);
    connect(m_encodingTask.data(), &EncodingTask::encodeDidFinish, this, &RenderTarget::encodeDidFinish);
}

RenderTarget::~RenderTarget()
{
    m_projectProxyRef = 0;
    m_currentGizmoRef = 0;
    m_lastTimeIndex = 0;
    m_currentTimeIndex = 0;
    m_grabbingGizmo = false;
    m_playing = false;
    m_dirty = false;
}

bool RenderTarget::handleMousePress(int x, int y)
{
    if (m_currentGizmoRef) {
        m_grabbingGizmo = m_currentGizmoRef->OnMouseDown(x, y);
        if (m_grabbingGizmo) {
            ModelProxy *modelProxy = m_projectProxyRef->currentModel();
            Q_ASSERT(modelProxy);
            switch (m_editMode) {
            case RotateMode:
                modelProxy->beginRotate(0);
                break;
            case MoveMode:
                modelProxy->beginTranslate(0);
                break;
            case SelectMode:
            default:
                break;
            }
            emit grabbingGizmoChanged();
        }
    }
    return m_grabbingGizmo;
}

void RenderTarget::handleMouseMove(int x, int y)
{
    if (m_currentGizmoRef) {
        m_currentGizmoRef->OnMouseMove(x, y);
        if (m_grabbingGizmo) {
            const ModelProxy *modelProxy = m_projectProxyRef->currentModel();
            Q_ASSERT(modelProxy);
            const BoneRefObject *boneProxy = modelProxy->firstTargetBone();
            Q_ASSERT(boneProxy);
            IBone *boneRef = boneProxy->data();
            Scalar rawMatrix[16];
            for (int i = 0; i < 16; i++) {
                rawMatrix[i] = static_cast<Scalar>(m_editMatrix.constData()[i]);
            }
            Transform transform(Transform::getIdentity());
            transform.setFromOpenGLMatrix(rawMatrix);
            boneRef->setLocalTranslation(transform.getOrigin());
            boneRef->setLocalOrientation(transform.getRotation());
        }
    }
}

void RenderTarget::handleMouseRelease(int x, int y)
{
    if (m_currentGizmoRef) {
        m_currentGizmoRef->OnMouseUp(x, y);
        if (m_grabbingGizmo) {
            ModelProxy *modelProxy = m_projectProxyRef->currentModel();
            Q_ASSERT(modelProxy);
            switch (m_editMode) {
            case RotateMode:
                modelProxy->endRotate();
                break;
            case MoveMode:
                modelProxy->endTranslate();
                break;
            case SelectMode:
            default:
                break;
            }
            m_grabbingGizmo = false;
            emit grabbingGizmoChanged();
        }
    }
}

bool RenderTarget::isInitialized() const
{
    return Scene::isInitialized();
}

qreal RenderTarget::currentTimeIndex() const
{
    return m_currentTimeIndex;
}

void RenderTarget::setCurrentTimeIndex(qreal value)
{
    if (value != m_currentTimeIndex) {
        m_currentTimeIndex = value;
        emit currentTimeIndexChanged();
        if (QQuickWindow *win = window()) {
            win->update();
        }
    }
}

qreal RenderTarget::lastTimeIndex() const
{
    return m_lastTimeIndex;
}

void RenderTarget::setLastTimeIndex(qreal value)
{
    if (value != m_lastTimeIndex) {
        m_lastTimeIndex = value;
        emit lastTimeIndexChanged();
        if (QQuickWindow *win = window()) {
            win->update();
        }
    }
}

qreal RenderTarget::currentFPS() const
{
    return m_counter.value();
}

ProjectProxy *RenderTarget::projectProxy() const
{
    return m_projectProxyRef;
}

Grid *RenderTarget::grid() const
{
    return m_grid.data();
}

void RenderTarget::setProjectProxy(ProjectProxy *value)
{
    Q_ASSERT(value);
    connect(value, &ProjectProxy::modelDidAdd, this, &RenderTarget::uploadModelAsync, Qt::DirectConnection);
    connect(value, &ProjectProxy::modelDidRemove, this, &RenderTarget::deleteModelAsync, Qt::DirectConnection);
    connect(value, &ProjectProxy::currentModelChanged, this, &RenderTarget::updateGizmo);
    connect(value, &ProjectProxy::projectDidCreate, this, &RenderTarget::resetSceneRef);
    connect(value, &ProjectProxy::projectDidLoad, this, &RenderTarget::resetSceneRef);
    connect(value, &ProjectProxy::undoDidPerform, this, &RenderTarget::updateGizmo);
    connect(value, &ProjectProxy::redoDidPerform, this, &RenderTarget::updateGizmo);
    connect(value->world(), &WorldProxy::simulationTypeChanged, this, &RenderTarget::prepareSyncMotionState);
    CameraRefObject *camera = value->camera();
    connect(camera, &CameraRefObject::lookAtChanged, this, &RenderTarget::markDirty);
    connect(camera, &CameraRefObject::angleChanged, this, &RenderTarget::markDirty);
    connect(camera, &CameraRefObject::distanceChanged, this, &RenderTarget::markDirty);
    connect(camera, &CameraRefObject::fovChanged, this, &RenderTarget::markDirty);
    connect(camera, &CameraRefObject::cameraDidReset, this, &RenderTarget::markDirty);
    m_grid->setProjectProxy(value);
    m_projectProxyRef = value;
}

bool RenderTarget::isPlaying() const
{
    return m_playing;
}

void RenderTarget::setPlaying(bool value)
{
    if (m_playing != value) {
        m_playing = value;
        emit playingChanged();
    }
}

bool RenderTarget::isDirty() const
{
    return m_dirty;
}

void RenderTarget::setDirty(bool value)
{
    if (m_dirty != value) {
        m_dirty = value;
        emit dirtyChanged();
    }
}

bool RenderTarget::isSnapGizmoEnabled() const
{
    return m_translationGizmo->IsUsingSnap();
}

void RenderTarget::setSnapGizmoEnabled(bool value)
{
    if (m_translationGizmo->IsUsingSnap() != value) {
        m_translationGizmo->UseSnap(value);
        emit enableSnapGizmoChanged();
    }
}

bool RenderTarget::grabbingGizmo() const
{
    return m_grabbingGizmo;
}

QColor RenderTarget::screenColor() const
{
    return m_screenColor;
}

void RenderTarget::setScreenColor(const QColor &value)
{
    if (m_screenColor != value) {
        m_screenColor = value;
        emit screenColorChanged();
    }
}

QRect RenderTarget::viewport() const
{
    return m_viewport;
}

void RenderTarget::setViewport(const QRect &value)
{
    if (m_viewport != value) {
        m_viewport = value;
        setDirty(true);
        emit viewportChanged();
    }
}

RenderTarget::EditModeType RenderTarget::editMode() const
{
    return m_editMode;
}

void RenderTarget::setEditMode(EditModeType value)
{
    if (m_editMode != value) {
        switch (value) {
        case RotateMode:
            m_currentGizmoRef = m_orientationGizmo.data();
            break;
        case MoveMode:
            m_currentGizmoRef = m_translationGizmo.data();
            break;
        case SelectMode:
        default:
            m_currentGizmoRef = 0;
            break;
        }
        m_editMode = value;
        emit editModeChanged();
    }
}

RenderTarget::VisibleGizmoMasks RenderTarget::visibleGizmoMasks() const
{
    return m_visibleGizmoMasks;
}

void RenderTarget::setVisibleGizmoMasks(VisibleGizmoMasks value)
{
    if (value != m_visibleGizmoMasks) {
        m_orientationGizmo->SetAxisMask(value);
        m_visibleGizmoMasks = value;
        emit visibleGizmoMasksChanged();
    }
}

QVector3D RenderTarget::snapGizmoStepSize() const
{
    return m_snapStepSize;
}

void RenderTarget::setSnapGizmoStepSize(const QVector3D &value)
{
    if (!qFuzzyCompare(value, m_snapStepSize)) {
        m_translationGizmo->SetSnap(value.x(), value.y(), value.z());
        m_snapStepSize = value;
        emit snapGizmoStepSizeChanged();
    }
}

QMatrix4x4 RenderTarget::viewMatrix() const
{
    return QMatrix4x4(glm::value_ptr(m_viewMatrix));
}

QMatrix4x4 RenderTarget::projectionMatrix() const
{
    return QMatrix4x4(glm::value_ptr(m_projectionMatrix));
}

GraphicsDevice *RenderTarget::graphicsDevice() const
{
    return m_graphicsDevice.data();
}

void RenderTarget::handleWindowChange(QQuickWindow *window)
{
    if (window) {
        connect(window, &QQuickWindow::sceneGraphInitialized, this, &RenderTarget::initialize, Qt::DirectConnection);
        connect(window, &QQuickWindow::beforeSynchronizing, this, &RenderTarget::syncImplicit, Qt::DirectConnection);
        window->setClearBeforeRendering(false);
    }
}

void RenderTarget::update()
{
    Q_ASSERT(window());
    window()->update();
}

void RenderTarget::render()
{
    Q_ASSERT(window());
    connect(window(), &QQuickWindow::beforeRendering, this, &RenderTarget::syncExplicit, Qt::DirectConnection);
}

void RenderTarget::exportImage(const QUrl &fileUrl, const QSize &size)
{
    Q_ASSERT(window());
    if (fileUrl.isEmpty() || !fileUrl.isValid()) {
        /* do nothing if url is empty or invalid */
        VPVL2_VLOG(2, "fileUrl is empty or invalid: url=" << fileUrl.toString().toStdString());
        return;
    }
    m_exportLocation = fileUrl;
    m_exportSize = size;
    if (!m_exportSize.isValid()) {
        m_exportSize = m_viewport.size();
    }
    connect(window(), &QQuickWindow::beforeRendering, this, &RenderTarget::drawOffscreenForImage, Qt::DirectConnection);
}

void RenderTarget::exportVideo(const QUrl &fileUrl)
{
    Q_ASSERT(window());
    if (fileUrl.isEmpty() || !fileUrl.isValid()) {
        /* do nothing if url is empty or invalid */
        VPVL2_VLOG(2, "fileUrl is empty or invalid: url=" << fileUrl.toString().toStdString());
        return;
    }
    m_exportLocation = fileUrl;
    m_encodingTask->reset();
    if (!m_exportSize.isValid()) {
        m_exportSize = m_viewport.size();
    }
    m_encodingTask->setSize(m_exportSize);
    m_encodingTask->setTitle(m_projectProxyRef->title());
    m_encodingTask->setOutputPath(fileUrl.toLocalFile());
    connect(window(), &QQuickWindow::beforeRendering, this, &RenderTarget::drawOffscreenForVideo, Qt::DirectConnection);
}

void RenderTarget::cancelExportVideo()
{
    Q_ASSERT(window());
    disconnect(window(), &QQuickWindow::beforeRendering, this, &RenderTarget::drawOffscreenForVideo);
    disconnect(window(), &QQuickWindow::afterRendering, this, &RenderTarget::startEncodingTask);
    m_encodingTask->stop();
    emit encodeDidCancel();
}

void RenderTarget::markDirty()
{
    m_dirty = true;
}

void RenderTarget::updateGizmo()
{
    Q_ASSERT(m_projectProxyRef);
    if (const ModelProxy *modelProxy = m_projectProxyRef->currentModel()) {
        switch (modelProxy->transformType()) {
        case ModelProxy::GlobalTransform:
            m_translationGizmo->SetLocation(IGizmo::LOCATE_WORLD);
            m_orientationGizmo->SetLocation(IGizmo::LOCATE_WORLD);
            break;
        case ModelProxy::LocalTransform:
            m_translationGizmo->SetLocation(IGizmo::LOCATE_LOCAL);
            m_orientationGizmo->SetLocation(IGizmo::LOCATE_LOCAL);
            break;
        case ModelProxy::ViewTransform:
            m_translationGizmo->SetLocation(IGizmo::LOCATE_VIEW);
            m_orientationGizmo->SetLocation(IGizmo::LOCATE_VIEW);
            break;
        }
        setSnapGizmoStepSize(m_snapStepSize);
        if (const BoneRefObject *boneProxy = modelProxy->firstTargetBone()) {
            const IBone *boneRef = boneProxy->data();
            Transform transform(boneRef->localOrientation(), boneRef->localTranslation());
            Scalar rawMatrix[16];
            transform.getOpenGLMatrix(rawMatrix);
            for (int i = 0; i < 16; i++) {
                m_editMatrix.data()[i] = static_cast<qreal>(rawMatrix[i]);
            }
            const Vector3 &v = boneRef->origin();
            m_translationGizmo->SetOffset(v.x(), v.y(), v.z());
            m_orientationGizmo->SetOffset(v.x(), v.y(), v.z());
        }
    }
    else {
        m_currentGizmoRef = 0;
    }
}

void RenderTarget::draw()
{
    Q_ASSERT(m_applicationContext);
    if (m_projectProxyRef) {
        emit renderWillPerform();
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        Scene::resetInitialOpenGLStates();
        updateViewport();
        clearScene();
        m_grid->draw(m_viewProjectionMatrix);
        drawScene();
        drawModelBones(m_projectProxyRef->currentModel());
        drawCurrentGizmo();
        glPopAttrib();
        bool flushed = false;
        m_counter.update(m_renderTimer.elapsed(), flushed);
        if (flushed) {
            emit currentFPSChanged();
        }
        emit renderDidPerform();
    }
}

void RenderTarget::drawOffscreenForImage()
{
    Q_ASSERT(window());
    QQuickWindow *win = window();
    disconnect(win, &QQuickWindow::beforeRendering, this, &RenderTarget::drawOffscreenForImage);
    connect(win, &QQuickWindow::afterRendering, this, &RenderTarget::writeExportedImage);
    QOpenGLFramebufferObject fbo(m_exportSize, ApplicationContext::framebufferObjectFormat(win));
    fbo.bind();
    Scene::resetInitialOpenGLStates();
    glViewport(0, 0, fbo.width(), fbo.height());
    clearScene();
    drawScene();
    fbo.bindDefault();
    m_exportImage = fbo.toImage();
}

void RenderTarget::drawOffscreenForVideo()
{
    Q_ASSERT(window());
    QQuickWindow *win = window();
    QOpenGLFramebufferObject *fbo = m_encodingTask->generateFramebufferObject(win);
    fbo->bind();
    Scene::resetInitialOpenGLStates();
    glViewport(0, 0, fbo->width(), fbo->height());
    clearScene();
    drawScene();
    fbo->bindDefault();
    if (qFuzzyIsNull(m_projectProxyRef->differenceTimeIndex(m_currentTimeIndex))) {
        m_encodingTask->setEstimatedFrameCount(m_currentTimeIndex);
        disconnect(win, &QQuickWindow::beforeRendering, this, &RenderTarget::drawOffscreenForVideo);
        connect(win, &QQuickWindow::afterRendering, this, &RenderTarget::startEncodingTask);
    }
    else {
        const qreal &currentTimeIndex = m_currentTimeIndex;
        const QString &path = m_encodingTask->generateFilename(currentTimeIndex);
        setCurrentTimeIndex(currentTimeIndex + 1);
        fbo->toImage().save(path);
        emit videoFrameDidSave(currentTimeIndex, m_projectProxyRef->durationTimeIndex());
    }
}

void RenderTarget::writeExportedImage()
{
    Q_ASSERT(window());
    disconnect(window(), &QQuickWindow::afterRendering, this, &RenderTarget::writeExportedImage);
    QFileInfo finfo(m_exportLocation.toLocalFile());
    const QString &suffix = finfo.suffix();
    if (suffix != "bmp") {
        QTemporaryFile tempFile;
        if (tempFile.open()) {
            m_exportImage.save(&tempFile, "bmp");
            QSaveFile saveFile(finfo.filePath());
            if (saveFile.open(QFile::WriteOnly)) {
                const QImage image(tempFile.fileName());
                image.save(&saveFile, qPrintable(suffix));
                if (!saveFile.commit()) {
                    VPVL2_LOG(WARNING, "Cannot commit the file to: path=" << saveFile.fileName().toStdString() << " reason=" << saveFile.errorString().toStdString());
                }
            }
            else {
                VPVL2_LOG(WARNING, "Cannot open the file to commit: path=" << saveFile.fileName().toStdString() << "  reason=" << saveFile.errorString().toStdString());
            }
        }
        else {
            VPVL2_LOG(WARNING, "Cannot open temporary file: path=" << tempFile.fileName().toStdString() << "  reason=" << tempFile.errorString().toStdString());
        }
    }
    else {
        QSaveFile saveFile(finfo.filePath());
        if (saveFile.open(QFile::WriteOnly)) {
            m_exportImage.save(&saveFile, qPrintable(suffix));
            saveFile.commit();
        }
        else {
            VPVL2_LOG(WARNING, "Cannot open file to commit: path=" << saveFile.fileName().toStdString() << " " << saveFile.errorString().toStdString());
        }
    }
    m_exportImage = QImage();
    m_exportSize = QSize();
}

void RenderTarget::startEncodingTask()
{
    Q_ASSERT(window());
    Q_ASSERT(m_projectProxyRef);
    disconnect(window(), &QQuickWindow::afterRendering, this, &RenderTarget::startEncodingTask);
    QThreadPool::globalInstance()->start(m_encodingTask.data());
    m_exportSize = QSize();
}

void RenderTarget::syncExplicit()
{
    Q_ASSERT(window());
    disconnect(window(), &QQuickWindow::beforeRendering, this, &RenderTarget::syncExplicit);
    if (m_projectProxyRef) {
        m_projectProxyRef->update(Scene::kUpdateAll | Scene::kForceUpdateAllMorphs);
    }
    draw();
}

void RenderTarget::syncMotionState()
{
    Q_ASSERT(window() && m_projectProxyRef);
    disconnect(window(), &QQuickWindow::beforeRendering, this, &RenderTarget::syncMotionState);
    m_projectProxyRef->update(Scene::kUpdateAll | Scene::kForceUpdateAllMorphs | Scene::kResetMotionState);
    draw();
}

void RenderTarget::syncImplicit()
{
    if (m_projectProxyRef) {
        int flags = m_playing ? Scene::kUpdateAll : (Scene::kUpdateCamera | Scene::kUpdateRenderEngines);
        m_projectProxyRef->update(flags);
    }
}

void RenderTarget::initialize()
{
    Q_ASSERT(window());
    QQuickWindow *win = window();
    if (!Scene::isInitialized()) {
        GLenum err = 0;
        Scene::initialize(&err);
        m_graphicsDevice.reset(new GraphicsDevice());
        m_graphicsDevice->initialize();
        emit graphicsDeviceChanged();
        m_applicationContext.reset(new ApplicationContext(m_projectProxyRef, &m_config));
        m_applicationContext->initialize(false);
        m_grid->load();
        QOpenGLContext *contextRef = win->openglContext();
        connect(contextRef, &QOpenGLContext::aboutToBeDestroyed, m_projectProxyRef, &ProjectProxy::reset, Qt::DirectConnection);
        connect(contextRef, &QOpenGLContext::aboutToBeDestroyed, this, &RenderTarget::release, Qt::DirectConnection);
        connect(win, &QQuickWindow::beforeRendering, this, &RenderTarget::draw, Qt::DirectConnection);
        disconnect(win, &QQuickWindow::sceneGraphInitialized, this, &RenderTarget::initialize);
        emit initializedChanged();
        m_renderTimer.start();
    }
}

void RenderTarget::release()
{
    m_currentGizmoRef = 0;
    m_translationGizmo.reset();
    m_orientationGizmo.reset();
    m_grid.reset();
    m_program.reset();
}

void RenderTarget::uploadModelAsync(ModelProxy *model)
{
    Q_ASSERT(window() && m_applicationContext);
    if (model) {
        m_applicationContext->enqueueModelProxyToUpload(model);
        connect(window(), &QQuickWindow::beforeRendering, this, &RenderTarget::performUploadingEnqueuedModels, Qt::DirectConnection);
    }
}

void RenderTarget::deleteModelAsync(ModelProxy *model)
{
    Q_ASSERT(m_applicationContext);
    if (model) {
        VPVL2_VLOG(1, "The model " << model->uuid().toString().toStdString() << " a.k.a " << model->name().toStdString() << " will be released from RenderTarget");
        m_applicationContext->enqueueModelProxyToDelete(model);
        if (QQuickWindow *win = window()) {
            connect(win, &QQuickWindow::beforeRendering, this, &RenderTarget::performDeletingEnqueuedModels, Qt::DirectConnection);
        }
        else {
            performDeletingEnqueuedModels();
        }
    }
}

void RenderTarget::performUploadingEnqueuedModels()
{
    Q_ASSERT(window() && m_applicationContext);
    disconnect(window(), &QQuickWindow::beforeRendering, this, &RenderTarget::performUploadingEnqueuedModels);
    const QList<ModelProxy *> &uploadedModelProxies = m_applicationContext->uploadEnqueuedModelProxies(m_projectProxyRef);
    foreach (ModelProxy *modelProxy, uploadedModelProxies) {
        VPVL2_VLOG(1, "The model " << modelProxy->uuid().toString().toStdString() << " a.k.a " << modelProxy->name().toStdString() << " is uploaded");
        connect(modelProxy, &ModelProxy::transformTypeChanged, this, &RenderTarget::updateGizmo);
        connect(modelProxy, &ModelProxy::firstTargetBoneChanged, this, &RenderTarget::updateGizmo);
        emit modelDidUpload(modelProxy);
    }
    emit allModelsDidUpload();
}

void RenderTarget::performDeletingEnqueuedModels()
{
    Q_ASSERT(m_applicationContext);
    if (QQuickWindow *win = window()) {
        disconnect(win, &QQuickWindow::beforeRendering, this, &RenderTarget::performDeletingEnqueuedModels);
    }
    const QList<ModelProxy *> &deletedModelProxies = m_applicationContext->deleteEnqueuedModelProxies(m_projectProxyRef);
    foreach (ModelProxy *modelProxy, deletedModelProxies) {
        VPVL2_VLOG(1, "The model " << modelProxy->uuid().toString().toStdString() << " a.k.a " << modelProxy->name().toStdString() << " is scheduled to be delete from RenderTarget and will be deleted");
        disconnect(modelProxy, &ModelProxy::transformTypeChanged, this, &RenderTarget::updateGizmo);
        disconnect(modelProxy, &ModelProxy::firstTargetBoneChanged, this, &RenderTarget::updateGizmo);
        modelProxy->deleteLater();
    }
    emit allModelsDidDelete();
}

void RenderTarget::resetSceneRef()
{
    Q_ASSERT(m_applicationContext && m_projectProxyRef);
    m_applicationContext->setSceneRef(m_projectProxyRef->projectInstanceRef());
}

void RenderTarget::clearScene()
{
    glClearColor(m_screenColor.redF(), m_screenColor.greenF(), m_screenColor.blueF(), m_screenColor.alphaF());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void RenderTarget::drawScene()
{
    Array<IRenderEngine *> enginesForPreProcess, enginesForStandard, enginesForPostProcess;
    Hash<HashPtr, IEffect *> nextPostEffects;
    Scene *scene = m_projectProxyRef->projectInstanceRef();
    scene->getRenderEnginesByRenderOrder(enginesForPreProcess,
                                         enginesForStandard,
                                         enginesForPostProcess,
                                         nextPostEffects);
    for (int i = 0, nengines = enginesForStandard.count(); i < nengines; i++) {
        IRenderEngine *engine = enginesForStandard[i];
        engine->renderModel();
        engine->renderEdge();
    }
}

void RenderTarget::drawModelBones(const ModelProxy *modelRef)
{
    if (!m_playing && modelRef && modelRef->isVisible() && m_editMode == SelectMode) {
        const QList<BoneRefObject *> &allBones = modelRef->allBoneRefs();
        QVarLengthArray<QVector3D> lineColor, lineVertices;
        lineColor.reserve(allBones.size() * 2);
        lineVertices.reserve(allBones.size() * 2);
        if (!m_program) {
            m_program.reset(new QOpenGLShaderProgram());
            m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/gui/grid.vsh");
            m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/gui/grid.fsh");
            m_program->bindAttributeLocation("inPosition", 0);
            m_program->bindAttributeLocation("inColor", 1);
            m_program->link();
            m_vao.reset(new QOpenGLVertexArrayObject());
            m_vao->create();
            if (m_vao->isCreated()) {
                m_vao->bind();
                m_program->enableAttributeArray(0);
                m_program->enableAttributeArray(1);
                m_vao->release();
            }
        }
        QColor color;
        QVector3D colorVertex;
        foreach (const BoneRefObject *bone, allBones) {
            const IBone *boneRef = bone->data();
            if (boneRef->isInteractive()) {
                const Vector3 &destination = boneRef->destinationOrigin();
                const QVector3D &origin = Util::fromVector3(boneRef->worldTransform().getOrigin());
                lineVertices.append(origin);
                lineVertices.append(Util::fromVector3(destination));
                if (modelRef->firstTargetBone() == bone) {
                    color = QColor(Qt::red);
                }
                else if (boneRef->hasInverseKinematics()) {
                    color = QColor(Qt::yellow);
                }
                else {
                    color = QColor(Qt::blue);
                }
                colorVertex.setX(color.redF());
                colorVertex.setY(color.greenF());
                colorVertex.setZ(color.blueF());
                lineColor.append(colorVertex);
                lineColor.append(colorVertex);
            }
        }
        glDisable(GL_DEPTH_TEST);
        m_program->bind();
        if (m_vao->isCreated()) {
            m_vao->bind();
        }
        else {
            m_program->enableAttributeArray(0);
            m_program->enableAttributeArray(1);
        }
        m_program->setUniformValue("modelViewProjectionMatrix", m_viewProjectionMatrixQt);
        m_program->setAttributeArray(0, lineVertices.data());
        m_program->setAttributeArray(1, lineColor.data());
        glDrawArrays(GL_LINES, 0, lineVertices.size());
        if (m_vao->isCreated()) {
            m_vao->release();
        }
        else {
            m_program->disableAttributeArray(0);
            m_program->disableAttributeArray(1);
        }
        m_program->release();
        glEnable(GL_DEPTH_TEST);
    }
}

void RenderTarget::drawCurrentGizmo()
{
    if (!m_playing && m_currentGizmoRef) {
        m_currentGizmoRef->Draw();
    }
}

void RenderTarget::updateViewport()
{
    Q_ASSERT(m_applicationContext);
    int w = m_viewport.width(), h = m_viewport.height();
    if (isDirty()) {
        glm::mat4 world, view, projection;
        glm::vec2 size(w, h);
        m_applicationContext->updateCameraMatrices(size);
        m_applicationContext->getCameraMatrices(world, view, projection);
        m_viewMatrix = view;
        m_projectionMatrix = projection;
        m_viewProjectionMatrix = projection * view;
        for (int i = 0; i < 16; i++) {
            m_viewProjectionMatrixQt.data()[i] = glm::value_ptr(m_viewProjectionMatrix)[i];
        }
        m_translationGizmo->SetScreenDimension(w, h);
        m_translationGizmo->SetCameraMatrix(glm::value_ptr(view), glm::value_ptr(projection));
        m_orientationGizmo->SetScreenDimension(w, h);
        m_orientationGizmo->SetCameraMatrix(glm::value_ptr(view), glm::value_ptr(projection));
        emit viewMatrixChanged();
        emit projectionMatrixChanged();
        setDirty(false);
    }
    glViewport(m_viewport.x(), m_viewport.y(), w, h);
}

void RenderTarget::prepareSyncMotionState()
{
    Q_ASSERT(window());
    connect(window(), &QQuickWindow::beforeRendering, this, &RenderTarget::syncMotionState);
}

#include "RenderTarget.moc"
