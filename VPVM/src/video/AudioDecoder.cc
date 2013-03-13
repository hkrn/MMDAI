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

#include "AudioDecoder.h"
#include "AVCommon.h"

#include <QtCore/QtCore>
#include <QtGui/QtGui>

using namespace vpvm;

namespace {

struct AVCodecContextCleaner
{
    static void cleanup(AVCodecContext *context) {
        if (context)
            avcodec_close(context);
    }
};

struct AVFormatContextCleaner
{
    static void cleanup(AVFormatContext *context) {
        av_free(context);
    }
};

typedef QScopedPointer<AVFormatContext, AVFormatContextCleaner> AVFormatContextPtr;
typedef QScopedPointer<AVCodecContext, AVCodecContextCleaner> AVCodecContextPtr;

bool UIOpenAudio(const QString &filename,
                 AVFormatContextPtr &formatContext,
                 AVCodecContextPtr &audioContext,
                 AVStream *&stream)
{
    formatContext.reset(OpenInputFormat(filename, "wav"));
    if (formatContext->nb_streams  < 1)
        return false;
    stream = formatContext->streams[0];
    audioContext.reset(stream->codec);
    if (audioContext->codec_type != AVMEDIA_TYPE_AUDIO)
        return false;
    OpenAVCodec(audioContext.data(), avcodec_find_decoder(audioContext->codec_id));
    return true;
}

}

namespace vpvm
{

AudioDecoder::AudioDecoder(QObject *parent)
    : QThread(parent),
      m_running(true)
{
}

AudioDecoder::~AudioDecoder()
{
    m_running = false;
}

bool AudioDecoder::canOpen() const
{
    AVFormatContextPtr formatContext;
    AVCodecContextPtr audioContext;
    bool ret = true;
    if (!m_filename.isEmpty()) {
        AVStream *stream = 0;
        if (UIOpenAudio(m_filename, formatContext, audioContext, stream))
            ret = audioContext->channels == 2 && audioContext->sample_rate == 44100;
    }
    else {
        ret = false;
    }
    return ret;
}

void AudioDecoder::setFileName(const QString &filename)
{
    m_filename = filename;
}

void AudioDecoder::startSession()
{
    m_running = true;
    start();
}

void AudioDecoder::stopSession()
{
    m_running = false;
}

void AudioDecoder::waitUntilComplete()
{
    stopSession();
    wait();
}

void AudioDecoder::run()
{
    AVFormatContextPtr formatContext;
    AVCodecContextPtr audioContext;
    QScopedArrayPointer<int16_t> samples;
    AVStream *stream = 0;
    AVPacket packet;
    av_init_packet(&packet);
    if (UIOpenAudio(m_filename, formatContext, audioContext, stream)) {
        samples.reset(new int16_t[AVCODEC_MAX_AUDIO_FRAME_SIZE]);
        qreal sampleRate = audioContext->sample_rate;
        /* フォーマットからパケット単位で読み取り、その音声パケットをデコードするの繰り返しを行う */
        QByteArray bytes;
        while (m_running) {
            int size = AVCODEC_MAX_AUDIO_FRAME_SIZE;
            if (av_read_frame(formatContext.data(), &packet) < 0)
                break;
            int len = avcodec_decode_audio3(audioContext.data(), samples.data(), &size, &packet);
            if (len < 0)
                break;
            bytes.setRawData(reinterpret_cast<const char *>(samples.data()), size);
            qreal position = packet.pts / sampleRate;
            decodeBuffer(bytes, position, audioContext->channels);
        }
        emit audioDidDecodeComplete();
    }
}

void AudioDecoder::decodeBuffer(const QByteArray &bytes, qreal /* position */, int /* channels */)
{
    emit audioDidDecode(bytes);
}

} /* namespace vpvm */

