/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2010  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn (libMMDAI)                         */
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

static bool QMAModelLoaderLoadTGA(QString path, QSize &size, unsigned char **ptr)
{
  /* parse TGA */
  QFile file(path);
  if (!file.open(QFile::ReadOnly | QFile::Unbuffered))
    return false;
  int s = file.size();
  char *data = static_cast<char *>(calloc(1, s));
  if (data == NULL) {
    file.close();
    return false;
  }
  if (file.read(data, s) != s) {
    file.close();
    free(data);
    return false;
  }
  file.close();

  /* support only Full-color images */
  unsigned char type = *((unsigned char *) (data + 2));
  if (type != 2 /* full color */ && type != 10 /* full color + RLE */) {
    free(data);
    return false;
  }
  short width = *((short *) (data + 12));
  short height = *((short *) (data + 14));
  unsigned char bit = *((unsigned char *) (data + 16)); /* 24 or 32 */
  unsigned char attrib = *((unsigned char *) (data + 17));
  int stride = bit / 8;
  unsigned char *body = reinterpret_cast<unsigned char *>(data) + 18;

  /* if RLE compressed, uncompress it */
  unsigned char *uncompressed = NULL;
  if (type == 10) {
    unsigned int datalen = width * height * stride;
    uncompressed = (unsigned char *)malloc(datalen);
    if (uncompressed == NULL) {
      free(data);
      return false;
    }
    unsigned char *src = body;
    unsigned char *dst = uncompressed;
    while ((unsigned long) dst - (unsigned long) uncompressed < datalen) {
      short len = (*src & 0x7f) + 1;
      if (*src & 0x80) {
        src++;
        for (short i = 0; i < len; i++) {
          memcpy(dst, src, stride);
          dst += stride;
        }
        src += stride;
      } else {
        src++;
        memcpy(dst, src, stride * len);
        dst += stride * len;
        src += stride * len;
      }
    }
    /* will load from uncompressed data */
    body = uncompressed;
  }

  /* prepare texture data area */
  unsigned char *textureData = (unsigned char *) malloc(width * height * 4);
  if (textureData == NULL) {
    free(data);
    if (uncompressed != NULL)
      free(uncompressed);
    return false;
  }
  unsigned char *ptmp = textureData;

  for (int h = 0; h < height; h++) {
    unsigned char *pLine = NULL;
    if (attrib & 0x20) { /* from up to bottom */
      pLine = body + h * width * stride;
    } else { /* from bottom to up */
      pLine = body + (height - 1 - h) * width * stride;
    }
    for (int w = 0; w < width; w++) {
      unsigned int idx = 0;
      if (attrib & 0x10) { /* from right to left */
        idx = (width - 1 - w) * stride;
      } else { /* from left to right */
        idx = w * stride;
      }
      /* BGR or BGRA -> RGBA */
      *(ptmp++) = pLine[idx + 2];
      *(ptmp++) = pLine[idx + 1];
      *(ptmp++) = pLine[idx ];
      *(ptmp++) = (bit == 32) ? pLine[idx+3] : 255;
    }
  }

  free(data);
  if (uncompressed)
    free(uncompressed);

  *ptr = textureData;
  size.setWidth(width);
  size.setHeight(height);

  return true;
}

static bool QMAModelLoaderLoadImage(QString &path, MMDAI::PMDTexture *texture)
{
  QImage image;
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
      qWarning() << "Cannot load TGA image:" << path;
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
      qWarning() << "Cannot load TGA image:" << path;
    }
  }
  else {
    qDebug() << "Image not found:" << path;
  }
  return false;
}

QMAModelLoader::QMAModelLoader(const QString &system, const char *filename)
  : m_dir(system)
{
  /* YEN SIGN (0x5c -> 0xa5) to SLASH */
  QString path = QFile::decodeName(filename).replace(QChar(0xa5), QChar('/'));
  if (QDir::isAbsolutePath(path))
    m_file = new QFile(path);
  else
    m_file = new QFile("mmdai:" + path);
  m_filename = strdup(m_file->fileName().toUtf8().constData());
}

QMAModelLoader::~QMAModelLoader()
{
  if (m_file->isOpen()) {
    qWarning() << "Leaked:" << m_file->fileName();
    m_file->close();
  }
  free(const_cast<char *>(m_filename));
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
