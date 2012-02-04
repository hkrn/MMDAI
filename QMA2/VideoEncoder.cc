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
#include <QtCore/QtCore>
#include <QtGui/QtGui>

#ifdef LIBAV_FOUND
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

namespace {

static void OpenEncodingCodec(AVCodecContext *codecContext)
{
    AVCodec *codec = avcodec_find_encoder(codecContext->codec_id);
    if (!codec)
        throw std::bad_exception();
    if (avcodec_open2(codecContext, codec, 0) < 0)
        throw std::bad_exception();
}

double ComputePresentTimeStamp(AVStream *stream)
{
    return stream ? static_cast<double>(stream->pts.val) * stream->time_base.num / stream->time_base.den : 0.0;
}

void RescalePresentTimeStamp(const AVFrame *codedFrame, const AVStream *stream, AVPacket &packet)
{
    if (codedFrame && codedFrame->pts != AV_NOPTS_VALUE)
        packet.pts = av_rescale_q(codedFrame->pts, stream->codec->time_base, stream->time_base);
}

static AVStream *OpenAudioStream(AVFormatContext *formatContext,
                                 AVOutputFormat *outputFormat,
                                 CodecID codecID,
                                 int bitrate,
                                 int sampleRate)
{
    AVStream *stream = av_new_stream(formatContext, 0);
    AVCodecContext *codec = stream->codec;
    codec->codec_id = codecID;
    codec->codec_type = AVMEDIA_TYPE_AUDIO;
    codec->sample_fmt = AV_SAMPLE_FMT_S16;
    codec->bit_rate = bitrate;
    codec->sample_rate = sampleRate;
    codec->channels = 2;
    if (outputFormat->flags & AVFMT_GLOBALHEADER)
        codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
    OpenEncodingCodec(codec);
    return stream;
}

static void WriteAudioFrame(AVFormatContext *formatContext,
                            AVStream *stream,
                            const int16_t *samples,
                            uint8_t *encodedFrameBuffer,
                            size_t encodedFrameBufferSize)
{
    if (!samples)
        return;
    AVPacket packet;
    av_init_packet(&packet);
    packet.size = avcodec_encode_audio(stream->codec, encodedFrameBuffer, encodedFrameBufferSize, samples);
    RescalePresentTimeStamp(stream->codec->coded_frame, stream, packet);
    packet.flags |= AV_PKT_FLAG_KEY;
    packet.stream_index = stream->index;
    packet.data = encodedFrameBuffer;
    av_interleaved_write_frame(formatContext, &packet);
}

static AVStream *OpenVideoStream(AVFormatContext *formatContext,
                                 AVOutputFormat *outputFormat,
                                 CodecID codecID,
                                 PixelFormat pixelFormat,
                                 const QSize &size,
                                 int bitrate,
                                 int fps)
{
    AVStream *stream = av_new_stream(formatContext, 0);
    AVCodecContext *codec = stream->codec;
    codec->codec_id = codecID;
    codec->codec_type = AVMEDIA_TYPE_VIDEO;
    codec->bit_rate = bitrate;
    codec->width = size.width();
    codec->height = size.height();
    codec->time_base.den = fps;
    codec->time_base.num = 1;
    codec->gop_size = 12;
    codec->pix_fmt = pixelFormat;
    if (outputFormat->flags & AVFMT_GLOBALHEADER)
        codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
    OpenEncodingCodec(codec);
    return stream;
}

static void WriteVideoFrame(AVFormatContext *formatContext,
                            AVStream *stream,
                            AVFrame *frame,
                            uint8_t *encodedFrameBuffer,
                            size_t encodedFrameBufferSize)
{
    AVPacket packet;
    av_init_packet(&packet);
    if (formatContext->oformat->flags & AVFMT_RAWPICTURE) {
        packet.flags |= AV_PKT_FLAG_KEY;
        packet.stream_index = stream->index;
        packet.data = reinterpret_cast<uint8_t *>(frame);
        packet.size = sizeof(AVPicture);
        av_interleaved_write_frame(formatContext, &packet);
    }
    else {
        int outputSize = avcodec_encode_video(stream->codec, encodedFrameBuffer, encodedFrameBufferSize, frame);
        if (outputSize > 0) {
            AVFrame *codedFrame = stream->codec->coded_frame;
            RescalePresentTimeStamp(codedFrame, stream, packet);
            if (codedFrame->key_frame)
                packet.flags |= AV_PKT_FLAG_KEY;
            packet.stream_index = stream->index;
            packet.data = encodedFrameBuffer;
            packet.size = outputSize;
            av_interleaved_write_frame(formatContext, &packet);
        }
    }
}

static AVOutputFormat *CreateVideoFormat(const QString &filename)
{
    AVOutputFormat *videoFormat = av_guess_format(0, filename.toLocal8Bit().constData(), 0);
    if (!videoFormat)
        videoFormat = av_guess_format("mpeg", 0, 0);
    return videoFormat;
}

static AVFormatContext *CreateVideoFormatContext(AVOutputFormat *videoFormat, const QString &filename)
{
    AVFormatContext *videoFormatContext = avformat_alloc_context();
    if (!videoFormatContext)
        throw std::bad_alloc();
    videoFormatContext->oformat = videoFormat;
    snprintf(videoFormatContext->filename,
             sizeof(videoFormatContext->filename),
             "%s",
             filename.toLocal8Bit().constData());
    return videoFormatContext;
}

static AVFrame *CreateVideoFrame(const QSize &size, enum PixelFormat format)
{
    AVFrame *frame = avcodec_alloc_frame();
    int width = size.width(), height = size.height();
    if (!frame)
        throw std::bad_alloc();
    int willAllocate = avpicture_get_size(format, width, height);
    uint8_t *buffer = static_cast<uint8_t *>(av_malloc(willAllocate));
    if (!buffer) {
        av_free(frame);
        throw std::bad_alloc();
    }
    avpicture_fill(reinterpret_cast<AVPicture *>(frame), buffer, format, width, height);
    return frame;
}

}
#endif /* LIBAV_FOUND */

bool VideoEncoder::isSupported()
{
#ifdef LIBAV_FOUND
    return true;
#else
    return false;
#endif /* LIBAV_FOUND */
}

void VideoEncoder::initialize()
{
#ifdef LIBAV_FOUND
    avcodec_init();
    avcodec_register_all();
    av_register_all();
#endif /* LIBAV_FOUND */
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

void VideoEncoder::run()
{
#ifdef LIBAV_FOUND
    CodecID videoCodecID = CODEC_ID_PNG;
    PixelFormat sourcePixelFormat = PIX_FMT_RGBA, destPixelFormat = PIX_FMT_RGB24;
    AVOutputFormat *videoOutputFormat = 0;
    AVFormatContext *videoFormatContext = 0;
    AVStream *audioStream = 0;
    AVStream *videoStream = 0;
    AVFrame *videoFrame = 0, *tmpFrame = 0;
    struct SwsContext *scaleContext = 0;
    uint8_t *encodedAudioFrameBuffer = 0, *encodedVideoFrameBuffer = 0;
    try {
        int width = m_size.width(), height = m_size.height();
        size_t encodedAudioFrameBufferSize = 192000;
        size_t encodedVideoFrameBufferSize = width * height * 4;
        encodedAudioFrameBuffer = new uint8_t[encodedAudioFrameBufferSize];
        encodedVideoFrameBuffer = new uint8_t[encodedVideoFrameBufferSize];
        videoOutputFormat = CreateVideoFormat(m_filename);
        videoFormatContext = CreateVideoFormatContext(videoOutputFormat, m_filename);
        /*
        audioStream = OpenAudioStream(videoFormatContext,
                                      videoOutputFormat,
                                      videoOutputFormat->audio_codec,
                                      m_audioBitrate,
                                      m_audioSampleRate);
                                      */
        videoStream = OpenVideoStream(videoFormatContext,
                                      videoOutputFormat,
                                      videoCodecID,
                                      destPixelFormat,
                                      m_size,
                                      m_videoBitrate,
                                      m_fps);
        if (!(videoOutputFormat->flags & AVFMT_NOFILE)) {
            if (avio_open(&videoFormatContext->pb, m_filename.toLocal8Bit().constData(), AVIO_FLAG_WRITE) < 0)
                throw std::bad_exception();
        }
        if (sourcePixelFormat != destPixelFormat) {
            scaleContext = sws_getContext(width, height, sourcePixelFormat,
                                          width, height, destPixelFormat,
                                          SWS_BICUBIC, 0, 0, 0);
        }
        videoFrame = CreateVideoFrame(m_size, destPixelFormat);
        tmpFrame = CreateVideoFrame(m_size, sourcePixelFormat);
        avformat_write_header(videoFormatContext, 0);
        double audioPTS = 0.0, videoPTS = 0.0;
        bool remainQueue = true;
        while (m_running || remainQueue) {
            m_mutex.lock();
            if (m_images.size() > 0) {
                const QImage &image = m_images.dequeue();
                m_mutex.unlock();
                audioPTS = ComputePresentTimeStamp(audioStream);
                videoPTS = ComputePresentTimeStamp(videoStream);
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
                if (scaleContext) {
                    sws_scale(scaleContext,
                              tmpFrame->data, tmpFrame->linesize, 0, h,
                              videoFrame->data, videoFrame->linesize);
                }
                if (!videoStream || (videoStream && audioStream && audioPTS < videoPTS)) {
                    WriteAudioFrame(videoFormatContext,
                                    audioStream,
                                    0,
                                    encodedAudioFrameBuffer,
                                    encodedAudioFrameBufferSize);
                }
                else {
                    WriteVideoFrame(videoFormatContext,
                                    videoStream,
                                    scaleContext ? videoFrame : tmpFrame,
                                    encodedVideoFrameBuffer,
                                    encodedVideoFrameBufferSize);
                }
            }
            else {
                m_mutex.unlock();
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
#endif /* LIBAV_FOUND */
}

void VideoEncoder::enqueueImage(const QImage &image)
{
    m_mutex.lock();
    m_images.enqueue(image);
    m_mutex.unlock();
}

void VideoEncoder::stop()
{
    m_running = false;
}
