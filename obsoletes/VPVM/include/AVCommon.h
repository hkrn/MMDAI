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

#ifndef VPVM_AVCOMMON_H
#define VPVM_AVCOMMON_H

#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/mathematics.h>
}

#include <QSize>
#include <QString>

namespace vpvm
{

double ComputePresentTimeStamp(const AVStream *stream);

void RescalePresentTimeStamp(const AVFrame *codedFrame, const AVStream *stream, AVPacket &packet);

void OpenAVCodec(AVCodecContext *context, AVCodec *codec);

void OpenEncodingCodec(AVCodecContext *codecContext);

void OpenDecodingCodec(AVCodecContext *codecContext);

AVStream *OpenAudioStream(AVFormatContext *formatContext,
                                 AVOutputFormat *outputFormat,
                                 CodecID codecID,
                                 int bitrate,
                                 int sampleRate);

void WriteAudioFrame(AVFormatContext *formatContext,
                            AVStream *stream,
                            const int16_t *samples,
                            uint8_t *encodedFrameBuffer,
                            size_t encodedFrameBufferSize);

AVStream *OpenVideoStream(AVFormatContext *formatContext,
                                 AVOutputFormat *outputFormat,
                                 CodecID codecID,
                                 PixelFormat pixelFormat,
                                 const QSize &size,
                                 int bitrate,
                                 int fps);

void WriteVideoFrame(AVFormatContext *formatContext,
                            AVStream *stream,
                            AVFrame *frame,
                            uint8_t *encodedFrameBuffer,
                            size_t encodedFrameBufferSize);

AVOutputFormat *CreateVideoFormat(const QString &filename);

AVFormatContext *CreateVideoFormatContext(AVOutputFormat *videoFormat, const QString &filename);

AVFormatContext *OpenInputFormat(const QString &filename, const char *shortname);

AVFrame *CreateVideoFrame(const QSize &size, enum PixelFormat format);

} /* namespace vpvm */

#endif // VPVM_AVCOMMON_H
