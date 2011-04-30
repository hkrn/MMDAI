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

/* headers */

#include "MMDME/MMDME.h"

namespace MMDAI {

PMDTexture::PMDTexture()
    : m_engine(NULL),
    m_native(NULL),
    m_isSphereMap(false),
    m_isSphereMapAdd(false),
    m_width(0),
    m_height(0),
    m_components(3),
    m_textureData(NULL)
{
}

PMDTexture::~PMDTexture()
{
    release();
    m_engine = NULL;
}

void PMDTexture::release()
{
    if (m_engine) {
        m_engine->releaseTexture(m_native);
    }
    MMDAIMemoryRelease(m_textureData);

    m_engine = NULL;
    m_native = NULL;
    m_isSphereMap = false;
    m_isSphereMapAdd = false;
    m_width = 0;
    m_height = 0;
    m_components = 3;
    m_textureData = NULL;
}

bool PMDTexture::loadTGAImage(const unsigned char *data, unsigned char **ptr, int *pwidth, int *pheight)
{
    /* support only Full-color images */
    unsigned char type = *((unsigned char *) (data + 2));
    if (type != 2 /* full color */ && type != 10 /* full color + RLE */) {
        MMDAILogWarnString("Loaded TGA is not full color");
        return false;
    }
    unsigned short width = *pwidth = *((short *) (data + 12));
    unsigned short height = *pheight = *((short *) (data + 14));
    unsigned char bit = *((unsigned char *) (data + 16)); /* 24 or 32 */
    unsigned char attrib = *((unsigned char *) (data + 17));
    int stride = bit / 8;
    unsigned char *body = const_cast<unsigned char *>(data) + 18;

    /* if RLE compressed, uncompress it */
    unsigned char *uncompressed = NULL;
    if (type == 10) {
        size_t datalen = width * height * stride;
        uncompressed = static_cast<unsigned char *>(MMDAIMemoryAllocate(datalen));
        if (uncompressed == NULL) {
            MMDAILogErrorString("Failed allocating memory");
            return false;
        }
        unsigned char *src = body;
        unsigned char *dst = uncompressed;
        while (static_cast<size_t>(dst - uncompressed) < datalen) {
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
    *ptr = static_cast<unsigned char *>(MMDAIMemoryAllocate(width * height * 4));
    if (*ptr == NULL) {
        MMDAILogErrorString("Failed allocating memory");
        MMDAIMemoryRelease(uncompressed);
        return false;
    }
    unsigned char *ptmp = *ptr;

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
    MMDAIMemoryRelease(uncompressed);

    return true;
}

void PMDTexture::loadBytes(const unsigned char *data, size_t size, int width, int height, int components, bool isSphereMap, bool isSphereMapAdd)
{
    assert(m_engine != NULL);

    m_engine->releaseTexture(m_native);
    if (m_textureData)
        MMDAIMemoryRelease(m_textureData);

    m_width = width;
    m_height = height;
    m_components = components;
    m_textureData = static_cast<unsigned char *>(MMDAIMemoryAllocate(size));
    m_isSphereMap = isSphereMap;
    m_isSphereMapAdd = isSphereMapAdd;
    if (m_textureData == NULL)
        return;
    memcpy(m_textureData, data, size);

    if (m_isSphereMap || m_isSphereMapAdd) {
        /* swap vertically */
        for (int h = 0; h < m_height / 2; h++) {
            unsigned char *l1 = m_textureData + h * m_width * m_components;
            unsigned char *l2 = m_textureData + (m_height - 1 - h) * m_width * m_components;
            for (int w = 0 ; w < m_width * m_components; w++) {
                unsigned char tmp = l1[w];
                l1[w] = l2[w];
                l2[w] = tmp;
            }
        }
    }

    m_native = m_engine->allocateTexture(data, width, height, components);
}

} /* namespace */

