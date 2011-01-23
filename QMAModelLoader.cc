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

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32)

// XXX: "#pragma pack" seems MSVC specific but gcc understand this and works correctly.
//      should use "#pragma pack"?
#pragma pack(push,1)

typedef struct tagRGBQUAD {
  uint8_t rgbBlue;
  uint8_t rgbGreen;
  uint8_t rgbRed;
  uint8_t rgbReserved;
} RGBQUAD;

typedef struct tagBITMAPFILEHEADER {
  uint16_t bfType;
  uint32_t bfSize;
  uint16_t bfReserved1;
  uint16_t bfReserved2;
  uint32_t bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
  uint32_t biSize;
  int32_t biWidth;
  int32_t biHeight;
  uint16_t biPlanes;
  uint16_t biBitCount;
  uint32_t biCompression;
  uint32_t biSizeImage;
  uint32_t biXPelsPerMeter;
  uint32_t biYPelsPerMeter;
  uint32_t biClrUsed;
  uint32_t biClrImportant;
} BITMAPINFOHEADER;

typedef struct tagBITMAPCOREHEADER{
  uint32_t bcSize;
  uint16_t bcWidth;
  uint16_t bcHeight;
  uint16_t bcPlanes;
  uint16_t bcBitCount;
} BITMAPCOREHEADER;

#pragma pack(pop)
#endif


static bool QMAModelLoaderLoadBMP(QString path, QSize &size, unsigned char **ptr)
{
  unsigned short bit;
  RGBQUAD *palette;
  unsigned char *head;
  unsigned char *body;
  bool reversed = false;
  BITMAPFILEHEADER *fh;
  unsigned long len;
  BITMAPINFOHEADER *ih;
  unsigned long lineByte;

  unsigned char *t;
  long h, w;

  unsigned char *tl;
  unsigned char ci;
  unsigned char mod;
  unsigned char bitmask;

  /* parse TGA */
  QFile file(path);
  if (!file.open(QFile::ReadOnly))
    return false;
  int s = file.size();
  char *data = static_cast<char *>(calloc(1, s));
  if (data == NULL)
    return false;
  if (file.read(data, s) != s) {
    free(data);
    return false;
  }
  file.close();

  int width = 0, height = 0;
  /* parse header */
  head = reinterpret_cast<unsigned char *>(data);
  if (head[0] != 'B' || head[1] != 'M') {
    free(data);
    return false;
  }

  fh = (BITMAPFILEHEADER *) head;
  body = reinterpret_cast<unsigned char*>(data + fh->bfOffBits);
  head += sizeof(BITMAPFILEHEADER);
  len = *((unsigned int *) head);
  if (len == sizeof(BITMAPCOREHEADER)) {
    free(data);
    return false;
  } else {
    ih = (BITMAPINFOHEADER *) head;
    width = ih->biWidth;
    if (ih->biHeight < 0) {
      height = -ih->biHeight;
      reversed = true;
    } else {
      height = ih->biHeight;
      reversed = false;
    }

    bit = ih->biBitCount;
    if (ih->biCompression != 0) {
      free(data);
      return false;
    }
    if (bit <= 8) {
      palette = (RGBQUAD *) (head + sizeof(BITMAPINFOHEADER));
    }
  }

  /* prepare texture data area */
  *ptr = t = (unsigned char *) malloc (width * height * 4);

  lineByte = (width * bit) / 8;
  if ((lineByte % 4) != 0)
    lineByte = ((lineByte / 4) + 1) * 4; /* force 4-byte alignment */

  /* read body into textureData */
  for (h = 0; h < height; h++) {
    if (reversed) {
      tl = body + h * lineByte;
    } else {
      tl = body + (height - h - 1) * lineByte;
    }
    for (w = 0; w < width; w++) {
      switch (bit) {
      case 1: {
          ci = tl[w / 8];
          mod = w % 8;
          bitmask = (mod == 0) ? 0x80 : (0x80 >> mod);
          ci = (ci & bitmask) ? 1 : 0;
          *t = palette[ci].rgbRed;
          t++;
          *t = palette[ci].rgbGreen;
          t++;
          *t = palette[ci].rgbBlue;
          t++;
          *t = 255;
          t++;
        }
        break;
      case 4: {
          ci = tl[w / 2];
          if (w % 2 == 0) ci = (ci >> 4) & 0x0f;
          else ci = ci & 0x0f;
          *t = palette[ci].rgbRed;
          t++;
          *t = palette[ci].rgbGreen;
          t++;
          *t = palette[ci].rgbBlue;
          t++;
          *t = 255;
          t++;
        }
        break;
      case 8: {
          ci = tl[w];
          *t = palette[ci].rgbRed;
          t++;
          *t = palette[ci].rgbGreen;
          t++;
          *t = palette[ci].rgbBlue;
          t++;
          *t = 255;
          t++;
        }
        break;
      case 24:
        /* BGR -> RGB */
        *t = tl[w*3+2];
        t++;
        *t = tl[w*3+1];
        t++;
        *t = tl[w*3 ];
        t++;
        *t = 255;
        t++;
        break;
      case 32:
        /* BGR0 -> RGB */
        *t = tl[w*4+2];
        t++;
        *t = tl[w*4+1];
        t++;
        *t = tl[w*4 ];
        t++;
        *t = 255;
        t++;
        break;
      }
    }
  }

  free(data);

  size.setWidth(width);
  size.setHeight(height);

  return true;

}

static bool QMAModelLoaderLoadTGA(QString path, QSize &size, unsigned char **ptr)
{
  /* parse TGA */
  QFile file(path);
  if (!file.open(QFile::ReadOnly))
    return false;
  int s = file.size();
  char *data = static_cast<char *>(calloc(1, s));
  if (data == NULL)
    return false;
  if (file.read(data, s) != s) {
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

static bool QMAModelLoaderLoadImage(QString &path, PMDTexture *texture)
{
  QImage image;
  if (QFile::exists(path)) {
    bool isSphereMap = false;
    bool isSphereMapAdd = false;
    isSphereMapAdd = path.endsWith(".spa");
    isSphereMap = isSphereMapAdd || path.endsWith(".sph");
    if (path.endsWith(".bmp") || isSphereMap || isSphereMapAdd) {
      QSize size;
      unsigned char *ptr = NULL;
      if (QMAModelLoaderLoadBMP(path, size, &ptr)) {
        int w = size.width();
        int h = size.height();
        int c = 4;
        int size = w * h * c;
        texture->loadBytes(ptr, size, w, h, c, isSphereMap, isSphereMapAdd);
        free(ptr);
        return true;
      }
      qWarning() << "Cannot load BMP image:" << path;

    }
    else if (path.endsWith(".tga")) {
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
      texture->loadBytes(image.constBits(), size, w, h, c, isSphereMap, isSphereMapAdd);
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

QMAModelLoader::QMAModelLoader(const char *filename)
{
  QDir dir(filename);
  QString path = dir.absolutePath();
  m_file = new QFile(path);
}

QMAModelLoader::~QMAModelLoader()
{
  if (m_file->isOpen()) {
    qWarning() << "Leaked:" << m_file->fileName();
    m_file->close();
  }
  delete m_file;
}

bool QMAModelLoader::loadModelData(unsigned char **ptr, size_t *size)
{
  *size = 0;
  if (m_file->exists() && m_file->open(QFile::ReadOnly)) {
    int s = m_file->size();
    char *p = static_cast<char *>(calloc(1, s));
    if (p != NULL && m_file->read(p, s) == s) {
      *ptr = reinterpret_cast<unsigned char *>(p);
      *size = s;
      return true;
    }
    if (p != NULL)
      free(p);
  }
  return false;
}

void QMAModelLoader::unloadModelData(unsigned char *ptr)
{
  m_file->close();
  free(ptr);
}

bool QMAModelLoader::loadImageTexture(PMDTexture *texture)
{
  QString path = m_file->fileName();
  return QMAModelLoaderLoadImage(path, texture);
}

bool QMAModelLoader::loadModelTexture(const char *name, PMDTexture *texture)
{
  QString path(name);
  QDir dir(m_file->fileName());
  dir.cdUp();
  path = dir.absoluteFilePath(name);
  return QMAModelLoaderLoadImage(path, texture);
}

bool QMAModelLoader::loadSystemTexture(int index, PMDTexture *texture)
{
  Q_UNUSED(index);
  Q_UNUSED(texture);
  return true;
}

const char *QMAModelLoader::getLocation()
{
  return m_file->fileName().toUtf8().constData();
}
