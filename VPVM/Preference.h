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

#ifndef PREFERENCE_H
#define PREFERENCE_H

#include <QObject>
#include <QRect>
#include <QSettings>

class Preference : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QRect windowRect READ windowRect WRITE setWindowRect NOTIFY windowRectChanged FINAL)
    Q_PROPERTY(QString fontFamily READ fontFamily WRITE setFontFamily NOTIFY fontFamilyChanged FINAL)
    Q_PROPERTY(QString baseLoggingDirectory READ baseLoggingDirectory WRITE setBaseLoggingDirectory NOTIFY baseLoggingDirectoryChanged FINAL)
    Q_PROPERTY(QString loggingDirectorySuffix READ loggingDirectorySuffix WRITE setLoggingDirectorySuffix NOTIFY loggingDirectorySuffixChanged FINAL)
    Q_PROPERTY(int verboseLogLevel READ verboseLogLevel WRITE setVerboseLogLevel NOTIFY verboseLogLevelChanged FINAL)
    Q_PROPERTY(int samples READ samples WRITE setSamples NOTIFY samplesChanged FINAL)

public:
    explicit Preference(QObject *parent = 0);
    ~Preference();

    Q_INVOKABLE void sync();
    Q_INVOKABLE void clear();
    QString initializeLoggingDirectory();

    QRect windowRect() const;
    void setWindowRect(const QRect &value);
    QString fontFamily() const;
    void setFontFamily(const QString &value);
    QString baseLoggingDirectory() const;
    void setBaseLoggingDirectory(const QString &value);
    QString loggingDirectorySuffix() const;
    void setLoggingDirectorySuffix(const QString &value);
    int verboseLogLevel() const;
    void setVerboseLogLevel(int value);
    int samples() const;
    void setSamples(int value);

signals:
    void windowRectChanged();
    void fontFamilyChanged();
    void baseLoggingDirectoryChanged();
    void loggingDirectorySuffixChanged();
    void verboseLogLevelChanged();
    void samplesChanged();

private:
    QSettings m_settings;
};

#endif // PREFERENCE_H
