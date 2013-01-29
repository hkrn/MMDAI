/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
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
#include "vpvl2/extensions/Archive.h"
#include "vpvl2/qt/RenderContext.h"
#include "vpvl2/qt/RenderContext.h"
#include "vpvl2/qt/Util.h"

#include "vpvl2/extensions/details/Archive.h"
#include "vpvl2/extensions/details/BaseRenderContext.h"
#include "vpvl2/extensions/details/World.h"

#include <QtCore>
#include <QColor>
#include <QImage>
#include <QMovie>

#ifdef VPVL2_LINK_NVTT
#include <nvcore/Debug.h>
#include <nvcore/Stream.h>
#include <nvimage/DirectDrawSurface.h>
#include <nvimage/Image.h>
#include <nvimage/ImageIO.h>
namespace {
struct MessageHandler : public nv::MessageHandler, public nv::AssertHandler {
    int assertion(const char *exp, const char *file, int line, const char *func) {
        qFatal("Assertion error: %s (%s in %s at %d)", exp, func, file, line);
        return 0;
    }
    void log(const char *str, va_list arg) {
        fprintf(stderr, str, arg);
    }
};
MessageHandler s_messageHandler;
}
#else
namespace nv {
class Stream {
public:
    Stream() {}
    virtual ~Stream() {}
};
}
#endif

using namespace vpvl2;
using namespace vpvl2::extensions;
using namespace vpvl2::qt;

namespace
{

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

RenderContext::RenderContext(Scene *sceneRef, const StringMap *settingsRef)
    : BaseRenderContext(sceneRef, settingsRef)
{
    m_timer.start();
#ifdef VPVL2_LINK_NVTT
    nv::debug::setAssertHandler(&s_messageHandler);
    nv::debug::setMessageHandler(&s_messageHandler);
#endif
}

RenderContext::~RenderContext()
{
    setSceneRef(0);
    m_msaaSamples = 0;
}

void RenderContext::getToonColor(const IString *name, const IString *dir, Color &value, void * /* context */)
{
    const QString &path = createQPath(dir, name);
    bool ok = false;
    /* ファイルが存在する、またはアーカイブ内にあると予想される場合はそちらを読み込む */
    if (m_archive || QFile::exists(path)) {
        getToonColorInternal(path, false, value, ok);
    }
    /* 上でなければシステム側のトゥーンテクスチャを読み込む */
    if (!ok) {
        String s(toonDirectory());
        const QString &fallback = createQPath(&s, name);
        getToonColorInternal(fallback, true, value, ok);
    }
}

void RenderContext::uploadAnimatedTexture(float offset, float speed, float seek, void *texture)
{
    GLuint textureID = *static_cast<GLuint *>(texture);
    QMovie *movie = 0;
    /* キャッシュを読み込む */
    if (m_texture2Movies.contains(textureID)) {
        movie = m_texture2Movies[textureID].data();
    }
    else {
        /* アニメーションテクスチャを読み込み、キャッシュに格納する */
        const QString &path = m_texture2Paths[textureID];
        m_texture2Movies.insert(textureID, QSharedPointer<QMovie>(new QMovie(path)));
        movie = m_texture2Movies[textureID].data();
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

#define VPVL2_USE_MMAP

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
        memcpy(buffer->address, bytes.constData(), size);
#endif
        buffer->size = size;
        buffer->opaque = file.take();
        return ok;
    }
    qWarning("Cannot load file %s: %s", qPrintable(file->fileName()), qPrintable(file->errorString()));
    return false;
}

bool RenderContext::unmapFile(MapBuffer *buffer) const
{
    if (QFile *file = static_cast<QFile *>(buffer->opaque)) {
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

void RenderContext::removeModel(IModel *model)
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

bool RenderContext::uploadTextureNVTT(const QString &suffix,
                                      const QString &path,
                                      QScopedPointer<nv::Stream> &stream,
                                      Texture &texture,
                                      ModelContext *modelContext)
{
#ifdef VPVL2_LINK_NVTT
    if (suffix == "dds") {
        nv::DirectDrawSurface surface;
        if (surface.load(stream.take())) {
            nv::Image nvimage;
            surface.mipmap(&nvimage, 0, 0);
            QImage image(UIConvertNVImageToQImage(nvimage));
            return generateTextureFromImage(image, path, texture, modelContext);
        }
        else {
            qWarning("%s cannot be loaded", qPrintable(path));
        }
    }
    else {
        QScopedPointer<nv::Image> nvimage(nv::ImageIO::load(path.toUtf8().constData(), *stream));
        if (nvimage) {
            QImage image(UIConvertNVImageToQImage(*nvimage));
            return generateTextureFromImage(image, path, texture, modelContext);
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

QString RenderContext::createQPath(const IString *dir, const IString *name)
{
    const UnicodeString &d = static_cast<const String *>(dir)->value();
    const UnicodeString &n = static_cast<const String *>(name)->value();
    const QString &d2 = Util::toQString(d);
    const QString &n2 = Util::toQString(n);
    return QDir(d2).absoluteFilePath(n2);
}

bool RenderContext::uploadTextureInternal(const UnicodeString &path, Texture &texture, void *context)
{
    const QString &newPath = Util::toQString(path);
    const QFileInfo info(newPath);
    ModelContext *modelContext = static_cast<ModelContext *>(context);
    /* テクスチャのキャッシュを検索する */
    if (modelContext && modelContext->findTextureCache(path, texture)) {
        return true;
    }
    /*
     * ZIP 圧縮からの読み込み (ただしシステムが提供する toon テクスチャは除く)
     * Archive が持つ仮想ファイルシステム上にあるため、キャッシュより後、物理ファイル上より先に検索しないといけない
     */
    const QString &suffix = info.suffix().toLower();
    if (m_archive && !texture.system) {
        if (const std::string *byteArray = m_archive->data(path)) {
            if (loadableTextureExtensions().contains(suffix)) {
                QImage image;
                image.loadFromData(QByteArray::fromRawData(byteArray->data(), byteArray->size()));
                return generateTextureFromImage(image, newPath, texture, modelContext);
            }
            else {
                QByteArray immutableBytes(QByteArray::fromRawData(byteArray->data(), byteArray->size()));
                QScopedPointer<nv::Stream> stream(new ReadonlyMemoryStream(immutableBytes));
                return uploadTextureNVTT(suffix, newPath, stream, texture, modelContext);
            }
        }
        return false;
    }
    /* ディレクトリの場合はスキップする。ただしトゥーンの場合は白テクスチャの読み込みを行う */
    else if (info.isDir()) {
        if (texture.toon) { /* force loading as white toon texture */
            String d(toonDirectory());
            const UnicodeString &newToonPath = createPath(&d, UnicodeString::fromUTF8("toon0.bmp"));
            if (modelContext && !modelContext->findTextureCache(newToonPath, texture)) {
                QImage image(newPath);
                return generateTextureFromImage(image, newPath, texture, modelContext);
            }
        }
        return true; /* skip */
    }
    else if (!info.exists()) {
        qWarning("Cannot load inexist \"%s\"", qPrintable(newPath));
        return true; /* skip */
    }
    else if (loadableTextureExtensions().contains(suffix)) {
        QImage image(newPath);
        return generateTextureFromImage(image, newPath, texture, modelContext);
    }
    else {
        QScopedPointer<nv::Stream> stream(new ReadonlyFileStream(newPath));
        return uploadTextureNVTT(suffix, newPath, stream, texture, modelContext);
    }
}

bool RenderContext::generateTextureFromImage(const QImage &image,
                                             const QString &path,
                                             Texture &texture,
                                             ModelContext *modelContext)
{
    if (!image.isNull()) {
        size_t width = image.width(), height = image.height();
        GLuint textureID = 0;
#ifdef VPVL2_LINK_GLEW
        textureID = createTexture(image.constBits(),
                                  Vector3(image.width(), image.height(), 0),
                                  GL_BGRA,
                                  GL_UNSIGNED_INT_8_8_8_8_REV,
                                  texture.mipmap,
                                  false);
#else
        QGLContext *context = const_cast<QGLContext *>(QGLContext::currentContext());
        textureID = context->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()),
                                         GL_TEXTURE_2D,
                                         GL_RGBA,
                                         UIGetTextureBindOptions(texture.mipmap));
#endif
        texture.opaque = textureID;
        texture.size.setValue(width, height, 0);
        texture.format = GL_RGBA;
        m_texture2Paths.insert(textureID, path);
        if (modelContext) {
            TextureCache cache(texture);
            modelContext->addTextureCache(Util::fromQString(path), cache);
        }
        qDebug("Loaded a texture (ID=%d, width=%ld, height=%ld): \"%s\"",
               textureID, width, height, qPrintable(path));
        bool ok = texture.ok = textureID != 0;
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
        if (const std::string *bytes = m_archive->data(Util::fromQString(path))) {
            image.loadFromData(bytes->data(), suffix.constData());
        }
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

}
}
