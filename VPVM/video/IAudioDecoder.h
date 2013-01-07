#ifndef VPVM_IAUDIODECODER_H
#define VPVM_IAUDIODECODER_H

#include <QtCore/QObject>
#include <QtGui/QImage>

namespace vpvm
{

class IAudioDecoder
{
public:
    virtual ~IAudioDecoder() {}

    virtual void startSession() = 0;
    virtual void stopSession() = 0;
    virtual void waitUntilComplete() = 0;
    virtual void setFileName(const QString &value) = 0;
    virtual bool isFinished() const = 0;
    virtual bool canOpen() = 0;

protected:
    virtual void audioDidDecode(const QByteArray &bytes) = 0;
};

} /* namespace vpvm */

Q_DECLARE_INTERFACE(vpvm::IAudioDecoder, "com.github.hkrn.vpvm.IAudioDecoder/1.0")

#endif // VPVM_IAUDIODECODER_H
