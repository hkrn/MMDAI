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

#include "QTKitPlugin.h"

#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QTKit/QTKit.h>

namespace {

class QTKitHandler : public QImageIOHandler
{
public:
    static const float kFPS;
    QTKitHandler();
    ~QTKitHandler();
    bool canRead() const;
    QByteArray name() const;
    bool read(QImage *image);
    bool write(const QImage &image);
    int currentImageNumber() const;
    int imageCount() const;
    bool jumpToImage(int imageNumber);
    bool jumpToNextImage();
    int loopCount() const;
    int nextImageDelay() const;
    static bool canRead(QIODevice *device);
    QVariant option(ImageOption option) const;
    void setOption(ImageOption option, const QVariant &value);
    bool supportsOption(ImageOption option) const;

    void initialize(QIODevice *device);

private:
    QTMovie *m_movie;
    int m_frameCount;
    int m_frameIndex;
};

const float QTKitHandler::kFPS = 30.0;

QTKitHandler::QTKitHandler()
    : m_movie(nil),
      m_frameCount(0),
      m_frameIndex(0)
{
}

QTKitHandler::~QTKitHandler()
{
    [m_movie release];
    m_frameCount = 0;
    m_frameIndex = 0;
}

void QTKitHandler::initialize(QIODevice *device)
{
    const QByteArray &bytes = device->read(device->bytesAvailable());
    NSData *data = [NSData dataWithBytes:bytes.constData() length:bytes.size()];
    NSError *error = nil;
    [m_movie release];
    m_movie = [[QTMovie alloc] initWithData:data error:&error];
    if (error != nil) {
        qWarning("Cannot initialize QTKitHandler: %s", [[error localizedDescription] UTF8String]);
    }
    else {
        const QTTime &duration = [m_movie duration];
        m_frameCount = int((duration.timeValue / duration.timeScale) * kFPS);
    }
}

bool QTKitHandler::canRead() const
{
    return canRead(device());
}

bool QTKitHandler::canRead(QIODevice *device)
{
    return device ? device->isReadable() : false;
}

QByteArray QTKitHandler::name() const
{
    return "qtkit";
}

bool QTKitHandler::read(QImage *image)
{
    NSImage *frameImage = [m_movie currentFrameImage];
    NSData *data = [frameImage TIFFRepresentation];
    image->loadFromData(static_cast<const uchar *>([data bytes]), [data length], "TIFF");
    jumpToNextImage();
    return true;
}

bool QTKitHandler::write(const QImage & /* image */)
{
    return false;
}

int QTKitHandler::currentImageNumber() const
{
    return m_frameIndex;
}

int QTKitHandler::imageCount() const
{
    return m_frameCount;
}

bool QTKitHandler::jumpToImage(int imageNumber)
{
    if (m_movie != nil) {
        const QTTime &timeToSeek = QTMakeTimeWithTimeInterval(imageNumber / kFPS);
        [m_movie setCurrentTime:timeToSeek];
        m_frameIndex = imageNumber;
        return true;
    }
    return false;
}

bool QTKitHandler::jumpToNextImage()
{
    if (m_movie != nil && m_frameCount > m_frameIndex) {
        jumpToImage(m_frameIndex + 1);
        return true;
    }
    return false;
}

int QTKitHandler::loopCount() const
{
    return 0;
}

int QTKitHandler::nextImageDelay() const
{
    return 0;
}

QVariant QTKitHandler::option(ImageOption option) const
{
    switch (option) {
    case QImageIOHandler::Animation:
        return true;
    default:
        return QVariant();
    }
}

void QTKitHandler::setOption(ImageOption /* option */, const QVariant & /* value */)
{
}

bool QTKitHandler::supportsOption(ImageOption option) const
{
    switch (option) {
    case QImageIOHandler::Animation:
        return true;
    default:
        return false;
    }
}

} /* namespace (anonymous) */

QStringList QTKitPlugin::keys() const
{
    return QStringList() << "avi" << "m4v" << "mov" << "mp4";
}

QImageIOPlugin::Capabilities QTKitPlugin::capabilities(QIODevice * /* device */, const QByteArray &format) const
{
    Capabilities caps;
    if (keys().contains(format))
        caps |= CanRead;
    return caps;
}

QImageIOHandler *QTKitPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QTKitHandler *handler = new QTKitHandler();
    handler->setDevice(device);
    handler->setFormat(format);
    handler->initialize(device);
    return handler;
}

Q_EXPORT_STATIC_PLUGIN(QTKitPlugin)
Q_EXPORT_PLUGIN2(pnp_qtkit, QTKitPlugin)
