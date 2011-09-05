#include "Delegate.h"

#include <QtOpenGL/QtOpenGL>
#include <QtGui/QtGui>
#include "util.h"

typedef QScopedPointer<uint8_t, QScopedPointerArrayDeleter<uint8_t> > ByteArrayPtr;

Delegate::Delegate(QGLWidget *widget)
    : m_widget(widget)
{
}

Delegate::~Delegate()
{
}

bool Delegate::loadTexture(const std::string &path, GLuint &textureID)
{
    QString pathString = QString::fromUtf8(path.c_str());
    if (pathString.endsWith(".tga", Qt::CaseInsensitive)) {
        uint8_t *rawData = 0;
        QImage image = loadTGA(pathString, rawData);
        textureID = m_widget->bindTexture(QGLWidget::convertToGLFormat(image));
        delete[] rawData;
    }
    else {
        QImage image(pathString);
        textureID = m_widget->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
    }
    qDebug("Loaded a texture (ID=%d): \"%s\"", textureID, pathString.toUtf8().constData());
    return textureID != 0;
}

bool Delegate::loadToonTexture(const std::string &name, const std::string &dir, GLuint &textureID)
{
    QString path = QString::fromUtf8(dir.c_str()) + "/" + QString::fromUtf8(name.c_str());
    if (!QFile::exists(path))
        path = QString(":/textures/%1").arg(name.c_str());
    return loadTexture(std::string(path.toUtf8()), textureID);
}

void Delegate::log(LogLevel level, const char *format...)
{
    QString message;
    va_list ap;
    va_start(ap, format);
    message.vsprintf(format, ap);
    switch (level) {
    case kLogInfo:
    default:
        qDebug("%s", qPrintable(message));
        break;
    case kLogWarning:
        qWarning("%s", qPrintable(message));
        break;
    }
    va_end(ap);
}

const std::string Delegate::toUnicode(const uint8_t *value)
{
    return std::string(internal::toQString(value).toUtf8());
}

QImage Delegate::loadTGA(const QString &path, uint8_t *&rawData) {
    QFile file(path);
    if (file.open(QFile::ReadOnly) && file.size() > 18) {
        QByteArray data = file.readAll();
        uint8_t *ptr = reinterpret_cast<uint8_t *>(data.data());
        uint8_t field = *reinterpret_cast<uint8_t *>(ptr);
        uint8_t type = *reinterpret_cast<uint8_t *>(ptr + 2);
        if (type != 2 /* full color */ && type != 10 /* full color + RLE */) {
            qWarning("Loaded TGA image type is not full color: %s", path.toUtf8().constData());
            return QImage();
        }
        uint16_t width = *reinterpret_cast<uint16_t *>(ptr + 12);
        uint16_t height = *reinterpret_cast<uint16_t *>(ptr + 14);
        uint8_t depth = *reinterpret_cast<uint8_t *>(ptr + 16); /* 24 or 32 */
        uint8_t flags = *reinterpret_cast<uint8_t *>(ptr + 17);
        if (width == 0 || height == 0 || (depth != 24 && depth != 32)) {
            qWarning("Invalid TGA image (width=%d, height=%d, depth=%d): %s",
                     width, height, depth, path.toUtf8().constData());
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
        datalen = (width * height) << 2;
        rawData = new uint8_t[datalen];
        ptr = rawData;
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
        return QImage(rawData, width, height, QImage::Format_ARGB32);
    }
    else {
        qWarning("Cannot open file %s: %s", path.toUtf8().constData(),
                 file.errorString().toUtf8().constData());
        return QImage();
    }
}
