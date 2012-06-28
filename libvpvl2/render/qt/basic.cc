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

#include <vpvl2/vpvl2.h>
#include <vpvl2/IRenderDelegate.h>

#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtOpenGL/QtOpenGL>

#ifndef VPVL2_NO_BULLET
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#else
VPVL2_DECLARE_HANDLE(btDiscreteDynamicsWorld)
#endif

#ifdef VPVL2_LINK_ASSIMP
#include <assimp.hpp>
#include <DefaultLogger.h>
#include <Logger.h>
#include <aiPostProcess.h>
#else
BT_DECLARE_HANDLE(aiScene);
#endif

/* internal headers */
#include "vpvl2/pmx/Bone.h"
#include "vpvl2/pmx/Joint.h"
#include "vpvl2/pmx/Label.h"
#include "vpvl2/pmx/Material.h"
#include "vpvl2/pmx/Model.h"
#include "vpvl2/pmx/Morph.h"
#include "vpvl2/pmx/RigidBody.h"
#include "vpvl2/pmx/Vertex.h"
#include "vpvl2/asset/Model.h"
#include "vpvl2/pmd/Model.h"

#include "vpvl2/vmd/Motion.h"

using namespace vpvl2;

namespace
{

typedef QScopedPointer<uint8_t, QScopedPointerArrayDeleter<uint8_t> > ByteArrayPtr;

static bool UISlurpFile(const QString &path, QByteArray &bytes) {
    QFile file(path);
    if (file.open(QFile::ReadOnly)) {
        bytes = file.readAll();
        file.close();
        return true;
    }
    else {
        qWarning("slurpFile error at %s: %s", qPrintable(path), qPrintable(file.errorString()));
        return false;
    }
}

class String : public IString {
public:
    String(const QString &s)
        : m_bytes(s.toUtf8()),
          m_value(s)
    {
    }
    ~String() {
    }

    bool startsWith(const IString *value) const {
        return m_value.startsWith(static_cast<const String *>(value)->m_value);
    }
    bool contains(const IString *value) const {
        return m_value.contains(static_cast<const String *>(value)->m_value);
    }
    bool endsWith(const IString *value) const {
        return m_value.endsWith(static_cast<const String *>(value)->m_value);
    }
    IString *clone() const {
        return new String(m_value);
    }
    const HashString toHashString() const {
        return HashString(m_bytes.constData());
    }
    bool equals(const IString *value) const {
        return m_value == static_cast<const String *>(value)->m_value;
    }
    const QString &value() const {
        return m_value;
    }
    const uint8_t *toByteArray() const {
        return reinterpret_cast<const uint8_t *>(m_bytes.constData());
    }
    size_t length() const {
        return m_bytes.length();
    }

private:
    QByteArray m_bytes;
    QString m_value;
};

class Encoding : public IEncoding {
public:
    Encoding()
        : m_sjis(QTextCodec::codecForName("Shift-JIS")),
          m_utf8(QTextCodec::codecForName("UTF-8")),
          m_utf16(QTextCodec::codecForName("UTF-16"))
    {
    }
    ~Encoding() {
    }

    const IString *stringConstant(ConstantType value) const {
        switch (value) {
        case kLeft: {
            static const String s("左");
            return &s;
        }
        case kRight: {
            static const String s("右");
            return &s;
        }
        case kFinger: {
            static const String s("指");
            return &s;
        }
        case kElbow: {
            static const String s("ひじ");
            return &s;
        }
        case kArm: {
            static const String s("腕");
            return &s;
        }
        case kWrist: {
            static const String s("手首");
            return &s;
        }
        case kCenter: {
            static const String s("センター");
            return &s;
        }
        default: {
            static const String s("");
            return &s;
        }
        }
    }
    IString *toString(const uint8_t *value, size_t size, IString::Codec codec) const {
        IString *s = 0;
        const char *str = reinterpret_cast<const char *>(value);
        switch (codec) {
        case IString::kShiftJIS:
            s = new String(m_sjis->toUnicode(str, size));
            break;
        case IString::kUTF8:
            s = new String(m_utf8->toUnicode(str, size));
            break;
        case IString::kUTF16:
            s = new String(m_utf16->toUnicode(str, size));
            break;
        }
        return s;
    }
    IString *toString(const uint8_t *value, IString::Codec codec, size_t maxlen) const {
        size_t size = qstrnlen(reinterpret_cast<const char *>(value), maxlen);
        return toString(value, size, codec);
    }
    uint8_t *toByteArray(const IString *value, IString::Codec codec) const {
        const String *s = static_cast<const String *>(value);
        QByteArray bytes;
        switch (codec) {
        case IString::kShiftJIS:
            bytes = m_sjis->fromUnicode(s->value());
            break;
        case IString::kUTF8:
            bytes = m_utf8->fromUnicode(s->value());
            break;
        case IString::kUTF16:
            bytes = m_utf16->fromUnicode(s->value());
            break;
        }
        size_t size = bytes.length();
        uint8_t *data = new uint8_t[size + 1];
        memcpy(data, bytes.constData(), size);
        data[size] = 0;
        return data;
    }
    void disposeByteArray(uint8_t *value) const {
        delete[] value;
    }

private:
    QTextCodec *m_sjis;
    QTextCodec *m_utf8;
    QTextCodec *m_utf16;
};

class DirectDrawSurface
{
public:
    DirectDrawSurface(QGLWidget *context)
        : m_context(context)
    {
        memset(&m_header, 0, sizeof(m_header));
    }
    ~DirectDrawSurface() {
    }

    bool parse(const uint8_t *data, size_t size, GLuint &textureID) {
        if (size > sizeof(m_header)) {
            m_header = *reinterpret_cast<const Header *>(data);
            if (!m_header.isValidSignature()) {
                return false;
            }
            if (!m_header.isValidDescriptionSize()) {
                return false;
            }
            if (!m_header.isValidPixelFormatSize()) {
                return false;
            }
            if (!m_header.isValidDescription()) {
                return false;
            }
            if (!m_header.isValidCapacity()) {
                return false;
            }
            m_header.correct();
            uint8_t *ptr;
            m_header.getDataPointer(data, ptr);
            glGenTextures(1, &textureID);
            if (m_header.hasCubemap()) {
                size_t offset = 0;
                glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
                if (m_header.hasCubemapPositiveX()) {
                    setCubeTexture(ptr, GL_TEXTURE_CUBE_MAP_POSITIVE_X, offset);
                    ptr += offset;
                }
                if (m_header.hasCubemapNegativeX()) {
                    setCubeTexture(ptr, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, offset);
                    ptr += offset;
                }
                if (m_header.hasCubemapPositiveY()) {
                    setCubeTexture(ptr, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, offset);
                    ptr += offset;
                }
                if (m_header.hasCubemapNegativeY()) {
                    setCubeTexture(ptr, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, offset);
                    ptr += offset;
                }
                if (m_header.hasCubemapPositiveZ()) {
                    setCubeTexture(ptr, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, offset);
                    ptr += offset;
                }
                if (m_header.hasCubemapNegativeZ()) {
                    setCubeTexture(ptr, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, offset);
                    ptr += offset;
                }
                glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
                return true;
            }
            // volume
            else if (m_header.hasVolume()) {
                glBindTexture(GL_TEXTURE_3D, textureID);
                setVolumeTexture(ptr);
                glBindTexture(GL_TEXTURE_3D, 0);
                return true;
            }
            // texture
            else {
            }
        }
        return false;
    }

private:
    void setCubeTexture(const uint8_t *ptr, GLenum target, size_t &offset) {
        uint32_t width = m_header.width(), height = m_header.height();
        GLenum format, internal, type;
        m_header.getTextureFormat(internal, format, type);
        offset = 0;
        if (m_header.hasMipmap()) {
            int level = 0;
            while (height > 1) {
                glTexImage2D(target, level, internal, width, height, 0, format, type, ptr + offset);
                offset += m_header.estimateSize(level);
                width  >>= 1;
                height >>= 1;
                btSetMax(width, uint32_t(1));
                btSetMax(height, uint32_t(1));
                level++;
            }
        }
        else {
            glTexImage2D(target, 0, internal, width, height, 0, format, type, ptr);
            offset += m_header.estimateSize(0);
        }
    }
    void setVolumeTexture(const uint8_t *ptr) {
        uint32_t width = m_header.width(), height = m_header.height(), depth = m_header.depth();
        GLenum format, internal, type;
        m_header.getTextureFormat(internal, format, type);
        if (m_header.hasMipmap()) {
            uint32_t mipmapCount = m_header.width();
            size_t offset = 0;
            for (uint32_t i = 0; i < mipmapCount; i++) {
                glTexImage3D(GL_TEXTURE_3D, i, internal, width, height, depth, 0, format, type, ptr);
                offset += m_header.estimateSize(i);
                width  >>= 1;
                height >>= 1;
                depth  >>= 1;
                btSetMax(width, uint32_t(1));
                btSetMax(height, uint32_t(1));
                btSetMax(depth, uint32_t(1));
            }
        }
        else {
            glTexImage3D(GL_TEXTURE_3D, 0, internal, width, height, depth, 0, format, type, ptr);
        }
    }

    struct Header {
        enum Flags {
            kCaps        = 0x00000001,
            kHeight      = 0x00000002,
            kWidth       = 0x00000004,
            kPitch       = 0x00000008,
            kPixelFormat = 0x00001000,
            kMipmapCount = 0x00020000,
            kLinearSize  = 0x00080000,
            kDepth       = 0x00800000
        };
        enum Type1Flags {
            kComplex = 0x00000008,
            kTexture = 0x00001000,
            kMipmap  = 0x00400000
        };
        enum Type2Flags {
            kCubemap          = 0x00000200,
            kCubemapPositiveX = 0x00000400,
            kCubemapNegativeX = 0x00000800,
            kCubemapPositiveY = 0x00001000,
            kCubemapNegativeY = 0x00002000,
            kCubemapPositiveZ = 0x00004000,
            kCubemapNegativeZ = 0x00008000,
            kVolume           = 0x00200000
        };
        uint8_t magic[4];
        struct Description {
            uint32_t size;
            uint32_t flags;
            uint32_t height;
            uint32_t width;
            uint32_t pitchOrLinearSize;
            uint32_t depth;
            uint32_t mipmapCount;
            uint32_t reserved[11];
            struct PixelFormat {
                uint32_t size;
                uint32_t flags;
                uint8_t fourcc[4];
                uint32_t bitsCount;
                struct {
                    uint32_t red;
                    uint32_t blue;
                    uint32_t green;
                    uint32_t alpha;
                } bitmask;
            } pixelFormat;
            struct Capacity {
                uint32_t type1;
                uint32_t type2;
                uint32_t reserved[2];
            } capacity;
        } description;
        bool isValidSignature() const { return memcmp(magic, "DDS ", sizeof(magic)) == 0; }
        bool isValidDescriptionSize() const { return description.size == 124; }
        bool isValidPixelFormatSize() const { return description.pixelFormat.size == 32; }
        uint32_t width() const { return description.width; }
        uint32_t height() const { return description.height; }
        uint32_t depth() const { return description.depth; }
        uint32_t mipmapCount() const { return description.mipmapCount; }
        size_t estimateSize(int level) const {
            size_t size = 0;
            level += 1;
            if (description.flags & kLinearSize)
                size = description.pitchOrLinearSize / (level * 4);
            else
                size = description.pitchOrLinearSize * (description.height / level);
            return btMax(size_t(8), size);
        }
        bool isValidCapacity() const {
            return (description.capacity.type1 & kTexture) == kTexture;
        }
        bool hasCubemapPositiveX() const {
            return (description.capacity.type2 & kCubemapPositiveX) == kCubemapPositiveX;
        }
        bool hasCubemapNegativeX() const {
            return (description.capacity.type2 & kCubemapNegativeX) == kCubemapNegativeX;
        }
        bool hasCubemapPositiveY() const {
            return (description.capacity.type2 & kCubemapPositiveY) == kCubemapPositiveY;
        }
        bool hasCubemapNegativeY() const {
            return (description.capacity.type2 & kCubemapNegativeY) == kCubemapNegativeY;
        }
        bool hasCubemapPositiveZ() const {
            return (description.capacity.type2 & kCubemapPositiveZ) == kCubemapPositiveZ;
        }
        bool hasCubemapNegativeZ() const {
            return (description.capacity.type2 & kCubemapNegativeZ) == kCubemapNegativeZ;
        }
        bool hasCubemap() const {
            Header::Description::Capacity c = description.capacity;
            return (c.type2 & kCubemap) == kCubemap;
        }
        bool hasVolume() const {
            Header::Description::Capacity c = description.capacity;
            return (c.type2 & kVolume) == kVolume;
        }
        bool hasMipmap() const {
            uint32_t type1 = description.capacity.type1;
            return (type1 & kComplex) == kComplex && (type1 & kMipmap) == kMipmap;
        }
        bool isValidDescription() const {
            bool ret = true;
            uint32_t sdflags = description.flags;
            ret &= (sdflags & kCaps) == kCaps;
            ret &= (sdflags & kPixelFormat) == kPixelFormat;
            ret &= (sdflags & kWidth) == kWidth;
            ret &= (sdflags & kHeight) == kHeight;
            return ret;
        }
        void correct() {
            btSetMax(description.width, uint32_t(1));
            btSetMax(description.height, uint32_t(1));
            btSetMax(description.depth, uint32_t(1));
            btSetMax(description.pitchOrLinearSize, uint32_t(1));
            btSetMax(description.mipmapCount, uint32_t(1));
        }
        void getDataPointer(const uint8_t *data, uint8_t *&ptr) const {
            ptr = const_cast<uint8_t *>(data) + description.size;
        }
        void getTextureFormat(GLenum &internal, GLenum &format, GLenum &type) const {
            const Header::Description::PixelFormat &pf = description.pixelFormat;
            switch (pf.bitsCount) {
            case 32:
                internal = GL_RGBA8;
                format   = GL_RGBA;
                type     = GL_UNSIGNED_BYTE;
                return;
            case 24:
                internal = GL_RGB8;
                format   = GL_RGB;
                type     = GL_UNSIGNED_BYTE;
                return;
            case 16:
                switch (pf.bitmask.green) {
                case 0x00f0:
                    internal = GL_RGBA16;
                    format   = GL_RGBA;
                    type     = GL_UNSIGNED_SHORT_4_4_4_4;
                    return;
                case 0x07e0:
                    internal = GL_RGB16;
                    format   = GL_RGB;
                    type     = GL_UNSIGNED_SHORT_5_6_5;
                    return;
                case 0x03e0:
                    internal = GL_RGBA16;
                    format   = GL_RGBA;
                    type     = GL_UNSIGNED_SHORT_5_5_5_1;
                    return;
                case 0x0000:
                    internal = GL_LUMINANCE8_ALPHA8;
                    format   = GL_LUMINANCE_ALPHA;
                    type     = GL_UNSIGNED_BYTE;
                    return;
                }
            case 8:
                if (pf.bitmask.alpha) {
                    internal = GL_ALPHA8;
                    format   = GL_ALPHA;
                }
                else {
                    internal = GL_R8;
                    format   = GL_RED;
                }
                type = GL_UNSIGNED_BYTE;
                return;
            }
            internal = GL_NONE;
            format   = GL_NONE;
            type     = GL_UNSIGNED_BYTE;
        }
    } m_header;
    QGLWidget *m_context;
};

class Delegate : public IRenderDelegate
{
public:
    struct PrivateContext {
        QHash<QString, Texture> textureCache;
    };
    Delegate(const QSettings *settings, const Scene *scene, QGLWidget *context)
        : m_settings(settings),
          m_scene(scene),
          m_systemDir(m_settings->value("dir.system.toon", "../../QMA2/resources/images").toString()),
          m_context(context)
    {
        m_timer.start();
    }
    ~Delegate()
    {
    }

    void allocateContext(const IModel *model, void *&context) {
        const IString *name = model->name();
        PrivateContext *ctx = new(std::nothrow) PrivateContext();
        context = ctx;
        qDebug("Allocated the context: %s", name ? name->toByteArray() : reinterpret_cast<const uint8_t *>("(null)"));
    }
    void releaseContext(const IModel *model, void *&context) {
        const IString *name = model->name();
        delete static_cast<PrivateContext *>(context);
        context = 0;
        qDebug("Released the context: %s", name ? name->toByteArray() : reinterpret_cast<const uint8_t *>("(null)"));
    }
    bool uploadTexture(const IString *name, const IString *dir, int flags, Texture &texture, void *context) {
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
                String s(m_systemDir.absolutePath());
                ret = uploadTextureInternal(createPath(&s, name), texture, true, mipmap, context);
            }
            return ret;
        }
        return false;
    }
    void getToonColor(const IString *name, const IString *dir, Color &value, void * /* context */) {
        const QString &path = createPath(dir, name);
        if (QFile::exists(path)) {
            getToonColorInternal(path, value);
        }
        else {
            String s(m_systemDir.absolutePath());
            getToonColorInternal(createPath(&s, name), value);
        }
    }
    IModel *findModel(const char * /* name */) const {
        return 0;
    }
    void getMatrix(float value[], const IModel *model, int flags) const {
        QMatrix4x4 m;
        if (flags & IRenderDelegate::kShadowMatrix) {
            if (flags & IRenderDelegate::kProjectionMatrix)
                m *= m_cameraProjectionMatrix;
            if (flags & IRenderDelegate::kViewMatrix)
                m *= m_cameraViewMatrix;
            if (flags & IRenderDelegate::kWorldMatrix) {
                static const Vector3 plane(0.0f, 1.0f, 0.0f);
                const Scene::ILight *light = m_scene->light();
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
    void getViewport(Vector3 &value) const {
        value.setValue(m_viewport.width(), m_viewport.height(), 0);
    }
    void getTime(float &value, bool sync) const {
        value = sync ? 0 : m_timer.elapsed() / 1000.0f;
    }
    void getElapsed(float &value, bool sync) const {
        value = sync ? 0 : 1.0 / 60.0;
    }
    void getMousePosition(Vector4 &value, MousePositionType type) const {
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
    void log(void * /* context */, LogLevel /* level */, const char *format, va_list ap) {
        vfprintf(stderr, format, ap);
        fprintf(stderr, "%s", "\n");
    }
    IString *loadKernelSource(KernelType type, void * /* context */) {
        QString file;
        switch (type) {
        case kModelSkinningKernel:
            file = "skinning.cl";
            break;
        case kMaxKernelType:
        default:
            break;
        }
        QByteArray bytes;
        QString path = m_settings->value("dir.system.kernels", "../../QMA2/resources/kernels").toString() + "/" + file;
        if (UISlurpFile(path, bytes)) {
            qDebug("Loaded a kernel: %s", qPrintable(path));
            return new(std::nothrow) String(bytes);
        }
        else {
            return 0;
        }
    }
    IString *loadShaderSource(ShaderType type, const IModel *model, const IString *dir, void * /* context */) {
        QString file;
        if (type == kModelEffectTechniques) {
            QDir d(static_cast<const String *>(dir)->value());
            QByteArray bytes;
            if (m_model2filename.contains(model)) {
                QFileInfo info(d.absoluteFilePath(m_model2filename[model]));
                QRegExp regexp("^.+\\[([^\\]]+)\\]$");
                const QString &name = info.baseName();
                const QString &basename = regexp.exactMatch(name) ? regexp.capturedTexts().at(1) : name;
                const QString &cgfx = d.absoluteFilePath(basename + ".cgfx");
                if (QFile::exists(cgfx))
                    return UISlurpFile(cgfx, bytes) ? new (std::nothrow) String(bytes) : 0;
                const QString &fx = d.absoluteFilePath(basename + ".fx");
                if (QFile::exists(fx))
                    return UISlurpFile(fx, bytes) ? new (std::nothrow) String(bytes) : 0;
            }
            return UISlurpFile(d.absoluteFilePath("effect.fx"), bytes) ? new(std::nothrow) String(bytes) : 0;
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
        QByteArray bytes;
        QString path = m_settings->value("dir.system.shaders", "../../QMA2/resources/shaders").toString() + "/" + file;
        if (UISlurpFile(path, bytes)) {
            qDebug("Loaded a shader: %s", qPrintable(path));
            return new(std::nothrow) String("#version 120\n" + bytes);
        }
        else {
            return 0;
        }
    }
    IString *toUnicode(const uint8_t *value) const {
        QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
        const QString &s = codec->toUnicode(reinterpret_cast<const char *>(value));
        return new(std::nothrow) String(s);
    }

    void updateMatrices(const QSize &size) {
        float matrix[16];
        m_viewport = size;
        m_scene->camera()->modelViewTransform().getOpenGLMatrix(matrix);
        for (int i = 0; i < 16; i++)
            m_cameraViewMatrix.data()[i] = matrix[i];
        m_cameraProjectionMatrix.setToIdentity();
        Scene::ICamera *camera = m_scene->camera();
        m_cameraProjectionMatrix.perspective(camera->fov(), size.width() / float(size.height()), camera->znear(), camera->zfar());
    }
    void setLightMatrices(const QMatrix4x4 &world, const QMatrix4x4 &view, const QMatrix4x4 &projection) {
        m_lightWorldMatrix = world;
        m_lightViewMatrix = view;
        m_lightProjectionMatrix = projection;
    }
    void setMousePosition(const Vector3 &value, bool pressed, MousePositionType type) {
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
    void addModelFilename(const IModel *model, const QString &filename) {
        if (model)
            m_model2filename.insert(model, filename);
    }

private:
    static const QString createPath(const IString *dir, const QString &name) {
        const QDir d(static_cast<const String *>(dir)->value());
        return d.absoluteFilePath(name);
    }
    static const QString createPath(const IString *dir, const IString *name) {
        const QDir d(static_cast<const String *>(dir)->value());
        return d.absoluteFilePath(static_cast<const String *>(name)->value());
    }
    static void setTextureID(const Texture &cache, bool isToon, Texture &output) {
        output = cache;
        if (!isToon) {
            GLuint textureID = *static_cast<GLuint *>(output.object);
            glTexParameteri(textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
    }
    static void addTextureCache(PrivateContext *context, const QString &path, const Texture &texture) {
        if (context)
            context->textureCache.insert(path, texture);
    }
    bool uploadTextureInternal(const QString &path, Texture &texture, bool isToon, bool mipmap, void *context) {
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
                DirectDrawSurface dds(m_context);
                GLuint textureID;
                if (!dds.parse(data, bytes.size(), textureID)) {
                    qDebug("Cannot parse a DDS texture %s", qPrintable(path));
                    return false;
                }
                setTextureID(texture, isToon, texture);
                addTextureCache(privateContext, path, texture);
                return true;
            }
            else {
                qDebug("Cannot open a DDS texture %s: %s", qPrintable(path), qPrintable(file.errorString()));
                return false;
            }
        }
        else {
            const QImage &image = QImage(path).rgbSwapped();
            QGLContext::BindOptions options = QGLContext::LinearFilteringBindOption
                    | QGLContext::InvertedYBindOption
                    | QGLContext::PremultipliedAlphaBindOption;
            if (mipmap)
                options |= QGLContext::MipmapBindOption;
            GLuint textureID = m_context->bindTexture(QGLWidget::convertToGLFormat(image), GL_TEXTURE_2D, GL_RGBA, options);
            m_textures.insert(textureID, textureID);
            texture.width = image.width();
            texture.height = image.height();
            texture.object = &m_textures[textureID];
            setTextureID(texture, isToon, texture);
            addTextureCache(privateContext, path, texture);
            qDebug("Loaded a texture (ID=%d, width=%d, height=%d): \"%s\"",
                   textureID, image.width(), image.height(), qPrintable(path));
            return textureID != 0;
        }
    }
    void getToonColorInternal(const QString &path, Color &value) {
        const QImage image(path);
        if (!image.isNull()) {
            const QRgb &rgb = image.pixel(image.width() - 1, image.height() - 1);
            const QColor color(rgb);
            value.setValue(color.redF(), color.greenF(), color.blueF(), color.alphaF());
        }
    }

    const QSettings *m_settings;
    const Scene *m_scene;
    const QDir m_systemDir;
    QGLWidget *m_context;
    QSize m_viewport;
    QHash<const IModel *, QString> m_model2filename;
    QMatrix4x4 m_lightWorldMatrix;
    QMatrix4x4 m_lightViewMatrix;
    QMatrix4x4 m_lightProjectionMatrix;
    QMatrix4x4 m_cameraViewMatrix;
    QMatrix4x4 m_cameraProjectionMatrix;
    QElapsedTimer m_timer;
    Vector4 m_mouseCursorPosition;
    Vector4 m_mouseLeftPressPosition;
    Vector4 m_mouseMiddlePressPosition;
    Vector4 m_mouseRightPressPosition;
    QHash<GLuint, GLuint> m_textures;
};
} /* namespace anonymous */

QDebug operator<<(QDebug debug, const Vector3 &v)
{
    debug.nospace() << "(x=" << v.x() << ", y=" << v.y() << ", z=" << v.z() << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const Color &v)
{
    debug.nospace() << "(r=" << v.x() << ", g=" << v.y() << ", b=" << v.z() << ", a=" << v.w() << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const IString *str)
{
    if (str) {
        debug.nospace() << static_cast<const String *>(str)->value();
    }
    else {
        debug.nospace() << "(null)";
    }
    return debug;
}

QDebug operator<<(QDebug debug, const pmx::Bone *bone)
{
    if (!bone) {
        debug.nospace() << "Bone is null";
        return debug.space();
    }
    debug.nospace();
    debug << "Bone id                          = " << bone->index();
    debug << "\n";
    debug << "     name                        = " << bone->name();
    debug << "\n";
    debug << "     english                     = " << bone->englishName();
    debug << "\n";
    debug << "     origin                      = " << bone->origin();
    debug << "\n";
    if (bone->parentBone()) {
        debug << "     parent                      = " << bone->parentBone()->name();
        debug << "\n";
    }
    debug << "     index                       = " << bone->layerIndex();
    debug << "\n";
    debug << "     offset                      = " << bone->origin();
    debug << "\n";
    if (bone->hasInverseKinematics()) {
        debug << "     targetBone                  = " << bone->targetBone()->name();
        debug << "\n";
        debug << "     constraintAngle             = " << bone->constraintAngle();
        debug << "\n";
    }
    if (bone->hasPositionInherence()) {
        debug << "     parentPositionInherenceBone = " << bone->parentInherenceBone()->name();
        debug << "\n";
        debug << "     weight                      = " << bone->weight();
        debug << "\n";
    }
    if (bone->hasRotationInherence()) {
        debug << "     parentRotationInherenceBone = " << bone->parentInherenceBone()->name();
        debug << "\n";
        debug << "     weight                      = " << bone->weight();
        debug << "\n";
    }
    if (bone->hasFixedAxes()) {
        debug << "     axis                        = " << bone->axis();
        debug << "\n";
    }
    if (bone->hasLocalAxes()) {
        debug << "     axisX                       = " << bone->axisX();
        debug << "\n";
        debug << "     axisZ                       = " << bone->axisZ();
        debug << "\n";
    }
    return debug.space();
}

QDebug operator<<(QDebug debug, const pmx::Material *material)
{
    if (!material) {
        debug.nospace() << "Material is null";
        return debug.space();
    }
    debug.nospace();
    debug << "Material id                      = " << material->index();
    debug << "\n";
    debug << "         name                    = " << material->name();
    debug << "\n";
    debug << "         english                 = " << material->englishName();
    debug << "\n";
    debug << "         mainTexture             = " << material->mainTexture();
    debug << "\n";
    debug << "         sphereTexture           = " << material->sphereTexture();
    debug << "\n";
    debug << "         toonTexture             = " << material->toonTexture();
    debug << "\n";
    debug << "         ambient                 = " << material->ambient();
    debug << "\n";
    debug << "         diffuse                 = " << material->diffuse();
    debug << "\n";
    debug << "         specular                = " << material->specular();
    debug << "\n";
    debug << "         edgeColor               = " << material->edgeColor();
    debug << "\n";
    debug << "         shininess               = " << material->shininess();
    debug << "\n";
    debug << "         edgeSize                = " << material->edgeSize();
    debug << "\n";
    debug << "         indices                 = " << material->indices();
    debug << "\n";
    debug << "         isSharedToonTextureUsed = " << material->isSharedToonTextureUsed();
    debug << "\n";
    debug << "         isCullDisabled          = " << material->isCullFaceDisabled();
    debug << "\n";
    debug << "         hasShadow               = " << material->hasShadow();
    debug << "\n";
    debug << "         isShadowMapDrawin       = " << material->isShadowMapDrawn();
    debug << "\n";
    debug << "         isEdgeDrawn             = " << material->isEdgeDrawn();
    debug << "\n";
    switch (material->sphereTextureRenderMode()) {
    case pmx::Material::kAddTexture:
        debug << "         sphere                  = add";
        break;
    case pmx::Material::kMultTexture:
        debug << "         sphere                  = modulate";
        break;
    case pmx::Material::kNone:
        debug << "         sphere                  = none";
        break;
    case pmx::Material::kSubTexture:
        debug << "         sphere                  = subtexture";
        break;
    }
    debug << "\n";
    return debug.space();
}

QDebug operator<<(QDebug debug, const pmx::Morph *morph)
{
    if (!morph) {
        debug.nospace() << "Morph is null";
        return debug.space();
    }
    debug.nospace();
    debug << "Morph id      = " << morph->index();
    debug << "\n";
    debug << "      name    = " << morph->name();
    debug << "\n";
    debug << "      english = " << morph->englishName();
    debug << "\n";
    debug << "      weight  = " << morph->weight();
    debug << "\n";
    return debug.space();
}

QDebug operator<<(QDebug debug, const pmx::Label *label)
{
    if (!label) {
        debug.nospace() << "Label is null";
        return debug.space();
    }
    debug.nospace();
    debug << "Label id        = " << label->index();
    debug << "\n";
    debug << "      name      = " << label->name();
    debug << "\n";
    debug << "      english   = " << label->englishName();
    debug << "\n";
    debug << "      isSpecial = " << label->isSpecial();
    debug << "\n";
    debug << "      count     = " << label->count();
    debug << "\n";
    for (int i = 0; i < label->count(); i++) {
        if (IBone *bone = label->bone(i))
            debug << "      bone      = " << bone->name();
        else if (IMorph *morph = label->morph(i))
            debug << "      morph     = " << morph->name();
        debug << "\n";
    }
    return debug.space();
}

QDebug operator<<(QDebug debug, const pmx::RigidBody *body)
{
    if (!body) {
        debug.nospace() << "RigidBody is null";
        return debug.space();
    }
    debug.nospace();
    debug << "RigidBody id                 = " << body->index();
    debug << "\n";
    debug << "          name               = " << body->name();
    debug << "\n";
    debug << "          english            = " << body->englishName();
    debug << "\n";
    debug << "          size               = " << body->size();
    debug << "\n";
    debug << "          position           = " << body->position();
    debug << "\n";
    debug << "          rotation           = " << body->rotation();
    debug << "\n";
    debug << "          mass               = " << body->mass();
    debug << "\n";
    debug << "          linearDamping      = " << body->linearDamping();
    debug << "\n";
    debug << "          angularDamping     = " << body->angularDamping();
    debug << "\n";
    debug << "          restitution        = " << body->restitution();
    debug << "\n";
    debug << "          friction           = " << body->friction();
    debug << "\n";
    debug << "          groupID            = " << body->groupID();
    debug << "\n";
    debug << "          collisionGroupMask = " << body->collisionGroupMask();
    debug << "\n";
    debug << "          collisionGroupID   = " << body->collisionGroupID();
    debug << "\n";
    return debug.space();
}

QDebug operator<<(QDebug debug, const pmx::Joint *joint)
{
    if (!joint) {
        debug.nospace() << "Joint is null";
        return debug.space();
    }
    debug.nospace();
    debug << "Joint id                 = " << joint->index();
    debug << "\n";
    debug << "      name               = " << joint->name();
    debug << "\n";
    debug << "      english            = " << joint->englishName();
    debug << "\n";
    debug << "      position           = " << joint->position();
    debug << "\n";
    debug << "      rotation           = " << joint->rotation();
    debug << "\n";
    debug << "      positionLowerLimit = " << joint->positionLowerLimit();
    debug << "\n";
    debug << "      positionUpperLimit = " << joint->positionUpperLimit();
    debug << "\n";
    debug << "      rotationLowerLimit = " << joint->rotationLowerLimit();
    debug << "\n";
    debug << "      rotationUpperLimit = " << joint->rotationUpperLimit();
    debug << "\n";
    debug << "      positionStiffness  = " << joint->positionStiffness();
    debug << "\n";
    debug << "      rotationStiffness  = " << joint->rotationStiffness();
    debug << "\n";
    if (joint->rigidBody1()) {
        debug << "      rigidBody1         = " << joint->rigidBody1()->name();
        debug << "\n";
    }
    if (joint->rigidBody2()) {
        debug << "      rigidBody2         = " << joint->rigidBody2()->name();
        debug << "\n";
    }
    return debug.space();
}

class UI : public QGLWidget
{
public:
    UI()
        : QGLWidget(QGLFormat(QGL::SampleBuffers), 0),
      #ifndef VPVL2_NO_BULLET
          m_dispatcher(&m_config),
          m_broadphase(Vector3(-10000.0f, -10000.0f, -10000.0f), Vector3(10000.0f, 10000.0f, 10000.0f), 1024),
          m_world(&m_dispatcher, &m_broadphase, &m_solver, &m_config),
      #endif /* VPVL2_NO_BULLET */
          m_settings(0),
          m_fbo(0),
          m_delegate(0),
          m_factory(0),
          m_encoding(0),
          m_depthTextureID(0),
          m_prevElapsed(0),
          m_currentFrameIndex(0)
    {
        Encoding *encoding = new Encoding();
        m_encoding = encoding;
        m_factory = new Factory(encoding);
#ifndef VPVL2_NO_BULLET
        m_world.setGravity(btVector3(0.0f, -9.8f * 2.0f, 0.0f));
        m_world.getSolverInfo().m_numIterations = static_cast<int>(10.0f);
#endif /* VPVL2_NO_BULLET */
        setMinimumSize(320, 240);
        setMouseTracking(true);
    }
    ~UI() {
#ifdef VPVL2_LINK_ASSIMP
        Assimp::DefaultLogger::kill();
#endif
        delete m_fbo;
        delete m_factory;
        delete m_encoding;
    }

    void load(const QString &filename) {
        m_settings = new QSettings(filename, QSettings::IniFormat, this);
        m_settings->setIniCodec("UTF-8");
        m_delegate = new Delegate(m_settings, &m_scene, this);
        m_delegate->updateMatrices(size());
        resize(m_settings->value("window.width", 640).toInt(), m_settings->value("window.height", 480).toInt());
        m_scene.setPreferredFPS(qMax(m_settings->value("scene.fps", 30).toFloat(), Scene::defaultFPS()));
        if (m_settings->value("enable.opencl", false).toBool())
            m_scene.setAccelerationType(Scene::kOpenCLAccelerationType1);
        else if (m_settings->value("enable.vss", false).toBool())
            m_scene.setAccelerationType(Scene::kVertexShaderAccelerationType1);
        Scene::ICamera *camera = m_scene.camera();
        camera->setZNear(qMax(m_settings->value("scene.znear", 0.1f).toFloat(), 0.1f));
        camera->setZFar(qMax(m_settings->value("scene.zfar", 10000.0).toFloat(), 100.0f));
        Scene::ILight *light = m_scene.light();
        light->setToonEnable(m_settings->value("enable.toon", true).toBool());
        light->setSoftShadowEnable(m_settings->value("enable.ss", true).toBool());
        if (m_fbo) {
            m_depthTextureID = m_fbo->texture();
            light->setDepthTextureSize(Vector3(m_fbo->width(), m_fbo->height(), 0.0));
            if (m_settings->value("enable.sm", false).toBool())
                light->setDepthTexture(&m_depthTextureID);
        }
        if (loadScene())
            m_timer.start();
        else
            qFatal("Unable to load scene");
    }
    void rotate(float x, float y) {
        Scene::ICamera *camera = m_scene.camera();
        Vector3 angle = camera->angle();
        angle.setX(angle.x() + x);
        angle.setY(angle.y() + y);
        camera->setAngle(angle);
    }
    void translate(float x, float y) {
        Scene::ICamera *camera = m_scene.camera();
        const Vector3 &diff = camera->modelViewTransform() * Vector3(x, y, 0);
        Vector3 position = camera->position() + diff;
        camera->setPosition(position + diff);
    }

protected:
    void initializeGL() {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        startTimer(1000.0f / 60.0f);
        Scene::ILight *light = m_scene.light();
        QGLFramebufferObjectFormat format;
        format.setAttachment(QGLFramebufferObject::Depth);
#if GL_ARB_texture_float
        format.setInternalTextureFormat(GL_RGBA32F_ARB);
        light->setHasFloatTexture(true);
#endif
        m_fbo = new QGLFramebufferObject(1024, 1024, format);
        GLuint textureID = m_depthTextureID = m_fbo->texture();
        if (textureID > 0) {
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
    void timerEvent(QTimerEvent *) {
        float elapsed = m_timer.elapsed() / static_cast<float>(60.0f);
        float diff = elapsed - m_prevElapsed;
        m_prevElapsed = elapsed;
        if (diff < 0)
            diff = elapsed;
        m_scene.advance(diff);
        const Array<IMotion *> &motions = m_scene.motions();
        const int nmotions = motions.count();
        for (int i = 0; i < nmotions; i++) {
            IMotion *motion = motions[i];
            if (motion->isReachedTo(motion->maxFrameIndex())) {
                motion->reset();
                m_currentFrameIndex = 0;
            }
        }
        m_world.stepSimulation(diff * (60.0 / m_scene.preferredFPS()));
        m_delegate->updateMatrices(size());
        m_scene.updateRenderEngines();
        updateGL();
    }
    void mousePressEvent(QMouseEvent *event) {
        m_prevPos = event->pos();
        setMousePositions(event);
    }
    void mouseMoveEvent(QMouseEvent *event) {
        if (event->buttons() & Qt::LeftButton) {
            Qt::KeyboardModifiers modifiers = event->modifiers();
            const QPoint &diff = event->pos() - m_prevPos;
            if (modifiers & Qt::ShiftModifier) {
                translate(diff.x() * -0.1f, diff.y() * 0.1f);
            }
            else {
                rotate(diff.y() * 0.5f, diff.x() * 0.5f);
            }
            m_prevPos = event->pos();
        }
        setMousePositions(event);
    }
    void mouseReleaseEvent(QMouseEvent *event) {
        setMousePositions(event);
    }
    void wheelEvent(QWheelEvent *event) {
        Qt::KeyboardModifiers modifiers = event->modifiers();
        Scene::ICamera *camera = m_scene.camera();
        if (modifiers & Qt::ControlModifier && modifiers & Qt::ShiftModifier) {
            const qreal step = 1.0;
            camera->setFov(qMax(event->delta() > 0 ? camera->fov() - step : camera->fov() + step, 0.0));
        }
        else {
            qreal step = 4.0;
            if (modifiers & Qt::ControlModifier)
                step *= 5.0f;
            else if (modifiers & Qt::ShiftModifier)
                step *= 0.2f;
            if (step != 0.0f)
                camera->setDistance(event->delta() > 0 ? camera->distance() - step : camera->distance() + step);
        }
    }
    void resizeGL(int w, int h) {
        glViewport(0, 0, w, h);
    }
    void paintGL() {
        if (!m_delegate) {
            glViewport(0, 0, width(), height());
            glClearColor(0, 0, 1, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            return;
        }
        const Array<IRenderEngine *> &engines = m_scene.renderEngines();
        const int nengines = engines.count();
        if (m_scene.light()->depthTexture()) {
            glDisable(GL_BLEND);
            m_fbo->bind();
            Vector3 target = kZeroV3, center;
            Scalar maxRadius = 0, radius;
            const Array<IModel *> &models = m_scene.models();
            const int nmodels = models.count();
            Array<Scalar> radiusArray;
            Array<Vector3> centerArray;
            for (int i = 0; i < nmodels; i++) {
                IModel *model = models[i];
                if (model->isVisible()) {
                    model->getBoundingSphere(center, radius);
                    radiusArray.add(radius);
                    centerArray.add(target);
                    target += center;
                }
            }
            target /= nmodels;
            for (int i = 0; i < nmodels; i++) {
                IModel *model = models[i];
                if (model->isVisible()) {
                    const Vector3 &c = centerArray[i];
                    const Scalar &r = radiusArray[i];
                    const Scalar &d = target.distance(c) + r;
                    btSetMax(maxRadius, d);
                }
            }
            const Scalar &angle = 45;
            const Scalar &distance = maxRadius / btSin(btRadians(angle) * 0.5);
            const Scalar &margin = 50;
            const Vector3 &eye = -m_scene.light()->direction().normalized() * maxRadius + target;
            QMatrix4x4 lightViewMatrix, lightProjectionMatrix;
            lightViewMatrix.lookAt(QVector3D(eye.x(), eye.y(), eye.z()),
                                   QVector3D(target.x(), target.y(), target.z()),
                                   QVector3D(0, 1, 0));
            lightProjectionMatrix.perspective(angle, 1, 1, distance + maxRadius + margin);
            QMatrix4x4 lightWorldMatrix;
            m_delegate->setLightMatrices(lightWorldMatrix, lightViewMatrix, lightProjectionMatrix);
            glViewport(0, 0, m_fbo->width(), m_fbo->height());
            glClearColor(1, 1, 1, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            for (int i = 0; i < nengines; i++) {
                IRenderEngine *engine = engines[i];
                engine->renderZPlot();
            }
            m_fbo->release();
#ifndef VPVL2_ENABLE_NVIDIA_CG
            lightWorldMatrix.scale(0.5);
            lightWorldMatrix.translate(1, 1, 1);
            m_delegate->setLightMatrices(lightWorldMatrix, lightViewMatrix, lightProjectionMatrix);
#endif
        }
        {
            glViewport(0, 0, width(), height());
            glEnable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
            glClearColor(0, 0, 1, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            for (int i = 0; i < nengines; i++) {
                IRenderEngine *engine = engines[i];
                engine->preparePostProcess();
            }
            for (int i = 0; i < nengines; i++) {
                IRenderEngine *engine = engines[i];
                engine->performPreProcess();
            }
            for (int i = 0; i < nengines; i++) {
                IRenderEngine *engine = engines[i];
                engine->renderModel();
                engine->renderEdge();
                engine->renderShadow();
            }
            for (int i = 0; i < nengines; i++) {
                IRenderEngine *engine = engines[i];
                engine->performPostProcess();
            }
        }
    }

private:
    void setMousePositions(QMouseEvent *event) {
        const QPointF &pos = event->posF();
        const QSizeF &size = geometry().size() / 2;
        const qreal w = size.width(), h = size.height();
        const Vector3 &value = Vector3((pos.x() - w) / w, (pos.y() - h) / -h, 0);
        Qt::MouseButtons buttons = event->buttons();
        m_delegate->setMousePosition(value, buttons & Qt::LeftButton, IRenderDelegate::kMouseLeftPressPosition);
        m_delegate->setMousePosition(value, buttons & Qt::MiddleButton, IRenderDelegate::kMouseMiddlePressPosition);
        m_delegate->setMousePosition(value, buttons & Qt::RightButton, IRenderDelegate::kMouseRightPressPosition);
        m_delegate->setMousePosition(value, false, IRenderDelegate::kMouseCursorPosition);
    }
    bool loadScene() {
#ifdef VPVL2_LINK_ASSIMP
        Assimp::Logger::LogSeverity severity = Assimp::Logger::VERBOSE;
        Assimp::DefaultLogger::create("", severity, aiDefaultLogStream_STDOUT);
        // addModel(QDir(kStageDir).absoluteFilePath(kStageName));
        // addModel(kStage2Name, kStageDir);
#endif
        const QString &modelPath = QDir(m_settings->value("dir.model").toString())
                .absoluteFilePath(m_settings->value("file.model").toString());
        const QString &assetPath = QDir(m_settings->value("dir.asset").toString())
                .absoluteFilePath(m_settings->value("file.asset").toString());
        const QString &modelMotionPath = QDir(m_settings->value("dir.motion").toString())
                .absoluteFilePath(m_settings->value("file.motion").toString());
        const QString &cameraMotionPath = QDir(m_settings->value("dir.camera").toString())
                .absoluteFilePath(m_settings->value("file.camera").toString());
        IModel *model = addModel(modelPath);
        if (model) {
            addMotion(modelMotionPath, model);
            model->setOpacity(1.0);
        }
        addModel(assetPath);
        QByteArray bytes;
        if (UISlurpFile(cameraMotionPath, bytes)) {
            bool ok = true;
            const uint8_t *data = reinterpret_cast<const uint8_t *>(bytes.constData());
            IMotion *motion = m_factory->createMotion(data, bytes.size(), 0, ok);
            qDebug() << "maxFrameIndex(camera):" << motion->maxFrameIndex();
            m_scene.camera()->setMotion(motion);
        }
        m_scene.seek(0);
        return true;
    }
    IModel *addModel(const QString &path) {
        QByteArray bytes;
        if (!UISlurpFile(path, bytes)) {
            qWarning("Failed loading the model");
            return 0;
        }
        const QFileInfo info(path);
        bool ok = true;
        const uint8_t *data = reinterpret_cast<const uint8_t *>(bytes.constData());
        IModel *model = m_factory->createModel(data, bytes.size(), ok);
        if (!ok) {
            qWarning("Failed parsing the model: %d", model->error());
            return 0;
        }
        m_delegate->addModelFilename(model, info.fileName());
        model->setEdgeWidth(m_settings->value("edge.width", 1.0).toFloat());
        model->joinWorld(&m_world);
        IRenderEngine *engine = m_scene.createRenderEngine(m_delegate, model);
        String s(info.absoluteDir().absolutePath());
        engine->upload(&s);
        m_scene.addModel(model, engine);
#if 0
        pmx::Model *pmx = dynamic_cast<pmx::Model*>(model);
        if (pmx) {
            const Array<pmx::Material *> &materials = pmx->materials();
            const int nmaterials = materials.count();
            for (int i = 0; i < nmaterials; i++)
                qDebug() << materials[i];
            const Array<pmx::Bone *> &bones = pmx->bones();
            const int nbones = bones.count();
            for (int i = 0; i < nbones; i++)
                qDebug() << bones[i];
            const Array<pmx::Morph *> &morphs = pmx->morphs();
            const int nmorphs = morphs.count();
            for (int i = 0; i < nmorphs; i++)
                qDebug() << morphs.at(i);
            const Array<pmx::Label *> &labels = pmx->labels();
            const int nlabels = labels.count();
            for (int i = 0; i < nlabels; i++)
                qDebug() << labels.at(i);
            const Array<pmx::RigidBody *> &bodies = pmx->rigidBodies();
            const int nbodies = bodies.count();
            for (int i = 0; i < nbodies; i++)
                qDebug() << bodies.at(i);
            const Array<pmx::Joint *> &joints = pmx->joints();
            const int njoints = joints.count();
            for (int i = 0; i < njoints; i++)
                qDebug() << joints.at(i);
        }
#endif
        return model;
    }
    void addMotion(const QString &path, IModel *model) {
        QByteArray bytes;
        if (model && UISlurpFile(path, bytes)) {
            bool ok = true;
            IMotion *motion = m_factory->createMotion(reinterpret_cast<const uint8_t *>(bytes.constData()), bytes.size(), model, ok);
            qDebug() << "maxFrameIndex(model):" << motion->maxFrameIndex();
            motion->seek(0);
            m_scene.addMotion(motion);
        }
        else {
            qWarning("Failed parsing the model motion, skipped...");
        }
    }

#ifndef VPVL2_NO_BULLET
    btDefaultCollisionConfiguration m_config;
    btCollisionDispatcher m_dispatcher;
    btAxisSweep3 m_broadphase;
    btSequentialImpulseConstraintSolver m_solver;
    btDiscreteDynamicsWorld m_world;
#endif /* VPVL2_NO_BULLET */
    QSettings *m_settings;
    QElapsedTimer m_timer;
    QGLFramebufferObject *m_fbo;
    QPoint m_prevPos;
    QMatrix4x4 m_projectionMatrix;
    QMatrix4x4 m_modelViewMatrix;
    Delegate *m_delegate;
    Scene m_scene;
    Factory *m_factory;
    IEncoding *m_encoding;
    GLuint m_depthTextureID;
    float m_prevElapsed;
    float m_currentFrameIndex;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
#if 1
    UI ui;
    ui.show();
    ui.load(QDir::current().absoluteFilePath("config.ini"));
    return app.exec();
#else
    UI *ui = new UI();
    ui->load(QDir::current().absoluteFilePath("config.ini"));
    ui->show();
    delete ui;
    return 0;
#endif
}
