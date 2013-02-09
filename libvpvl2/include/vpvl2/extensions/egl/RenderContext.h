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

#pragma once
#ifndef VPVL2_EXTENSIONS_EGL_RENDERCONTEXT_H_
#define VPVL2_EXTENSIONS_EGL_RENDERCONTEXT_H_

#include <EGL/egl.h>

/* embed stb_image.c as a header */
#include <vpvl2/extensions/egl/stb_image.c>
#include <stdio.h>

/* libvpvl2 */
#include <vpvl2/extensions/BaseRenderContext.h>
#include <vpvl2/extensions/icu4c/String.h>

#ifdef VPVL2_LINK_NVTT
#include <nvcore/Stream.h>
#include <nvcore/Timer.h>
#include <nvimage/DirectDrawSurface.h>
#include <nvimage/Image.h>
#include <nvimage/ImageIO.h>
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
namespace egl
{
using namespace icu4c;

class RenderContext : public BaseRenderContext {
public:
    RenderContext(Scene *sceneRef, StringMap *configRef)
        : BaseRenderContext(sceneRef, configRef)
    {
    }
    ~RenderContext()
    {
    }

    void *findProcedureAddress(const void ** /* candidatesPtr */) const {
        return 0;
    }
    void getToonColor(const IString *name, const IString *dir, Color &value, void *context) {
        (void) name;
        (void) dir;
        (void) context;
        value.setValue(1, 1, 1, 1);
    }
    void uploadAnimatedTexture(float offset, float speed, float seek, void *texture) {
        (void) offset;
        (void) speed;
        (void) seek;
        (void) texture;
    }
    void getTime(float &value, bool sync) const {
        (void) value;
        (void) sync;
    }
    void getElapsed(float &value, bool sync) const {
        (void) value;
        (void) sync;
    }
    bool mapFile(const UnicodeString &path, MapBuffer *buffer) const {
        const std::string &s = String::toStdString(path);
        if (FILE *fp = fopen(s.c_str(), "rb")) {
            fseek(fp, 0, SEEK_END);
            size_t size = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            buffer->address = new uint8_t[size];
            buffer->size = size;
            buffer->opaque = fp;
            fread(buffer->address, size, 1, fp);
            return true;
        }
        return false;
    }
    bool unmapFile(MapBuffer *buffer) const {
        if (FILE *fp = static_cast<FILE *>(buffer->opaque)) {
            delete[] buffer->address;
            buffer->address = 0;
            buffer->size = 0;
            buffer->opaque = 0;
            fclose(fp);
            return true;
        }
        return false;
    }
    bool existsFile(const UnicodeString &path) const {
        const std::string &s = String::toStdString(path);
        if (FILE *fp = fopen(s.c_str(), "rb")) {
            fclose(fp);
            return true;
        }
        return false;
    }

private:
    bool uploadTextureInternal(const UnicodeString &path, Texture &texture, void *context) {
        if (path[path.length() - 1] == '/') {
            return true;
        }
        ModelContext *modelContext = static_cast<ModelContext *>(context);
        if (modelContext && modelContext->findTextureCache(path, texture)) {
            return true;
        }
        size_t width = 0, height = 0;
        GLuint textureID = 0;
        int x = 0, y = 0, *comp = 0;
        if (path.endsWith(".dds")) {
#ifdef VPVL2_LINK_NVTT
            nv::DirectDrawSurface dds;
            std::string bytes;
            MapBuffer buffer(this);
            if (!mapFile(path, &buffer) || !dds.load(new ReadonlyMemoryStream(bytes)))
                return false;
            nv::Image nvimage;
            dds.mipmap(&nvimage, 0, 0);
            width = nvimage.width();
            height = nvimage.height();
            textureID = createTexture(nvimage.pixels(), glm::ivec3(width, height, 0),
                                      GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, texture.mipmap, texture.toon, false);
#else
            return false;
#endif
        }
        else if (stbi_uc *ptr = stbi_load(String::toStdString(path).c_str(), &x, &y, comp, 4)) {
            textureID = createTexture(ptr, glm::ivec3(width, height, 0),
                                      GL_RGBA, GL_UNSIGNED_BYTE, texture.mipmap, texture.toon, false);
            stbi_image_free(ptr);
        }
        bool ok = true;
        if (textureID) {
            TextureCache cache(texture);
            if (modelContext)
                modelContext->addTextureCache(path, cache);
            ok = texture.ok = textureID != 0;
        }
        return true;
    }

    VPVL2_DISABLE_COPY_AND_ASSIGN(RenderContext)

};

} /* namespace egl */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif /* VPVL2_EXTENSIONS_EGL_RENDERCONTEXT_H_ */
