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

#include "Preference.h"

#include <QApplication>
#include <QDir>
#include <QScreen>
#include <QStandardPaths>

Preference::Preference(QObject *parent)
    : QObject(parent),
      m_settings(QSettings::IniFormat, QSettings::UserScope, "MMDAI", "VPVM")
{
}

Preference::~Preference()
{
}

void Preference::sync()
{
    m_settings.sync();
}

void Preference::clear()
{
    m_settings.clear();
}

QString Preference::initializeLoggingDirectory()
{
    QDir dir(baseLoggingDirectory().toLocalFile());
    const QString &suffix = loggingDirectorySuffix();
    if (!dir.exists(suffix)) {
        dir.mkpath(suffix);
    }
    return dir.absoluteFilePath(suffix);
}

QRect Preference::windowRect() const
{
    if (m_settings.contains("windowRect")) {
        return m_settings.value("windowRect").toRect();
    }
    else {
        const QSize windowSize(960, 620), &margin = (qApp->primaryScreen()->availableSize() - windowSize) / 2;
        const QPoint windowPosition(margin.width(), margin.height());
        return QRect(windowPosition, windowSize);
    }
}

void Preference::setWindowRect(const QRect &value)
{
    if (value != windowRect()) {
        m_settings.setValue("windowRect", value);
        emit windowRectChanged();
    }
}

QString Preference::fontFamily() const
{
#if defined(Q_OS_MACX)
    static const QString fontFamily = "Osaka";
#elif defined(Q_OS_WIN32)
    static const QString fontFamily = "Meiryo";
#else
    static const QString fontFamily;
#endif
    return m_settings.value("fontFamily", fontFamily).toString();
}

void Preference::setFontFamily(const QString &value)
{
    if (value != fontFamily()) {
        m_settings.setValue("fontFamily", value);
        emit fontFamilyChanged();
    }
}

QUrl Preference::baseLoggingDirectory() const
{
#ifndef QT_NO_DEBUG
    static const QUrl kDefaultLoggingDirectory = QUrl::fromLocalFile(QCoreApplication::applicationDirPath());
#else
    static const QUrl kDefaultLoggingDirectory = QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
#endif
    return m_settings.value("baseLoggingDirectory", kDefaultLoggingDirectory).toUrl();
}

void Preference::setBaseLoggingDirectory(const QUrl &value)
{
    if (value != baseLoggingDirectory()) {
        m_settings.setValue("baseLoggingDirectory", value);
        emit baseLoggingDirectoryChanged();
    }
}

QString Preference::loggingDirectorySuffix() const
{
    return m_settings.value("loggingDirectorySuffix", "log").toString();
}

void Preference::setLoggingDirectorySuffix(const QString &value)
{
    if (value != loggingDirectorySuffix()) {
        m_settings.setValue("loggingDirectorySuffix", value);
        emit loggingDirectorySuffixChanged();
    }
}

int Preference::verboseLogLevel() const
{
#ifndef QT_NO_DEBUG
    static int kDefaultVerboseLogLevel = 2;
#else
    static int kDefaultVerboseLogLevel = 1;
#endif
    return m_settings.value("verboseLogLevel", kDefaultVerboseLogLevel).toInt();
}

void Preference::setVerboseLogLevel(int value)
{
    if (value != verboseLogLevel()) {
        m_settings.setValue("verboseLogLevel", value);
        emit verboseLogLevelChanged();
    }
}

int Preference::samples() const
{
    return qMax(m_settings.value("samples", 4).toInt(), 0);
}

void Preference::setSamples(int value)
{
    if (value != samples()) {
        m_settings.setValue("samples", value);
        emit samplesChanged();
    }
}

bool Preference::isFontFamilyToGUIShared() const
{
    return m_settings.value("fontFamilyToGUIShared", false).toBool();
}

void Preference::setFontFamilyToGUIShared(bool value)
{
    if (value != isFontFamilyToGUIShared()) {
        m_settings.setValue("fontFamilyToGUIShared", value);
        emit fontFamilyToGUISharedChanged();
    }
}

bool Preference::isTransparentWindowEnabled() const
{
    return m_settings.value("transparentWindow", false).toBool();
}

void Preference::setTransparentWindowEnabled(bool value)
{
    if (value != isTransparentWindowEnabled()) {
        m_settings.setValue("transparentWindow", value);
        emit transparentWindowEnabledChanged();
    }
}
