#ifndef VPVM_IVIDEOENCODER_H
#define VPVM_IVIDEOENCODER_H

#include <QtCore/QObject>
#include <QtGui/QImage>

namespace vpvm
{

class IVideoEncoder
{
public:
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

protected:
    virtual void audioSamplesDidQueue(const QByteArray &value) = 0;
    virtual void videoFrameDidQueue(const QImage &value) = 0;
};

} /* namespace vpvm */

Q_DECLARE_INTERFACE(vpvm::IVideoEncoder, "com.github.hkrn.vpvm.IVideoEncoder/1.0")

#endif // VPVM_IVIDEOENCODER_H
