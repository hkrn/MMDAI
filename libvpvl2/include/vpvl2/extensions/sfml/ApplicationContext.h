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
#include <vpvl2/extensions/BaseApplicationContext.h>

/* SFML */
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#if !defined(_WIN32)
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
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
#ifdef _WIN32
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

    ApplicationContext(Scene *sceneRef, IEncoding *encodingRef, icu4c::StringMap *configRef)
        : BaseApplicationContext(sceneRef, encodingRef, configRef)
    {
    }
    ~ApplicationContext() {
    }

    void *findProcedureAddress(const void **candidatesPtr) const {
        const char **candidates = reinterpret_cast<const char **>(candidatesPtr);
        const char *candidate = candidates[0];
        int i = 0;
        while (candidate) {
            void *address = 0; //SFML_GL_GetProcAddress(candidate);
            if (address) {
                return address;
            }
            candidate = candidates[++i];
        }
        return 0;
    }
    bool mapFile(const UnicodeString &path, MapBuffer *buffer) const {
        return mapFileDescriptor(path, buffer->address, buffer->size, buffer->opaque);
    }
    bool unmapFile(MapBuffer *buffer) const {
        return unmapFileDescriptor(buffer->address, buffer->size, buffer->opaque);
    }
    bool existsFile(const UnicodeString &path) const {
#ifdef _WIN32
        FILE *fp = 0;
        bool exists = ::fopen_s(&fp, icu4c::String::toStdString(path).c_str(), "r") == 0;
        fclose(fp);
        return exists;
#else
        return ::access(icu4c::String::toStdString(path).c_str(), R_OK) == 0;
#endif
    }

#ifdef VPVL2_ENABLE_NVIDIA_CG
    void getToonColor(const IString * /* name */, Color &value, void * /* userData */) {
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
    }
#endif

private:
    sf::Clock m_elapsedTicks;
    mutable sf::Time m_lastTicks;
    sf::Time m_baseTicks;

    VPVL2_DISABLE_COPY_AND_ASSIGN(ApplicationContext)
};

} /* namespace sfml */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif /* VPVL2_EXTENSIONS_SFML_APPLICATIONCONTEXT_H_ */
