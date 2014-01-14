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

#ifndef GRAPHICSDEVICE_H
#define GRAPHICSDEVICE_H

#include <QObject>
#include <QString>
#include <QStringList>

class GraphicsDevice : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString version MEMBER m_version CONSTANT FINAL)
    Q_PROPERTY(QString renderer MEMBER m_renderer CONSTANT FINAL)
    Q_PROPERTY(QString vendor MEMBER m_vendor CONSTANT FINAL)
    Q_PROPERTY(QString shadingLanguage MEMBER m_shadingLanguage CONSTANT FINAL)
    Q_PROPERTY(QString extensionsText READ extensionsText CONSTANT FINAL)

public:
    explicit GraphicsDevice(QObject *parent = 0);
    ~GraphicsDevice();

    void initialize();
    QString extensionsText() const;

private:
    QString m_version;
    QString m_renderer;
    QString m_vendor;
    QString m_shadingLanguage;
    QStringList m_extensions;
};

#endif // GRAPHICSDEVICE_H
