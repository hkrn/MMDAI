/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
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
#ifndef VPVL2_EXTENSIONS_SDL_RENDERCONTEXT_H_
#define VPVL2_EXTENSIONS_SDL_RENDERCONTEXT_H_

/* libvpvl2 */
#define VPVL2_INCLUDE_EXTENSIONS_BASERENDERCONTEXT_SOURCE
#include <vpvl2/extensions/BaseRenderContext.h>
#undef VPVL2_INCLUDE_EXTENSIONS_BASERENDERCONTEXT_SOURCE

/* SDL */
#include <SDL.h>
#include <SDL_image.h>
#ifndef VPVL2_LINK_GLEW
#include <SDL_opengl.h>
#endif /* VPVL2_LINK_GLEW */

/* XXX: currently support based on OSX/Linux for mmap */
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#ifdef VPVL2_LINK_NVTT

#include <nvcore/FileSystem.h>
#include <nvcore/Stream.h>
#include <nvimage/DirectDrawSurface.h>
#include <nvimage/Image.h>

namespace {

class ReadonlyMemoryStream : public nv::Stream {
public:
    ReadonlyMemoryStream(SDL_RWops *bufferRef)
        : m_bufferRef(bufferRef),
          m_size(0)
    {
        size_t pos = tell();
        SDL_RWseek(m_bufferRef, 0, RW_SEEK_END);
        m_size = tell();
        seek(pos);
    }
    ~ReadonlyMemoryStream() {
        m_bufferRef = 0;
        m_size = 0;
    }

    bool isSaving() const { return false; }
    bool isError() const { return false; }
    void seek(uint pos) { SDL_RWseek(m_bufferRef, pos, RW_SEEK_SET); }
    uint tell() const { return SDL_RWtell(m_bufferRef); }
    uint size() const { return m_size; }
    void clearError() {}
    bool isAtEnd() const { return tell() == m_size; }
    bool isSeekable() const { return true; }
    bool isLoading() const { return true; }
    uint serialize(void *data, uint len) { return SDL_RWread(m_bufferRef, data, len, 1); }

private:
    SDL_RWops *m_bufferRef;
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
namespace sdl
{
using namespace icu;

class RenderContext : public BaseRenderContext {
public:
    RenderContext(Scene *sceneRef, StringMap *configRef)
        : BaseRenderContext(sceneRef, configRef),
          m_colorSwapSurface(0),
          m_elapsedTicks(0),
          m_baseTicks(SDL_GetTicks())
    {
        m_colorSwapSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, 0, 0, 32,
                                                  0x00ff0000,
                                                  0x0000ff00,
                                                  0x000000ff,
                                                  0xff000000);
    }
    ~RenderContext()
    {
        SDL_FreeSurface(m_colorSwapSurface);
        m_colorSwapSurface = 0;
        m_elapsedTicks = 0;
        m_baseTicks = 0;
    }

    void *findProcedureAddress(const void **candidatesPtr) const {
        const char **candidates = reinterpret_cast<const char **>(candidatesPtr);
        const char *candidate = candidates[0];
        int i = 0;
        while (candidate) {
            void *address = SDL_GL_GetProcAddress(candidate);
            if (address) {
                return address;
            }
            candidate = candidates[++i];
        }
        return 0;
    }
    bool mapFile(const UnicodeString &path, MapBuffer *buffer) const {
        int fd = ::open(String::toStdString(path).c_str(), O_RDONLY);
        if (fd == -1) {
            return false;
        }
        struct stat sb;
        if (::fstat(fd, &sb) == -1) {
            return false;
        }
        uint8_t *address = static_cast<uint8_t *>(::mmap(0, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0));
        if (address == reinterpret_cast<uint8_t *>(-1)) {
            return false;
        }
        buffer->address = address;
        buffer->size = sb.st_size;
        return true;
    }
    bool unmapFile(MapBuffer *buffer) const {
        if (uint8_t *address = buffer->address) {
            ::munmap(address, buffer->size);
            return true;
        }
        return false;
    }
    bool existsFile(const UnicodeString &path) const {
#ifdef VPVL2_LINK_NVTT
        return nv::FileSystem::exists(String::toStdString(path).c_str());
#else
        bool exists = false;
        if (SDL_RWops *handle = SDL_RWFromFile(String::toStdString(path).c_str(), "rb")) {
            exists = true;
            SDL_RWclose(handle);
        }
        return exists;
#endif
    }

#ifdef VPVL2_ENABLE_NVIDIA_CG
    void getToonColor(const IString *name, const IString *dir, Color &value, void * /* context */) {
        const UnicodeString &path = createPath(dir, name);
        SDL_Surface *surface = createSurface(path);
        if (!surface) {
            value.setValue(1, 1, 1, 1);
        }
        else {
            SDL_LockSurface(surface);
            uint8_t *pixels = static_cast<uint8_t *>(surface->pixels) + (surface->h - 1) * surface->pitch;
            uint8_t r = 0, g = 0, b = 0, a = 0;
            SDL_GetRGBA(*reinterpret_cast<uint32_t *>(pixels), surface->format, &r, &g, &b, &a);
            SDL_UnlockSurface(surface);
            static const float den = 255.0;
            value.setValue(r / den, g / den, b / den, a / den);
        }
    }
    void getTime(float &value, bool sync) const {
        value = sync ? 0 : (SDL_GetTicks() - m_baseTicks) / 1000.0f;
    }
    void getElapsed(float &value, bool sync) const {
        uint32_t currentTicks = SDL_GetTicks();
        value = sync ? 0 : (m_elapsedTicks > 0 ? currentTicks - m_elapsedTicks : 0);
        m_elapsedTicks = currentTicks;
    }
    void uploadAnimatedTexture(float /* offset */, float /* speed */, float /* seek */, void * /* texture */) {
    }
#endif

private:
    bool uploadTextureInternal(const UnicodeString &path, Texture &texture, void *context) {
        ModelContext *modelcontext = static_cast<ModelContext *>(context);
        /* テクスチャのキャッシュを検索する */
        if (modelcontext && modelcontext->findTextureCache(path, texture)) {
            return true;
        }
        SDL_Surface *surface = createSurface(path);
        if (!surface) {
            texture.ok = false;
            return true;
        }
        size_t width = surface->w, height = surface->h;
        SDL_LockSurface(surface);
        GLuint textureID = createTexture(surface->pixels, glm::ivec3(width, height, 0), GL_BGRA,
                                         GL_UNSIGNED_INT_8_8_8_8_REV, texture.mipmap, texture.toon, true);
        SDL_UnlockSurface(surface);
        SDL_FreeSurface(surface);
        texture.size.setValue(width, height, 0);
        texture.opaque = textureID;
        if (modelcontext) {
            TextureCache cache(texture);
            modelcontext->addTextureCache(path, cache);
        }
        bool ok = texture.ok = textureID != 0;
        return ok;
    }
    SDL_Surface *createSurface(const UnicodeString &path) const {
        MapBuffer buffer(this);
        if (!mapFile(path, &buffer)) {
            return 0;
        }
        SDL_Surface *surface = 0;
        SDL_RWops *source = SDL_RWFromConstMem(buffer.address, buffer.size);
        const UnicodeString &lowerPath = path.tempSubString().toLower();
        char extension[4] = { 0 };
        if (lowerPath.endsWith(".sph") || lowerPath.endsWith(".spa")) {
            memcpy(extension, "BMP" ,sizeof(extension));
        }
        else if (lowerPath.endsWith(".tga")) {
            memcpy(extension, "TGA" ,sizeof(extension));
        }
        else if (lowerPath.endsWith(".png") && IMG_isPNG(source)) {
            memcpy(extension, "PNG" ,sizeof(extension));
        }
        else if (lowerPath.endsWith(".bmp") && IMG_isBMP(source)) {
            memcpy(extension, "BMP" ,sizeof(extension));
        }
        else if (lowerPath.endsWith(".jpg") && IMG_isJPG(source)) {
            memcpy(extension, "JPG" ,sizeof(extension));
        }
        if (*extension) {
            surface = IMG_LoadTyped_RW(source, 0, extension);
            surface = SDL_ConvertSurface(surface, m_colorSwapSurface->format, SDL_SWSURFACE);
        }
#ifdef VPVL2_LINK_NVTT
        else if (lowerPath.endsWith(".dds")) {
            nv::DirectDrawSurface dds;
            if (dds.load(new ReadonlyMemoryStream(source))) {
                nv::Image image;
                dds.mipmap(&image, 0, 0);
                surface = SDL_CreateRGBSurfaceFrom(image.pixels(),
                                                   image.width(),
                                                   image.height(),
                                                   32,
                                                   image.width() * 4,
                                                   0x00ff0000,
                                                   0x0000ff00,
                                                   0x000000ff,
                                                   0xff000000);
                surface = SDL_ConvertSurface(surface, m_colorSwapSurface->format, SDL_SWSURFACE);
            }
        }
#endif
        SDL_FreeRW(source);
        return surface;
    }

    SDL_Surface *m_colorSwapSurface;
    mutable uint32_t m_elapsedTicks;
    uint32_t m_baseTicks;

    VPVL2_DISABLE_COPY_AND_ASSIGN(RenderContext)
};

} /* namespace sdl */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif /* VPVL2_EXTENSIONS_SDL_RENDERCONTEXT_H_ */
