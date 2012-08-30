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

namespace {

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

}

bool VideoEncoder::isSupported()
{
    return true;
}

void VideoEncoder::initialize()
{
    //avcodec_init();
    avcodec_register_all();
    av_register_all();
    av_lockmgr_register(UIAVCodecLockCallback);
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
    m_running = false;
}

int VideoEncoder::sizeOfVideoQueue() const
{
    m_videoQueueMutex.lock();
    int size = m_images.size();
    m_videoQueueMutex.unlock();
    return size;
}

int VideoEncoder::sizeOfAudioBuffer() const
{
    m_audioBufferMutex.lock();
    int size = m_audioBuffer.size();
    m_audioBufferMutex.unlock();
    return size;
}

void VideoEncoder::stop()
{
    m_running = false;
}

void VideoEncoder::run()
{
    CodecID audioCodecID = CODEC_ID_PCM_S16LE, videoCodecID = CODEC_ID_PNG;
    PixelFormat sourcePixelFormat = PIX_FMT_RGBA, destPixelFormat = PIX_FMT_RGB32;
    AVOutputFormat *videoOutputFormat = 0;
    AVFormatContext *videoFormatContext = 0;
    AVStream *audioStream = 0;
    AVStream *videoStream = 0;
    AVFrame *videoFrame = 0, *tmpFrame = 0;
    struct SwsContext *scaleContext = 0;
    QScopedArrayPointer<uint8_t> encodedAudioFrameBuffer, encodedVideoFrameBuffer;
    try {
        /* 動画と音声のフォーマット(AVFormatContext)をまず先に作成し、それからコーデック(AVCodecContext)を作成する */
        videoOutputFormat = CreateVideoFormat(m_filename);
        videoFormatContext = CreateVideoFormatContext(videoOutputFormat, m_filename);
        if (m_audioBitrate > 0 && m_audioSampleRate > 0)
            audioStream = OpenAudioStream(videoFormatContext,
                                          videoOutputFormat,
                                          audioCodecID,
                                          m_audioBitrate,
                                          m_audioSampleRate);
        videoStream = OpenVideoStream(videoFormatContext,
                                      videoOutputFormat,
                                      videoCodecID,
                                      destPixelFormat,
                                      m_size,
                                      m_videoBitrate,
                                      m_fps);
        /* エンコード用のフレームを確保 */
        AVCodecContext *audioCodec = 0;
        int encodedAudioFrameBufferSize = 0;
        if (audioStream) {
            audioCodec = audioStream->codec;
            encodedAudioFrameBufferSize = audioCodec->sample_rate
                    * audioCodec->frame_size
                    * audioCodec->channels
                    * av_get_bytes_per_sample(audioCodec->sample_fmt);
            encodedAudioFrameBuffer.reset(new uint8_t[encodedAudioFrameBufferSize]);
        }
        int width = m_size.width(), height = m_size.height();
        int encodedVideoFrameBufferSize = width * height * 4;
        encodedVideoFrameBuffer.reset(new uint8_t[encodedVideoFrameBufferSize]);
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
        bool remainQueue = true;
        QImage image;
        QByteArray bytes;
        /* stop() で m_running が false になるが、キューが全て空になるまで終了しない */
        while (m_running || remainQueue) {
            double audioPTS = ComputePresentTimeStamp(audioStream);
            double videoPTS = ComputePresentTimeStamp(videoStream);
            /* 音声バッファが残っている */
            if (audioCodec && audioPTS < videoPTS && sizeOfAudioBuffer() > encodedAudioFrameBufferSize) {
                /* 音声フレーム取り出し */
                dequeueAudioBuffer(bytes, encodedAudioFrameBufferSize);
                /* 音声フレーム書き出し */
                WriteAudioFrame(videoFormatContext,
                                audioStream,
                                reinterpret_cast<const int16_t *>(bytes.constData()),
                                encodedAudioFrameBuffer.data(),
                                encodedAudioFrameBufferSize);
            }
            /* 画像キューが残っている */
            else if (sizeOfVideoQueue() > 0) {
                /* キューから画像取り出し */
                dequeueImage(image);
                /* 空フレームがきたらエンコードを終了させる */
                if (image.isNull()) {
                    remainQueue = false;
                    break;
                }
                /* 仮フレームを作成し、そこに画像データを埋める */
                const int w = image.width(), h = image.height();
                uint8_t *data = tmpFrame->data[0];
                int stride = tmpFrame->linesize[0];
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
                /* 画像フレーム書き出し */
                WriteVideoFrame(videoFormatContext,
                                videoStream,
                                scaleContext ? videoFrame : tmpFrame,
                                encodedVideoFrameBuffer.data(),
                                encodedVideoFrameBufferSize);
            }
        }
        av_write_trailer(videoFormatContext);
        avcodec_close(videoStream->codec);
    } catch (std::exception &e) {
        /* TODO: エラーメッセージをわかりやすくしたい... */
        qWarning() << e.what();
    }
    /* メモリの解放処理 */
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
    m_audioBufferMutex.lock();
    m_audioBuffer.append(bytes);
    m_audioBufferMutex.unlock();
}

void VideoEncoder::dequeueImage(QImage &image)
{
    m_videoQueueMutex.lock();
    image = m_images.dequeue();
    m_videoQueueMutex.unlock();
}

void VideoEncoder::dequeueAudioBuffer(QByteArray &bytes, int size)
{
    m_audioBufferMutex.lock();
    bytes = m_audioBuffer.remove(0, size);
    m_audioBufferMutex.unlock();
}
