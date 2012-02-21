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

#include "AudioDecoder.h"
#include "AVCommon.h"

#include <QtCore/QtCore>
#include <QtGui/QtGui>

using namespace internal;

AudioDecoder::AudioDecoder(const QString &filename)
    : m_filename(filename),
      m_running(true)
{
}

AudioDecoder::~AudioDecoder()
{
    m_running = false;
}

void AudioDecoder::run()
{
    AVFormatContext *formatContext = 0;
    AVCodecContext *audioContext = 0;
    int16_t *samples = 0;
    AVPacket packet;
    av_init_packet(&packet);
    try {
        formatContext = OpenInputFormat(m_filename, "wav");
        if (formatContext->nb_streams  < 1)
            throw std::bad_exception();
        audioContext = formatContext->streams[0]->codec;
        if (audioContext->codec_type != AVMEDIA_TYPE_AUDIO)
            throw std::bad_exception();
        OpenAVCodec(audioContext, avcodec_find_decoder(CODEC_ID_PCM_S16LE));
        samples = new int16_t[AVCODEC_MAX_AUDIO_FRAME_SIZE];
        if (!samples)
            throw std::bad_alloc();
        while (m_running) {
            int size = AVCODEC_MAX_AUDIO_FRAME_SIZE;
            if (av_read_frame(formatContext, &packet) < 0)
                break;
            int len = avcodec_decode_audio3(audioContext, samples, &size, &packet);
            if (len < 0)
                break;
            emit audioDidDecode(QByteArray(reinterpret_cast<const char *>(samples), size));
        }
    } catch (std::exception &e) {
        qWarning() << e.what();
    }
    if (samples)
        delete[] samples;
    if (audioContext)
        avcodec_close(audioContext);
    if (formatContext)
        av_free(formatContext);
}

void AudioDecoder::stop()
{
    m_running = false;
}
