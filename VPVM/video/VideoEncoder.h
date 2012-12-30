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

#ifndef VIDEOENCODER_H
#define VIDEOENCODER_H

#include <QtCore/QtCore>
#include <QtGui/QImage>
#include "IVideoEncoder.h"

namespace vpvm
{

class VideoEncoder : public QThread, public IVideoEncoder
{
    Q_OBJECT

public:
    static bool isSupported();
    static void initializeEncoder();

    explicit VideoEncoder(QObject *parent);
    ~VideoEncoder();

    void startSession();
    void stopSession();
    void waitUntilComplete();
    void setFileName(const QString &value);
    void setSceneSize(const QSize &value);
    void setSceneFPS(int value);
    bool isRunning() const { return m_running; }
    bool isFinished() const { return !m_running; }
    int64_t sizeofVideoFrameQueue() const;
    int64_t sizeofAudioSampleQueue() const;

protected:
    virtual void run();

private slots:
    void videoFrameDidQueue(const QImage &image);
    void audioSamplesDidQueue(const QByteArray &bytes);

private:
    void dequeueVideoFrame(QImage &image);
    void dequeueAudioSamples(QByteArray &bytes, int size);

    mutable QMutex m_videoQueueMutex;
    mutable QMutex m_audioBufferMutex;
    QString m_filename;
    QByteArray m_audioBuffer;
    QQueue<QImage> m_images;
    QSize m_size;
    int m_fps;
    int m_videoBitrate;
    int m_audioBitrate;
    int m_audioSampleRate;
    volatile bool m_running;

    Q_DISABLE_COPY(VideoEncoder)
};

} /* namespace vpvm */

#endif // VIDEOENCODER_H
