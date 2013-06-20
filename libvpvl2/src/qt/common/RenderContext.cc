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

/* include ICU first to resolve an issue of stdint.h on MSVC */
#include <unicode/unistr.h>
#include <vpvl2/qt/RenderContext.h>

#include <vpvl2/vpvl2.h>
#include <vpvl2/extensions/Archive.h>
#include <vpvl2/qt/Util.h>

#include <QtCore>
#include <QColor>
#include <QImage>
#include <QMovie>

#ifndef VPVL2_LINK_GLEW
#include <QGLContext>
#endif

using namespace vpvl2;
using namespace vpvl2::extensions;
using namespace vpvl2::qt;

namespace vpvl2
{
namespace qt
{

using namespace extensions::gl;

QSet<QString> RenderContext::loadableTextureExtensions()
{
    /* QImage に読み込ませる画像の拡張子を返す */
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

RenderContext::RenderContext(Scene *sceneRef, IEncoding *encodingRef, const StringMap *settingsRef)
    : BaseRenderContext(sceneRef, encodingRef, settingsRef)
{
    m_timer.start();
}

RenderContext::~RenderContext()
{
}

#ifdef VPVL2_ENABLE_NVIDIA_CG
void RenderContext::getToonColor(const IString *name, Color &value, void *userData)
{
    const ModelContext *modelContext = static_cast<const ModelContext *>(userData);
    const QString &path = createQPath(modelContext->directoryRef(), name);
    QImage image;
    bool ok = false;
    /* ファイルが存在する、またはアーカイブ内にあると予想される場合はそちらを読み込む */
#ifdef VPVL2_ENABLE_EXTENSIONS_ARCHIVE
    const Archive *archiveRef = modelContext->archiveRef();
    if (archiveRef) {
        QByteArray suffix = QFileInfo(path).suffix().toLower().toUtf8();
        if (suffix == "sph" || suffix == "spa") {
            suffix.setRawData("bmp", 3);
        }
        if (const std::string *bytes = archiveRef->dataRef(Util::fromQString(path))) {
            image.loadFromData(bytes->data(), suffix.constData());
        }
    }
    else {
        image.load(path);
        getToonColorInternal(image, value);
    }
#endif
    /* 上でなければシステム側のトゥーンテクスチャを読み込む */
    if (!ok) {
        String s(toonDirectory());
        const QString &fallback = createQPath(&s, name);
        image.load(fallback);
        getToonColorInternal(image, value);
    }
}

void RenderContext::uploadAnimatedTexture(float offset, float speed, float seek, void *texture)
{
    ITexture *textureRef = static_cast<ITexture *>(texture);
    QMovie *movie = 0;
    /* キャッシュを読み込む */
    if (m_texture2Movies.contains(textureRef)) {
        movie = m_texture2Movies[textureRef].data();
    }
    else {
        /* アニメーションテクスチャを読み込み、キャッシュに格納する */
        const QString &path = m_texture2Paths[textureRef];
        m_texture2Movies.insert(textureRef, QSharedPointer<QMovie>(new QMovie(path)));
        movie = m_texture2Movies[textureRef].data();
        movie->setCacheMode(QMovie::CacheAll);
    }
    /* アニメーションテクスチャが読み込み可能な場合はパラメータを設定してテクスチャを取り出す */
    if (movie->isValid()) {
        offset *= Scene::defaultFPS();
        int frameCount = movie->frameCount();
        offset = qBound(0, int(offset), frameCount);
        int left = int(seek * speed * Scene::defaultFPS() + frameCount - offset);
        int right = qMax(int(frameCount - offset), 1);
        int frameIndex = left % right + int(offset);
        /* アニメーションテクスチャ内のフレーム移動を行い、該当の画像をテクスチャに変換する */
        if (movie->jumpToFrame(frameIndex)) {
            const QImage &image = movie->currentImage();
#ifdef VPVL2_LINK_GLEW
            textureRef->bind();
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width(), image.height(),
                            GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image.constBits());
            glBindTexture(GL_TEXTURE_2D, 0);
#else
            const QImage &textureImage = QGLWidget::convertToGLFormat(image.mirrored());
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width(), image.height(), GL_RGBA, GL_UNSIGNED_BYTE, textureImage.constBits());
            glBindTexture(GL_TEXTURE_2D, 0);
#endif
        }
    }
}

void RenderContext::getTime(float &value, bool sync) const
{
    value = sync ? 0 : m_timer.elapsed() / 1000.0f;
}

void RenderContext::getElapsed(float &value, bool sync) const
{
    value = sync ? 0 : 1.0 / 60.0;
}
#endif

void *RenderContext::findProcedureAddress(const void **candidatesPtr) const
{
#ifndef VPVL2_LINK_GLEW
    const QGLContext *context = QGLContext::currentContext();
    const char **candidates = reinterpret_cast<const char **>(candidatesPtr);
    const char *candidate = candidates[0];
    int i = 0;
    while (candidate) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        void *address = reinterpret_cast<void *>(context->getProcAddress(candidate));
#elif defined(WIN32)
        void *address = wglGetProcAddress(candidate);
#else
        void *address = context->getProcAddress(candidate);
#endif
        if (address) {
            return address;
        }
        candidate = candidates[++i];
    }
#else
    Q_UNUSED(candidatesPtr)
#endif
    return 0;
}

//#define VPVL2_USE_MMAP

bool RenderContext::mapFile(const UnicodeString &path, MapBuffer *buffer) const
{
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
        std::memcpy(buffer->address, bytes.constData(), size);
#endif
        buffer->size = size;
        buffer->opaque = reinterpret_cast<intptr_t>(file.take());
        return ok;
    }
    VPVL2_LOG(WARNING, "Cannot load " << qPrintable(file->fileName()) << ": " << qPrintable(file->errorString()));
    return false;
}

bool RenderContext::unmapFile(MapBuffer *buffer) const
{
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

#undef VPVL2_USE_MMAP

bool RenderContext::existsFile(const UnicodeString &path) const
{
    return QFile::exists(Util::toQString(path));
}

void RenderContext::removeModel(IModel * /* model */)
{
#if 0
    /* ファイル名からモデルインスタンスのハッシュの全ての参照を削除 */
    QMutableHashIterator<const QString, IModel *> it(m_basename2ModelRefs);
    while (it.hasNext()) {
        it.next();
        IModel *m = it.value();
        if (m == model) {
            it.remove();
        }
    }
    /* エフェクトインスタンスからモデルインスタンスのハッシュの全ての参照を削除 */
    QMutexLocker locker(&m_effect2modelsLock); Q_UNUSED(locker);
    QMutableHashIterator<const IEffect *, IModel *> it2(m_effectRef2modelRefs);
    while (it2.hasNext()) {
        it2.next();
        IModel *m = it2.value();
        if (m == model) {
            it2.remove();
        }
    }
#endif
}

QString RenderContext::createQPath(const IString *dir, const IString *name)
{
    const UnicodeString &d = static_cast<const String *>(dir)->value();
    const UnicodeString &n = static_cast<const String *>(name)->value();
    const QString &d2 = Util::toQString(d);
    const QString &n2 = Util::toQString(n);
    return QDir(d2).absoluteFilePath(n2);
}

bool RenderContext::uploadTextureOpaque(const uint8_t *data, size_t size, const UnicodeString &key, ModelContext *context, TextureDataBridge &bridge)
{
    QImage image;
    image.loadFromData(data, size);
    return uploadTextureQt(image, key, context, bridge);
}

bool RenderContext::uploadTextureOpaque(const UnicodeString &path, ModelContext *context, TextureDataBridge &bridge)
{
    QImage image(Util::toQString(path));
    return uploadTextureQt(image, path, context, bridge);
}

bool RenderContext::uploadTextureQt(const QImage &image, const UnicodeString &key, ModelContext *modelContext, TextureDataBridge &texture)
{
    /* use Qt's pluggable image loader (jpg/png is loaded with libjpeg/libpng) */
    BaseSurface::Format format(GL_BGRA, GL_RGBA8, GL_UNSIGNED_INT_8_8_8_8_REV, GL_TEXTURE_2D);
    const Vector3 size(image.width(), image.height(), 1);
    ITexture *texturePtr = modelContext->uploadTexture(image.constBits(), format, size, texture.mipmap, false);
    return modelContext->cacheTexture(key, texturePtr, texture);
}

bool RenderContext::generateTextureFromImage(const QImage &image,
                                             const QString &path,
                                             TextureDataBridge &texture,
                                             ModelContext *modelContext)
{
    if (!image.isNull()) {
        BaseSurface::Format format(GL_BGRA, GL_RGBA8, GL_UNSIGNED_INT_8_8_8_8_REV, GL_TEXTURE_2D);
        const Vector3 size(image.width(), image.height(), 1);
        ITexture *textureRef = modelContext->uploadTexture(image.constBits(), format, size, texture.mipmap, false);
        texture.dataRef = textureRef;
        m_texture2Paths.insert(textureRef, path);
        if (modelContext) {
            modelContext->addTextureCache(Util::fromQString(path), textureRef);
        }
        VPVL2_VLOG(2, "Loaded a texture: ID=" << textureRef << " width=" << size.x() << " height=" << size.y() << " depth=" << size.z() << " path=" << qPrintable(path));
        bool ok = texture.ok = textureRef != 0;
        return ok;
    }
    else {
        VPVL2_LOG(WARNING, "Failed loading a image to convert the texture: " << qPrintable(path));
        return false;
    }
}

void RenderContext::getToonColorInternal(const QImage &image, Color &value)
{
    if (!image.isNull()) {
        const QRgb &rgb = image.pixel(image.width() - 1, image.height() - 1);
        const QColor color(rgb);
        value.setValue(color.redF(), color.greenF(), color.blueF(), color.alphaF());
    }
    else {
        value.setValue(1, 1, 1, 1);
    }
}

} /* namespace qt */
} /* namespace vpvl2 */
