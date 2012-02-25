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

#include "VideoEncoder.h"
#include "AVCommon.h"

#include <QtCore/QtCore>
#include <QtGui/QtGui>

using namespace internal;

bool VideoEncoder::isSupported()
{
    return true;
}

void VideoEncoder::initialize()
{
    avcodec_init();
    avcodec_register_all();
    av_register_all();
}

VideoEncoder::VideoEncoder(const QString &filename,
                           const QSize &size,
                           int fps,
                           int videoBitrate,
                           int audioBitrate,
                           int audioSampleRate,
                           QObject *parent)
    : QThread(parent),
      m_filename(filename),
      m_size(size),
      m_fps(fps),
      m_videoBitrate(videoBitrate),
      m_audioBitrate(audioBitrate),
      m_audioSampleRate(audioSampleRate),
      m_running(true)
{
}

VideoEncoder::~VideoEncoder()
{
    m_images.clear();
    m_audioBuffer.clear();
    m_running = false;
}

int VideoEncoder::sizeOfVideoQueue() const
{
    int size;
    m_videoQueueMutex.lock();
    size = m_images.size();
    m_videoQueueMutex.unlock();
    return size;
}

int VideoEncoder::sizeOfAudioQueue() const
{
    int size;
    m_audioQueueMutex.lock();
    size = m_audioBuffer.size();
    m_audioQueueMutex.unlock();
    return size;
}

void VideoEncoder::stop()
{
    m_running = false;
}

void VideoEncoder::run()
{
    CodecID videoCodecID = CODEC_ID_PNG;
    PixelFormat sourcePixelFormat = PIX_FMT_RGBA, destPixelFormat = PIX_FMT_RGB32;
    AVOutputFormat *videoOutputFormat = 0;
    AVFormatContext *videoFormatContext = 0;
    AVStream *audioStream = 0;
    AVStream *videoStream = 0;
    AVFrame *videoFrame = 0, *tmpFrame = 0;
    struct SwsContext *scaleContext = 0;
    uint8_t *encodedAudioFrameBuffer = 0, *encodedVideoFrameBuffer = 0;
    try {
        /* 動画と音声のフォーマット(AVFormatContext)をまず先に作成し、それからコーデック(AVCodecContext)を作成する */
        int width = m_size.width(), height = m_size.height();
        size_t encodedAudioFrameBufferSize = AVCODEC_MAX_AUDIO_FRAME_SIZE;
        size_t encodedVideoFrameBufferSize = width * height * 4;
        encodedAudioFrameBuffer = new uint8_t[encodedAudioFrameBufferSize];
        encodedVideoFrameBuffer = new uint8_t[encodedVideoFrameBufferSize];
        videoOutputFormat = CreateVideoFormat(m_filename);
        videoFormatContext = CreateVideoFormatContext(videoOutputFormat, m_filename);
        if (m_audioBitrate > 0 && m_audioSampleRate > 0)
            audioStream = OpenAudioStream(videoFormatContext,
                                          videoOutputFormat,
                                          videoOutputFormat->audio_codec,
                                          m_audioBitrate,
                                          m_audioSampleRate);
        videoStream = OpenVideoStream(videoFormatContext,
                                      videoOutputFormat,
                                      videoCodecID,
                                      destPixelFormat,
                                      m_size,
                                      m_videoBitrate,
                                      m_fps);
        /* 書き出し準備を行う。ファイルなので常に true になる */
        if (!(videoOutputFormat->flags & AVFMT_NOFILE)) {
            if (avio_open(&videoFormatContext->pb, m_filename.toLocal8Bit().constData(), AVIO_FLAG_WRITE) < 0)
                throw std::bad_exception();
        }
        /* libswscale の初期化。ピクセルのフォーマットが互いに異なるので、常に true になる */
        if (sourcePixelFormat != destPixelFormat) {
            scaleContext = sws_getContext(width, height, sourcePixelFormat,
                                          width, height, destPixelFormat,
                                          SWS_BICUBIC, 0, 0, 0);
        }
        /* フレームを作成。2つあるのは上の実際のフレームと libswscale でスケールするための仮フレームがあるため */
        videoFrame = CreateVideoFrame(m_size, destPixelFormat);
        tmpFrame = CreateVideoFrame(m_size, sourcePixelFormat);
        avformat_write_header(videoFormatContext, 0);
        double audioPTS = 0.0, videoPTS = 0.0;
        bool remainQueue = true;
        QImage image, nullImage;
        QByteArray bytes, nullBytes;
        /* stop() で m_running が false になるが、キューが全て空になるまで終了しない */
        while (m_running || remainQueue) {
            int videoQueueSize = sizeOfVideoQueue();
            int audioQueueSize = sizeOfAudioQueue();
            /* 音声キューまたは画像キューに残りがある */
            if (videoQueueSize > 0 || audioQueueSize > 0) {
                audioPTS = ComputePresentTimeStamp(audioStream);
                videoPTS = ComputePresentTimeStamp(videoStream);
                /* 音声フレーム取り出し */
                if (audioQueueSize > 0) {
                    dequeueAudioBuffer(bytes);
                }
                /* 画像フレーム取り出し */
                if (videoQueueSize > 0) {
                    dequeueImage(image);
                    const int w = image.width(), h = image.height();
                    uint8_t *data = tmpFrame->data[0];
                    int stride = tmpFrame->linesize[0];
                    /* 画像データを仮のフレームに埋める */
                    for (int y = 0; y < h; y++) {
                        const int ystride = y * stride;
                        for (int x = 0; x < w; x++) {
                            int index = ystride + x * 4;
                            const QRgb &rgb = image.pixel(x, y);
                            data[index + 0] = qRed(rgb);
                            data[index + 1] = qGreen(rgb);
                            data[index + 2] = qBlue(rgb);
                            data[index + 3] = qAlpha(rgb);
                        }
                    }
                    /* 画像フレームを指定されたサイズとフォーマットでスケーリング */
                    if (scaleContext) {
                        sws_scale(scaleContext,
                                  tmpFrame->data, tmpFrame->linesize, 0, h,
                                  videoFrame->data, videoFrame->linesize);
                    }
                }
                /* ここの部分はまだ不完全。音声書き出しと画像書き出しでフレームに漏れを起こす */
                /* 音声フレーム書き出し */
                if (!videoStream || (videoStream && audioStream && audioPTS < videoPTS && !bytes.isEmpty())) {
                    WriteAudioFrame(videoFormatContext,
                                    audioStream,
                                    reinterpret_cast<const int16_t *>(bytes.constData()),
                                    encodedAudioFrameBuffer,
                                    encodedAudioFrameBufferSize);
                    bytes = nullBytes;
                }
                /* 画像フレーム書き出し */
                else if (!image.isNull()) {
                    WriteVideoFrame(videoFormatContext,
                                    videoStream,
                                    scaleContext ? videoFrame : tmpFrame,
                                    encodedVideoFrameBuffer,
                                    encodedVideoFrameBufferSize);
                    image = nullImage;
                }
            }
            else {
                /* 音声または画像のキューが両方共空になった */
                remainQueue = false;
            }
        }
        av_write_trailer(videoFormatContext);
        avcodec_close(videoStream->codec);
    } catch (std::exception &e) {
        qWarning() << e.what();
    }
    delete[] encodedAudioFrameBuffer;
    delete[] encodedVideoFrameBuffer;
    if (videoFrame) {
        avpicture_free(reinterpret_cast<AVPicture *>(videoFrame));
        av_free(videoFrame);
    }
    if (tmpFrame) {
        avpicture_free(reinterpret_cast<AVPicture *>(tmpFrame));
        av_free(tmpFrame);
    }
    if (scaleContext)
        sws_freeContext(scaleContext);
    if (videoFormatContext) {
        const int nstreams = videoFormatContext->nb_streams;
        for (int i = 0; i < nstreams; i++) {
            AVStream *stream = videoFormatContext->streams[i];
            av_freep(&stream->codec);
            av_freep(&stream);
        }
        if (!(videoFormatContext->flags & AVFMT_NOFILE) && videoFormatContext->pb)
            avio_close(videoFormatContext->pb);
        av_free(videoFormatContext);
    }
}

void VideoEncoder::enqueueImage(const QImage &image)
{
    m_videoQueueMutex.lock();
    m_images.enqueue(image);
    m_videoQueueMutex.unlock();
}

void VideoEncoder::enqueueAudioBuffer(const QByteArray &bytes)
{
    m_audioQueueMutex.lock();
    m_audioBuffer.enqueue(bytes);
    m_audioQueueMutex.unlock();
}

void VideoEncoder::dequeueImage(QImage &image)
{
    m_videoQueueMutex.lock();
    image = m_images.dequeue();
    m_videoQueueMutex.unlock();
}

void VideoEncoder::dequeueAudioBuffer(QByteArray &bytes)
{
    m_audioQueueMutex.lock();
    bytes = m_audioBuffer.dequeue();
    m_audioQueueMutex.unlock();
}
