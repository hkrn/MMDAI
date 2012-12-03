/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
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

#include "vpvl2/vpvl2.h"
#include "vpvl2/extensions/gl/FrameBufferObject.h"
#include "vpvl2/qt/Archive.h"
#include "vpvl2/qt/CString.h"
#include "vpvl2/qt/RenderContext.h"
#include "vpvl2/qt/Util.h"

#include <QtCore/QtCore>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtConcurrent/QtConcurrent>
#endif

#ifdef VPVL2_ENABLE_NVIDIA_CG
/* to cast IEffect#internalPointer and IEffect#internalContext */
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#endif

#ifdef VPVL2_LINK_NVTT
#include <nvcore/Stream.h>
#include <nvimage/DirectDrawSurface.h>
#include <nvimage/Image.h>
#include <nvimage/ImageIO.h>
#else
namespace nv {
class Stream {
public:
    Stream() {}
    virtual ~Stream() {}
};
}
#endif

namespace
{

using namespace vpvl2;
using namespace vpvl2::qt;

#ifdef VPVL2_LINK_NVTT

class ReadonlyFileStream : public nv::Stream {
public:
    ReadonlyFileStream(const QString &path) : m_file(path) { m_file.open(QFile::ReadOnly); }
    ~ReadonlyFileStream() {}

    bool isSaving() const { return false; }
    bool isError() const { return m_file.error() != QFile::NoError; }
    void seek(uint pos) { m_file.seek(pos); }
    uint tell() const { return m_file.pos(); }
    uint size() const { return m_file.size(); }
    void clearError() {}
    bool isAtEnd() const { return m_file.atEnd(); }
    bool isSeekable() const { return m_file.isSequential(); }
    bool isLoading() const { return true; }
    uint serialize(void *data, uint len) { return m_file.read(static_cast<char *>(data), len); }

private:
    QFile m_file;
};

class ReadonlyMemoryStream : public nv::Stream {
public:
    ReadonlyMemoryStream(QByteArray &bytes) : m_buffer(&bytes) { m_buffer.open(QBuffer::ReadOnly); }
    ~ReadonlyMemoryStream() {}

    bool isSaving() const { return false; }
    bool isError() const { return false; }
    void seek(uint pos) { m_buffer.seek(pos); }
    uint tell() const { return m_buffer.pos(); }
    uint size() const { return m_buffer.size(); }
    void clearError() {}
    bool isAtEnd() const { return m_buffer.atEnd(); }
    bool isSeekable() const { return m_buffer.isSequential(); }
    bool isLoading() const { return true; }
    uint serialize(void *data, uint len) { return m_buffer.read(static_cast<char *>(data), len); }

private:
    QBuffer m_buffer;
};

#else /* VPVL2_LINK_NVTT */

class ReadonlyFileStream : public nv::Stream {
public:
    ReadonlyFileStream(const QString &/*path*/) {}
    ~ReadonlyFileStream() {}
};

class ReadonlyMemoryStream : public nv::Stream {
public:
    ReadonlyMemoryStream(QByteArray &/*bytes*/) {}
    ~ReadonlyMemoryStream() {}
};

#endif /* VPVL2_LINK_NVTT */

static void UIConcatModelTransformMatrix(const IModel *model, QMatrix4x4 &m)
{
    Transform transform;
    transform.setOrigin(model->worldPosition());
    transform.setRotation(model->worldRotation());
    QMatrix4x4 worldMatrix;
    Scalar matrix[16];
    transform.getOpenGLMatrix(matrix);
    for (int i = 0; i < 16; i++)
        worldMatrix.data()[i] = matrix[i];
    m *= worldMatrix;
    const IBone *bone = model->parentBone();
    if (bone) {
        transform = bone->worldTransform();
        transform.getOpenGLMatrix(matrix);
        for (int i = 0; i < 16; i++)
            worldMatrix.data()[i] = matrix[i];
        m *= worldMatrix;
    }
}

const QString UICreatePath(const IString *dir, const IString *name)
{
    const QDir d(static_cast<const CString *>(dir)->value());
    QString path = d.absoluteFilePath(static_cast<const CString *>(name)->value());
    path.replace(0x5c, '/');
    return path;
}

QGLContext::BindOptions UIGetTextureBindOptions(bool enableMipmap)
{
    // disable premultiplified alpha option
    QGLContext::BindOptions options = QGLContext::LinearFilteringBindOption
            | QGLContext::InvertedYBindOption;
    if (enableMipmap)
        options |= QGLContext::MipmapBindOption;
    return options;
}

#ifdef VPVL2_LINK_NVTT
static const QImage UIConvertNVImageToQImage(const nv::Image &image)
{
    const uint8_t *pixels = reinterpret_cast<const uchar *>(image.pixels());
    QImage::Format format = image.format() == nv::Image::Format_ARGB
            ? QImage::Format_ARGB32 : QImage::Format_RGB32;
    return QImage(pixels, image.width(), image.height(), format);
}
#endif

}

namespace vpvl2
{
namespace qt
{

using namespace extensions::gl;

QSet<QString> RenderContext::loadableTextureExtensions()
{
    static QSet<QString> extensions;
    if (extensions.isEmpty()) {
        extensions << "jpg";
        extensions << "png";
        extensions << "bmp";
        extensions << "sph";
        extensions << "spa";
    }
    return extensions;
}

QString RenderContext::readAllAsync(const QString &path)
{
    QByteArray bytes;
    return UISlurpFile(path, bytes) ? bytes : QString();
}

RenderContext::RenderContext(const QHash<QString, QString> &settings, Scene *scene, QGLWidget *context)
    : m_settings(settings),
      m_systemDir(m_settings.value("dir.system.toon", "../../VPVM/resources/images")),
      m_scene(scene),
      m_context(context),
      m_archive(0),
      m_msaaSamples(0),
      m_frameBufferObjectBound(false)
{
    for (int i = 0; i < 4; i++)
        m_previousFrameBufferPtrs.insert(i, 0);
    m_timer.start();
}

RenderContext::~RenderContext()
{
    setSceneRef(0);
    qDeleteAll(m_renderTargets);
    m_renderTargets.clear();
    delete m_archive;
    m_lightWorldMatrix.setToIdentity();
    m_lightViewMatrix.setToIdentity();
    m_lightProjectionMatrix.setToIdentity();
    m_cameraModelMatrix.setToIdentity();
    m_cameraViewMatrix.setToIdentity();
    m_cameraProjectionMatrix.setToIdentity();
    m_mouseCursorPosition.setZero();
    m_mouseLeftPressPosition.setZero();
    m_mouseMiddlePressPosition.setZero();
    m_mouseRightPressPosition.setZero();
    m_context = 0;
    m_archive = 0;
    m_msaaSamples = 0;
}

void RenderContext::allocateUserData(const IModel *model, void *&context)
{
    const IString *name = model->name();
    InternalContext *ctx = new(std::nothrow) InternalContext();
    context = ctx;
    qDebug("Allocated the context: %s", name ? name->toByteArray() : reinterpret_cast<const uint8_t *>("(null)"));
}

void RenderContext::releaseUserData(const IModel *model, void *&context)
{
    const IString *name = model->name();
    delete static_cast<InternalContext *>(context);
    context = 0;
    qDebug("Released the context: %s", name ? name->toByteArray() : reinterpret_cast<const uint8_t *>("(null)"));
}

bool RenderContext::uploadTexture(const IString *name, const IString *dir, int flags, Texture &texture, void *context)
{
    bool mipmap = flags & IRenderContext::kGenerateTextureMipmap;
    bool isToon = flags & IRenderContext::kToonTexture;
    InternalTexture t(&texture, mipmap, isToon);
    if (flags & IRenderContext::kTexture2D) {
        const QString &path = UICreatePath(dir, name);
        return uploadTextureInternal(path, t, context);
    }
    else if (flags & IRenderContext::kToonTexture) {
        bool ret = false;
        if (dir) {
            const QString &path = UICreatePath(dir, name);
            ret = uploadTextureInternal(path, t, context);
        }
        if (!t.ok) {
            CString s(m_systemDir.absolutePath());
            const QString &path = UICreatePath(&s, name);
            t.isSystem = true;
            ret = uploadTextureInternal(path, t, context);
            qDebug("Loaded a system texture: %s", qPrintable(path));
        }
        return ret;
    }
    return false;
}

void RenderContext::getToonColor(const IString *name, const IString *dir, Color &value, void * /* context */)
{
    const QString &path = UICreatePath(dir, name);
    bool ok = false;
    if (m_archive || QFile::exists(path)) {
        getToonColorInternal(path, false, value, ok);
    }
    if (!ok) {
        CString s(m_systemDir.absolutePath());
        const QString &fallback = UICreatePath(&s, name);
        getToonColorInternal(fallback, true, value, ok);
    }
}

void RenderContext::uploadAnimatedTexture(float offset, float speed, float seek, void *texture)
{
    GLuint textureID = *static_cast<GLuint *>(texture);
    QMovie *movie = 0;
    if (m_texture2Movies.contains(textureID)) {
        movie = m_texture2Movies[textureID];
    }
    else {
        const QString &path = m_texture2Paths[textureID];
        m_texture2Movies.insert(textureID, new QMovie(path));
        movie = m_texture2Movies[textureID];
        movie->setCacheMode(QMovie::CacheAll);
    }
    if (movie->isValid()) {
        offset *= Scene::defaultFPS();
        int frameCount = movie->frameCount();
        offset = qBound(0, int(offset), frameCount);
        int left = int(seek * speed * Scene::defaultFPS() + frameCount - offset);
        int right = qMax(int(frameCount - offset), 1);
        int frameIndex = left % right + int(offset);
        if (movie->jumpToFrame(frameIndex)) {
            const QImage &image = movie->currentImage();
            const QImage &textureImage = QGLWidget::convertToGLFormat(image.mirrored());
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width(), image.height(), GL_RGBA, GL_UNSIGNED_BYTE, textureImage.constBits());
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
}

void RenderContext::getMatrix(float value[], const IModel *model, int flags) const
{
    QMatrix4x4 m;
    if (flags & IRenderContext::kShadowMatrix) {
        if (flags & IRenderContext::kProjectionMatrix)
            m *= m_cameraProjectionMatrix;
        if (flags & IRenderContext::kViewMatrix)
            m *= m_cameraViewMatrix;
        if (flags & IRenderContext::kWorldMatrix) {
            static const Vector3 plane(0.0f, 1.0f, 0.0f);
            const ILight *light = m_scene->light();
            const Vector3 &direction = light->direction();
            const Scalar dot = plane.dot(-direction);
            QMatrix4x4 shadowMatrix;
            for (int i = 0; i < 4; i++) {
                int offset = i * 4;
                for (int j = 0; j < 4; j++) {
                    int index = offset + j;
                    shadowMatrix.data()[index] = plane[i] * direction[j];
                    if (i == j)
                        shadowMatrix.data()[index] += dot;
                }
            }
            UIConcatModelTransformMatrix(model, m);
            m *= shadowMatrix;
            m *= m_cameraModelMatrix;
            m.scale(model->scaleFactor());
        }
    }
    else if (flags & IRenderContext::kCameraMatrix) {
        if (flags & IRenderContext::kProjectionMatrix)
            m *= m_cameraProjectionMatrix;
        if (flags & IRenderContext::kViewMatrix)
            m *= m_cameraViewMatrix;
        if (flags & IRenderContext::kWorldMatrix) {
            UIConcatModelTransformMatrix(model, m);
            m *= m_cameraModelMatrix;
            m.scale(model->scaleFactor());
        }
    }
    else if (flags & IRenderContext::kLightMatrix) {
        if (flags & IRenderContext::kProjectionMatrix)
            m *= m_lightProjectionMatrix;
        if (flags & IRenderContext::kViewMatrix)
            m *= m_lightViewMatrix;
        if (flags & IRenderContext::kWorldMatrix) {
            m *= m_lightWorldMatrix;
            m.scale(model->scaleFactor());
        }
    }
    if (flags & IRenderContext::kInverseMatrix)
        m = m.inverted();
    if (flags & IRenderContext::kTransposeMatrix)
        m = m.transposed();
    for (int i = 0; i < 16; i++) {
        value[i] = float(m.constData()[i]);
    }
}

void RenderContext::getViewport(Vector3 &value) const
{
    value.setValue(m_viewport.width(), m_viewport.height(), 0);
}

void RenderContext::getTime(float &value, bool sync) const
{
    value = sync ? 0 : m_timer.elapsed() / 1000.0f;
}

void RenderContext::getElapsed(float &value, bool sync) const
{
    value = sync ? 0 : 1.0 / 60.0;
}

void RenderContext::getMousePosition(Vector4 &value, MousePositionType type) const
{
    switch (type) {
    case kMouseLeftPressPosition:
        value.setValue(m_mouseLeftPressPosition.x(),
                       m_mouseLeftPressPosition.y(),
                       m_mouseLeftPressPosition.z(),
                       m_mouseLeftPressPosition.w());
        break;
    case kMouseMiddlePressPosition:
        value.setValue(m_mouseMiddlePressPosition.x(),
                       m_mouseMiddlePressPosition.y(),
                       m_mouseMiddlePressPosition.z(),
                       m_mouseMiddlePressPosition.w());
        break;
    case kMouseRightPressPosition:
        value.setValue(m_mouseRightPressPosition.x(),
                       m_mouseRightPressPosition.y(),
                       m_mouseRightPressPosition.z(),
                       m_mouseRightPressPosition.w());
        break;
    case kMouseCursorPosition:
        value.setValue(m_mouseCursorPosition.x(),
                       m_mouseCursorPosition.y(),
                       m_mouseCursorPosition.z(),
                       m_mouseCursorPosition.w());
        break;
    default:
        break;
    }
}

void RenderContext::log(void * /* context */, LogLevel /* level */, const char *format, va_list ap)
{
    vfprintf(stderr, format, ap);
    fprintf(stderr, "%s", "\n");
}

IString *RenderContext::loadKernelSource(KernelType type, void * /* context */)
{
    QString file;
    switch (type) {
    case kModelSkinningKernel:
        file = "skinning.cl";
        break;
    case kMaxKernelType:
    default:
        break;
    }
    const QString &path = QDir(m_settings.value("dir.system.kernels", "../../VPVM/resources/kernels")).absoluteFilePath(file);
    const QFuture<QString> &future = QtConcurrent::run(&RenderContext::readAllAsync, path);
    const QString &source = future.result();
    if (!source.isNull() && !future.isCanceled()) {
        qDebug("Loaded a kernel: %s", qPrintable(path));
        return new(std::nothrow) CString(source);
    }
    else {
        return 0;
    }
}

IString *RenderContext::loadShaderSource(ShaderType type, const IString *path)
{
    if (type == kModelEffectTechniques) {
        const QFuture<QString> &future = QtConcurrent::run(&RenderContext::readAllAsync, static_cast<const CString *>(path)->value());
        const QString &source = future.result();
        return !source.isNull() ? new (std::nothrow) CString(source) : 0;
    }
    return 0;
}

IString *RenderContext::loadShaderSource(ShaderType type, const IModel *model, const IString *dir, void * /* context */)
{
    QString file;
    if (type == kModelEffectTechniques) {
        const QString &filename = effectFilePath(model, dir);
        const QFuture<QString> &future = QtConcurrent::run(&RenderContext::readAllAsync, filename);
        const QString &source = future.result();
        return !source.isNull() ? new (std::nothrow) CString(source) : 0;
    }
    switch (model->type()) {
    case IModel::kAsset:
        file += "asset/";
        break;
    case IModel::kPMD:
    case IModel::kPMX:
        file += "pmx/";
        break;
    default:
        break;
    }
    switch (type) {
    case kEdgeVertexShader:
        file += "edge.vsh";
        break;
    case kEdgeFragmentShader:
        file += "edge.fsh";
        break;
    case kModelVertexShader:
        file += "model.vsh";
        break;
    case kModelFragmentShader:
        file += "model.fsh";
        break;
    case kShadowVertexShader:
        file += "shadow.vsh";
        break;
    case kShadowFragmentShader:
        file += "shadow.fsh";
        break;
    case kZPlotVertexShader:
        file += "zplot.vsh";
        break;
    case kZPlotFragmentShader:
        file += "zplot.fsh";
        break;
    case kEdgeWithSkinningVertexShader:
        file += "skinning/edge.vsh";
        break;
    case kModelWithSkinningVertexShader:
        file += "skinning/model.vsh";
        break;
    case kShadowWithSkinningVertexShader:
        file += "skinning/shadow.vsh";
        break;
    case kZPlotWithSkinningVertexShader:
        file += "skinning/zplot.vsh";
        break;
    case kModelEffectTechniques:
    case kMaxShaderType:
    default:
        break;
    }
    const QString &path = QDir(m_settings.value("dir.system.shaders", "../../VPVM/resources/shaders")).absoluteFilePath(file);
    const QFuture<QString> &future = QtConcurrent::run(&RenderContext::readAllAsync, path);
    const QString &source = future.result();
    if (!source.isNull() && !future.isCanceled()) {
        qDebug("Loaded a shader: %s", qPrintable(path));
        return new(std::nothrow) CString(m_shaderSourcePrefix + source);
    }
    else {
        return 0;
    }
}

IString *RenderContext::toUnicode(const uint8_t *value) const
{
    QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
    const QString &s = codec->toUnicode(reinterpret_cast<const char *>(value));
    return new(std::nothrow) CString(s);
}

bool RenderContext::hasExtension(const void *namePtr) const
{
    return m_extensions.contains(static_cast<const char *>(namePtr));
}

void *RenderContext::findProcedureAddress(const void **candidatesPtr) const
{
    const QGLContext *context = QGLContext::currentContext();
    const char **candidates = reinterpret_cast<const char **>(candidatesPtr);
    const char *candidate = candidates[0];
    int i = 0;
    while (candidate) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        void *address = reinterpret_cast<void *>(context->getProcAddress(candidate));
#else
        void *address = context->getProcAddress(candidate);
#endif
        if (address) {
            return address;
        }
        candidate = candidates[++i];
    }
    return 0;
}

void RenderContext::startProfileSession(ProfileType type, const void *arg)
{
    m_profilerTimers[ProfilerKey(type, arg)].restart();
}

void RenderContext::stopProfileSession(ProfileType type, const void *arg)
{
    m_profilerTimers[ProfilerKey(type, arg)].elapsed();
}

void RenderContext::setArchive(Archive *value)
{
    delete m_archive;
    m_archive = value;
}

void RenderContext::setSceneRef(Scene *value)
{
#ifdef VPVL2_LINK_NVIDIA_CG
    m_offscreenTextures.clear();
#endif
    m_model2Paths.clear();
    QMutexLocker locker(&m_effectCachesLock); Q_UNUSED(locker);
    qDeleteAll(m_texture2Movies);
    qDeleteAll(m_effectCaches);
    m_texture2Movies.clear();
    m_effectCaches.clear();
    m_scene = value;
}

void RenderContext::updateMatrices(const QSizeF &size)
{
    float matrix[16];
    ICamera *camera = m_scene->camera();
    camera->modelViewTransform().getOpenGLMatrix(matrix);
    for (int i = 0; i < 16; i++)
        m_cameraViewMatrix.data()[i] = matrix[i];
    m_cameraProjectionMatrix.setToIdentity();
    m_cameraProjectionMatrix.perspective(camera->fov(), size.width() / size.height(), camera->znear(), camera->zfar());
    m_viewport = size;
}

void RenderContext::getCameraMatrices(QMatrix4x4 &world, QMatrix4x4 &view, QMatrix4x4 &projection)
{
    world = m_cameraModelMatrix;
    view = m_cameraViewMatrix;
    projection = m_cameraProjectionMatrix;
}

void RenderContext::setCameraModelMatrix(const QMatrix4x4 &value)
{
    m_cameraModelMatrix = value;
}

void RenderContext::getLightMatrices(QMatrix4x4 &world, QMatrix4x4 &view, QMatrix4x4 &projection)
{
    world = m_lightWorldMatrix;
    view = m_lightViewMatrix;
    projection = m_lightProjectionMatrix;
}

void RenderContext::setLightMatrices(const QMatrix4x4 &world, const QMatrix4x4 &view, const QMatrix4x4 &projection)
{
    m_lightWorldMatrix = world;
    m_lightViewMatrix = view;
    m_lightProjectionMatrix = projection;
}

void RenderContext::setMousePosition(const Vector3 &value, bool pressed, MousePositionType type)
{
    switch (type) {
    case kMouseLeftPressPosition:
        m_mouseLeftPressPosition.setValue(value.x(), value.y(), pressed, 0);
        break;
    case kMouseMiddlePressPosition:
        m_mouseMiddlePressPosition.setValue(value.x(), value.y(), pressed, 0);
        break;
    case kMouseRightPressPosition:
        m_mouseRightPressPosition.setValue(value.x(), value.y(), pressed, 0);
        break;
    case kMouseCursorPosition:
        m_mouseCursorPosition.setValue(value.x(), value.y(), 0, 0);
        break;
    default:
        break;
    }
}

void RenderContext::addModelPath(IModel *model, const QString &filename)
{
    if (model) {
        QFileInfo finfo(filename);
        m_filename2Models.insert(finfo.fileName(), model);
        QMutexLocker locker(&m_model2PathLock); Q_UNUSED(locker);
        m_model2Paths.insert(model, filename);
    }
}

const QString RenderContext::findModelPath(const IModel *model) const
{
    QMutexLocker locker(&m_model2PathLock); Q_UNUSED(locker);
    const QString s = m_model2Paths[model];
    return s;
}

const QString RenderContext::effectFilePath(const IModel *model, const IString *dir) const
{
    QDir d(static_cast<const CString *>(dir)->value());
    const QString &path = findModelPath(model);
    if (!path.isEmpty()) {
        const QString &s = QFileInfo(path).completeBaseName();
        const QRegExp regexp("^.+\\[([^\\\\.]+)(?:\\.(?:cg)?fx)?\\]$");
        const QString &basename = regexp.exactMatch(s) ? regexp.capturedTexts().at(1) : s;
        const QString &cgfx = d.absoluteFilePath(basename + ".cgfx");
        if (QFile::exists(cgfx))
            return cgfx;
    }
    return d.absoluteFilePath("default.cgfx");
}

void RenderContext::removeModel(IModel *model)
{
    QMutableHashIterator<const QString, IModel *> it(m_filename2Models);
    while (it.hasNext()) {
        it.next();
        IModel *m = it.value();
        if (m == model) {
            it.remove();
        }
    }
    QMutexLocker locker(&m_effect2modelsLock); Q_UNUSED(locker);
    QMutableHashIterator<const IEffect *, IModel *> it2(m_effect2models);
    while (it2.hasNext()) {
        it2.next();
        IModel *m = it2.value();
        if (m == model) {
            it2.remove();
        }
    }
}

void RenderContext::initialize(bool enableMSAA)
{
    const QGLContext *context = QGLContext::currentContext();
    initializeGLFunctions(context);
    if (enableMSAA)
        glGetIntegerv(GL_MAX_SAMPLES, &m_msaaSamples);
    const QString extensions(reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS)));
    const GLubyte *shaderVersionString = glGetString(GL_SHADING_LANGUAGE_VERSION);
    int shaderVersion = qMax(QString(reinterpret_cast<const char *>(shaderVersionString)).toFloat(), 1.2f) * 100;
    m_shaderSourcePrefix.sprintf("#version %d", shaderVersion);
    Q_FOREACH (const QString &extension, extensions.split(' ', QString::SkipEmptyParts)) {
        m_extensions.insert(extension.trimmed());
    }
#ifndef __APPLE__
    glDrawBuffersPROC = reinterpret_cast<PFNGLDRAWBUFFERSPROC>(context->getProcAddress("glDrawBuffers"));
    if (!glDrawBuffersPROC)
        glDrawBuffersPROC = reinterpret_cast<PFNGLDRAWBUFFERSPROC>(context->getProcAddress("glDrawBuffersARB"));
#endif /* __APPLE__ */
}

bool RenderContext::uploadTextureNVTT(const QString &suffix,
                                      const QString &path,
                                      QScopedPointer<nv::Stream> &stream,
                                      InternalTexture &internalTexture,
                                      InternalContext *internalContext)
{
#ifdef VPVL2_LINK_NVTT
    if (suffix == "dds") {
        nv::DirectDrawSurface surface;
        if (surface.load(stream.take())) {
            nv::Image nvimage;
            surface.mipmap(&nvimage, 0, 0);
            QImage image(UIConvertNVImageToQImage(nvimage));
            return generateTextureFromImage(image, path, internalTexture, internalContext);
        }
        else {
            qWarning("%s cannot be loaded", qPrintable(path));
        }
    }
    else {
        QScopedPointer<nv::Image> nvimage(nv::ImageIO::load(path.toUtf8().constData(), *stream));
        if (nvimage) {
            QImage image(UIConvertNVImageToQImage(*nvimage));
            return generateTextureFromImage(image, path, internalTexture, internalContext);
        }
        else {
            qWarning("%s cannot be loaded", qPrintable(path));
        }
    }
#else
    Q_UNUSED(suffix)
    Q_UNUSED(path)
    Q_UNUSED(stream)
    Q_UNUSED(internalTexture)
    Q_UNUSED(internalContext)
#endif
    return true;
}

bool RenderContext::uploadTextureInternal(const QString &path, InternalTexture &internalTexture, void *context)
{
    const QFileInfo info(path);
    InternalContext *internalContext = static_cast<InternalContext *>(context);
    /* テクスチャのキャッシュを検索する */
    if (internalContext && internalContext->findTextureCache(path, internalTexture)) {
        return true;
    }
    /*
     * ZIP 圧縮からの読み込み (ただしシステムが提供する toon テクスチャは除く)
     * Archive が持つ仮想ファイルシステム上にあるため、キャッシュより後、物理ファイル上より先に検索しないといけない
     */
    const QString &suffix = info.suffix().toLower();
    if (m_archive && !internalTexture.isSystem) {
        const QByteArray &bytes = m_archive->data(path);
        if (loadableTextureExtensions().contains(suffix)) {
            QImage image;
            image.loadFromData(bytes);
            return generateTextureFromImage(image, path, internalTexture, internalContext);
        }
        else {
            QByteArray immutableBytes(bytes);
            QScopedPointer<nv::Stream> stream(new ReadonlyMemoryStream(immutableBytes));
            return uploadTextureNVTT(suffix, path, stream, internalTexture, internalContext);
        }
    }
    else if (info.isDir()) {
        if (internalTexture.isToon) { /* force loading as white toon texture */
            const QString &newPath = m_systemDir.absoluteFilePath("toon0.bmp");
            if (internalContext && !internalContext->findTextureCache(newPath, internalTexture)) {
                QImage image(newPath);
                return generateTextureFromImage(image, newPath, internalTexture, internalContext);
            }
        }
        return true; /* skip */
    }
    else if (!info.exists()) {
        qWarning("Cannot load inexist \"%s\"", qPrintable(path));
        return true; /* skip */
    }
    else if (loadableTextureExtensions().contains(suffix)) {
        QImage image(path);
        return generateTextureFromImage(image, path, internalTexture, internalContext);
    }
    else {
        QScopedPointer<nv::Stream> stream(new ReadonlyFileStream(path));
        return uploadTextureNVTT(suffix, path, stream, internalTexture, internalContext);
    }
}

bool RenderContext::generateTextureFromImage(const QImage &image,
                                             const QString &path,
                                             InternalTexture &internalTexture,
                                             InternalContext *internalContext)
{
    if (!image.isNull()) {
        size_t width = image.width(), height = image.height();
        GLuint textureID = m_context->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()),
                                                  GL_TEXTURE_2D,
                                                  GL_RGBA,
                                                  UIGetTextureBindOptions(internalTexture.mipmap));
        TextureCache cache(width, height, textureID, GL_RGBA);
        m_texture2Paths.insert(textureID, path);
        internalTexture.assign(cache);
        if (internalContext)
            internalContext->addTextureCache(path, cache);
        qDebug("Loaded a texture (ID=%d, width=%ld, height=%ld): \"%s\"",
               textureID, width, height, qPrintable(path));
        bool ok = internalTexture.ok = textureID != 0;
        return ok;
    }
    else {
        qWarning("Failed loading a image to convert the texture: %s", qPrintable(path));
        return false;
    }
}

void RenderContext::getToonColorInternal(const QString &path, bool isSystem, Color &value, bool &ok)
{
    QImage image(path);
    if (!isSystem && m_archive) {
        QByteArray suffix = QFileInfo(path).suffix().toLower().toUtf8();
        if (suffix == "sph" || suffix == "spa")
            suffix.setRawData("bmp", 3);
        const QByteArray &bytes = m_archive->data(path);
        image.loadFromData(bytes, suffix.constData());
    }
    if (!image.isNull()) {
        const QRgb &rgb = image.pixel(image.width() - 1, image.height() - 1);
        const QColor color(rgb);
        value.setValue(color.redF(), color.greenF(), color.blueF(), color.alphaF());
        ok = true;
    }
    else if (QFileInfo(path).isDir()) { // skip empty toon path
        value.setValue(1, 1, 1, 1);
        ok = true;
    }
    else {
        value.setValue(0, 0, 0, 1);
        ok = false;
    }
}


#ifdef VPVL2_ENABLE_NVIDIA_CG

IModel *RenderContext::findModel(const IString *name) const
{
    IModel *model = m_scene->findModel(name);
    if (!model) {
        const QString &s = static_cast<const CString *>(name)->value();
        model = m_filename2Models[s];
    }
    return model;
}

IModel *RenderContext::effectOwner(const IEffect *effect) const
{
    QMutexLocker locker(&m_effect2modelsLock); Q_UNUSED(locker);
    IModel *model = m_effect2models[effect];
    return model;
}

const QString RenderContext::effectOwnerName(const IEffect *effect) const
{
    QMutexLocker locker(&m_effectOwnersLock); Q_UNUSED(locker);
    const QString name = m_effectOwners[effect];
    return name;
}

void RenderContext::setEffectOwner(const IEffect *effect, IModel *model)
{
    const CString *name = static_cast<const CString *>(model->name());
    const QString &n = name ? name->value() : findModelPath(model);
    {
        QMutexLocker locker(&m_effectOwnersLock); Q_UNUSED(locker);
        m_effectOwners.insert(effect, n);
    }
    {
        QMutexLocker locker(&m_effect2modelsLock); Q_UNUSED(locker);
        m_effect2models.insert(effect, model);
    }
}

FrameBufferObject *RenderContext::findRenderTarget(const GLuint textureID, size_t width, size_t height)
{
    FrameBufferObject *buffer = 0;
    if (textureID > 0) {
        if (!m_renderTargets.contains(textureID)) {
            QScopedPointer<FrameBufferObject> fbo(new FrameBufferObject(width, height, m_msaaSamples));
            m_renderTargets.insert(textureID, fbo.data());
            fbo->create();
            buffer = fbo.take();
        }
        else {
            buffer = m_renderTargets[textureID];
        }
    }
    return buffer;
}

void RenderContext::setRenderColorTargets(const void *targets, const int ntargets)
{
    glDrawBuffersPROC(ntargets, static_cast<const GLenum *>(targets));
    m_frameBufferObjectBound = ntargets > 0;
    if (ntargets == 0)
        glDrawBuffer(GL_BACK);
}

FrameBufferObject *RenderContext::createFrameBufferObject()
{
    return new FrameBufferObject(m_viewport.width(), m_viewport.height(), m_msaaSamples);
}

bool RenderContext::hasFrameBufferObjectBound() const
{
    return m_frameBufferObjectBound;
}

void RenderContext::bindOffscreenRenderTarget(const OffscreenTexture &texture, bool /* enableAA */)
{
    const IEffect::OffscreenRenderTarget &rt = texture.renderTarget;
    if (FrameBufferObject *buffer = findRenderTarget(texture.textureID, rt.width, rt.height)) {
        buffer->bindTexture(texture.textureID, texture.textureFormat, 0);
        buffer->bindDepthStencilBuffer();
    }
}

void RenderContext::releaseOffscreenRenderTarget(const OffscreenTexture &texture, bool enableAA)
{
    const IEffect::OffscreenRenderTarget &rt = texture.renderTarget;
    if (FrameBufferObject *buffer = findRenderTarget(texture.textureID, rt.width, rt.height)) {
        if (enableAA) {
            buffer->blit(0);
        }
        buffer->unbind();
        setRenderColorTargets(0, 0);
    }
}

void RenderContext::parseOffscreenSemantic(IEffect *effect, const QDir &dir)
{
    if (effect) {
        static const QRegExp kExtensionReplaceRegExp(".fx(sub)?$");
        Array<IEffect::OffscreenRenderTarget> offscreenRenderTargets;
        effect->getOffscreenRenderTargets(offscreenRenderTargets);
        const int nOffscreenRenderTargets = offscreenRenderTargets.count();
        /* オフスクリーンレンダーターゲットの設定 */
        for (int i = 0; i < nOffscreenRenderTargets; i++) {
            const IEffect::OffscreenRenderTarget &renderTarget = offscreenRenderTargets[i];
            const CGparameter parameter = static_cast<const CGparameter>(renderTarget.textureParameter);
            const CGannotation annotation = cgGetNamedParameterAnnotation(parameter, "DefaultEffect");
            const QStringList defaultEffect = QString(cgGetStringAnnotationValue(annotation)).split(";");
            QList<EffectAttachment> attachments;
            Q_FOREACH (const QString &line, defaultEffect) {
                const QStringList &pair = line.split('=');
                if (pair.size() == 2) {
                    const QString &key = pair.at(0).trimmed();
                    const QString &value = pair.at(1).trimmed();
                    QRegExp regexp(key, Qt::CaseSensitive, QRegExp::Wildcard);
                    if (key == "self") {
                        const QString &name = effectOwnerName(effect);
                        regexp.setPattern(name);
                    }
                    if (value != "hide" && value != "none") {
                        QString path = dir.absoluteFilePath(value);
                        path.replace(kExtensionReplaceRegExp, ".cgfx");
                        CString s2(path);
                        const QFuture<IEffect *> &future = QtConcurrent::run(this, &RenderContext::createEffectAsync, &s2);
                        IEffect *offscreenEffect = future.result();
                        offscreenEffect->setParentEffect(effect);
                        attachments.append(EffectAttachment(regexp, offscreenEffect));
                    }
                    else {
                        attachments.append(EffectAttachment(regexp, 0));
                    }
                }
            }
            m_offscreenTextures.append(OffscreenTexture(renderTarget, attachments));
        }
    }
}

void RenderContext::renderOffscreen(const QSize &size)
{
    const Array<IRenderEngine *> &engines = m_scene->renderEngines();
    const int nengines = engines.count();
    QSize s;
    static const GLuint buffers[] = { GL_COLOR_ATTACHMENT0 };
    static const int nbuffers = sizeof(buffers) / sizeof(buffers[0]);
    Q_FOREACH (const RenderContext::OffscreenTexture &offscreen, offscreenTextures()) {
        const IEffect::OffscreenRenderTarget &renderTarget = offscreen.renderTarget;
        const CGparameter parameter = static_cast<CGparameter>(renderTarget.textureParameter);
        const CGannotation antiAlias = cgGetNamedParameterAnnotation(parameter, "AntiAlias");
        bool enableAA = false;
        if (cgIsAnnotation(antiAlias)) {
            int nvalues;
            const CGbool *values = cgGetBoolAnnotationValues(antiAlias, &nvalues);
            enableAA = nvalues > 0 ? values[0] == CG_TRUE : false;
        }
        bindOffscreenRenderTarget(offscreen, enableAA);
        setRenderColorTargets(buffers, nbuffers);
        const CGannotation clearColor = cgGetNamedParameterAnnotation(parameter, "ClearColor");
        if (cgIsAnnotation(clearColor)) {
            int nvalues;
            const float *color = cgGetFloatAnnotationValues(clearColor, &nvalues);
            if (nvalues == 4) {
                glClearColor(color[0], color[1], color[2], color[3]);
            }
        }
        else {
            glClearColor(1, 1, 1, 1);
        }
        const CGannotation clearDepth = cgGetNamedParameterAnnotation(parameter, "ClearDepth");
        if (cgIsAnnotation(clearDepth)) {
            int nvalues;
            const float *depth = cgGetFloatAnnotationValues(clearDepth, &nvalues);
            if (nvalues == 1) {
                glClearDepth(depth[0]);
            }
        }
        else {
            glClearDepth(0);
        }
        size_t width = renderTarget.width, height = renderTarget.height;
        s.setWidth(width);
        s.setHeight(height);
        updateMatrices(s);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        for (int i = 0; i < nengines; i++) {
            IRenderEngine *engine = engines[i];
            if (engine->hasPreProcess() || engine->hasPostProcess())
                continue;
            const IModel *model = engine->model();
            const IString *name = model->name();
            const QString &n = name ? static_cast<const CString *>(name)->value() : findModelPath(model);
            Q_FOREACH (const RenderContext::EffectAttachment &attachment, offscreen.attachments) {
                IEffect *effect = attachment.second;
                if (attachment.first.exactMatch(n)) {
                    engine->setEffect(IEffect::kStandardOffscreen, effect, 0);
                    break;
                }
            }
            engine->update();
            engine->renderModel();
            engine->renderEdge();
        }
        releaseOffscreenRenderTarget(offscreen, enableAA);
    }
    updateMatrices(size);
}

IEffect *RenderContext::createEffectAsync(const IString *path)
{
    IEffect *effect = 0;
    const QString &key = static_cast<const CString *>(path)->value();
    QMutexLocker locker(&m_effectCachesLock);
    if (m_effectCaches.contains(key)) {
        qDebug("Fetched an effect from cache: %s", qPrintable(key));
        effect = m_effectCaches[key];
    }
    else if (QFile::exists(key)) {
        locker.unlock();
        effect = m_scene->createEffect(path, this);
        qDebug("Loading an effect: %s", qPrintable(key));
        if (!effect->internalPointer()) {
            qWarning("%s cannot be compiled", qPrintable(key));
            qWarning() << cgGetLastListing(static_cast<CGcontext>(effect->internalContext()));
        }
        locker.relock();
        m_effectCaches.insert(key, effect);
    }
    else {
        qDebug("Cannot load an effect: %s", qPrintable(key));
    }
    return effect;
}

IEffect *RenderContext::createEffectAsync(IModel *model, const IString *dir)
{
    IEffect *effect = 0;
    const IString *name = model->name();
    const QString &key = effectFilePath(model, dir);
    QMutexLocker locker(&m_effectCachesLock);
    if (m_effectCaches.contains(key)) {
        qDebug("Fetched an effect from cache: %s", qPrintable(key));
        effect = m_effectCaches[key];
    }
    else if (QFile::exists(key)) {
        locker.unlock();
        effect = m_scene->createEffect(dir, model, this);
        qDebug("Loading an effect for %s: %s", name ? name->toByteArray() : 0, qPrintable(key));
        if (!effect->internalPointer()) {
            qWarning("%s cannot be compiled", qPrintable(key)) ;
            qWarning() << cgGetLastListing(static_cast<CGcontext>(effect->internalContext()));
        }
        locker.relock();
        m_effectCaches.insert(key, effect);
        setEffectOwner(effect, model);
    }
    else {
        qDebug("Cannot load an effect for %s: %s", name ? name->toByteArray() : 0, qPrintable(key));
    }
    return effect;
}

#endif

}
}
