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
#ifndef VPVL2_EXTENSIONS_ALLEGRO_APPLICATIONCONTEXT_H_
#define VPVL2_EXTENSIONS_ALLEGRO_APPLICATIONCONTEXT_H_

/* libvpvl2 */
#include <vpvl2/extensions/BaseApplicationContext.h>

/* Allegro */
#include <allegro5/allegro.h>

namespace vpvl2
{
namespace extensions
{
namespace allegro
{

class ApplicationContext : public BaseApplicationContext {
public:
    static bool mapFileDescriptor(const UnicodeString &path, uint8 *&address, vsize &size, intptr_t &fd) {
        ALLEGRO_FILE *handle = al_fopen(icu4c::String::toStdString(path).c_str(), "rb");
        if (!handle) {
            return false;
        }
        al_fseek(handle, 0, ALLEGRO_SEEK_END);
        size = al_ftell(handle);
        al_fseek(handle, 0, ALLEGRO_SEEK_SET);
        address = new uint8_t[size + 1];
        if (al_fread(handle, address, size) != size) {
            delete[] address;
            address = 0;
            return false;
        }
        fd = reinterpret_cast<intptr_t>(handle);
        return true;
    }
    static bool unmapFileDescriptor(uint8 *address, vsize size, intptr_t fd) {
        if (address && size > 0) {
            delete[] address;
        }
        if (ALLEGRO_FILE *handle = reinterpret_cast<ALLEGRO_FILE *>(fd)) {
            al_fclose(handle);
        }
        return true;
    }

    ApplicationContext(Scene *sceneRef, IEncoding *encodingRef, icu4c::StringMap *configRef)
        : BaseApplicationContext(sceneRef, encodingRef, configRef),
          m_elapsedTicks(0),
          m_baseTicks(al_get_time())
    {
    }
    ~ApplicationContext() {
        m_elapsedTicks = 0;
        m_baseTicks = 0;
    }

    void *findProcedureAddress(const void **candidatesPtr) const {
        const char **candidates = reinterpret_cast<const char **>(candidatesPtr);
        const char *candidate = candidates[0];
        int i = 0;
        while (candidate) {
            void *address = 0; // SDL_GL_GetProcAddress(candidate);
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
        bool exists = false;
        if (ALLEGRO_FILE *handle = al_fopen(icu4c::String::toStdString(path).c_str(), "rb")) {
            exists = true;
            al_fclose(handle);
        }
        return exists;
    }

#ifdef VPVL2_ENABLE_NVIDIA_CG
    void getToonColor(const IString *name, Color &value, void *userData) {
        ModelContext *modelContext = static_cast<ModelContext *>(userData);
        const UnicodeString &path = createPath(modelContext->directoryRef(), name);
        /* TODO: implement this */
        (void) path;
        value.setValue(1, 1, 1, 1);
    }
    void getTime(float &value, bool sync) const {
        value = sync ? 0 : (al_get_time() - m_baseTicks) / 1000.0f;
    }
    void getElapsed(float &value, bool sync) const {
        uint32 currentTicks = al_get_time();
        value = sync ? 0 : (m_elapsedTicks > 0 ? currentTicks - m_elapsedTicks : 0);
        m_elapsedTicks = currentTicks;
    }
    void uploadAnimatedTexture(float /* offset */, float /* speed */, float /* seek */, void * /* texture */) {
        /* FIXME: implement this */
    }
#endif

private:
    mutable uint32 m_elapsedTicks;
    uint32 m_baseTicks;

    VPVL2_DISABLE_COPY_AND_ASSIGN(ApplicationContext)
};

} /* namespace allegro */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif /* VPVL2_EXTENSIONS_ALLEGRO_APPLICATIONCONTEXT_H_ */
