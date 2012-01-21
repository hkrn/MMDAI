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

#ifndef DELEGATE_H
#define DELEGATE_H

#include <QtCore/QtCore>
#include <QtOpenGL/QtOpenGL>
#include "util.h"

#include <vpvl/Common.h>
#ifdef VPVL_ENABLE_GLSL
#include <vpvl/gl2/Renderer.h>
#else
#include <vpvl/gl/Renderer.h>
#endif

namespace internal {

typedef QScopedPointer<uint8_t, QScopedPointerArrayDeleter<uint8_t> > ByteArrayPtr;

#ifdef VPVL_ENABLE_GLSL
using namespace vpvl::gl2;
class Delegate : public Renderer::IDelegate
#else
using namespace vpvl::gl;
class Delegate : public IDelegate
#endif
{
public:
    Delegate(QGLWidget *widget)
        : m_widget(widget)
    {
    }
    ~Delegate() {
    }

    bool uploadTexture(const std::string &path, GLuint &textureID, bool isToon) {
        QString pathString = QString::fromLocal8Bit(path.c_str());
        pathString.replace("\\", "/");
        QFileInfo info(pathString);
        if (info.isDir() || !info.exists()) {
            qWarning("Loading texture %s doesn't exists", qPrintable(info.absoluteFilePath()));
            return false;
        }
        QScopedArrayPointer<uint8_t> ptr;
        QImage image = QImage(pathString).rgbSwapped();
        if (image.isNull() && pathString.endsWith(".tga", Qt::CaseInsensitive)) {
            image = loadTGA(pathString, ptr);
        }
        if (image.isNull()) {
            qWarning("Loading texture %s cannot decode", qPrintable(info.absoluteFilePath()));
            return false;
        }
        if (pathString.endsWith(".sph") || pathString.endsWith(".spa")) {
            QTransform transform;
            transform.scale(1, -1);
            image = image.transformed(transform);
        }
        QGLContext::BindOptions options = QGLContext::LinearFilteringBindOption|QGLContext::InvertedYBindOption;
        textureID = m_widget->bindTexture(QGLWidget::convertToGLFormat(image), GL_TEXTURE_2D,
                                          image.depth() == 32 ? GL_RGBA : GL_RGB, options);
        if (!isToon) {
            glTexParameteri(textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        qDebug("Loaded a texture (ID=%d): \"%s\"", textureID, qPrintable(pathString));
        return textureID != 0;
    }
    bool uploadToonTexture(const std::string &name, const std::string &dir, GLuint &textureID) {
        const QString &filename = QString::fromLocal8Bit(name.c_str());
        QString path = QString::fromLocal8Bit(dir.c_str()) + "/" + filename;
        path.replace("\\", "/");
        if (!QFile::exists(path))
            path = QString(":/textures/%1").arg(filename);
        return uploadTexture(std::string(path.toLocal8Bit()), textureID, true);
    }
    const std::string loadShader(Renderer::ShaderType type) {
        QString filename;
        switch (type) {
        case Renderer::kAssetVertexShader:
            filename = "asset.vsh";
            break;
        case Renderer::kAssetFragmentShader:
            filename = "asset.fsh";
            break;
        case Renderer::kEdgeVertexShader:
            filename = "edge.vsh";
            break;
        case Renderer::kEdgeFragmentShader:
            filename = "edge.fsh";
            break;
        case Renderer::kModelVertexShader:
            filename = "model.vsh";
            break;
        case Renderer::kModelFragmentShader:
            filename = "model.fsh";
            break;
        case Renderer::kShadowVertexShader:
            filename = "shadow.vsh";
            break;
        case Renderer::kShadowFragmentShader:
            filename = "shadow.fsh";
            break;
        case Renderer::kZPlotVertexShader:
            filename = "zplot.vsh";
            break;
        case Renderer::kZPlotFragmentShader:
            filename = "zplot.fsh";
            break;
        }
        const QString path = QString(":/shaders/%1").arg(filename);
        QFile file(path);
        if (file.open(QFile::ReadOnly)) {
            QByteArray bytes = file.readAll();
            file.close();
            log(Renderer::kLogInfo, "Loaded a shader: %s", qPrintable(path));
            return std::string(reinterpret_cast<const char *>(bytes.constData()), bytes.size());
        }
        else {
            return std::string();
        }
    }
    const std::string loadKernel(Renderer::KernelType type) {
        QString filename;
        switch (type) {
        case Renderer::kModelSkinningKernel:
            filename = "skinning.cl";
            break;
        }
        const QString path = QString(":/kernels/%1").arg(filename);
        QFile file(path);
        if (file.open(QFile::ReadOnly)) {
            QByteArray bytes = file.readAll();
            file.close();
            log(Renderer::kLogInfo, "Loaded a kernel: %s", qPrintable(path));
            return std::string(reinterpret_cast<const char *>(bytes.constData()), bytes.size());
        }
        else {
            return std::string();
        }
    }
    void log(Renderer::LogLevel level, const char *format...) {
        QString message;
        va_list ap;
        va_start(ap, format);
        message.vsprintf(format, ap);
        switch (level) {
        case Renderer::kLogInfo:
        default:
            qDebug("%s", qPrintable(message));
            break;
        case Renderer::kLogWarning:
            qWarning("%s", qPrintable(message));
            break;
        }
        va_end(ap);
    }
    const std::string toUnicode(const uint8_t *value) {
        return std::string(internal::toQString(value).toUtf8());
    }

private:
    QImage loadTGA(const QString &path, QScopedArrayPointer<uint8_t> &dataPtr) {
        QFile file(path);
        if (file.open(QFile::ReadOnly) && file.size() > 18) {
            QByteArray data = file.readAll();
            uint8_t *ptr = reinterpret_cast<uint8_t *>(data.data());
            uint8_t field = *reinterpret_cast<uint8_t *>(ptr);
            uint8_t type = *reinterpret_cast<uint8_t *>(ptr + 2);
            if (type != 2 /* full color */ && type != 10 /* full color + RLE */) {
                qWarning("Loaded TGA image type is not full color: %s", qPrintable(path));
                return QImage();
            }
            uint16_t width = *reinterpret_cast<uint16_t *>(ptr + 12);
            uint16_t height = *reinterpret_cast<uint16_t *>(ptr + 14);
            uint8_t depth = *reinterpret_cast<uint8_t *>(ptr + 16); /* 24 or 32 */
            uint8_t flags = *reinterpret_cast<uint8_t *>(ptr + 17);
            if (width == 0 || height == 0 || (depth != 24 && depth != 32)) {
                qWarning("Invalid TGA image (width=%d, height=%d, depth=%d): %s",
                         width, height, depth, qPrintable(path));
                return QImage();
            }
            int component = depth >> 3;
            uint8_t *body = ptr + 18 + field;
            /* if RLE compressed, uncompress it */
            size_t datalen = width * height * component;
            ByteArrayPtr uncompressedPtr(new uint8_t[datalen]);
            if (type == 10) {
                uint8_t *uncompressed = uncompressedPtr.data();
                uint8_t *src = body;
                uint8_t *dst = uncompressed;
                while (static_cast<size_t>(dst - uncompressed) < datalen) {
                    int16_t len = (*src & 0x7f) + 1;
                    if (*src & 0x80) {
                        src++;
                        for (int i = 0; i < len; i++) {
                            memcpy(dst, src, component);
                            dst += component;
                        }
                        src += component;
                    }
                    else {
                        src++;
                        memcpy(dst, src, component * len);
                        dst += component * len;
                        src += component * len;
                    }
                }
                /* will load from uncompressed data */
                body = uncompressed;
            }
            /* prepare texture data area */
            datalen = (width * height) * 4;
            dataPtr.reset(new uint8_t[datalen]);
            ptr = dataPtr.data();
            for (uint16_t h = 0; h < height; h++) {
                uint8_t *line = NULL;
                if (flags & 0x20) /* from up to bottom */
                    line = body + h * width * component;
                else /* from bottom to up */
                    line = body + (height - 1 - h) * width * component;
                for (uint16_t w = 0; w < width; w++) {
                    uint32_t index = 0;
                    if (flags & 0x10)/* from right to left */
                        index = (width - 1 - w) * component;
                    else /* from left to right */
                        index = w * component;
                    /* BGR or BGRA -> ARGB */
                    *ptr++ = line[index + 2];
                    *ptr++ = line[index + 1];
                    *ptr++ = line[index + 0];
                    *ptr++ = (depth == 32) ? line[index + 3] : 255;
                }
            }
            return QImage(dataPtr.data(), width, height, QImage::Format_ARGB32);
        }
        else {
            qWarning("Cannot open file %s: %s", qPrintable(path), qPrintable(file.errorString()));
            return QImage();
        }
    }

    QGLWidget *m_widget;

    Q_DISABLE_COPY(Delegate)
};

}

#endif // DELEGATE_H
