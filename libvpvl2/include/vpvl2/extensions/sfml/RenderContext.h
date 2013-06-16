/**

 Copyright (c) 2010-2013  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#pragma once
#ifndef VPVL2_EXTENSIONS_SFML_RENDERCONTEXT_H_
#define VPVL2_EXTENSIONS_SFML_RENDERCONTEXT_H_

/* libvpvl2 */
#include <vpvl2/extensions/BaseRenderContext.h>

/* SFML */
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#ifdef VPVL2_LINK_NVTT
namespace {

class ReadonlyMemoryStream : public nv::Stream {
public:
    ReadonlyMemoryStream(const std::string &buffer)
        : m_buffer(buffer),
          m_size(buffer.size())
    {
    }
    ~ReadonlyMemoryStream() {
        m_size = 0;
    }

    bool isSaving() const { return false; }
    bool isError() const { return false; }
    void seek(uint pos) { m_buffer.seekg(pos, std::ios_base::beg); }
    uint tell() const { return uint(m_buffer.tellg()); }
    uint size() const { return m_size; }
    void clearError() {}
    bool isAtEnd() const { return tell() == m_size; }
    bool isSeekable() const { return true; }
    bool isLoading() const { return true; }
    uint serialize(void *data, uint len) {
        m_buffer.read(static_cast<char *>(data), len);
        return len;
    }

private:
    mutable std::istringstream m_buffer;
    size_t m_size;
};

}

#else

namespace nv {
class Stream {
public:
    Stream() {}
    virtual ~Stream() {}
};
}

class ReadonlyFileStream : public nv::Stream {
public:
    ReadonlyFileStream(const std::string & /* path */) {}
    ~ReadonlyFileStream() {}
};

class ReadonlyMemoryStream : public nv::Stream {
public:
    ReadonlyMemoryStream(std::string & /* bytes */) {}
    ~ReadonlyMemoryStream() {}
};

#endif /* VPVL2_LINK_NVTT */

namespace vpvl2
{
namespace extensions
{
namespace sfml
{
using namespace icu;

class RenderContext : public BaseRenderContext {
public:
    RenderContext(Scene *sceneRef, UIStringMap *configRef)
        : BaseRenderContext(sceneRef, configRef)
    {
    }
    ~RenderContext()
    {
    }

    void *findProcedureAddress(const void ** /* candidatesPtr */) const {
        return 0;
    }
    bool loadFile(const UnicodeString &path, std::string &bytes) {
        FILE *fp = ::fopen(String::toStdString(path).c_str(), "rb");
        bool ret = false;
        if (fp) {
            ::fseek(fp, 0, SEEK_END);
            size_t size = ::ftell(fp);
            ::fseek(fp, 0, SEEK_SET);
            std::vector<char> data(size);
            ::fread(&data[0], size, 1, fp);
            bytes.assign(data.begin(), data.end());
            ::fclose(fp);
            ret = true;
        }
        return ret;
    }

private:
    bool uploadTextureInternal(const UnicodeString &path, InternalTexture &texture, void *context) {
        if (path[path.length() - 1] == '/') {
            return true;
        }
        InternalContext *internalContext = static_cast<InternalContext *>(context);
        if (internalContext && internalContext->findTextureCache(path, texture)) {
            return true;
        }
        sf::Image image;
        size_t width = 0, height = 0;
        GLuint textureID = 0;
        if (path.endsWith(".dds")) {
#ifdef VPVL2_LINK_NVTT
            nv::DirectDrawSurface dds;
            std::string bytes;
            if (!loadFile(path, bytes) || !dds.load(new ReadonlyMemoryStream(bytes)))
                return false;
            nv::Image nvimage;
            dds.mipmap(&nvimage, 0, 0);
            width = nvimage.width();
            height = nvimage.height();
            textureID = createTexture(nvimage.pixels(), width, height,
                                      GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, texture.mipmap, false);
#else
            return false;
#endif
        }
        else if (image.loadFromFile(String::toStdString(path))) {
            const sf::Vector2u &size = image.getSize();
            width = size.x;
            height = size.y;
            textureID = createTexture(image.getPixelsPtr(), width, height,
                                      GL_RGBA, GL_UNSIGNED_BYTE, texture.mipmap, false);
        }
        bool ok = true;
        if (textureID) {
            TextureCache cache(width, height, textureID);
            texture.assign(cache);
            if (internalContext)
                internalContext->addTextureCache(path, cache);
            ok = texture.ok = textureID != 0;
        }
        return true;
    }

    VPVL2_DISABLE_COPY_AND_ASSIGN(RenderContext)
};

} /* namespace sdl */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif /* VPVL2_EXTENSIONS_SFML_RENDERCONTEXT_H_ */
