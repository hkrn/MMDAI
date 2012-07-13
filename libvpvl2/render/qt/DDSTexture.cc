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
#include <QtCore/QtCore>

namespace vpvl2
{
namespace render
{
namespace qt
{

DDSTexture::DDSTexture(QGLWidget *context)
    : m_context(context)
{
    memset(&m_header, 0, sizeof(m_header));
}

DDSTexture::~DDSTexture()
{
}

bool DDSTexture::parse(const uint8_t *data, size_t size, GLuint &textureID)
{
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
        uint8_t *ptr;
        m_header.correct();
        m_header.getDataPointer(data, ptr);
        size -= ptr - data;
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
            setVolumeTexture(ptr, size);
            glBindTexture(GL_TEXTURE_3D, 0);
            return true;
        }
        // texture
        else {
            glBindTexture(GL_TEXTURE_2D, textureID);
            setTexture(ptr, size);
            glBindTexture(GL_TEXTURE_2D, 0);
            return true;
        }
    }
    return false;
}

void DDSTexture::setCubeTexture(const uint8_t *ptr, GLenum target, size_t &offset)
{
    uint32_t width = m_header.width(), height = m_header.height();
    GLenum format, internal, type;
    size_t bitCount;
    offset = 0;
    if (m_header.validateTextureFormat(internal, format, type, bitCount)) {
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
}

void DDSTexture::setVolumeTexture(const uint8_t *ptr, size_t size)
{
    uint32_t width = m_header.width(), height = m_header.height(), depth = m_header.depth();
    GLenum format, internal, type;
    size_t bitCount;
    if (m_header.validateTextureFormat(internal, format, type, bitCount)) {
        size_t actualSize = bitCount * width * height * depth;
        if (m_header.hasMipmap() && size > actualSize) {
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
        else if (actualSize == size) {
            glTexImage3D(GL_TEXTURE_3D, 0, internal, width, height, depth, 0, format, type, ptr);
        }
    }
}

void DDSTexture::setTexture(const uint8_t *ptr, size_t size)
{
    uint32_t width = m_header.width(), height = m_header.height();
    GLenum format, internal, type;
    size_t bitCount;
    if (m_header.validateTextureFormat(internal, format, type, bitCount)) {
        size_t actualSize = bitCount * width * height;
        if (m_header.hasMipmap() && size > actualSize) {
            uint32_t mipmapCount = m_header.width();
            size_t offset = 0;
            for (uint32_t i = 0; i < mipmapCount; i++) {
                glTexImage2D(GL_TEXTURE_2D, i, internal, width, height, 0, format, type, ptr);
                offset += m_header.estimateSize(i);
                width  >>= 1;
                height >>= 1;
                btSetMax(width, uint32_t(1));
                btSetMax(height, uint32_t(1));
            }
        }
        else if (actualSize == size) {
            glTexImage2D(GL_TEXTURE_2D, 0, internal, width, height, 0, format, type, ptr);
        }
    }
}

}
}
}
