/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
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

#include "AudioPlayer.h"

namespace vpvm
{

AudioPlayer::AudioPlayer()
    : AudioDecoder(),
      m_stream(0),
      m_position(0)
{
}

AudioPlayer::~AudioPlayer()
{
    AudioDecoder::stopSession();
    if (m_stream) {
        Pa_CloseStream(m_stream);
        m_stream = 0;
    }
}

bool AudioPlayer::initalize()
{
    PaStreamParameters parameters;
    parameters.device = Pa_GetDefaultOutputDevice();
    if (canOpen() && parameters.device != paNoDevice) {
        const PaDeviceInfo *info = Pa_GetDeviceInfo(parameters.device);
        parameters.channelCount = 2;
        parameters.sampleFormat = paInt16;
        parameters.suggestedLatency = info->defaultLowOutputLatency;
        parameters.hostApiSpecificStreamInfo = 0;
        qDebug("name: %s", info->name);
        qDebug() << "sampleRate:" << info->defaultSampleRate;
        qDebug() << "defaultLowInputLatency:" << info->defaultLowInputLatency;
        qDebug() << "defualtLowOutputLatency:" << info->defaultLowOutputLatency;
        PaError err = Pa_OpenStream(&m_stream, 0, &parameters, 44100.0, 1024, paClipOff, 0, 0);
        if (err == paNoError) {
            return true;
        }
        else {
            qWarning("%s: %s", qPrintable(tr("Cannot open stream from device")), Pa_GetErrorText(err));
        }
    }
    else {
        qWarning("%s", qPrintable(tr("Cannot open audio file or not found audio device")));
    }
    return false;
}

void AudioPlayer::stopSession()
{
    AudioDecoder::stopSession();
    if (m_stream)
        Pa_StopStream(m_stream);
}

void AudioPlayer::run()
{
    Pa_StartStream(m_stream);
    AudioDecoder::run();
}

void AudioPlayer::decodeBuffer(const QByteArray &bytes, float position, int channels)
{
    int size = bytes.length() / (channels * sizeof(int16_t));
    Pa_WriteStream(m_stream, bytes.constData(), size);
    emit positionDidAdvance(position - m_position);
    m_position = position;
}

} /* namespace vpvm */

