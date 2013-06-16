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

#ifndef VPVM_BACKGROUNDIMAGE_H_
#define VPVM_BACKGROUNDIMAGE_H_

#include <QSize>
#include <QMovie>
#include <stdint.h> /* for intptr_t */

namespace vpvl2 {
namespace qt {
class TextureDrawHelper;
}
}

namespace vpvm {

using namespace vpvl2::qt;

class BackgroundImage {
public:
    BackgroundImage(const QSize &size);
    ~BackgroundImage();

    void resize(const QSize &size);
    void setImage(const QString &filename);
    void setTimeIndex(int value);
    void draw();

    const QSize &imageSize() const { return m_backgroundImageSize; }
    const QString &imageFilename() const { return m_backgroundImageFilename; }
    const QPoint &imagePosition() const { return m_backgroundImagePosition; }
    void setImagePosition(const QPoint &value) { m_backgroundImagePosition = value; }
    bool isUniformEnabled() const { return m_uniformImage; }
    void setUniformEnable(bool value) { m_uniformImage = value; }

private:
    void release();
    void generateTextureFromImage(const QImage &image);

    QScopedPointer<TextureDrawHelper> m_backgroundDrawer;
    QMovie m_movie;
    QSize m_backgroundImageSize;
    QPoint m_backgroundImagePosition;
    QString m_backgroundImageFilename;
    intptr_t m_backgroundTexture;
    bool m_uniformImage;
};

} /* namespace vpvm */

#endif // BACKGROUNDIMAGE_H
