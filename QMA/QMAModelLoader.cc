/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn                                    */
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

#include "QMAModelLoader.h"

#include <QtCore/QTextCodec>
#include <QtGui/QImage>

static bool QMAModelLoaderLoadTGA(QString path, QSize &size, unsigned char **ptr)
{
    /* parse TGA */
    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Unbuffered))
        return false;
    int s = file.size();
    char *data = static_cast<char *>(MMDAIMemoryAllocate(s));
    if (data == NULL) {
        MMDAILogWarnString("Failed allocating memory");
        file.close();
        return false;
    }
    memset(data, 0, s);
    if (file.read(data, s) != s) {
        QByteArray reason = file.errorString().toUtf8();
        MMDAILogWarn("Failed reading a TGA file", reason.constData());
        MMDAIMemoryRelease(data);
        file.close();
        return false;
    }
    file.close();

    int width = 0, height = 0;
    bool ret = MMDAI::PMDTexture::loadTGAImage(reinterpret_cast<unsigned char *>(data), ptr, &width, &height);
    MMDAIMemoryRelease(data);

    size.setWidth(width);
    size.setHeight(height);

    return ret;
}

static bool QMAModelLoaderLoadImage(QString &path, MMDAI::PMDTexture *texture)
{
    QImage image;
    QByteArray filename = path.toUtf8();
    if (QFile::exists(path)) {
        bool isSphereMap = false;
        bool isSphereMapAdd = false;
        isSphereMapAdd = path.endsWith(".spa");
        isSphereMap = isSphereMapAdd || path.endsWith(".sph");
        if (path.endsWith(".tga")) {
            QSize size;
            unsigned char *ptr = NULL;
            if (QMAModelLoaderLoadTGA(path, size, &ptr)) {
                int w = size.width();
                int h = size.height();
                int c = 4;
                int size = w * h * c;
                texture->loadBytes(ptr, size, w, h, c, isSphereMap, isSphereMapAdd);
                free(ptr);
                return true;
            }
            MMDAILogWarn("Cannot load TGA image: %s", filename.constData());
        }
        else if (image.load(path)) {
            int w = image.width();
            int h = image.height();
            int c = image.depth() / 8;
            int size = w * h * c;
#if QT_VERSION >= QT_VERSION_CHECK(4, 7, 0)
            texture->loadBytes(image.rgbSwapped().constBits(), size, w, h, c, isSphereMap, isSphereMapAdd);
#else
            texture->loadBytes(image.rgbSwapped().bits(), size, w, h, c, isSphereMap, isSphereMapAdd);
#endif
            return true;
        }
        else {
            MMDAILogWarn("Cannot load TGA image: %s", filename.constData());
        }
    }
    else {
        MMDAILogDebug("Image not found: %s", filename.constData());
    }
    return false;
}

QMAModelLoader::QMAModelLoader(const QString &system, const char *filename)
    : m_dir(system)
{
    QString path = QFile::decodeName(filename);
    if (QDir::isAbsolutePath(path))
        m_file = new QFile(path);
    else
        m_file = new QFile("MMDAIUserData:/" + path);
    m_filename = MMDAIStringClone(m_file->fileName().toUtf8().constData());
}

QMAModelLoader::~QMAModelLoader()
{
    if (m_file->isOpen()) {
        MMDAILogWarn("File is still open, it just leaked: %s", m_filename);
        m_file->close();
    }
    MMDAIMemoryRelease(const_cast<char *>(m_filename));
    delete m_file;
}

bool QMAModelLoader::loadModelData(unsigned char **ptr, size_t *size)
{
    *size = 0;
    if (m_file->exists() && m_file->open(QFile::ReadOnly | QFile::Unbuffered)) {
        size_t s = m_file->size();
        unsigned char *p = m_file->map(0, s);
        if (p != NULL) {
            *ptr = p;
            *size = s;
            return true;
        }
        else {
            QByteArray reason = m_file->errorString().toUtf8();
            MMDAILogDebug("Failed mapping file: %s", reason.constData());
        }
    }
    else {
        MMDAILogDebug("Model is not found: %s", m_filename);
    }
    return false;
}

void QMAModelLoader::unloadModelData(unsigned char *ptr)
{
    m_file->unmap(ptr);
    m_file->close();
}

bool QMAModelLoader::loadMotionData(unsigned char **ptr, size_t *size)
{
    return loadModelData(ptr, size);
}

void QMAModelLoader::unloadMotionData(unsigned char *ptr)
{
    unloadModelData(ptr);
}

bool QMAModelLoader::loadImageTexture(MMDAI::PMDTexture *texture)
{
    QString path = m_file->fileName();
    return QMAModelLoaderLoadImage(path, texture);
}

bool QMAModelLoader::loadModelTexture(const char *name, MMDAI::PMDTexture *texture)
{
    QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
    QString textureName = codec->toUnicode(name, strlen(name));
    QDir dir(m_file->fileName());
    dir.cdUp();
    QString path = dir.absoluteFilePath(textureName);
    return QMAModelLoaderLoadImage(path, texture);
}

bool QMAModelLoader::loadSystemTexture(int index, MMDAI::PMDTexture *texture)
{
    int fill = index == 0 ? 1 : 2;
    QString path = m_dir.absoluteFilePath(QString("toon%1.bmp").arg(index, fill, 10, QChar('0')));
    return QMAModelLoaderLoadImage(path, texture);
}

const char *QMAModelLoader::getLocation() const
{
    return m_filename;
}
