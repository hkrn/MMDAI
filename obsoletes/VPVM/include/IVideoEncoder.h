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

#ifndef VPVM_IVIDEOENCODER_H
#define VPVM_IVIDEOENCODER_H

#include <QString>
#include <QSize>
#include <QImage>

namespace vpvm
{

class IVideoEncoder
{
public:
    struct Setting {
        Setting(const QString &n, const QString &f, const QString &c, const QString &d)
            : format(f), codec(c), name(n), description(d)
        {
        }
        QString format;
        QString codec;
        QString name;
        QString description;
    };

    virtual ~IVideoEncoder() {}

    virtual void startSession() = 0;
    virtual void stopSession() = 0;
    virtual void waitUntilComplete() = 0;
    virtual void setFileName(const QString &value) = 0;
    virtual void setSceneSize(const QSize &value) = 0;
    virtual void setSceneFPS(int value) = 0;
    virtual bool isRunning() const = 0;
    virtual bool isFinished() const = 0;
    virtual int64_t sizeofVideoFrameQueue() const = 0;
    virtual QList<Setting> availableAudioSettings() const = 0;
    virtual void selectAudioSetting(const Setting &value) = 0;
    virtual QList<Setting> availableVideoSettings() const = 0;
    virtual void selectVideoSetting(const Setting &value) = 0;
    virtual const QObject *toQObject() const = 0;

protected:
    virtual void audioSamplesDidQueue(const QByteArray &value) = 0;
    virtual void videoFrameDidQueue(const QImage &value) = 0;
};

} /* namespace vpvm */

Q_DECLARE_INTERFACE(vpvm::IVideoEncoder, "com.github.hkrn.vpvm.IVideoEncoder/1.0")

#endif // VPVM_IVIDEOENCODER_H
