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
#ifndef VPVL2_EXTENSIONS_GLFW_APPLICATIONCONTEXT_H_
#define VPVL2_EXTENSIONS_GLFW_APPLICATIONCONTEXT_H_

/* libvpvl2 */
#include <vpvl2/extensions/BaseApplicationContext.h>
#include <vpvl2/extensions/icu4c/String.h>

/* GLFW */
#include <GLFW/glfw3.h>

#if !defined(_WIN32)
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#endif

namespace vpvl2
{
namespace extensions
{
using namespace icu4c;

namespace glfw
{

class ApplicationContext : public BaseApplicationContext {
public:
    static bool mapFileDescriptor(const std::string &path, uint8 *&address, vsize &size, intptr_t &fd) {
#ifdef _WIN32
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
#ifdef _WIN32
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
        : BaseApplicationContext(sceneRef, encodingRef, configRef),
          m_elapsedTicks(0),
          m_baseTicks(glfwGetTime())
    {
    }
    ~ApplicationContext()
    {
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
#ifdef _WIN32
        FILE *fp = 0;
        bool exists = ::fopen_s(&fp, path.c_str(), "r") == 0;
        fclose(fp);
        return exists;
#else
        return ::access(path.c_str(), R_OK) == 0;
#endif
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
        bool hasExtension(const char *name) const {
            if (const bool *ptr = supportedTable.find(name)) {
                return *ptr;
            }
            else {
                static const std::string kPrefix("GL_");
                bool result = glfwExtensionSupported((kPrefix + name).c_str());
                supportedTable.insert(name, result);
                return result;
            }
        }
        void *resolveSymbol(const char *name) const {
            if (void *const *ptr = addressTable.find(name)) {
                return *ptr;
            }
            else {
                void *address = reinterpret_cast<void *>(glfwGetProcAddress(name));
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
                return true; //gl::isContextCoreProfile(this);
            }
            default:
                return 0;
            }
        }
        mutable Hash<HashString, bool> supportedTable;
        mutable Hash<HashString, void *> addressTable;
    };
    FunctionResolver *sharedFunctionResolverInstance() const {
        static Resolver resolver;
        return &resolver;
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
        value = sync ? 0 : float32(glfwGetTime() - m_baseTicks);
    }
    void getElapsed(float32 &value, bool sync) const {
        double currentTicks = glfwGetTime();
        value = sync ? 0 : (m_elapsedTicks > 0 ? currentTicks - m_elapsedTicks : 0);
        m_elapsedTicks = float32(currentTicks);
    }
    void uploadAnimatedTexture(float /* offset */, float /* speed */, float /* seek */, void * /* texture */) {
        /* FIXME: implement this */
    }
#endif

private:
    mutable double m_elapsedTicks;
    double m_baseTicks;

    VPVL2_DISABLE_COPY_AND_ASSIGN(ApplicationContext)
};

} /* namespace glfw */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif /* VPVL2_EXTENSIONS_GLFW_APPLICATIONCONTEXT_H_ */
