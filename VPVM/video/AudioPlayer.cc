/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
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
#include "AVCommon.h"

namespace {

bool g_initialized = false;

void Pa_Terminate_Wrapper() {
    PaError err = Pa_Terminate();
    if (err != paNoError) {
        qWarning("Pa_Terminate failed: %s", Pa_GetErrorText(err));
    }
}

}

namespace vpvm
{

// rhs: copied UIAVCodecLockCallback from VideoEncoder.cc
static int UIAVCodecLockCallback(void ** /* mutex */, AVLockOp op)
{
    /*
     * Video::initialize の内部で行なっている av_lockmgr_register で必要
     * これがないとスレッドセーフではないことが明記されている avcodec_open で失敗する
     */
    static QMutex mutex;
    switch (op) {
    case AV_LOCK_CREATE:
    default:
        return 0;
    case AV_LOCK_OBTAIN:
        return mutex.tryLock() ? 0 : -1;
    case AV_LOCK_RELEASE:
    case AV_LOCK_DESTROY:
        mutex.unlock();
        return 0;
    }
}


void AudioPlayer::initializePlayer()
{
    if (!g_initialized) {
        // rhs: initialize libav
        avcodec_register_all();
        av_register_all();
        av_lockmgr_register(UIAVCodecLockCallback);
        PaError err = Pa_Initialize();
        if (err == paNoError) {
            atexit(Pa_Terminate_Wrapper);
            g_initialized = true;
        }
        else {
            qWarning("Pa_Initialize failed: %s", Pa_GetErrorText(err));
        }
    }
}

AudioPlayer::AudioPlayer()
    : AudioDecoder(),
      m_stream(0),
      m_position(0)
{
    initializePlayer();
}

AudioPlayer::~AudioPlayer()
{
    AudioDecoder::stopSession();
    if (m_stream) {
        Pa_CloseStream(m_stream);
        m_stream = 0;
    }
}

bool AudioPlayer::openOutputDevice()
{
    PaStreamParameters parameters;
    parameters.device = Pa_GetDefaultOutputDevice();
    if (canOpen() && parameters.device != paNoDevice) {
        const PaDeviceInfo *info = Pa_GetDeviceInfo(parameters.device);
        parameters.channelCount = audioChannels();
        parameters.sampleFormat = paInt16;
        parameters.suggestedLatency = info->defaultHighOutputLatency;
        parameters.hostApiSpecificStreamInfo = 0;
        qDebug("name: %s", info->name);
        qDebug() << "sampleRate:" << audioSampleRate();  // rhs
        qDebug() << "defaultHighOutputLatency:" << info->defaultHighOutputLatency;
        //PaError err = Pa_OpenStream(&m_stream, 0, &parameters, 44100.0, 1024, paClipOff, 0, 0);
        PaError err = Pa_OpenStream(&m_stream, 0, &parameters, audioSampleRate(), 1024, paClipOff, 0, 0);
        if (err == paNoError) {
            return true;
        }
        else {
            qWarning("%s: %s", qPrintable(tr("Cannot open stream from device")), Pa_GetErrorText(err));
        }
    }
    else if (parameters.device == paNoDevice) {
        qWarning("Cannot find audio device");
    }
    else {
        qWarning("Cannot open audio file");
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

void AudioPlayer::decodeBuffer(const QByteArray &bytes, qreal position, int channels)
{
    int size = bytes.length() / (channels * sizeof(int16_t));
    Pa_WriteStream(m_stream, bytes.constData(), size);
    emit positionDidAdvance(position - m_position);
    m_position = position;
}

} /* namespace vpvm */

