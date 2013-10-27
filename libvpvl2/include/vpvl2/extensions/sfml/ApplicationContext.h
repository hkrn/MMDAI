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
#ifndef VPVL2_EXTENSIONS_SFML_APPLICATIONCONTEXT_H_
#define VPVL2_EXTENSIONS_SFML_APPLICATIONCONTEXT_H_

/* libvpvl2 */
#include <vpvl2/config.h>
#include <vpvl2/extensions/BaseApplicationContext.h>

/* SFML */
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#if !defined(VPVL2_OS_WINDOWS)
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#ifdef VPVL2_HAS_OPENGL_GLX
#include <GL/glx.h>
#endif
#endif

#ifdef VPVL2_OS_OSX
#include <dlfcn.h>
#endif

namespace vpvl2
{
namespace extensions
{
namespace sfml
{

class ApplicationContext : public BaseApplicationContext {
public:
    static bool mapFileDescriptor(const UnicodeString &path, uint8 *&address, vsize &size, intptr_t &fd) {
#ifdef VPVL2_OS_WINDOWS
        FILE *fp = 0;
        errno_t err = ::fopen_s(&fp, icu4c::String::toStdString(path).c_str(), "rb");
        if (err != 0) {
            return false;
        }
        fd = reinterpret_cast<intptr_t>(fp);
        err = ::fseek(fp, 0, SEEK_END);
        if (err != 0) {
            return false;
        }
        size = ::ftell(fp);
        err = ::fseek(fp, 0, SEEK_SET);
        if (err != 0) {
            return false;
        }
        address = new uint8_t[size];
        vsize readSize = ::fread_s(address, size, sizeof(uint8_t), size, fp);
        if (readSize != size) {
            delete[] address;
            address = 0;
            return false;
        }
#else
        fd = ::open(icu4c::String::toStdString(path).c_str(), O_RDONLY);
        if (fd == -1) {
            return false;
        }
        struct stat sb;
        if (::fstat(fd, &sb) == -1) {
            return false;
        }
        size = sb.st_size;
        address = static_cast<uint8 *>(::mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0));
        if (address == reinterpret_cast<uint8 *>(-1)) {
            return false;
        }
#endif
        return true;
    }
    static bool unmapFileDescriptor(uint8 *address, vsize size, intptr_t fd) {
#ifdef VPVL2_OS_WINDOWS
        if (address && size > 0) {
            delete[] address;
        }
        if (FILE *fp = reinterpret_cast<FILE *>(fd)) {
            fclose(fp);
        }
#else
        if (address && size > 0) {
            ::munmap(address, size);
        }
        if (fd >= 0) {
            ::close(fd);
        }
#endif
        return true;
    }

    ApplicationContext(Scene *sceneRef, IEncoding *encodingRef, icu4c::StringMap *configRef)
        : BaseApplicationContext(sceneRef, encodingRef, configRef)
    {
    }
    ~ApplicationContext() {
    }

    bool mapFile(const UnicodeString &path, MapBuffer *buffer) const {
        return mapFileDescriptor(path, buffer->address, buffer->size, buffer->opaque);
    }
    bool unmapFile(MapBuffer *buffer) const {
        return unmapFileDescriptor(buffer->address, buffer->size, buffer->opaque);
    }
    bool existsFile(const UnicodeString &path) const {
#ifdef VPVL2_OS_WINDOWS
        FILE *fp = 0;
        bool exists = ::fopen_s(&fp, icu4c::String::toStdString(path).c_str(), "r") == 0;
        fclose(fp);
        return exists;
#else
        return ::access(icu4c::String::toStdString(path).c_str(), R_OK) == 0;
#endif
    }
    bool uploadTextureOpaque(const uint8 *data, vsize size, const UnicodeString &key, ModelContext *context, TextureDataBridge &bridge) {
        sf::Image image;
        if (image.loadFromMemory(data, size)) {
            return uploadTextureSFML(image, key, context, bridge);
        }
        return context->uploadTexture(data, size, key, bridge);
    }
    bool uploadTextureOpaque(const UnicodeString &path, ModelContext *context, TextureDataBridge &bridge) {
        sf::Image image;
        if (image.loadFromFile(icu4c::String::toStdString(path))) {
            return uploadTextureSFML(image, path, context, bridge);
        }
        return context->uploadTexture(path, bridge);
    }
    bool uploadTextureSFML(const sf::Image &image, const UnicodeString &key, ModelContext *context, TextureDataBridge &bridge) {
        static const gl::BaseSurface::Format format(GL_RGBA, GL_RGBA8, GL_UNSIGNED_INT_8_8_8_8_REV, GL_TEXTURE_2D);
        const Vector3 size(image.getSize().x, image.getSize().y, 1);
        ITexture *texturePtr = context->createTexture(image.getPixelsPtr(), format, size, (bridge.flags & kGenerateTextureMipmap) != 0);
        return context->cacheTexture(key, texturePtr, bridge);
    }

    struct Resolver : FunctionResolver {
        Resolver()
            : getStringi(0),
              imageHandle(0),
              coreProfile(false)
        {
#ifdef VPVL2_OS_OSX
            imageHandle = dlopen("/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL", RTLD_LAZY);
            VPVL2_CHECK(imageHandle);
#endif
            GLint flags;
            glGetIntegerv(gl::kGL_CONTEXT_FLAGS, &flags);
            coreProfile = (flags & gl::kGL_CONTEXT_CORE_PROFILE_BIT) != 0;
            getStringi = reinterpret_cast<PFNGLGETSTRINGIPROC>(resolveSymbol("glGetStringi"));
        }
        ~Resolver() {
#ifdef VPVL2_OS_OSX
            dlclose(imageHandle);
#endif
        }

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
            else {
                const char *extensions = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));
                bool found = strstr(extensions, name) != NULL;
                supportedTable.insert(name, found);
                return found;
            }
        }
        void *resolveSymbol(const char *name) const {
            if (void *const *ptr = addressTable.find(name)) {
                return *ptr;
            }
            else {
                void *address = 0;
#if defined(VPVL2_OS_WINDOWS)
                address = wglGetProcAddress(name);
#elif defined(VPVL2_OS_OSX)
                address = dlsym(imageHandle, name);
#elif defined(VPVL2_HAS_OPENGL_GLX)
                address = glXGetProcAddress(name);
#endif
                addressTable.insert(name, address);
                return address;
            }
        }
        int query(QueryType type) const {
            switch (type) {
            case kQueryVersion: {
                if (const GLubyte *s = glGetString(GL_VERSION)) {
                    int major = s[0] - '0', minor = minor = s[2] - '0';
                    return gl::makeVersion(major, minor);
                }
                return 0;
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
        void *imageHandle;
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
        const UnicodeString &path = createPath(modelContext->directoryRef(), name);
        /* TODO: implement this */
        (void) path;
        value.setValue(1, 1, 1, 1);
    }
    void getTime(float32 &value, bool sync) const {
        value = sync ? 0 : (m_elapsedTicks.getElapsedTime() - m_baseTicks).asMilliseconds() / 1000.0f;
    }
    void getElapsed(float32 &value, bool sync) const {
        sf::Time currentTicks = m_elapsedTicks.getElapsedTime();
        value = sync ? 0 : ((m_lastTicks > sf::seconds(0) ? currentTicks - m_lastTicks : sf::seconds(0)).asMilliseconds() / 1000.0f);
        m_lastTicks = currentTicks;
    }
    void uploadAnimatedTexture(float /* offset */, float /* speed */, float /* seek */, void * /* texture */) {
        /* FIXME: implement this */
    }
#endif

private:
    mutable sf::Time m_lastTicks;
    sf::Clock m_elapsedTicks;
    sf::Time m_baseTicks;

    VPVL2_DISABLE_COPY_AND_ASSIGN(ApplicationContext)
};

} /* namespace sfml */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif /* VPVL2_EXTENSIONS_SFML_APPLICATIONCONTEXT_H_ */
