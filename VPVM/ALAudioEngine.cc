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

#include "ALAudioEngine.h"
#include "ALAudioContext.h"

#include <QtCore>
#include <vpvl2/vpvl2.h>

#if defined(Q_OS_WIN32)
#define ALURE_STATIC_LIBRARY
#endif
#include <AL/alure.h>

/* alway uses OpenAL soft */
#define AL_ALEXT_PROTOTYPES
#include <AL/alext.h>
#undef AL_ALEXT_PROTOTYPES

using namespace vpvl2;

ALAudioEngine::ALAudioEngine(QObject *parent)
    : QObject(parent),
      m_audioSource(0),
      m_audioBuffer(0),
      m_timerID(0)
{
}

ALAudioEngine::~ALAudioEngine()
{
    release();
}

void ALAudioEngine::play()
{
    if (!m_source.isEmpty()) {
        if (!m_timerID) {
            alurePlaySource(m_audioSource, &ALAudioEngine::stopCallback, this);
            m_timerID = startTimer(0);
            emit playingDidPerform();
        }
    }
    else {
        emit playingNotPerformed();
    }
}

void ALAudioEngine::stop()
{
    if (!m_source.isEmpty()) {
        if (m_timerID) {
            alureStopSource(m_audioSource, AL_FALSE);
            killTimer(m_timerID);
            m_timerID = 0;
            emit stoppingDidPerform();
        }
    }
    else {
        emit stoppingNotPerformed();
    }
}

void ALAudioEngine::release()
{
    killTimer(m_timerID);
    m_timerID = 0;
    if (m_audioSource) {
        alDeleteSources(1, &m_audioSource);
        m_audioSource = 0;
    }
    if (m_audioBuffer) {
        alDeleteBuffers(1, &m_audioBuffer);
        m_audioBuffer = 0;
    }
}

QUrl ALAudioEngine::source() const
{
    return m_source;
}

void ALAudioEngine::setSource(const QUrl &value)
{
    if (value != m_source) {
        release();
        alGenSources(1, &m_audioSource);
        alGenBuffers(1, &m_audioBuffer);
        if (alureBufferDataFromFile(value.toLocalFile().toUtf8().constData(), m_audioBuffer)) {
            alSourcei(m_audioSource, AL_BUFFER, m_audioBuffer);
            m_source = value;
            emit sourceChanged();
        }
        else {
            VPVL2_LOG(WARNING, "Cannot load audio file from " << value.toLocalFile().toStdString() << ": " << alureGetErrorString());
            emit errorDidHappen();
        }
    }
}

qreal ALAudioEngine::timeIndex() const
{
    if (m_audioSource) {
        double values[] = { 0, 0 };
        alGetSourcedvSOFT(m_audioSource, AL_SEC_OFFSET_LATENCY_SOFT, values);
        return static_cast<qreal>(qRound64(((values[0] - values[1]) * Scene::defaultFPS())));
    }
    return 0;
}

void ALAudioEngine::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timerID) {
        alureUpdate();
        emit timeIndexChanged();
    }
    QObject::timerEvent(event);
}

void ALAudioEngine::stopCallback(void *userData, ALuint /* source */)
{
    ALAudioEngine *self = static_cast<ALAudioEngine *>(userData);
    self->stop();
}
