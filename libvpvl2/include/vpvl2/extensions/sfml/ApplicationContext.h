/**

 Copyright (c) 2010-2014  hkrn

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
#include <vpvl2/extensions/BaseApplicationContext.h>
#include <vpvl2/extensions/icu4c/String.h>
#include <unicode/regex.h>

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
using namespace icu4c;

namespace sfml
{

class ApplicationContext : public BaseApplicationContext {
public:
    static bool mapFileDescriptor(const std::string &path, uint8 *&address, vsize &size, intptr_t &fd) {
#ifdef VPVL2_OS_WINDOWS
        FILE *fp = 0;
        errno_t err = ::fopen_s(&fp, path.c_str(), "rb");
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
        fd = ::open(path.c_str(), O_RDONLY);
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

    ApplicationContext(Scene *sceneRef, IEncoding *encodingRef, StringMap *configRef)
        : BaseApplicationContext(sceneRef, encodingRef, configRef)
    {
    }
    ~ApplicationContext() {
    }

    bool mapFile(const std::string &path, MapBuffer *buffer) const {
        return mapFileDescriptor(path, buffer->address, buffer->size, buffer->opaque);
    }
    bool unmapFile(MapBuffer *buffer) const {
        return unmapFileDescriptor(buffer->address, buffer->size, buffer->opaque);
    }
    bool existsFile(const std::string &path) const {
#ifdef VPVL2_OS_WINDOWS
        FILE *fp = 0;
        bool exists = ::fopen_s(&fp, path.c_str(), "r") == 0;
        fclose(fp);
        return exists;
#else
        return ::access(path.c_str(), R_OK) == 0;
#endif
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
    bool uploadTextureOpaque(const uint8 *data, vsize size, const std::string &key, int flags, ModelContext *context, ITexture *&texturePtr) {
        sf::Image image;
        if (image.loadFromMemory(data, size)) {
            return uploadTextureSFML(image, key, flags, context, texturePtr);
        }
        return context->uploadTexture(data, size, key, flags, texturePtr);
    }
    bool uploadTextureOpaque(const std::string &path, int flags, ModelContext *context, ITexture *&texturePtr) {
        sf::Image image;
        if (image.loadFromFile(path)) {
            return uploadTextureSFML(image, path, flags, context, texturePtr);
        }
        return context->uploadTexture(path, flags, texturePtr);
    }
    bool uploadTextureSFML(const sf::Image &image, const std::string &key, int flags, ModelContext *context, ITexture *&texturePtr) {
        const Vector3 size(image.getSize().x, image.getSize().y, 1);
        texturePtr = context->createTexture(image.getPixelsPtr(), defaultTextureFormat(), size, (flags & IApplicationContext::kGenerateTextureMipmap) != 0);
        return context->storeTexture(key, flags, texturePtr);
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
        const std::string &path = static_cast<const String *>(modelContext->directoryRef())->toStdString()
                + "/" + static_cast<const String *>(name)->toStdString();
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
