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

#ifndef AUDIODECODER_H
#define AUDIODECODER_H

#include <QtCore/QtCore>
#include "IAudioDecoder.h"
#include "AVCommon.h"

namespace vpvm
{

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

class AudioDecoder : public QThread, public IAudioDecoder
{
    Q_OBJECT

public:
    AudioDecoder();
    ~AudioDecoder();

    void startSession();
    void stopSession();
    void waitUntilComplete();
    void setFileName(const QString &value);
    bool canOpen();
    bool isFinished() const { return !m_running; }
    int  audioChannels() const;
    float  audioSampleRate() const;

protected:
    virtual void run();
    virtual void decodeBuffer(const QByteArray &bytes, qreal position, int channels);

signals:
    void audioDidDecode(const QByteArray &bytes);
    void audioDidDecodeComplete();
    void audioDidDecodeError();

private:
    QString m_filename;
    volatile bool m_running;
    AVFormatContextPtr m_formatContext;
    AVCodecContextPtr m_audioContext;

    Q_DISABLE_COPY(AudioDecoder)
};

}

#endif // AUDIODECODER_H
