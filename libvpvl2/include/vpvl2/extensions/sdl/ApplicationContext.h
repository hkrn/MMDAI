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
#ifndef VPVL2_EXTENSIONS_SDL_APPLICATIONCONTEXT_H_
#define VPVL2_EXTENSIONS_SDL_APPLICATIONCONTEXT_H_

/* libvpvl2 */
#include <vpvl2/extensions/BaseApplicationContext.h>
#include <vpvl2/extensions/icu4c/String.h>
#include <unicode/regex.h>

/* SDL */
#include <SDL.h>
//#include <SDL_image.h>
#ifndef VPVL2_LINK_GLEW
#include <SDL_opengl.h>
#endif /* VPVL2_LINK_GLEW */

namespace vpvl2
{
namespace extensions
{
using namespace icu4c;

namespace sdl
{

class ApplicationContext : public BaseApplicationContext {
public:
    static bool mapFileDescriptor(const std::string &path, uint8 *&address, vsize &size, intptr_t &fd) {
        SDL_RWops *ops = SDL_RWFromFile(path.c_str(), "rb");
        if (!ops) {
            return false;
        }
        size = SDL_RWsize(ops);
        address = new uint8_t[size];
        if (SDL_RWread(ops, address, size, 1) == 0) {
            return false;
        }
        fd = reinterpret_cast<intptr_t>(ops);
        return true;
    }
    static bool unmapFileDescriptor(uint8 *address, vsize size, intptr_t fd) {
        if (address && size > 0) {
            delete[] address;
        }
        if (SDL_RWops *ops = reinterpret_cast<SDL_RWops *>(fd)) {
            SDL_RWclose(ops);
        }
        return true;
    }

    ApplicationContext(Scene *sceneRef, IEncoding *encodingRef, StringMap *configRef)
        : BaseApplicationContext(sceneRef, encodingRef, configRef),
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
    ~ApplicationContext() {
        SDL_FreeSurface(m_colorSwapSurface);
        m_colorSwapSurface = 0;
        m_elapsedTicks = 0;
        m_baseTicks = 0;
    }

    bool mapFile(const std::string &path, MapBuffer *buffer) const {
        return mapFileDescriptor(path, buffer->address, buffer->size, buffer->opaque);
    }
    bool unmapFile(MapBuffer *buffer) const {
        return unmapFileDescriptor(buffer->address, buffer->size, buffer->opaque);
    }
    bool existsFile(const std::string &path) const {
        bool exists = false;
        if (SDL_RWops *handle = SDL_RWFromFile(path.c_str(), "rb")) {
            exists = true;
            SDL_RWclose(handle);
        }
        return exists;
    }
    bool extractFilePath(const std::string &path, std::string &filename, std::string &basename) const {
        UErrorCode status = U_ZERO_ERROR;
        RegexMatcher filenameMatcher(".+/((.+)\\.\\w+)$", 0, status);
        filenameMatcher.reset(UnicodeString::fromUTF8(path));
        if (filenameMatcher.find()) {
            basename = String::toStdString(filenameMatcher.group(1, status));
            filename = String::toStdString(filenameMatcher.group(2, status));
            return true;
        }
        return false;
    }
    bool extractModelNameFromFileName(const std::string &path, std::string &modelName) const {
        UErrorCode status = U_ZERO_ERROR;
        RegexMatcher extractMatcher("^.+\\[(.+)(?:\\.(?:cg)?fx)?\\]$", 0, status);
        extractMatcher.reset(UnicodeString::fromUTF8(path));
        if (extractMatcher.find()) {
            status = U_ZERO_ERROR;
            modelName = String::toStdString(extractMatcher.group(1, status));
            return true;
        }
        return false;
    }
    bool uploadTextureOpaque(const uint8 *data, vsize size, const std::string &key, ModelContext *context, TextureDataBridge &bridge) {
        if (context->uploadTexture(data, size, key, bridge)) {
            // context->optimizeTexture(bridge.dataRef);
            return true;
        }
        return false;
    }
    bool uploadTextureOpaque(const std::string &key, ModelContext *context, TextureDataBridge &bridge) {
        if (context->uploadTexture(key, bridge)) {
            // context->optimizeTexture(bridge.dataRef);
            return true;
        }
        return false;
    }

    struct Resolver : FunctionResolver {
        Resolver()
            : getStringi(reinterpret_cast<PFNGLGETSTRINGIPROC>(resolveSymbol("glGetStringi"))),
              coreProfile(false)
        {
            GLint flags;
            glGetIntegerv(gl::kGL_CONTEXT_FLAGS, &flags);
            coreProfile = (flags & gl::kGL_CONTEXT_CORE_PROFILE_BIT) != 0;
        }
        ~Resolver() {}

        bool hasExtension(const char *name) const {
            if (const bool *ptr = supportedTable.find(name)) {
                return *ptr;
            }
            else if (coreProfile) {
                GLint nextensions;
                glGetIntegerv(kGL_NUM_EXTENSIONS, &nextensions);
                const std::string &needle = std::string("GL_") + name;
                for (int i = 0; i < nextensions; i++) {
                    if (needle == reinterpret_cast<const char *>(getStringi(GL_EXTENSIONS, i))) {
                        supportedTable.insert(name, true);
                        return true;
                    }
                }
                supportedTable.insert(name, false);
                return false;
            }
            else if (const char *extensions = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS))) {
                bool found = strstr(extensions, name) != NULL;
                supportedTable.insert(name, found);
                return found;
            }
            else {
                supportedTable.insert(name, false);
                return false;
            }
        }
        void *resolveSymbol(const char *name) const {
            if (void *const *ptr = addressTable.find(name)) {
                return *ptr;
            }
            else {
                void *address = reinterpret_cast<void *>(SDL_GL_GetProcAddress(name));
                addressTable.insert(name, address);
                return address;
            }
        }
        int query(QueryType type) const {
            switch (type) {
            case kQueryVersion: {
                return gl::makeVersion(reinterpret_cast<const char *>(glGetString(GL_VERSION)));
            }
            case kQueryShaderVersion: {
                return gl::makeVersion(reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION)));
            }
            case kQueryCoreProfile: {
                return coreProfile;
            }
            default:
                return 0;
            }
        }

        static const GLenum kGL_NUM_EXTENSIONS = 0x821D;
        typedef const GLubyte * (GLAPIENTRY * PFNGLGETSTRINGIPROC) (gl::GLenum pname, gl::GLuint index);
        PFNGLGETSTRINGIPROC getStringi;
        mutable Hash<HashString, bool> supportedTable;
        mutable Hash<HashString, void *> addressTable;
        bool coreProfile;
    };
    static inline FunctionResolver *staticSharedFunctionResolverInstance() {
        static Resolver resolver;
        return &resolver;
    }
    FunctionResolver *sharedFunctionResolverInstance() const {
        return staticSharedFunctionResolverInstance();
    }

#if defined(VPVL2_ENABLE_NVIDIA_CG) || defined(VPVL2_LINK_NVFX)
    void getToonColor(const IString *name, Color &value, void *userData) {
        ModelContext *modelContext = static_cast<ModelContext *>(userData);
        const std::string &path = static_cast<const String *>(modelContext->directoryRef())->toStdString()
                + "/" + static_cast<const String *>(name)->toStdString();
        SDL_Surface *surface = createSurface(path);
        if (!surface) {
            value.setValue(1, 1, 1, 1);
        }
        else {
            SDL_LockSurface(surface);
            uint8 *pixels = static_cast<uint8 *>(surface->pixels) + (surface->h - 1) * surface->pitch;
            uint8 r = 0, g = 0, b = 0, a = 0;
            SDL_GetRGBA(*reinterpret_cast<uint32 *>(pixels), surface->format, &r, &g, &b, &a);
            SDL_UnlockSurface(surface);
            static const float den = 255.0;
            value.setValue(r / den, g / den, b / den, a / den);
        }
    }
    void getTime(float &value, bool sync) const {
        value = sync ? 0 : (SDL_GetTicks() - m_baseTicks) / 1000.0f;
    }
    void getElapsed(float &value, bool sync) const {
        uint32 currentTicks = SDL_GetTicks();
        value = sync ? 0 : (m_elapsedTicks > 0 ? currentTicks - m_elapsedTicks : 0);
        m_elapsedTicks = currentTicks;
    }
    void uploadAnimatedTexture(float /* offset */, float /* speed */, float /* seek */, void * /* texture */) {
        /* FIXME: implement this */
    }
#endif

private:
    SDL_Surface *createSurface(const std::string &path) const {
        MapBuffer buffer(this);
        if (!mapFile(path, &buffer)) {
            return 0;
        }
        SDL_Surface *surface = 0;
        SDL_RWops *source = SDL_RWFromConstMem(buffer.address, buffer.size);
        const UnicodeString &lowerPath = UnicodeString::fromUTF8(path).tempSubString().toLower();
#if 0
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
#else
        (void) lowerPath;
#endif
        SDL_FreeRW(source);
        return surface;
    }

    SDL_Surface *m_colorSwapSurface;
    mutable uint32 m_elapsedTicks;
    uint32 m_baseTicks;

    VPVL2_DISABLE_COPY_AND_ASSIGN(ApplicationContext)
};

} /* namespace sdl */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif /* VPVL2_EXTENSIONS_SDL_APPLICATIONCONTEXT_H_ */
