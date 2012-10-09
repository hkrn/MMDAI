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

#include "AVCommon.h"

namespace vpvm
{

double ComputePresentTimeStamp(AVStream *stream)
{
    return stream ? static_cast<double>(stream->pts.val) * stream->time_base.num / stream->time_base.den : 0.0;
}

void RescalePresentTimeStamp(const AVFrame *codedFrame, const AVStream *stream, AVPacket &packet)
{
    if (codedFrame && codedFrame->pts != AV_NOPTS_VALUE)
        packet.pts = av_rescale_q(codedFrame->pts, stream->codec->time_base, stream->time_base);
}

void OpenAVCodec(AVCodecContext *context, AVCodec *codec)
{
    if (!context || !codec || avcodec_open2(context, codec, 0) < 0)
        throw std::bad_exception();
}

void OpenEncodingCodec(AVCodecContext *codecContext)
{
    OpenAVCodec(codecContext, avcodec_find_encoder(codecContext->codec_id));
}

void OpenDecodingCodec(AVCodecContext *codecContext)
{
    OpenAVCodec(codecContext, avcodec_find_decoder(codecContext->codec_id));
}

AVStream *OpenAudioStream(AVFormatContext *formatContext,
                          AVOutputFormat *outputFormat,
                          CodecID codecID,
                          int bitrate,
                          int sampleRate)
{
    AVStream *stream = avformat_new_stream(formatContext, 0);
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

void WriteAudioFrame(AVFormatContext *formatContext,
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

AVStream *OpenVideoStream(AVFormatContext *formatContext,
                          AVOutputFormat *outputFormat,
                          CodecID codecID,
                          PixelFormat pixelFormat,
                          const QSize &size,
                          int bitrate,
                          int fps)
{
    //AVStream *stream = av_new_stream(formatContext, 0);
    AVStream *stream = avformat_new_stream(formatContext, 0);
    AVCodecContext *codec = stream->codec;
    codec->codec_id = codecID;
    codec->me_method = 1;
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

void WriteVideoFrame(AVFormatContext *formatContext,
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

AVOutputFormat *CreateVideoFormat(const QString &filename)
{
    AVOutputFormat *videoFormat = av_guess_format(0, filename.toLocal8Bit().constData(), 0);
    if (!videoFormat)
        videoFormat = av_guess_format("mpeg", 0, 0);
    return videoFormat;
}

AVFormatContext *CreateVideoFormatContext(AVOutputFormat *videoFormat, const QString &filename)
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

AVFrame *CreateVideoFrame(const QSize &size, enum PixelFormat format)
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

AVFormatContext *OpenInputFormat(const QString &filename, const char *shortname)
{
    AVFormatContext *formatContext = 0;
    AVInputFormat *inputFormat = av_find_input_format(shortname);
    if (!inputFormat)
        throw std::bad_exception();
    if (avformat_open_input(&formatContext, filename.toLocal8Bit().constData(), inputFormat, 0) < 0)
        throw std::bad_exception();
    return formatContext;
}

} /* namespace vpvm */
