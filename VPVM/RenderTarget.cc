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
#include <vpvl2/extensions/gl/Global.h>
#include <vpvl2/extensions/BaseApplicationContext.h>
#include <vpvl2/extensions/XMLProject.h>

#include "Grid.h"

/* hack for qopengl.h compilation errors */
#define GL_ARB_shader_objects
#define GL_KHR_debug

#include <QtCore>
#include <QtMultimedia>
#include <QQuickWindow>
#include <QOpenGLBuffer>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

#include <LinearMath/btIDebugDraw.h>
#include <IGizmo.h>
#include <glm/gtc/matrix_transform.hpp>

#include "BoneRefObject.h"
#include "CameraRefObject.h"
#include "GraphicsDevice.h"
#include "LightRefObject.h"
#include "MotionProxy.h"
#include "RenderTarget.h"
#include "ModelProxy.h"
#include "ProjectProxy.h"
#include "Util.h"
#include "WorldProxy.h"

using namespace vpvl2;
using namespace vpvl2::extensions;
using namespace vpvl2::extensions::icu4c;

namespace {

struct Resolver : IApplicationContext::FunctionResolver {
    bool hasExtension(const char *name) const {
        QSet<QByteArray> extensionSet;
        if (extensionSet.isEmpty()) {
            QString extensions(reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS)));
            foreach (const QString extension, extensions.split(' ')) {
                extensionSet.insert(extension.toUtf8());
            }
        }
        return extensionSet.contains(name);
    }
    void *resolveSymbol(const char *name) {
        return reinterpret_cast<void *>(QOpenGLContext::currentContext()->getProcAddress(name));
    }
};
Q_GLOBAL_STATIC(Resolver, g_functionResolverInstance)

}

class RenderTarget::ApplicationContext : public BaseApplicationContext {
public:
    typedef QPair<ModelProxy *, bool> ModelProxyPair;

    ApplicationContext(const ProjectProxy *proxy, const StringMap *stringMap)
        : BaseApplicationContext(proxy->projectInstanceRef(), proxy->encodingInstanceRef(), stringMap),
          m_orderIndex(1)
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
    FunctionResolver *sharedFunctionResolverInstance() const {
        return g_functionResolverInstance;
    }

    bool uploadTextureQt(const QImage &image, const UnicodeString &key, ModelContext *modelContext, TextureDataBridge &bridge) {
        /* use Qt's pluggable image loader (jpg/png is loaded with libjpeg/libpng) */
        gl::BaseSurface::Format format(GL_BGRA, GL_RGBA8, GL_UNSIGNED_INT_8_8_8_8_REV, GL_TEXTURE_2D);
        const Vector3 size(image.width(), image.height(), 1);
        ITexture *texturePtr = modelContext->uploadTexture(image.constBits(), format, size, (bridge.flags & kGenerateTextureMipmap) != 0);
        return modelContext->cacheTexture(key, texturePtr, bridge);
    }
    QList<ModelProxyPair> uploadEnqueuedModelProxies(ProjectProxy *projectProxy) {
        QList<ModelProxyPair> uploadedModelProxies;
        XMLProject *projectRef = projectProxy->projectInstanceRef();
        while (!m_uploadingModels.empty()) {
            const ModelProxyPair &pair = m_uploadingModels.dequeue();
            ModelProxy *modelProxy = pair.first;
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
                projectRef->addModel(modelRef, engine.release(), uuid, m_orderIndex++);
                projectProxy->setModelSetting(modelProxy, QString::fromStdString(XMLProject::kSettingNameKey), modelProxy->name());
                projectProxy->setModelSetting(modelProxy, QString::fromStdString(XMLProject::kSettingURIKey), modelProxy->fileUrl().toLocalFile());
                if (!pair.second) {
                    projectProxy->setModelSetting(modelProxy, "selected", false);
                }
                addModelPath(modelRef, Util::fromQString(fileInfo.absoluteFilePath()));
                setEffectOwner(effectRef, modelRef);
                uploadedModelProxies.append(pair);
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
    void enqueueModelProxyToUpload(ModelProxy *model, bool isProject) {
        m_uploadingModels.enqueue(ModelProxyPair(model, isProject));
    }
    void enqueueModelProxyToDelete(ModelProxy *model) {
        m_deletingModels.enqueue(model);
    }
    void resetOrderIndex(int startOrderIndex) {
        m_orderIndex = startOrderIndex;
    }

    static QOpenGLFramebufferObjectFormat framebufferObjectFormat(const QQuickWindow *win) {
        QOpenGLFramebufferObjectFormat format;
        format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        format.setSamples(win->format().samples());
        return format;
    }

private:
    QQueue<ModelProxyPair> m_uploadingModels;
    QQueue<ModelProxy *> m_deletingModels;
    int m_orderIndex;
};

class RenderTarget::DebugDrawer : public btIDebugDraw {
public:
    DebugDrawer()
        : m_flags(0),
          m_index(0)
    {
    }
    ~DebugDrawer() {
    }

    void initialize() {
        if (!m_program) {
            m_program.reset(new QOpenGLShaderProgram());
            m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/gui/grid.vsh");
            m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/gui/grid.fsh");
            m_program->bindAttributeLocation("inPosition", 0);
            m_program->bindAttributeLocation("inColor", 1);
            m_program->link();
            Q_ASSERT(m_program->isLinked());
            allocateBuffer(QOpenGLBuffer::VertexBuffer, m_vbo);
            allocateBuffer(QOpenGLBuffer::VertexBuffer, m_cbo);
            m_vao.reset(new QOpenGLVertexArrayObject());
            if (m_vao->create()) {
                m_vao->bind();
                bindAttributeBuffers();
                m_vao->release();
            }
        }
    }

    void drawContactPoint(const btVector3 &PointOnB,
                          const btVector3 &normalOnB,
                          btScalar distance,
                          int /* lifeTime */,
                          const btVector3 &color) {
        drawLine(PointOnB, PointOnB + normalOnB * distance, color);
    }
    void drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color) {
        int vertexIndex = m_index, colorIndex = m_index;
        m_vertices[vertexIndex++] = from;
        m_vertices[vertexIndex++] = to;
        m_colors[colorIndex++] = color;
        m_colors[colorIndex++] = color;
        m_index += 2;
        if (m_index >= kPreAllocatedSize) {
            flush();
        }
    }
    void drawLine(const btVector3 &from,
                  const btVector3 &to,
                  const btVector3 &fromColor,
                  const btVector3 & /* toColor */) {
        drawLine(from, to, fromColor);
    }
    void draw3dText(const btVector3 & /* location */, const char *textString) {
        VPVL2_VLOG(1, textString);
    }
    void reportErrorWarning(const char *warningString) {
        VPVL2_LOG(WARNING, warningString);
    }
    int getDebugMode() const {
        return m_flags;
    }
    void setDebugMode(int debugMode) {
        m_flags = debugMode;
    }

    void flush() {
        Q_ASSERT(m_index % 2 == 0);
        m_vbo->bind();
        m_vbo->write(0, m_vertices, m_index * sizeof(m_vertices[0]));
        m_cbo->bind();
        m_cbo->write(0, m_colors, m_index * sizeof(m_colors[0]));
        bindProgram();
        m_program->setUniformValue("modelViewProjectionMatrix", m_modelViewProjectionMatrix);
        glDrawArrays(GL_LINES, 0, m_index / 2);
        releaseProgram();
        m_index = 0;
    }
    void setModelViewProjectionMatrix(const QMatrix4x4 &value) {
        m_modelViewProjectionMatrix = value;
    }

private:
    enum {
        kPreAllocatedSize = 1024000
    };
    static void allocateBuffer(QOpenGLBuffer::Type type, QScopedPointer<QOpenGLBuffer> &buffer) {
        buffer.reset(new QOpenGLBuffer(type));
        buffer->create();
        buffer->bind();
        buffer->setUsagePattern(QOpenGLBuffer::DynamicDraw);
        buffer->allocate(sizeof(QVector3D) * kPreAllocatedSize);
        buffer->release();
    }
    void bindAttributeBuffers() {
        m_program->enableAttributeArray(0);
        m_program->enableAttributeArray(1);
        m_vbo->bind();
        m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(m_vertices[0]));
        m_cbo->bind();
        m_program->setAttributeBuffer(1, GL_FLOAT, 0, 3, sizeof(m_colors[0]));
    }
    void bindProgram() {
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        m_program->bind();
        if (m_vao->isCreated()) {
            m_vao->bind();
        }
        else {
            bindAttributeBuffers();
        }
    }
    void releaseProgram() {
        if (m_vao->isCreated()) {
            m_vao->release();
        }
        else {
            m_vbo->release();
            m_cbo->release();
            m_program->disableAttributeArray(0);
            m_program->disableAttributeArray(1);
        }
        m_program->release();
        glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
    }

    QScopedPointer<QOpenGLShaderProgram> m_program;
    QScopedPointer<QOpenGLVertexArrayObject> m_vao;
    QScopedPointer<QOpenGLBuffer> m_vbo;
    QScopedPointer<QOpenGLBuffer> m_cbo;
    QMatrix4x4 m_modelViewProjectionMatrix;
    Vector3 m_vertices[kPreAllocatedSize];
    Vector3 m_colors[kPreAllocatedSize];
    int m_flags;
    int m_index;
};

class RenderTarget::EncodingTask : public QObject {
    Q_OBJECT

public:
    EncodingTask(QObject *parent = 0)
        : QObject(parent),
          m_lastState(QProcess::NotRunning),
          m_estimatedFrameCount(0)
    {
    }
    ~EncodingTask() {
        stop();
    }

    inline bool isRunning() const {
        return m_process && m_process->state() == QProcess::Running;
    }
    void setSize(const QSize &value) {
        m_size = value;
    }
    void setTitle(const QString &value) {
        m_title = value;
    }
    void setInputImageFormat(const QString &value) {
        if (!value.isEmpty()) {
            m_inputImageFormat = value;
        }
    }
    void setOutputPath(const QString &value) {
        m_outputPath = value;
    }
    void setOutputFormat(const QString &value) {
        const QStringList &format = value.split(":");
        if (format.size() == 2) {
            m_outputFormat = format.at(0);
            m_pixelFormat = format.at(1);
        }
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
        m_pixelFormat = "rgb24";
        m_fbo.reset();
    }
    QOpenGLFramebufferObject *generateFramebufferObject(QQuickWindow *win) {
        if (!m_fbo) {
            m_fbo.reset(new QOpenGLFramebufferObject(m_size, ApplicationContext::framebufferObjectFormat(win)));
            Q_ASSERT(m_fbo->isValid());
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
        if (isRunning()) {
            m_process->kill();
            VPVL2_LOG(INFO, "Tried killing encode process " << m_process->pid());
            m_process->waitForFinished(5000);
            if (isRunning()) {
                m_process->terminate();
                VPVL2_LOG(INFO, "Tried terminating encode process " << m_process->pid());
                m_process->waitForFinished(5000);
            }
            if (isRunning()) {
                VPVL2_LOG(WARNING, "Cannot stop process: error=" << m_process->error());
            }
        }
    }
    void release() {
        m_process.reset();
        m_workerDir.reset();
        m_fbo.reset();
        m_executable.reset();
        m_estimatedFrameCount = 0;
    }

    void handleStarted() {
        emit encodeDidBegin();
        VPVL2_VLOG(1, "Started encoding task");
    }
    void handleReadyRead() {
        static const QRegExp regexp("^frame\\s*=\\s*(\\d+)");
        const QByteArray &output = m_process->readAll();
        VPVL2_VLOG(2, output.constData());
        if (regexp.indexIn(output) >= 0) {
            quint64 proceeded = regexp.cap(1).toLongLong();
            emit encodeDidProceed(proceeded, m_estimatedFrameCount);
        }
    }
    void handleStateChanged() {
        QProcess::ProcessState state = m_process->state();
        if (m_lastState != state && m_lastState == QProcess::Running && state == QProcess::NotRunning) {
            QProcess::ExitStatus status = m_process->exitStatus();
            VPVL2_VLOG(1, "Finished encoding task: code=" << m_process->exitCode() << " status=" << status);
            m_estimatedFrameCount = 0;
            emit encodeDidFinish(status == QProcess::NormalExit);
        }
        m_lastState = state;
    }
    void launch() {
        stop();
        QStringList arguments;
        m_executable.reset(QTemporaryFile::createLocalFile(":libav/avconv"));
        m_executable->setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
        const QString &executablePath = m_executable->fileName();
        getArguments(arguments);
        m_process.reset(new QProcess(this));
        m_process->setArguments(arguments);
        m_process->setProgram(executablePath);
        m_process->setProcessChannelMode(QProcess::MergedChannels);
        /* disable color output from standard output */
        QStringList environments = m_process->environment();
        environments << "AV_LOG_FORCE_NOCOLOR" << "1";
        m_process->setEnvironment(environments);
        connect(m_process.data(), &QProcess::started, this, &EncodingTask::handleStarted);
        connect(m_process.data(), &QProcess::readyRead, this, &EncodingTask::handleReadyRead);
        connect(m_process.data(), &QProcess::stateChanged, this, &EncodingTask::handleStateChanged);
        m_process->start();
        VPVL2_VLOG(1, "executable=" << m_process->program().toStdString() << " arguments=" << arguments.join(" ").toStdString());
        VPVL2_VLOG(2, "Waiting for starting encoding task");
    }

signals:
    void encodeDidBegin();
    void encodeDidProceed(quint64 proceed, quint64 estimated);
    void encodeDidFinish(bool isNormalExit);

private:
    void getArguments(QStringList &arguments) {
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
        arguments.append("-pix_fmt:v");
        arguments.append(m_pixelFormat);
        arguments.append("-y");
        arguments.append(m_outputPath);
    }

    QScopedPointer<QProcess> m_process;
    QScopedPointer<QOpenGLFramebufferObject> m_fbo;
    QScopedPointer<QTemporaryDir> m_workerDir;
    QScopedPointer<QTemporaryFile> m_executable;
    QProcess::ProcessState m_lastState;
    QDir m_workerDirPath;
    QString m_workerId;
    QSize m_size;
    QString m_title;
    QString m_inputPath;
    QString m_outputPath;
    QString m_inputImageFormat;
    QString m_outputFormat;
    QString m_pixelFormat;
    quint64 m_estimatedFrameCount;
};

class RenderTarget::ModelDrawer : public QObject {
    Q_OBJECT

public:
    ModelDrawer(QObject *parent = 0)
        : QObject(parent),
          m_currentModelRef(0),
          m_currentBoneRef(0),
          m_nvertices(0)
    {
    }
    ~ModelDrawer() {
    }

    void initialize() {
        if (!m_program) {
            m_program.reset(new QOpenGLShaderProgram());
            m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/gui/grid.vsh");
            m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/gui/grid.fsh");
            m_program->bindAttributeLocation("inPosition", 0);
            m_program->bindAttributeLocation("inColor", 1);
            m_program->link();
            Q_ASSERT(m_program->isLinked());
            allocateBuffer(m_vbo);
            allocateBuffer(m_cbo);
            m_vao.reset(new QOpenGLVertexArrayObject());
            if (m_vao->create()) {
                m_vao->bind();
                bindAttributeBuffers();
                m_vao->release();
            }
        }
    }
    void setModelProxyRef(const ModelProxy *value) {
        const QList<BoneRefObject *> &allBones = value ? value->allBoneRefs() : QList<BoneRefObject *>();
        const size_t reserve = allBones.size() * 2;
        const BoneRefObject *currentBoneRef = value ? value->firstTargetBone() : 0;
        QColor color;
        QVector3D colorVertex;
        if (m_currentModelRef != value) {
            QVarLengthArray<QVector3D> vertices, colors;
            vertices.reserve(reserve);
            colors.reserve(reserve);
            if (m_currentModelRef) {
                disconnect(value, &ModelProxy::targetBonesDidCommitTransform, this, &ModelDrawer::updateModel);
                foreach (const BoneRefObject *bone, m_currentModelRef->allBoneRefs()) {
                    disconnect(bone, &BoneRefObject::localTranslationChanged, this, &ModelDrawer::updateModel);
                    disconnect(bone, &BoneRefObject::localOrientationChanged, this, &ModelDrawer::updateModel);
                }
            }
            connect(value, &ModelProxy::targetBonesDidCommitTransform, this, &ModelDrawer::updateModel);
            foreach (const BoneRefObject *bone, allBones) {
                const IBone *boneRef = bone->data();
                connect(bone, &BoneRefObject::localTranslationChanged, this, &ModelDrawer::updateModel);
                connect(bone, &BoneRefObject::localOrientationChanged, this, &ModelDrawer::updateModel);
                if (boneRef->isInteractive()) {
                    const QVector3D &destination = Util::fromVector3(boneRef->destinationOrigin());
                    const QVector3D &origin = Util::fromVector3(boneRef->worldTransform().getOrigin());
                    if (value->firstTargetBone() == bone) {
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
                    vertices.append(origin);
                    vertices.append(destination);
                    colors.append(colorVertex);
                    colors.append(colorVertex);
                }
            }
            m_vbo->bind();
            m_vbo->allocate(vertices.data(), vertices.size() * sizeof(vertices[0]));
            m_vbo->release();
            m_cbo->bind();
            m_cbo->allocate(colors.data(), colors.size() * sizeof(colors[0]));
            m_cbo->release();
            m_nvertices = vertices.size();
            m_currentModelRef = value;
            m_currentBoneRef = 0;
        }
        else if (m_currentBoneRef != currentBoneRef) {
            QVarLengthArray<QVector3D> colors;
            colors.reserve(reserve);
            foreach (const BoneRefObject *bone, allBones) {
                const IBone *boneRef = bone->data();
                if (boneRef->isInteractive()) {
                    if (value->firstTargetBone() == bone) {
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
                    colors.append(colorVertex);
                    colors.append(colorVertex);
                }
            }
            m_cbo->bind();
            m_cbo->allocate(colors.data(), colors.size() * sizeof(colors[0]));
            m_cbo->release();
            m_currentBoneRef = currentBoneRef;
        }
    }
    const ModelProxy *currentModelProxyRef() const {
        return m_currentModelRef;
    }
    void setModelViewProjectionMatrix(const QMatrix4x4 &value) {
        m_modelViewProjectionMatrix = value;
    }
    void draw() {
        bindProgram();
        m_program->setUniformValue("modelViewProjectionMatrix", m_modelViewProjectionMatrix);
        glDrawArrays(GL_LINES, 0, m_nvertices);
        releaseProgram();
    }

public slots:
    void updateModel() {
        Q_ASSERT(m_currentModelRef);
        const QList<BoneRefObject *> &allBones = m_currentModelRef->allBoneRefs();
        const size_t reserve = allBones.size() * 2;
        QVarLengthArray<QVector3D> vertices, colors;
        QColor color;
        QVector3D colorVertex;
        vertices.reserve(reserve);
        colors.reserve(reserve);
        foreach (const BoneRefObject *bone, allBones) {
            const IBone *boneRef = bone->data();
            if (boneRef->isInteractive()) {
                const QVector3D &destination = Util::fromVector3(boneRef->destinationOrigin());
                const QVector3D &origin = Util::fromVector3(boneRef->worldTransform().getOrigin());
                if (m_currentModelRef->firstTargetBone() == bone) {
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
                vertices.append(origin);
                vertices.append(destination);
                colors.append(colorVertex);
                colors.append(colorVertex);
            }
        }
        m_vbo->bind();
        m_vbo->write(0, vertices.data(), vertices.size() * sizeof(vertices[0]));
        m_vbo->release();
        m_cbo->bind();
        m_cbo->write(0, colors.data(), colors.size() * sizeof(colors[0]));
        m_cbo->release();
        m_nvertices = vertices.size();
    }

private:
    static void allocateBuffer(QScopedPointer<QOpenGLBuffer> &buffer) {
        buffer.reset(new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer));
        buffer->create();
        buffer->bind();
        buffer->setUsagePattern(QOpenGLBuffer::DynamicDraw);
        buffer->release();
    }
    void bindAttributeBuffers() {
        m_program->enableAttributeArray(0);
        m_program->enableAttributeArray(1);
        m_vbo->bind();
        m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3);
        m_cbo->bind();
        m_program->setAttributeBuffer(1, GL_FLOAT, 0, 3);
    }
    void bindProgram() {
        glDisable(GL_DEPTH_TEST);
        m_program->bind();
        if (m_vao->isCreated()) {
            m_vao->bind();
        }
        else {
            bindAttributeBuffers();
        }
    }
    void releaseProgram() {
        if (m_vao->isCreated()) {
            m_vao->release();
        }
        else {
            m_vbo->release();
            m_cbo->release();
            m_program->disableAttributeArray(0);
            m_program->disableAttributeArray(1);
        }
        m_program->release();
        glEnable(GL_DEPTH_TEST);
    }

    const ModelProxy *m_currentModelRef;
    const BoneRefObject *m_currentBoneRef;
    QScopedPointer<QOpenGLShaderProgram> m_program;
    QScopedPointer<QOpenGLVertexArrayObject> m_vao;
    QScopedPointer<QOpenGLBuffer> m_vbo;
    QScopedPointer<QOpenGLBuffer> m_cbo;
    QMatrix4x4 m_modelViewProjectionMatrix;
    int m_nvertices;
};

class RenderTarget::VideoSurface : public QAbstractVideoSurface {
    Q_OBJECT

public:
    VideoSurface(QMediaPlayer *playerRef, QObject *parent = 0)
        : QAbstractVideoSurface(parent),
          m_createdThreadRef(QThread::currentThread()),
          m_playerRef(playerRef),
          m_textureHandle(0)
    {
        connect(playerRef, &QMediaPlayer::mediaStatusChanged, this, &VideoSurface::handleMediaStatusChanged);
        playerRef->setVideoOutput(this);
    }
    ~VideoSurface() {
        m_playerRef = 0;
    }

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const {
        switch (handleType) {
        case QAbstractVideoBuffer::NoHandle:
            return QList<QVideoFrame::PixelFormat>()
                    << QVideoFrame::Format_ARGB32
                    << QVideoFrame::Format_ARGB32_Premultiplied
                    << QVideoFrame::Format_RGB32;
        default:
            return QList<QVideoFrame::PixelFormat>();
        }
    }
    bool isFormatSupported(const QVideoSurfaceFormat &format) const {
        const QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(format.pixelFormat());
        return imageFormat != QImage::Format_Invalid && !format.frameSize().isEmpty() && format.handleType() == QAbstractVideoBuffer::NoHandle;
    }
    bool present(const QVideoFrame &frame) {
        const QVideoSurfaceFormat &s = surfaceFormat();
        if (s.pixelFormat() != frame.pixelFormat() || s.frameSize() != frame.size()) {
            setError(IncorrectFormatError);
            stop();
            return false;
        }
        else {
            assignVideoFrame(frame);
            return true;
        }
    }
    bool start(const QVideoSurfaceFormat &format) {
        if (isFormatSupported(format)) {
            return QAbstractVideoSurface::start(format);
        }
        return false;
    }
    void stop() {
        assignVideoFrame(QVideoFrame());
        QAbstractVideoSurface::stop();
    }

    void initialize() {
        Q_ASSERT(m_createdThreadRef != QThread::currentThread());
        const QSize &size = surfaceFormat().frameSize();
        if (size.isValid() && !m_program) {
            m_program.reset(new QOpenGLShaderProgram());
            m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/gui/texture.vsh");
            m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/gui/texture.fsh");
            m_program->bindAttributeLocation("inPosition", 0);
            m_program->bindAttributeLocation("inTexCoord", 1);
            m_program->link();
            Q_ASSERT(m_program->isLinked());
            QVarLengthArray<QVector2D> positions;
            /* left is for inPosition, right is for inTexCoord */
            positions.append(QVector2D(-1, -1)); positions.append(QVector2D(0, 1));
            positions.append(QVector2D(1, -1));  positions.append(QVector2D(1, 1));
            positions.append(QVector2D(-1, 1));  positions.append(QVector2D(0, 0));
            positions.append(QVector2D(1, 1));   positions.append(QVector2D(1, 0));
            allocateBuffer(positions.data(), positions.size() * sizeof(QVector2D), m_vbo);
            m_vao.reset(new QOpenGLVertexArrayObject());
            if (m_vao->create()) {
                m_vao->bind();
                m_program->enableAttributeArray(0);
                m_program->enableAttributeArray(1);
                bindAttributeBuffers();
                m_vao->release();
            }
            glGenTextures(1, &m_textureHandle);
            glBindTexture(GL_TEXTURE_2D, m_textureHandle);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.width(), size.height(), 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
    void release() {
        Q_ASSERT(m_createdThreadRef != QThread::currentThread());
        m_program.reset();
        m_vao.reset();
        m_vbo.reset();
        glDeleteTextures(1, &m_textureHandle);
    }
    void renderVideoFrame() {
        Q_ASSERT(m_createdThreadRef != QThread::currentThread());
        QVideoFrame localVideoFrame;
        {
            QMutexLocker locker(&m_videoFrameLock); Q_UNUSED(locker);
            localVideoFrame = m_videoFrame;
        }
        const QSize &size = localVideoFrame.size();
        if (m_program && localVideoFrame.isValid() && !size.isEmpty()) {
            if (localVideoFrame.map(QAbstractVideoBuffer::ReadOnly)) {
                bindProgram();
                glBindTexture(GL_TEXTURE_2D, m_textureHandle);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.width(), size.height(), GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, localVideoFrame.bits());
                m_program->setUniformValue("mainTexture", 0);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                glBindTexture(GL_TEXTURE_2D, 0);
                releaseProgram();
                localVideoFrame.unmap();
            }
        }
    }

public slots:
    void handleMediaStatusChanged(QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::LoadedMedia) {
            m_playerRef->stop();
            m_playerRef->pause();
        }
    }

private:
    static void allocateBuffer(const void *data, size_t size, QScopedPointer<QOpenGLBuffer> &buffer) {
        buffer.reset(new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer));
        buffer->create();
        buffer->bind();
        buffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
        buffer->allocate(data, size);
        buffer->release();
    }
    void assignVideoFrame(const QVideoFrame &value) {
        QMutexLocker locker(&m_videoFrameLock); Q_UNUSED(locker);
        m_videoFrame = value;
    }
    void bindAttributeBuffers() {
        static const size_t kStride = sizeof(QVector2D) * 2;
        m_vbo->bind();
        m_program->setAttributeBuffer(0, GL_FLOAT, 0, 2, kStride);
        m_program->setAttributeBuffer(1, GL_FLOAT, sizeof(QVector2D), 2, kStride);
    }
    void bindProgram() {
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        m_program->bind();
        if (m_vao->isCreated()) {
            m_vao->bind();
        }
        else {
            m_program->enableAttributeArray(0);
            m_program->enableAttributeArray(1);
            bindAttributeBuffers();
        }
    }
    void releaseProgram() {
        if (m_vao->isCreated()) {
            m_vao->release();
        }
        else {
            m_program->disableAttributeArray(0);
            m_program->disableAttributeArray(1);
            m_vbo->release();
        }
        m_program->release();
        glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
    }

    const QThread *m_createdThreadRef; /* for assertion purpose only */
    QScopedPointer<QOpenGLShaderProgram> m_program;
    QScopedPointer<QOpenGLVertexArrayObject> m_vao;
    QScopedPointer<QOpenGLBuffer> m_vbo;
    QMediaPlayer *m_playerRef;
    QVideoFrame m_videoFrame;
    QMutex m_videoFrameLock;
    GLuint m_textureHandle;
};

const QVector3D RenderTarget::kDefaultShadowMapSize = QVector3D(1024, 1024, 1);

RenderTarget::RenderTarget(QQuickItem *parent)
    : QQuickItem(parent),
      m_grid(new Grid()),
      m_shadowMapSize(kDefaultShadowMapSize),
      m_editMode(SelectMode),
      m_projectProxyRef(0),
      m_currentGizmoRef(0),
      m_lastTimeIndex(0),
      m_currentTimeIndex(0),
      m_snapStepSize(5, 5, 5),
      m_visibleGizmoMasks(AxisX | AxisY | AxisZ | AxisScreen),
      m_grabbingGizmo(false),
      m_playing(false),
      m_dirty(false)
{
    connect(this, &RenderTarget::windowChanged, this, &RenderTarget::handleWindowChange);
    connect(this, &RenderTarget::shadowMapSizeChanged, this, &RenderTarget::prepareUpdatingLight);
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

void RenderTarget::toggleRunning(bool value)
{
    Q_ASSERT(window());
    if (value) {
        connect(window(), &QQuickWindow::beforeRendering, this, &RenderTarget::draw, Qt::DirectConnection);
    }
    else {
        disconnect(window(), &QQuickWindow::beforeRendering, this, &RenderTarget::draw);
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
        seekVideo(value);
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
    connect(value, &ProjectProxy::motionDidLoad, this, &RenderTarget::prepareSyncMotionState);
    connect(value, &ProjectProxy::currentModelChanged, this, &RenderTarget::updateGizmo);
    connect(value, &ProjectProxy::projectWillCreate, this, &RenderTarget::prepareProject);
    connect(value, &ProjectProxy::projectDidCreate, this, &RenderTarget::activateProject);
    connect(value, &ProjectProxy::projectWillLoad, this, &RenderTarget::prepareProject);
    connect(value, &ProjectProxy::projectDidLoad, this, &RenderTarget::activateProject);
    connect(value, &ProjectProxy::undoDidPerform, this, &RenderTarget::syncExplicit);
    connect(value, &ProjectProxy::undoDidPerform, this, &RenderTarget::updateGizmo);
    connect(value, &ProjectProxy::redoDidPerform, this, &RenderTarget::syncExplicit);
    connect(value, &ProjectProxy::redoDidPerform, this, &RenderTarget::updateGizmo);
    connect(value, &ProjectProxy::currentTimeIndexChanged, this, &RenderTarget::seekMediaFromProject);
    connect(value, &ProjectProxy::rewindDidPerform, this, &RenderTarget::resetCurrentTimeIndex);
    connect(value, &ProjectProxy::rewindDidPerform, this, &RenderTarget::resetLastTimeIndex);
    connect(value, &ProjectProxy::rewindDidPerform, this, &RenderTarget::prepareSyncMotionState);
    connect(value->world(), &WorldProxy::simulationTypeChanged, this, &RenderTarget::prepareSyncMotionState);
    CameraRefObject *camera = value->camera();
    connect(camera, &CameraRefObject::lookAtChanged, this, &RenderTarget::markDirty);
    connect(camera, &CameraRefObject::angleChanged, this, &RenderTarget::markDirty);
    connect(camera, &CameraRefObject::distanceChanged, this, &RenderTarget::markDirty);
    connect(camera, &CameraRefObject::fovChanged, this, &RenderTarget::markDirty);
    connect(camera, &CameraRefObject::cameraDidReset, this, &RenderTarget::markDirty);
    LightRefObject *light = value->light();
    connect(light, &LightRefObject::directionChanged, this, &RenderTarget::prepareUpdatingLight);
    connect(light, &LightRefObject::shadowTypeChanged, this, &RenderTarget::prepareUpdatingLight);
    const QUrl &url = value->globalSetting("video.url").toUrl();
    if (!url.isEmpty() && url.isValid()) {
        setVideoUrl(url);
    }
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
    return translationGizmo()->IsUsingSnap();
}

void RenderTarget::setSnapGizmoEnabled(bool value)
{
    IGizmo *translationGizmoRef = translationGizmo();
    if (translationGizmoRef->IsUsingSnap() != value) {
        translationGizmoRef->UseSnap(value);
        emit enableSnapGizmoChanged();
    }
}

bool RenderTarget::grabbingGizmo() const
{
    return m_grabbingGizmo;
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

QVector3D RenderTarget::shadowMapSize() const
{
    return m_shadowMapSize;
}

void RenderTarget::setShadowMapSize(const QVector3D &value)
{
    Q_ASSERT(m_projectProxyRef);
    if (value != m_shadowMapSize) {
        m_projectProxyRef->setGlobalString("shadow.texture.size", value);
        m_shadowMapSize = value;
        emit shadowMapSizeChanged();
    }
}

QUrl RenderTarget::audioUrl() const
{
    return QUrl();
}

void RenderTarget::setAudioUrl(const QUrl &value)
{
    if (value != audioUrl()) {
        QScopedPointer<QAudioDecoder> decoder(new QAudioDecoder());
        decoder->setSourceFilename(value.toLocalFile());
        /* XXX: under construction */
        while (decoder->bufferAvailable()) {
            const QAudioBuffer &buffer = decoder->read();
            qDebug() << buffer.startTime() << buffer.frameCount() << buffer.sampleCount();
        }
        emit audioUrlChanged();
    }
}

QUrl RenderTarget::videoUrl() const
{
    return mediaPlayer()->media().canonicalUrl();
}

void RenderTarget::setVideoUrl(const QUrl &value)
{
    if (value != videoUrl()) {
        Q_ASSERT(m_projectProxyRef);
        QMediaPlayer *mediaPlayerRef = mediaPlayer();
        mediaPlayerRef->setMedia(value);
        m_projectProxyRef->projectInstanceRef()->setGlobalSetting("video.url", value.toString().toStdString());
        emit videoUrlChanged();
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
            m_currentGizmoRef = orientationGizmo();
            break;
        case MoveMode:
            m_currentGizmoRef = translationGizmo();
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
        orientationGizmo()->SetAxisMask(value);
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
        translationGizmo()->SetSnap(value.x(), value.y(), value.z());
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

void RenderTarget::exportVideo(const QUrl &fileUrl, const QSize &size, const QString &videoType, const QString &frameImageType)
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
    EncodingTask *encodingTaskRef = encodingTask();
    encodingTaskRef->reset();
    encodingTaskRef->setSize(m_exportSize);
    encodingTaskRef->setTitle(m_projectProxyRef->title());
    encodingTaskRef->setInputImageFormat(frameImageType);
    encodingTaskRef->setOutputFormat(videoType);
    encodingTaskRef->setOutputPath(fileUrl.toLocalFile());
    connect(window(), &QQuickWindow::beforeRendering, this, &RenderTarget::drawOffscreenForVideo, Qt::DirectConnection);
}

void RenderTarget::cancelExportingVideo()
{
    Q_ASSERT(window());
    disconnect(window(), &QQuickWindow::beforeRendering, this, &RenderTarget::drawOffscreenForVideo);
    disconnect(window(), &QQuickWindow::afterRendering, this, &RenderTarget::launchEncodingTask);
    if (m_encodingTask && m_encodingTask->isRunning()) {
        m_encodingTask->stop();
        emit encodeDidCancel();
    }
}

void RenderTarget::resetCurrentTimeIndex()
{
    m_currentTimeIndex = 0;
    emit currentTimeIndexChanged();
}

void RenderTarget::resetLastTimeIndex()
{
    m_lastTimeIndex = 0;
    emit lastTimeIndexChanged();
}

void RenderTarget::markDirty()
{
    m_dirty = true;
}

void RenderTarget::updateGizmo()
{
    Q_ASSERT(m_projectProxyRef);
    if (const ModelProxy *modelProxy = m_projectProxyRef->currentModel()) {
        IGizmo *translationGizmoRef = translationGizmo(), *orientationGizmoRef = orientationGizmo();
        switch (modelProxy->transformType()) {
        case ModelProxy::GlobalTransform:
            translationGizmoRef->SetLocation(IGizmo::LOCATE_WORLD);
            orientationGizmoRef->SetLocation(IGizmo::LOCATE_WORLD);
            break;
        case ModelProxy::LocalTransform:
            translationGizmoRef->SetLocation(IGizmo::LOCATE_LOCAL);
            orientationGizmoRef->SetLocation(IGizmo::LOCATE_LOCAL);
            break;
        case ModelProxy::ViewTransform:
            translationGizmoRef->SetLocation(IGizmo::LOCATE_VIEW);
            orientationGizmoRef->SetLocation(IGizmo::LOCATE_VIEW);
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
            translationGizmoRef->SetOffset(v.x(), v.y(), v.z());
            orientationGizmoRef->SetOffset(v.x(), v.y(), v.z());
        }
    }
    else {
        m_currentGizmoRef = 0;
    }
}

void RenderTarget::seekMediaFromProject()
{
    Q_ASSERT(m_projectProxyRef);
    seekVideo(m_projectProxyRef->currentTimeIndex());
}

void RenderTarget::handleAudioDecoderError(QAudioDecoder::Error error)
{
    const QString &message = ""; //mediaPlayer()->errorString();
    VPVL2_LOG(WARNING, "The audio " << audioUrl().toString().toStdString() << " cannot be loaded: code=" << error << " message=" << message.toStdString());
    emit errorDidHappen(QStringLiteral("%1 (code=%2)").arg(message).arg(error));
}

void RenderTarget::handleMediaPlayerError(QMediaPlayer::Error error)
{
    const QString &message = mediaPlayer()->errorString();
    VPVL2_LOG(WARNING, "The video " << videoUrl().toString().toStdString() << " cannot be loaded: code=" << error << " message=" << message.toStdString());
    emit errorDidHappen(QStringLiteral("%1 (code=%2)").arg(message).arg(error));
}

void RenderTarget::draw()
{
    Q_ASSERT(m_applicationContext);
    if (m_projectProxyRef) {
        emit renderWillPerform();
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        Scene::resetInitialOpenGLStates();
        drawShadowMap();
        updateViewport();
        clearScene();
        if (m_videoSurface) {
            m_videoSurface->initialize();
            m_videoSurface->renderVideoFrame();
        }
        m_grid->draw(m_viewProjectionMatrix);
        drawScene();
        drawDebug();
        drawModelBones();
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
    drawOffscreen(&fbo);
    m_exportImage = fbo.toImage();
}

void RenderTarget::drawOffscreenForVideo()
{
    Q_ASSERT(window());
    QQuickWindow *win = window();
    EncodingTask *encodingTaskRef = encodingTask();
    QOpenGLFramebufferObject *fbo = encodingTaskRef->generateFramebufferObject(win);
    drawOffscreen(fbo);
    if (qFuzzyIsNull(m_projectProxyRef->differenceTimeIndex(m_currentTimeIndex))) {
        encodingTaskRef->setEstimatedFrameCount(m_currentTimeIndex);
        disconnect(win, &QQuickWindow::beforeRendering, this, &RenderTarget::drawOffscreenForVideo);
        connect(win, &QQuickWindow::afterRendering, this, &RenderTarget::launchEncodingTask);
    }
    else {
        const qreal &currentTimeIndex = m_currentTimeIndex;
        const QString &path = encodingTaskRef->generateFilename(currentTimeIndex);
        setCurrentTimeIndex(currentTimeIndex + 1);
        m_projectProxyRef->update(Scene::kUpdateAll);
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
    if (suffix != "bmp" && !QQuickWindow::hasDefaultAlphaBuffer()) {
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

void RenderTarget::launchEncodingTask()
{
    Q_ASSERT(window());
    disconnect(window(), &QQuickWindow::afterRendering, this, &RenderTarget::launchEncodingTask);
    encodingTask()->launch();
    m_exportSize = QSize();
}

void RenderTarget::prepareSyncMotionState()
{
    Q_ASSERT(window());
    connect(window(), &QQuickWindow::beforeRendering, this, &RenderTarget::syncMotionState, Qt::DirectConnection);
}

void RenderTarget::prepareUpdatingLight()
{
    if (QQuickWindow *win = window()) {
        connect(win, &QQuickWindow::beforeRendering, this, &RenderTarget::performUpdatingLight, Qt::DirectConnection);
    }
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
        m_grid->load(m_applicationContext->sharedFunctionResolverInstance());
        QOpenGLContext *contextRef = win->openglContext();
        connect(contextRef, &QOpenGLContext::aboutToBeDestroyed, m_projectProxyRef, &ProjectProxy::reset, Qt::DirectConnection);
        connect(contextRef, &QOpenGLContext::aboutToBeDestroyed, this, &RenderTarget::release, Qt::DirectConnection);
        toggleRunning(true);
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
    m_modelDrawer.reset();
    m_applicationContext->releaseShadowMap();
    if (m_videoSurface) {
        m_videoSurface->release();
    }
    m_grid.reset();
}

void RenderTarget::uploadModelAsync(ModelProxy *model, bool isProject)
{
    Q_ASSERT(window() && model && m_applicationContext);
    const QUuid &uuid = model->uuid();
    VPVL2_VLOG(1, "Enqueued uploading the model " << uuid.toString().toStdString() << " a.k.a " << model->name().toStdString());
    m_applicationContext->enqueueModelProxyToUpload(model, isProject);
    connect(window(), &QQuickWindow::beforeRendering, this, &RenderTarget::performUploadingEnqueuedModels, Qt::DirectConnection);
}

void RenderTarget::deleteModelAsync(ModelProxy *model)
{
    Q_ASSERT(m_applicationContext);
    if (model) {
        VPVL2_VLOG(1, "The model " << model->uuid().toString().toStdString() << " a.k.a " << model->name().toStdString() << " will be released from RenderTarget");
        if (m_modelDrawer && model == m_modelDrawer->currentModelProxyRef()) {
            m_modelDrawer->setModelProxyRef(0);
        }
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
    const QList<ApplicationContext::ModelProxyPair> &uploadedModelProxies = m_applicationContext->uploadEnqueuedModelProxies(m_projectProxyRef);
    foreach (const ApplicationContext::ModelProxyPair &pair, uploadedModelProxies) {
        ModelProxy *modelProxy = pair.first;
        VPVL2_VLOG(1, "The model " << modelProxy->uuid().toString().toStdString() << " a.k.a " << modelProxy->name().toStdString() << " is uploaded" << (pair.second ? " from the project." : "."));
        connect(modelProxy, &ModelProxy::transformTypeChanged, this, &RenderTarget::updateGizmo);
        connect(modelProxy, &ModelProxy::firstTargetBoneChanged, this, &RenderTarget::updateGizmo);
        emit modelDidUpload(modelProxy, pair.second);
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

void RenderTarget::performUpdatingLight()
{
    Q_ASSERT(window() && m_applicationContext && m_projectProxyRef);
    disconnect(window(), &QQuickWindow::beforeRendering, this, &RenderTarget::performUpdatingLight);
    const LightRefObject *light = m_projectProxyRef->light();
    const qreal &shadowDistance = light->shadowDistance();
    const Vector3 &direction = light->data()->direction(),
            &eye = -direction * shadowDistance,
            &center = direction * shadowDistance;
    const glm::mediump_float &aspectRatio = m_shadowMapSize.x() / float(m_shadowMapSize.y());
    const glm::mat4 &lightView = glm::lookAt(glm::vec3(eye.x(), eye.y(), eye.z()),
                                             glm::vec3(center.x(), center.y(), center.z()),
                                             glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::mat4 &lightProjection = glm::infinitePerspective(45.0f, aspectRatio, 0.1f);
    m_applicationContext->setLightMatrices(glm::mat4(), lightView, lightProjection);
    Scene *scene = m_projectProxyRef->projectInstanceRef();
    if (light->shadowType() == LightRefObject::SelfShadow) {
        const Vector3 size(m_shadowMapSize.x(), m_shadowMapSize.y(), 1);
        m_applicationContext->createShadowMap(size);
    }
    else {
        scene->setShadowMapRef(0);
    }
}

void RenderTarget::prepareProject()
{
    /* disable below signals behavior while loading project */
    disconnect(m_projectProxyRef, &ProjectProxy::motionDidLoad, this, &RenderTarget::prepareSyncMotionState);
    disconnect(this, &RenderTarget::shadowMapSizeChanged, this, &RenderTarget::prepareUpdatingLight);
    disconnect(m_projectProxyRef, &ProjectProxy::rewindDidPerform, this, &RenderTarget::prepareSyncMotionState);
    disconnect(m_projectProxyRef->world(), &WorldProxy::simulationTypeChanged, this, &RenderTarget::prepareSyncMotionState);
    disconnect(m_projectProxyRef->light(), &LightRefObject::directionChanged, this, &RenderTarget::prepareUpdatingLight);
    disconnect(m_projectProxyRef->light(), &LightRefObject::shadowTypeChanged, this, &RenderTarget::prepareUpdatingLight);
}

void RenderTarget::activateProject()
{
    Q_ASSERT(m_applicationContext && m_projectProxyRef);
    setShadowMapSize(m_projectProxyRef->globalSetting("shadow.texture.size", kDefaultShadowMapSize));
    m_applicationContext->setSceneRef(m_projectProxyRef->projectInstanceRef());
    m_applicationContext->resetOrderIndex(m_projectProxyRef->modelProxies().count() + 1);
    connect(this, &RenderTarget::shadowMapSizeChanged, this, &RenderTarget::prepareUpdatingLight);
    connect(m_projectProxyRef, &ProjectProxy::motionDidLoad, this, &RenderTarget::prepareSyncMotionState);
    connect(m_projectProxyRef, &ProjectProxy::rewindDidPerform, this, &RenderTarget::prepareSyncMotionState);
    connect(m_projectProxyRef->world(), &WorldProxy::simulationTypeChanged, this, &RenderTarget::prepareSyncMotionState);
    connect(m_projectProxyRef->light(), &LightRefObject::directionChanged, this, &RenderTarget::prepareUpdatingLight);
    connect(m_projectProxyRef->light(), &LightRefObject::shadowTypeChanged, this, &RenderTarget::prepareUpdatingLight);
    prepareSyncMotionState();
    prepareUpdatingLight();
}

QMediaPlayer *RenderTarget::mediaPlayer() const
{
    if (!m_mediaPlayer) {
        m_mediaPlayer.reset(new QMediaPlayer());
        m_videoSurface.reset(new VideoSurface(m_mediaPlayer.data()));
        connect(m_mediaPlayer.data(), SIGNAL(error(QMediaPlayer::Error)), this, SLOT(handleMediaPlayerError(QMediaPlayer::Error)));
    }
    return m_mediaPlayer.data();
}

RenderTarget::EncodingTask *RenderTarget::encodingTask() const
{
    if (!m_encodingTask) {
        m_encodingTask.reset(new EncodingTask());
        connect(m_encodingTask.data(), &EncodingTask::encodeDidBegin, this, &RenderTarget::encodeDidBegin);
        connect(m_encodingTask.data(), &EncodingTask::encodeDidProceed, this, &RenderTarget::encodeDidProceed);
        connect(m_encodingTask.data(), &EncodingTask::encodeDidFinish, this, &RenderTarget::encodeDidFinish);
    }
    return m_encodingTask.data();
}

IGizmo *RenderTarget::translationGizmo() const
{
    if (!m_translationGizmo) {
        m_translationGizmo.reset(CreateMoveGizmo());
        m_translationGizmo->SetSnap(m_snapStepSize.x(), m_snapStepSize.y(), m_snapStepSize.z());
        m_translationGizmo->SetEditMatrix(const_cast<float *>(m_editMatrix.data()));
    }
    return m_translationGizmo.data();
}

IGizmo *RenderTarget::orientationGizmo() const
{
    if (!m_orientationGizmo) {
        m_orientationGizmo.reset(CreateRotateGizmo());
        m_orientationGizmo->SetEditMatrix(const_cast<float *>(m_editMatrix.data()));
        m_orientationGizmo->SetAxisMask(m_visibleGizmoMasks);
    }
    return m_orientationGizmo.data();
}

void RenderTarget::clearScene()
{
    const QColor &color = m_projectProxyRef ? m_projectProxyRef->screenColor() : QColor(Qt::white);
    glClearColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void RenderTarget::drawShadowMap()
{
    m_applicationContext->renderShadowMap();
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
    const bool isProjectiveShadow = m_projectProxyRef->light()->shadowType() == LightRefObject::ProjectiveShadow;
    for (int i = 0, nengines = enginesForStandard.count(); i < nengines; i++) {
        IRenderEngine *engine = enginesForStandard[i];
        engine->renderModel();
        engine->renderEdge();
        if (isProjectiveShadow) {
            engine->renderShadow();
        }
    }
}

void RenderTarget::drawDebug()
{
    Q_ASSERT(m_projectProxyRef);
    WorldProxy *worldProxy = m_projectProxyRef->world();
    if (worldProxy->isDebugEnabled() && worldProxy->simulationType() != WorldProxy::DisableSimulation) {
        if (!m_debugDrawer) {
            m_debugDrawer.reset(new DebugDrawer());
            m_debugDrawer->initialize();
            m_debugDrawer->setModelViewProjectionMatrix(Util::fromMatrix4(m_viewProjectionMatrix));
            m_debugDrawer->setDebugMode(btIDebugDraw::DBG_DrawAabb |
                                        // btIDebugDraw::DBG_DrawConstraintLimits |
                                        // btIDebugDraw::DBG_DrawConstraints |
                                        // btIDebugDraw::DBG_DrawContactPoints |
                                        // btIDebugDraw::DBG_DrawFeaturesText |
                                        // btIDebugDraw::DBG_DrawText |
                                        // btIDebugDraw::DBG_FastWireframe |
                                        // btIDebugDraw::DBG_DrawWireframe |
                                        0);
            worldProxy->setDebugDrawer(m_debugDrawer.data());
        }
        worldProxy->debugDraw();
        m_debugDrawer->flush();
    }
}

void RenderTarget::drawModelBones()
{
    Q_ASSERT(m_projectProxyRef);
    ModelProxy *currentModelRef = m_projectProxyRef->currentModel();
    if (!m_playing && m_editMode == SelectMode && currentModelRef && currentModelRef->isVisible()) {
        if (!m_modelDrawer) {
            m_modelDrawer.reset(new ModelDrawer());
            connect(m_projectProxyRef, &ProjectProxy::undoDidPerform, m_modelDrawer.data(), &RenderTarget::ModelDrawer::updateModel);
            connect(m_projectProxyRef, &ProjectProxy::redoDidPerform, m_modelDrawer.data(), &RenderTarget::ModelDrawer::updateModel);
            m_modelDrawer->initialize();
            m_modelDrawer->setModelViewProjectionMatrix(Util::fromMatrix4(m_viewProjectionMatrix));
            m_modelDrawer->setModelProxyRef(currentModelRef);
        }
        m_modelDrawer->setModelProxyRef(currentModelRef);
        m_modelDrawer->draw();
    }
}

void RenderTarget::drawCurrentGizmo()
{
    if (!m_playing && m_currentGizmoRef) {
        m_currentGizmoRef->Draw();
    }
}

void RenderTarget::drawOffscreen(QOpenGLFramebufferObject *fbo)
{
    Scene::resetInitialOpenGLStates();
    drawShadowMap();
    Q_ASSERT(fbo->isValid());
    fbo->bind();
    glm::mat4 modelMatrix, viewMatrix, projectionMatrix;
    m_applicationContext->getCameraMatrices(modelMatrix, viewMatrix, projectionMatrix);
    const ICamera *camera = m_projectProxyRef->camera()->data();
    const glm::mediump_float &aspect = fbo->width() > fbo->height()
            ? fbo->width() / glm::mediump_float(fbo->height())
            : fbo->height() / glm::mediump_float(fbo->width());
    Q_ASSERT(aspect != 0);
    const glm::mat4 &newProjectionMatrix = glm::infinitePerspective(camera->fov(), aspect, camera->znear());
    m_applicationContext->setCameraMatrices(modelMatrix, viewMatrix, newProjectionMatrix);
    glViewport(0, 0, fbo->width(), fbo->height());
    clearScene();
    drawScene();
    m_applicationContext->setCameraMatrices(modelMatrix, viewMatrix, projectionMatrix);
    fbo->bindDefault();
}

void RenderTarget::updateViewport()
{
    Q_ASSERT(m_applicationContext);
    int w = m_viewport.width(), h = m_viewport.height();
    if (isDirty()) {
        glm::mat4 cameraWorld, cameraView, cameraProjection;
        glm::vec2 size(w, h);
        m_applicationContext->updateCameraMatrices(size);
        m_applicationContext->getCameraMatrices(cameraWorld, cameraView, cameraProjection);
        m_viewMatrix = cameraView;
        m_projectionMatrix = cameraProjection;
        m_viewProjectionMatrix = cameraProjection * cameraView;
        if (m_debugDrawer) {
            m_debugDrawer->setModelViewProjectionMatrix(Util::fromMatrix4(m_viewProjectionMatrix));
        }
        if (m_modelDrawer) {
            m_modelDrawer->setModelViewProjectionMatrix(Util::fromMatrix4(m_viewProjectionMatrix));
        }
        IGizmo *translationGizmoRef = translationGizmo(), *orientationGizmoRef = orientationGizmo();
        translationGizmoRef->SetScreenDimension(w, h);
        translationGizmoRef->SetCameraMatrix(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection));
        orientationGizmoRef->SetScreenDimension(w, h);
        orientationGizmoRef->SetCameraMatrix(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection));
        emit viewMatrixChanged();
        emit projectionMatrixChanged();
        setDirty(false);
    }
    glViewport(m_viewport.x(), m_viewport.y(), w, h);
}

void RenderTarget::seekVideo(const qreal &value)
{
    if (m_mediaPlayer) {
        const quint64 &position = qRound64((value / Scene::defaultFPS()) * 1000);
        m_mediaPlayer->setPosition(position);
    }
}

#include "RenderTarget.moc"
