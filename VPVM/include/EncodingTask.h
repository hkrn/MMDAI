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

#pragma once
#ifndef ENCODINGTASK_H_
#define ENCODINGTASK_H_

#include <QDir>
#include <QObject>
#include <QProcess>
#include <QSize>

class QOpenGLFramebufferObject;
class QQuickWindow;
class QTemporaryDir;
class QTemporaryFile;

class EncodingTask : public QObject
{
    Q_OBJECT

public:
    EncodingTask(QObject *parent = 0);
    ~EncodingTask();

    bool isRunning() const;
    void setSize(const QSize &value);
    void setTitle(const QString &value);
    void setInputImageFormat(const QString &value);
    void setOutputPath(const QString &value);
    void setOutputFormat(const QString &value);
    void setEstimatedFrameCount(const qint64 value);

    void reset();
    QOpenGLFramebufferObject *generateFramebufferObject(QQuickWindow *win);
    QString generateFilename(const qreal &timeIndex);

    void stop();
    void release();

    void handleStarted();
    void handleReadyRead();
    void handleStateChanged();
    void launch();

signals:
    void encodeDidBegin();
    void encodeDidProceed(quint64 proceed, quint64 estimated);
    void encodeDidFinish(bool isNormalExit);

private:
    void getArguments(QStringList &arguments);

    QScopedPointer<QProcess> m_process;
    QScopedPointer<QOpenGLFramebufferObject> m_fbo;
    QScopedPointer<QTemporaryDir> m_workerDir;
    QScopedPointer<QTemporaryFile> m_executable;
    QProcess::ProcessState m_lastState;
    QDir m_workerDirPath;
    QString m_workerId;
    QSize m_size;
    QString m_title;
    QString m_inputPath;
    QString m_outputPath;
    QString m_inputImageFormat;
    QString m_outputFormat;
    QString m_pixelFormat;
    quint64 m_estimatedFrameCount;
};

#endif
