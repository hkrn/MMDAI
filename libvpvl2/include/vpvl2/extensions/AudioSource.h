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
#ifndef VPVL2_EXTENSIONS_AUDIOSOURCE_H_
#define VPVL2_EXTENSIONS_AUDIOSOURCE_H_

#include <vpvl2/Common.h>

#ifdef VPVL2_OS_OSX
#undef __APPLE__ /* workaround for including AL/al.h of OpenAL soft */
#include <OpenAL/alure.h>
#define __APPLE__
#else
#if defined(VPVL2_OS_WINDOWS)
#define ALURE_STATIC_LIBRARY
#endif
#include <AL/alure.h>
#endif

/* alway uses OpenAL soft */
#define AL_ALEXT_PROTOTYPES
#if defined(WIN32) || defined(_WIN32)
#include <alext.h>
#else
#include <AL/alext.h>
#endif
#undef AL_ALEXT_PROTOTYPES

namespace vpvl2
{
namespace extensions
{
class AudioSource VPVL2_DECL_FINAL {
public:
    static bool initialize() {
        return alureInitDevice(0, 0) == AL_TRUE;
    }
    static bool terminate() {
        return alureShutdownDevice() == AL_TRUE;
    }

    AudioSource()
        : m_source(0),
          m_buffer(0)
    {
    }
    ~AudioSource() {
        deleteSource();
        deleteBuffer();
    }

    bool load(const char *path) {
        deleteSource();
        alGenSources(1, &m_source);
        deleteBuffer();
        alGenBuffers(1, &m_buffer);
        if (alureBufferDataFromFile(path, m_buffer)) {
            alSourcei(m_source, AL_BUFFER, m_buffer);
            return true;
        }
        else {
            return false;
        }
    }
    bool play() {
        return alurePlaySource(m_source, 0, 0) == AL_TRUE;
    }
    bool stop() {
        return alureStopSource(m_source, AL_FALSE) == AL_TRUE;
    }
    bool isRunning() const {
        ALenum state;
        alGetSourcei(m_source, AL_SOURCE_STATE, &state);
        return state == AL_PLAYING;
    }
    void getOffsetLatency(double &offset, double &latency) const {
        offset = latency = 0;
        if (m_source) {
            double values[2] = { 0 };
            alGetSourcedvSOFT(m_source, AL_SEC_OFFSET_LATENCY_SOFT, values);
            offset = values[0] * 1000.0;
            latency = values[1] * 1000.0;
        }
    }
    void update() {
        if (m_buffer) {
            alureUpdate();
        }
    }
    bool isLoaded() const {
        return m_source != 0 && m_buffer != 0;
    }
    const char *errorString() const {
        return alureGetErrorString();
    }

private:
    void deleteSource() {
        if (m_source) {
            stop();
            alSourcei(m_source, AL_BUFFER, 0);
            alDeleteSources(1, &m_source);
            m_source = 0;
        }
    }
    void deleteBuffer() {
        if (m_buffer) {
            alDeleteBuffers(1, &m_buffer);
            m_buffer = 0;
        }
    }

    ALuint m_source;
    ALuint m_buffer;
};

} /* namespace extensions */
} /* namespace vpvl2 */

#endif
