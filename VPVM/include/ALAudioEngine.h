/**

 Copyright (c) 2010-2014  hkrn

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

#ifndef ALAUDIOENGINE_H
#define ALAUDIOENGINE_H

#include <QObject>
#include <QBasicTimer>
#include <QUrl>

#include <AL/al.h>

class ALAudioEngine : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(qreal timeIndex READ timeIndex NOTIFY timeIndexChanged)
    Q_PROPERTY(bool seekable READ seekable NOTIFY seekableChanged FINAL)

public:
    explicit ALAudioEngine(QObject *parent = 0);
    ~ALAudioEngine();

    Q_INVOKABLE void play();
    Q_INVOKABLE void stop();
    void release();

    QUrl source() const;
    void setSource(const QUrl &value);
    qreal timeIndex() const;
    bool seekable() const;

protected:
    void timerEvent(QTimerEvent *event);

signals:
    void playingDidPerform();
    void playingNotPerformed();
    void stoppingDidPerform();
    void stoppingNotPerformed();
    void audioSourceDidLoad();
    void errorDidHappen();
    void sourceChanged();
    void seekableChanged();
    void timeIndexChanged();

private:
    static void stopCallback(void *userData, ALuint source);

    QBasicTimer m_timer;
    QUrl m_source;
    ALuint m_audioSource;
    ALuint m_audioBuffer;
};

#endif // ALAUDIOENGINE_H
