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

#include "DDSTexture.h"
#include "Delegate.h"
#include "CString.h"
#include "Util.h"

#include <vpvl2/vpvl2.h>
#include <vpvl2/IRenderDelegate.h>
#include <QtCore/QtCore>
#include <QtGui/QtGui>

using namespace vpvl2;

namespace vpvl2
{
namespace render
{
namespace qt
{

QString Delegate::readAllAsync(const QString &path)
{
    QByteArray bytes;
    return UISlurpFile(path, bytes) ? bytes : QString();
}

QImage Delegate::loadImageAsync(const QString &path)
{
    return QGLWidget::convertToGLFormat(QImage(path).rgbSwapped());
}

Delegate::Delegate(const QSettings *settings, const Scene *scene, QGLWidget *context)
    : m_settings(settings),
      m_scene(scene),
      m_systemDir(m_settings->value("dir.system.toon", "../../QMA2/resources/images").toString()),
      m_context(context)
{
    m_timer.start();
}

Delegate::~Delegate()
{
    qDeleteAll(m_texture2Movies);
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
    bool mipmap = flags & IRenderDelegate::kGenerateTextureMipmap;
    if (flags & IRenderDelegate::kTexture2D) {
        return uploadTextureInternal(createPath(dir, name), texture, false, mipmap, context);
    }
    else if (flags & IRenderDelegate::kToonTexture) {
        bool ret = false;
        if (dir) {
            ret = uploadTextureInternal(createPath(dir, name), texture, true, mipmap, context);
        }
        if (!ret) {
            CString s(m_systemDir.absolutePath());
            ret = uploadTextureInternal(createPath(&s, name), texture, true, mipmap, context);
        }
        return ret;
    }
    return false;
}

void Delegate::getToonColor(const IString *name, const IString *dir, Color &value, void * /* context */)
{
    const QString &path = createPath(dir, name);
    if (QFile::exists(path)) {
        getToonColorInternal(path, value);
    }
    else {
        CString s(m_systemDir.absolutePath());
        getToonColorInternal(createPath(&s, name), value);
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
            const IBone *bone = model->parentBone();
            if (bone) {
                const Transform &transform = bone->worldTransform();
                Scalar matrix[16];
                transform.getOpenGLMatrix(matrix);
                QMatrix4x4 worldMatrix;
                for (int i = 0; i < 16; i++)
                    worldMatrix.data()[i] = matrix[i];
                m *= worldMatrix;
            }
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
    const QString &path = QDir(m_settings->value("dir.system.kernels", "../../QMA2/resources/kernels").toString()).absoluteFilePath(file);
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
        file += "pmd/";
        break;
    case IModel::kPMX:
        file += "pmx/";
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
    const QString &path = QDir(m_settings->value("dir.system.shaders", "../../QMA2/resources/shaders").toString()).absoluteFilePath(file);
    const QFuture<QString> &future = QtConcurrent::run(&Delegate::readAllAsync, path);
    const QString &source = future.result();
    if (!source.isNull() && !future.isCanceled()) {
        qDebug("Loaded a shader: %s", qPrintable(path));
        return new(std::nothrow) CString("#version 120\n" + source);
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

void Delegate::updateMatrices(const QSize &size)
{
    float matrix[16];
    ICamera *camera = m_scene->camera();
    camera->modelViewTransform().getOpenGLMatrix(matrix);
    for (int i = 0; i < 16; i++)
        m_cameraViewMatrix.data()[i] = matrix[i];
    m_cameraProjectionMatrix.setToIdentity();
    m_cameraProjectionMatrix.perspective(camera->fov(), size.width() / float(size.height()), camera->znear(), camera->zfar());
    m_viewport = size;
}

void Delegate::setCameraModelMatrix(const QMatrix4x4 &value)
{
    m_cameraModelMatrix = value;
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

void Delegate::addModelPath(const IModel *model, const QString &filename)
{
    if (model) {
        m_model2PathLock.lock();
        m_model2Paths.insert(model, filename);
        m_model2PathLock.unlock();
    }
}

const QString Delegate::findModelPath(const IModel *model) const
{
    m_model2PathLock.lock();
    const QString s = m_model2Paths[model];
    m_model2PathLock.unlock();
    return s;
}

const QString Delegate::effectFilePath(const IModel *model, const IString *dir) const
{
    QDir d(static_cast<const CString *>(dir)->value());
    const QString &s = findModelPath(model);
    if (!s.isEmpty()) {
        QFileInfo info(d.absoluteFilePath(s));
        const QRegExp regexp("^.+\\[([^\\]]+)\\]$");
        const QString &name = info.baseName();
        const QString &basename = regexp.exactMatch(name) ? regexp.capturedTexts().at(1) : name;
        const QString &cgfx = d.absoluteFilePath(basename + ".cgfx");
        if (QFile::exists(cgfx))
            return cgfx;
    }
    return d.absoluteFilePath("default.cgfx");
}

IModel *Delegate::effectOwner(const IEffect *effect) const
{
    m_effect2modelsLock.lock();
    IModel *model = m_effect2models[effect];
    m_effect2modelsLock.unlock();
    return model;
}

const QString Delegate::effectOwnerName(const IEffect *effect) const
{
    m_effectOwnersLock.lock();
    const QString name = m_effectOwners[effect];
    m_effectOwnersLock.unlock();
    return name;
}

void Delegate::setEffectOwner(const IEffect *effect, IModel *model)
{
    const CString *name = static_cast<const CString *>(model->name());
    const QString &n = name ? name->value() : findModelPath(model);
    m_effectOwnersLock.lock();
    m_effectOwners.insert(effect, n);
    m_effectOwnersLock.unlock();
    m_effect2modelsLock.lock();
    m_effect2models.insert(effect, model);
    m_effect2modelsLock.unlock();
}

const QString Delegate::createPath(const IString *dir, const QString &name)
{
    const QDir d(static_cast<const CString *>(dir)->value());
    return d.absoluteFilePath(name);
}

const QString Delegate::createPath(const IString *dir, const IString *name)
{
    const QDir d(static_cast<const CString *>(dir)->value());
    return d.absoluteFilePath(static_cast<const CString *>(name)->value());
}

void Delegate::setTextureID(const TextureCache &cache, bool isToon, Texture &output)
{
    output.width = cache.width;
    output.height = cache.height;
    *const_cast<GLuint *>(static_cast<const GLuint *>(output.object)) = cache.id;
    if (!isToon) {
        GLuint textureID = *static_cast<const GLuint *>(output.object);
        glTexParameteri(textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
}

void Delegate::addTextureCache(PrivateContext *context, const QString &path, const TextureCache &texture)
{
    if (context)
        context->textureCache.insert(path, texture);
}

bool Delegate::uploadTextureInternal(const QString &path, Texture &texture, bool isToon, bool mipmap, void *context)
{
    const QFileInfo info(path);
    if (info.isDir())
        return false;
    if (!info.exists()) {
        qWarning("Cannot loading inexist \"%s\"", qPrintable(path));
        return false;
    }
    PrivateContext *privateContext = static_cast<PrivateContext *>(context);
    if (privateContext && privateContext->textureCache.contains(path)) {
        setTextureID(privateContext->textureCache[path], isToon, texture);
        return true;
    }
    if (path.endsWith(".dds")) {
        QFile file(path);
        if (file.open(QFile::ReadOnly)) {
            const QByteArray &bytes = file.readAll();
            const uint8_t *data = reinterpret_cast<const uint8_t *>(bytes.constData());
            DDSTexture dds(m_context);
            GLuint textureID;
            if (!dds.parse(data, bytes.size(), textureID)) {
                qDebug("Cannot parse a DDS texture %s", qPrintable(path));
                return false;
            }
            TextureCache cache;
            cache.width = dds.width();
            cache.height = dds.height();
            cache.id = textureID;
            setTextureID(cache, isToon, texture);
            addTextureCache(privateContext, path, cache);
            return true;
        }
        else {
            qDebug("Cannot open a DDS texture %s: %s", qPrintable(path), qPrintable(file.errorString()));
            return false;
        }
    }
    else {
        const QFuture<QImage> &future = QtConcurrent::run(&Delegate::loadImageAsync, path);
        const QImage &image = future.result();
        QGLContext::BindOptions options = QGLContext::LinearFilteringBindOption
                | QGLContext::InvertedYBindOption
                | QGLContext::PremultipliedAlphaBindOption;
        if (mipmap)
            options |= QGLContext::MipmapBindOption;
        GLuint textureID = m_context->bindTexture(image, GL_TEXTURE_2D, GL_RGBA, options);
        TextureCache cache;
        cache.width = image.width();
        cache.height = image.height();
        cache.id = textureID;
        m_texture2Paths.insert(textureID, path);
        setTextureID(cache, isToon, texture);
        addTextureCache(privateContext, path, cache);
        qDebug("Loaded a texture (ID=%d, width=%d, height=%d): \"%s\"",
               textureID, image.width(), image.height(), qPrintable(path));
        return textureID != 0;
    }
}

void Delegate::getToonColorInternal(const QString &path, Color &value)
{
    const QImage image(path);
    if (!image.isNull()) {
        const QRgb &rgb = image.pixel(image.width() - 1, image.height() - 1);
        const QColor color(rgb);
        value.setValue(color.redF(), color.greenF(), color.blueF(), color.alphaF());
    }
}

}
}
}
