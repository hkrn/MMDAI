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

#ifndef VPVM_AUDIOPLAYER_H
#define VPVM_AUDIOPLAYER_H

#include "AudioDecoder.h"
#include "IAudioPlayer.h"

#include <portaudio.h>

namespace vpvm
{

class AudioPlayer : public AudioDecoder, public IAudioPlayer
{
    Q_OBJECT

public:
    static void initializePlayer();

    explicit AudioPlayer(QObject *parent = 0);
    ~AudioPlayer();

    bool openOutputDevice();
    void stopSession();

    bool isRunning() const { return AudioDecoder::isRunning(); }
    void startSession() { AudioDecoder::startSession(); }
    void setFileName(const QString &value) { AudioDecoder::setFileName(value); }
    const QObject *toQObject() const { return AudioDecoder::toQObject(); }

protected:
    void run();
    void decodeBuffer(const QByteArray &bytes, qreal position, int channels);

signals:
    void positionDidAdvance(qreal delta);

private:
    PaStream *m_stream;
    float m_position;

    Q_DISABLE_COPY(AudioPlayer)
};

} /* namespace vpvm */

#endif // VPVM_AUDIOPLAYER_H
