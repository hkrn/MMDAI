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
#ifndef VPVL2_EXTENSIONS_EGL_RENDERCONTEXT_H_
#define VPVL2_EXTENSIONS_EGL_RENDERCONTEXT_H_

#include <EGL/egl.h>

#include <stdio.h>

/* libvpvl2 */
#include <vpvl2/extensions/BaseApplicationContext.h>
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

class ApplicationContext : public BaseApplicationContext {
public:
    ApplicationContext(Scene *sceneRef, IEncoding *encodingRef, const icu4c::StringMap *configRef)
        : BaseApplicationContext(sceneRef, encodingRef, configRef)
    {
    }
    ~ApplicationContext()
    {
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
        const std::string &s = icu4c::String::toStdString(path);
        if (FILE *fp = fopen(s.c_str(), "rb")) {
            fseek(fp, 0, SEEK_END);
            vsize size = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            buffer->address = new uint8[size];
            buffer->size = size;
            buffer->opaque = reinterpret_cast<intptr_t>(fp);
            fread(buffer->address, size, 1, fp);
            return true;
        }
        return false;
    }
    bool unmapFile(MapBuffer *buffer) const {
        if (FILE *fp = reinterpret_cast<FILE *>(buffer->opaque)) {
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
        const std::string &s = icu4c::String::toStdString(path);
        if (FILE *fp = fopen(s.c_str(), "rb")) {
            fclose(fp);
            return true;
        }
        return false;
    }
    void getToonColor(const IString *name, Color &value, void *userData) {
        (void) name;
        (void) userData;
        // ModelContext *modelContext = static_cast<ModelContext *>(userData);
        value.setValue(1, 1, 1, 1);
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
                void *address = reinterpret_cast<void *>(eglGetProcAddress(name));
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
        bool coreProfile;
    };
    static inline FunctionResolver *staticSharedFunctionResolverInstance() {
        static Resolver resolver;
        return &resolver;
    }
    FunctionResolver *sharedFunctionResolverInstance() const {
        return staticSharedFunctionResolverInstance();
    }

private:
    VPVL2_DISABLE_COPY_AND_ASSIGN(ApplicationContext)

};

} /* namespace egl */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif /* VPVL2_EXTENSIONS_EGL_RENDERCONTEXT_H_ */
