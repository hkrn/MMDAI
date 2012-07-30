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

#ifndef VPVL2_QT_DDSTEXTURE_H_
#define VPVL2_QT_DDSTExTURE_H_

#include "vpvl2/Common.h"

#include <QtOpenGL/QtOpenGL>

namespace vpvl2
{
namespace qt
{

class DDSTexture
{
public:
    DDSTexture(QGLWidget *context);
    ~DDSTexture();

    bool parse(const uint8_t *data, size_t size, GLuint &textureID);
    size_t width() const { return m_header.description.width; }
    size_t height() const { return m_header.description.height; }

private:
    void setCubeTexture(const uint8_t *ptr, GLenum target, size_t &offset);
    void setVolumeTexture(const uint8_t *ptr, size_t size);
    void setTexture(const uint8_t *ptr, size_t size);

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
            ptr = const_cast<uint8_t *>(data) + sizeof(magic) + description.size;
        }
        bool validateTextureFormat(GLenum &internal, GLenum &format, GLenum &type, size_t &bitCount) const {
            const Header::Description::PixelFormat &pf = description.pixelFormat;
            bitCount = pf.bitsCount;
            switch (bitCount) {
            case 32:
                internal = GL_RGBA8;
                format   = GL_RGBA;
                type     = GL_UNSIGNED_BYTE;
                bitCount /= 8;
                return true;
            case 24:
                internal = GL_RGB8;
                format   = GL_RGB;
                type     = GL_UNSIGNED_BYTE;
                bitCount /= 8;
                return true;
            case 16:
                switch (pf.bitmask.green) {
                case 0x00f0:
                    internal = GL_RGBA16;
                    format   = GL_RGBA;
                    type     = GL_UNSIGNED_SHORT_4_4_4_4;
                    bitCount /= 8;
                    return true;
                case 0x07e0:
                    internal = GL_RGB16;
                    format   = GL_RGB;
                    type     = GL_UNSIGNED_SHORT_5_6_5;
                    bitCount /= 8;
                    return true;
                case 0x03e0:
                    internal = GL_RGBA16;
                    format   = GL_RGBA;
                    type     = GL_UNSIGNED_SHORT_5_5_5_1;
                    bitCount /= 8;
                    return true;
                case 0x0000:
                    internal = GL_LUMINANCE8_ALPHA8;
                    format   = GL_LUMINANCE_ALPHA;
                    type     = GL_UNSIGNED_BYTE;
                    bitCount /= 8;
                    return true;
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
                bitCount /= 8;
                return true;
            }
            internal = GL_NONE;
            format   = GL_NONE;
            type     = GL_UNSIGNED_BYTE;
            bitCount = 0;
            return false;
        }
    } m_header;
    QGLWidget *m_context;

    VPVL2_DISABLE_COPY_AND_ASSIGN(DDSTexture)
};

}
}

#endif
