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

#include "vpvl2/qt/Archive.h"
#include "vpvl2/qt/CString.h"
#include "vpvl2/qt/DDSTexture.h"
#include "vpvl2/qt/Delegate.h"
#include "vpvl2/qt/Util.h"

#include <vpvl2/vpvl2.h>
#include <vpvl2/IRenderDelegate.h>
#include <QtCore/QtCore>

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

#ifdef __APPLE__
#define glBlitFramebufferPROC glBlitFramebuffer
#define glDrawBuffersPROC glDrawBuffers
#define glRenderbufferStorageMultisamplePROC glRenderbufferStorageMultisample
#else
PFNGLBLITFRAMEBUFFERPROC glBlitFramebufferPROC;
PFNGLDRAWBUFFERSPROC glDrawBuffersPROC;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glRenderbufferStorageMultisamplePROC;
#endif

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
    transform.setOrigin(model->position());
    transform.setRotation(model->rotation());
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

static void UISetTexture(const Delegate::TextureCache &cache, bool isToon, Delegate::Texture &output)
{
    output.width = cache.width;
    output.height = cache.height;
    *static_cast<GLuint *>(output.object) = cache.id;
    if (!isToon) {
        GLuint textureID = *static_cast<const GLuint *>(output.object);
        glTexParameteri(textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
}

static void UIAddTextureCache(Delegate::PrivateContext *context,
                              const QString &path,
                              const Delegate::TextureCache &texture)
{
    if (context)
        context->textureCache.insert(path, texture);
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

class Delegate::FrameBufferObject : protected QGLFunctions {
public:
    FrameBufferObject(size_t width, size_t height, int samples, GLuint texture)
        : fbo(0),
          depth(0),
          fboAA(0),
          colorAA(0),
          depthAA(0),
          texture(texture),
          width(width),
          height(height),
          samples(samples)
    {
        const QGLContext *context = QGLContext::currentContext();
        initializeGLFunctions(context);
        glGenFramebuffers(1, &fbo);
        glGenRenderbuffers(1, &depth);
        glBindRenderbuffer(GL_RENDERBUFFER, depth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        if (samples > 0) {
            glGenFramebuffers(1, &fboAA);
            glGenRenderbuffers(1, &colorAA);
            glGenRenderbuffers(1, &depthAA);
            glBindRenderbuffer(GL_RENDERBUFFER, colorAA);
            glRenderbufferStorageMultisamplePROC(GL_RENDERBUFFER, samples, GL_RGBA8, width, height);
            glBindRenderbuffer(GL_RENDERBUFFER, depthAA);
            glRenderbufferStorageMultisamplePROC(GL_RENDERBUFFER, samples, GL_DEPTH_COMPONENT24, width, height);
        }
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
    ~FrameBufferObject() {
        glDeleteFramebuffers(1, &fbo);
        glDeleteRenderbuffers(1, &depth);
        glDeleteFramebuffers(1, &fboAA);
        glDeleteRenderbuffers(1, &colorAA);
        glDeleteRenderbuffers(1, &depthAA);
        fbo = depth = fboAA = colorAA = depthAA = width = height = samples = 0;
    }
    void blit() {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fboAA);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
        glBlitFramebufferPROC(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }

    GLuint fbo;
    GLuint depth;
    GLuint fboAA;
    GLuint colorAA;
    GLuint depthAA;
    GLuint texture;
    size_t width;
    size_t height;
    int samples;
};

QSet<QString> Delegate::loadableTextureExtensions()
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

QString Delegate::readAllAsync(const QString &path)
{
    QByteArray bytes;
    return UISlurpFile(path, bytes) ? bytes : QString();
}

Delegate::Delegate(const QHash<QString, QString> &settings, Scene *scene, QGLWidget *context)
    : m_settings(settings),
      m_systemDir(m_settings.value("dir.system.toon", "../../VPVM/resources/images")),
      m_scene(scene),
      m_context(context),
      m_archive(0),
      m_msaaSamples(0)
{
    for (int i = 0; i < 4; i++)
        m_previousFrameBufferPtrs.insert(i, 0);
    m_timer.start();
}

Delegate::~Delegate()
{
    setScenePtr(0);
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

void Delegate::allocateContext(const IModel *model, void *&context)
{
    const IString *name = model->name();
    PrivateContext *ctx = new(std::nothrow) PrivateContext();
    context = ctx;
    qDebug("Allocated the context: %s", name ? name->toByteArray() : reinterpret_cast<const uint8_t *>("(null)"));
}

void Delegate::releaseContext(const IModel *model, void *&context)
{
    const IString *name = model->name();
    delete static_cast<PrivateContext *>(context);
    context = 0;
    qDebug("Released the context: %s", name ? name->toByteArray() : reinterpret_cast<const uint8_t *>("(null)"));
}

bool Delegate::uploadTexture(const IString *name, const IString *dir, int flags, Texture &texture, void *context)
{
    bool mipmap = flags & IRenderDelegate::kGenerateTextureMipmap, ok = false;
    if (flags & IRenderDelegate::kTexture2D) {
        const QString &path = UICreatePath(dir, name);
        return uploadTextureInternal(path, false, false, mipmap, ok, texture, context);
    }
    else if (flags & IRenderDelegate::kToonTexture) {
        bool ret = false;
        if (dir) {
            const QString &path = UICreatePath(dir, name);
            ret = uploadTextureInternal(path, true, false, mipmap, ok, texture, context);
        }
        if (!ok) {
            CString s(m_systemDir.absolutePath());
            const QString &path = UICreatePath(&s, name);
            ret = uploadTextureInternal(path, true, true, mipmap, ok, texture, context);
            qDebug("Loaded a system texture: %s", qPrintable(path));
        }
        return ret;
    }
    return false;
}

void Delegate::getToonColor(const IString *name, const IString *dir, Color &value, void * /* context */)
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

void Delegate::uploadAnimatedTexture(float offset, float speed, float seek, void *texture)
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

void Delegate::getMatrix(float value[], const IModel *model, int flags) const
{
    QMatrix4x4 m;
    if (flags & IRenderDelegate::kShadowMatrix) {
        if (flags & IRenderDelegate::kProjectionMatrix)
            m *= m_cameraProjectionMatrix;
        if (flags & IRenderDelegate::kViewMatrix)
            m *= m_cameraViewMatrix;
        if (flags & IRenderDelegate::kWorldMatrix) {
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
    else if (flags & IRenderDelegate::kCameraMatrix) {
        if (flags & IRenderDelegate::kProjectionMatrix)
            m *= m_cameraProjectionMatrix;
        if (flags & IRenderDelegate::kViewMatrix)
            m *= m_cameraViewMatrix;
        if (flags & IRenderDelegate::kWorldMatrix) {
            UIConcatModelTransformMatrix(model, m);
            m *= m_cameraModelMatrix;
            m.scale(model->scaleFactor());
        }
    }
    else if (flags & IRenderDelegate::kLightMatrix) {
        if (flags & IRenderDelegate::kWorldMatrix) {
            m *= m_lightWorldMatrix;
            m.scale(model->scaleFactor());
        }
        if (flags & IRenderDelegate::kProjectionMatrix)
            m *= m_lightProjectionMatrix;
        if (flags & IRenderDelegate::kViewMatrix)
            m *= m_lightViewMatrix;
    }
    if (flags & IRenderDelegate::kInverseMatrix)
        m = m.inverted();
    if (flags & IRenderDelegate::kTransposeMatrix)
        m = m.transposed();
    for (int i = 0; i < 16; i++) {
        value[i] = float(m.constData()[i]);
    }
}

void Delegate::getViewport(Vector3 &value) const
{
    value.setValue(m_viewport.width(), m_viewport.height(), 0);
}

void Delegate::getTime(float &value, bool sync) const
{
    value = sync ? 0 : m_timer.elapsed() / 1000.0f;
}

void Delegate::getElapsed(float &value, bool sync) const
{
    value = sync ? 0 : 1.0 / 60.0;
}

void Delegate::getMousePosition(Vector4 &value, MousePositionType type) const
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

void Delegate::log(void * /* context */, LogLevel /* level */, const char *format, va_list ap)
{
    vfprintf(stderr, format, ap);
    fprintf(stderr, "%s", "\n");
}

IString *Delegate::loadKernelSource(KernelType type, void * /* context */)
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
    const QFuture<QString> &future = QtConcurrent::run(&Delegate::readAllAsync, path);
    const QString &source = future.result();
    if (!source.isNull() && !future.isCanceled()) {
        qDebug("Loaded a kernel: %s", qPrintable(path));
        return new(std::nothrow) CString(source);
    }
    else {
        return 0;
    }
}

IString *Delegate::loadShaderSource(ShaderType type, const IString *path)
{
    if (type == kModelEffectTechniques) {
        const QFuture<QString> &future = QtConcurrent::run(&Delegate::readAllAsync, static_cast<const CString *>(path)->value());
        const QString &source = future.result();
        return !source.isNull() ? new (std::nothrow) CString(source) : 0;
    }
    return 0;
}

IString *Delegate::loadShaderSource(ShaderType type, const IModel *model, const IString *dir, void * /* context */)
{
    QString file;
    if (type == kModelEffectTechniques) {
        const QString &filename = effectFilePath(model, dir);
        const QFuture<QString> &future = QtConcurrent::run(&Delegate::readAllAsync, filename);
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
    const QFuture<QString> &future = QtConcurrent::run(&Delegate::readAllAsync, path);
    const QString &source = future.result();
    if (!source.isNull() && !future.isCanceled()) {
        qDebug("Loaded a shader: %s", qPrintable(path));
        return new(std::nothrow) CString(m_shaderSourcePrefix + source);
    }
    else {
        return 0;
    }
}

IString *Delegate::toUnicode(const uint8_t *value) const
{
    QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
    const QString &s = codec->toUnicode(reinterpret_cast<const char *>(value));
    return new(std::nothrow) CString(s);
}

bool Delegate::hasExtension(const void *namePtr) const
{
    return m_extensions.contains(static_cast<const char *>(namePtr));
}

void *Delegate::findProcedureAddress(const void **candidatesPtr) const
{
    const QGLContext *context = QGLContext::currentContext();
    const char **candidates = reinterpret_cast<const char **>(candidatesPtr);
    const char *candidate = candidates[0];
    int i = 0;
    while (candidate) {
        void *address = context->getProcAddress(candidate);
        if (address) {
            return address;
        }
        candidate = candidates[++i];
    }
    return 0;
}

void Delegate::setArchive(Archive *value)
{
    delete m_archive;
    m_archive = value;
}

void Delegate::setScenePtr(Scene *value)
{
    m_offscreens.clear();
    m_model2Paths.clear();
    QMutexLocker locker(&m_effectCachesLock); Q_UNUSED(locker)
            qDeleteAll(m_texture2Movies);
    qDeleteAll(m_effectCaches);
    m_texture2Movies.clear();
    m_effectCaches.clear();
    m_scene = value;
}

void Delegate::updateMatrices(const QSizeF &size)
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

void Delegate::getCameraMatrices(QMatrix4x4 &world, QMatrix4x4 &view, QMatrix4x4 &projection)
{
    world = m_cameraModelMatrix;
    view = m_cameraViewMatrix;
    projection = m_cameraProjectionMatrix;
}

void Delegate::setCameraModelMatrix(const QMatrix4x4 &value)
{
    m_cameraModelMatrix = value;
}

void Delegate::getLightMatrices(QMatrix4x4 &world, QMatrix4x4 &view, QMatrix4x4 &projection)
{
    world = m_lightWorldMatrix;
    view = m_lightViewMatrix;
    projection = m_lightProjectionMatrix;
}

void Delegate::setLightMatrices(const QMatrix4x4 &world, const QMatrix4x4 &view, const QMatrix4x4 &projection)
{
    m_lightWorldMatrix = world;
    m_lightViewMatrix = view;
    m_lightProjectionMatrix = projection;
}

void Delegate::setMousePosition(const Vector3 &value, bool pressed, MousePositionType type)
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

void Delegate::addModelPath(IModel *model, const QString &filename)
{
    if (model) {
        QFileInfo finfo(filename);
        m_filename2Models.insert(finfo.fileName(), model);
        QMutexLocker locker(&m_model2PathLock); Q_UNUSED(locker);
        m_model2Paths.insert(model, filename);
    }
}

const QString Delegate::findModelPath(const IModel *model) const
{
    QMutexLocker locker(&m_model2PathLock); Q_UNUSED(locker);
    const QString s = m_model2Paths[model];
    return s;
}

const QString Delegate::effectFilePath(const IModel *model, const IString *dir) const
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

IModel *Delegate::findModel(const IString *name) const
{
    IModel *model = m_scene->findModel(name);
    if (!model) {
        const QString &s = static_cast<const CString *>(name)->value();
        model = m_filename2Models[s];
    }
    return model;
}

IModel *Delegate::effectOwner(const IEffect *effect) const
{
    QMutexLocker locker(&m_effect2modelsLock); Q_UNUSED(locker);
    IModel *model = m_effect2models[effect];
    return model;
}

const QString Delegate::effectOwnerName(const IEffect *effect) const
{
    QMutexLocker locker(&m_effectOwnersLock); Q_UNUSED(locker);
    const QString name = m_effectOwners[effect];
    return name;
}

void Delegate::setEffectOwner(const IEffect *effect, IModel *model)
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

void Delegate::removeModel(IModel *model)
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

void Delegate::initialize(bool enableMSAA)
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
    glBlitFramebufferPROC = reinterpret_cast<PFNGLBLITFRAMEBUFFERPROC>(context->getProcAddress("glBlitFramebuffer"));
    glDrawBuffersPROC = reinterpret_cast<PFNGLDRAWBUFFERSPROC>(context->getProcAddress("glDrawBuffers"));
    glRenderbufferStorageMultisamplePROC = reinterpret_cast<PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC>(context->getProcAddress("glRenderbufferStorageMultisample"));
#endif /* __APPLE__ */
}

void Delegate::setRenderColorTargets(const void *targets, const int ntargets)
{
    glDrawBuffersPROC(ntargets, static_cast<const GLuint *>(targets));
}

//#define DEBUG_OUTPUT_TEXTURE

void Delegate::bindRenderColorTarget(void *texture, size_t width, size_t height, int index, bool enableAA)
{
    GLuint textureID = *static_cast<const GLuint *>(texture);
    FrameBufferObject *buffer = findRenderTarget(textureID, width, height);
    if (buffer) {
        FrameBufferObject *fbo = m_previousFrameBufferPtrs[index];
        if (enableAA && fbo) {
            fbo->blit();
#ifdef DEBUG_OUTPUT_TEXTURE
            size_t w = fbo->width, h = fbo->height;
            QImage image(w, h, QImage::Format_ARGB32_Premultiplied);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo->fbo);
            glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            const QString &filename = QString("%1/texture%2_fbo%3_AA.png")
                    .arg(QDir::homePath())
                    .arg(textureID)
                    .arg(fbo->fboAA);
            image.rgbSwapped().mirrored().save(filename);
            qDebug("Dumped texture at bindRenderColorTarget as %s", qPrintable(filename));
            image.pixel(0, 0);
#endif
        }
#ifdef DEBUG_OUTPUT_TEXTURE
        static FrameBufferObject *g_fbos[4] = { 0, 0, 0, 0 };
        if (g_fbos[index]) {
            FrameBufferObject *g_fbo = g_fbos[index];
            size_t w = g_fbo->width, h = g_fbo->height;
            GLuint fbo = g_fbo->fbo;
            QImage image(w, h, QImage::Format_ARGB32_Premultiplied);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            const QString &filename = QString("%1/texture%2_fbo%3.png")
                    .arg(QDir::homePath())
                    .arg(g_fbo->texture)
                    .arg(fbo);
            image.rgbSwapped().mirrored().save(filename);
            qDebug("Dumped texture at bindRenderColorTarget as %s", qPrintable(filename));
            image.pixel(0, 0);
        }
        g_fbos[index] = buffer;
#endif
        const int target = GL_COLOR_ATTACHMENT0 + index;
        glBindFramebuffer(GL_FRAMEBUFFER, buffer->fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, target, GL_TEXTURE_2D, textureID, 0);
        if (enableAA && buffer->fboAA) {
            glBindFramebuffer(GL_FRAMEBUFFER, buffer->fboAA);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, target, GL_RENDERBUFFER, buffer->colorAA);
            m_previousFrameBufferPtrs.insert(index, buffer);
        }
    }
}

void Delegate::releaseRenderColorTarget(void *texture, size_t width, size_t height, int index, bool enableAA)
{
    GLuint textureID = *static_cast<const GLuint *>(texture);
    FrameBufferObject *buffer = findRenderTarget(textureID, width, height);
    if (buffer) {
        FrameBufferObject *fbo = m_previousFrameBufferPtrs[index];
        if (enableAA && fbo) {
            fbo->blit();
            fbo = 0;
        }
#ifdef DEBUG_OUTPUT_TEXTURE
        size_t w = buffer->width, h = buffer->height;
        QImage image(w, h, QImage::Format_ARGB32_Premultiplied);
        glBindFramebuffer(GL_FRAMEBUFFER, buffer->fbo);
        glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        const QString &filename = QString("%1/texture%2_fbo%3.png")
                .arg(QDir::homePath())
                .arg(textureID)
                .arg(buffer->fbo);
        image.rgbSwapped().mirrored().save(filename);
        qDebug("Dumped texture at releaseRenderColorTarget as %s", qPrintable(filename));
        image.pixel(0, 0);
#endif
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Delegate::bindRenderDepthStencilTarget(void *texture,
                                            void *depth,
                                            void *stencil,
                                            size_t width,
                                            size_t height,
                                            bool enableAA)
{
    GLuint textureID = *static_cast<const GLuint *>(texture);
    FrameBufferObject *buffer = findRenderTarget(textureID, width, height);
    if (buffer) {
        GLuint depthBuffer = *static_cast<GLuint *>(depth);
        GLuint stencilBuffer = *static_cast<GLuint *>(stencil);
        glBindFramebuffer(GL_FRAMEBUFFER, buffer->fbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencilBuffer);
        if (enableAA && buffer->fboAA) {
            glBindFramebuffer(GL_FRAMEBUFFER, buffer->fboAA);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, buffer->depthAA);
        }
    }
}

void Delegate::releaseRenderDepthStencilTarget(void *texture,
                                               void * /* depth */,
                                               void * /* stencil */,
                                               size_t width,
                                               size_t height,
                                               bool enableAA)
{
    GLuint textureID = *static_cast<const GLuint *>(texture);
    FrameBufferObject *buffer = findRenderTarget(textureID, width, height);
    if (buffer) {
        if (enableAA && buffer->fboAA) {
            glBindFramebuffer(GL_FRAMEBUFFER, buffer->fbo);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, buffer->fboAA);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        else {
            glBindFramebuffer(GL_FRAMEBUFFER, buffer->fbo);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }
}

void Delegate::bindOffscreenRenderTarget(GLuint textureID, size_t width, size_t height, bool enableAA)
{
    FrameBufferObject *buffer = findRenderTarget(textureID, width, height);
    if (buffer) {
        glBindFramebuffer(GL_FRAMEBUFFER, buffer->fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, buffer->depth);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, buffer->depth);
        if (enableAA && buffer->fboAA) {
            glBindFramebuffer(GL_FRAMEBUFFER, buffer->fboAA);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, buffer->colorAA);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, buffer->depthAA);
        }
    }
}

void Delegate::releaseOffscreenRenderTarget(GLuint textureID, size_t width, size_t height, bool enableAA)
{
    FrameBufferObject *buffer = findRenderTarget(textureID, width, height);
    if (buffer) {
        if (enableAA && buffer->fboAA) {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, buffer->fboAA);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, buffer->fbo);
            glBlitFramebufferPROC(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }
#ifdef DEBUG_OUTPUT_TEXTURE
        QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
        glBindFramebuffer(GL_FRAMEBUFFER, buffer->fbo);
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        image.rgbSwapped().mirrored().save(QString("%1/texture%2_offscreen%3.png")
                                           .arg(QDir::homePath())
                                           .arg(textureID)
                                           .arg(buffer->fbo));
        image.pixel(0, 0);
#endif
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Delegate::parseOffscreenSemantic(IEffect *effect, const QDir &dir)
{
#ifdef VPVL2_ENABLE_NVIDIA_CG
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
                        const QFuture<IEffect *> &future = QtConcurrent::run(this, &Delegate::createEffectAsync, &s2);
                        IEffect *offscreenEffect = future.result();
                        offscreenEffect->setParentEffect(effect);
                        attachments.append(EffectAttachment(regexp, offscreenEffect));
                    }
                    else {
                        attachments.append(EffectAttachment(regexp, 0));
                    }
                }
            }
            CGparameter sampler = static_cast<CGparameter>(renderTarget.samplerParameter);
            OffscreenRenderTarget offscreen;
            offscreen.attachments = attachments;
            offscreen.renderTarget = renderTarget;
            offscreen.textureID = cgGLGetTextureParameter(sampler);
            m_offscreens.append(offscreen);
        }
    }
#else
    Q_UNUSED(effect)
    Q_UNUSED(dir)
#endif
}

IEffect *Delegate::createEffectAsync(const IString *path)
{
    IEffect *effect = 0;
#ifdef VPVL2_ENABLE_NVIDIA_CG
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
#else
    Q_UNUSED(path)
#endif
    return effect;
}

IEffect *Delegate::createEffectAsync(IModel *model, const IString *dir)
{
    IEffect *effect = 0;
#ifdef VPVL2_ENABLE_NVIDIA_CG
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
#else
    Q_UNUSED(model)
    Q_UNUSED(dir)
#endif
    return effect;
}

bool Delegate::uploadTextureNVTT(const QString &suffix,
                                 const QString &path,
                                 bool isToon,
                                 bool mipmap,
                                 QScopedPointer<nv::Stream> &stream,
                                 Texture &texture,
                                 PrivateContext *privateContext)
{
#ifdef VPVL2_LINK_NVTT
    if (suffix == "dds") {
        nv::DirectDrawSurface surface;
        if (surface.load(stream.take())) {
            nv::Image nvimage;
            surface.mipmap(&nvimage, 0, 0);
            QImage image(UIConvertNVImageToQImage(nvimage));
            return generateTextureFromImage(image, path, isToon, mipmap, texture, privateContext);
        }
        else {
            qWarning("%s cannot be loaded", qPrintable(path));
        }
    }
    else {
        QScopedPointer<nv::Image> nvimage(nv::ImageIO::load(path.toUtf8().constData(), *stream));
        if (nvimage) {
            QImage image(UIConvertNVImageToQImage(*nvimage));
            return generateTextureFromImage(image, path, isToon, mipmap, texture, privateContext);
        }
        else {
            qWarning("%s cannot be loaded", qPrintable(path));
        }
    }
#else
    Q_UNUSED(suffix)
    Q_UNUSED(path)
    Q_UNUSED(isToon)
    Q_UNUSED(mipmap)
    Q_UNUSED(stream)
    Q_UNUSED(texture)
    Q_UNUSED(privateContext)
#endif
    return true;
}

bool Delegate::uploadTextureInternal(const QString &path,
                                     bool isToon,
                                     bool isSystem,
                                     bool mipmap,
                                     bool &ok,
                                     Texture &texture,
                                     void *context)
{
    const QFileInfo info(path);
    PrivateContext *privateContext = static_cast<PrivateContext *>(context);
    /* テクスチャのキャッシュを検索する */
    if (privateContext && privateContext->textureCache.contains(path)) {
        UISetTexture(privateContext->textureCache[path], isToon, texture);
        ok = true;
        return true;
    }
    /*
     * ZIP 圧縮からの読み込み (ただしシステムが提供する toon テクスチャは除く)
     * Archive が持つ仮想ファイルシステム上にあるため、キャッシュより後、物理ファイル上より先に検索しないといけない
     */
    const QString &suffix = info.suffix().toLower();
    if (m_archive && !isSystem) {
        const QByteArray &bytes = m_archive->data(path);
        if (loadableTextureExtensions().contains(suffix)) {
            QImage image;
            image.loadFromData(bytes);
            return generateTextureFromImage(image, path, isToon, mipmap, texture, privateContext);
        }
        else {
            QByteArray immutableBytes(bytes);
            QScopedPointer<nv::Stream> stream(new ReadonlyMemoryStream(immutableBytes));
            return uploadTextureNVTT(suffix, path, isToon, mipmap, stream, texture, privateContext);
        }
    }
    else if (info.isDir()) {
        return true; /* skip */
    }
    else if (!info.exists()) {
        qWarning("Cannot load inexist \"%s\"", qPrintable(path));
        return true; /* skip */
    }
    else if (loadableTextureExtensions().contains(suffix)) {
        QImage image(path);
        return generateTextureFromImage(image, path, isToon, mipmap, texture, privateContext);
    }
    else {
        QScopedPointer<nv::Stream> stream(new ReadonlyFileStream(path));
        return uploadTextureNVTT(suffix, path, isToon, mipmap, stream, texture, privateContext);
    }
}

bool Delegate::generateTextureFromImage(const QImage &image,
                                        const QString &path,
                                        bool isToon,
                                        bool mipmap,
                                        Texture &texture,
                                        PrivateContext *privateContext)
{
    GLuint textureID = m_context->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()),
                                              GL_TEXTURE_2D,
                                              GL_RGBA,
                                              UIGetTextureBindOptions(mipmap));
    size_t width = image.width(), height = image.height();
    TextureCache cache(width, height, textureID);
    m_texture2Paths.insert(textureID, path);
    UISetTexture(cache, isToon, texture);
    UIAddTextureCache(privateContext, path, cache);
    qDebug("Loaded a texture (ID=%d, width=%ld, height=%ld): \"%s\"",
           textureID, width, height, qPrintable(path));
    return textureID != 0;
}

void Delegate::getToonColorInternal(const QString &path, bool isSystem, Color &value, bool &ok)
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
    else {
        value.setValue(0, 0, 0, 1);
        ok = false;
    }
}

Delegate::FrameBufferObject *Delegate::findRenderTarget(const GLuint textureID, size_t width, size_t height)
{
    FrameBufferObject *buffer = 0;
    if (textureID > 0) {
        if (!m_renderTargets.contains(textureID)) {
            m_renderTargets.insert(textureID, new FrameBufferObject(width, height, m_msaaSamples, textureID));
            buffer = m_renderTargets[textureID];
        }
        else {
            buffer = m_renderTargets[textureID];
        }
    }
    return buffer;
}

}
}
