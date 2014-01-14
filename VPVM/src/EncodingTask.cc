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

#include "ApplicationContext.h"
#include "EncodingTask.h"

#include <QtCore>
#include <vpvl2/vpvl2.h>

using namespace vpvl2;

EncodingTask::EncodingTask(QObject *parent)
    : QObject(parent),
      m_lastState(QProcess::NotRunning),
      m_estimatedFrameCount(0)
{
}

EncodingTask::~EncodingTask()
{
    stop();
}

bool EncodingTask::isRunning() const
{
    return m_process && m_process->state() == QProcess::Running;
}

void EncodingTask::setSize(const QSize &value)
{
    m_size = value;
}

void EncodingTask::setTitle(const QString &value)
{
    m_title = value;
}

void EncodingTask::setInputImageFormat(const QString &value)
{
    if (!value.isEmpty()) {
        m_inputImageFormat = value;
    }
}

void EncodingTask::setOutputPath(const QString &value)
{
    m_outputPath = value;
}

void EncodingTask::setOutputFormat(const QString &value)
{
    const QStringList &format = value.split(":");
    if (format.size() == 2) {
        m_outputFormat = format.at(0);
        m_pixelFormat = format.at(1);
    }
}

void EncodingTask::setEstimatedFrameCount(const qint64 value)
{
    m_estimatedFrameCount = value;
}

void EncodingTask::reset()
{
    m_workerId = QUuid::createUuid().toByteArray().toHex();
    m_workerDir.reset(new QTemporaryDir());
    m_workerDirPath = m_workerDir->path();
    m_inputImageFormat = "bmp";
    m_outputFormat = "png";
    m_pixelFormat = "rgb24";
    m_fbo.reset();
}

QOpenGLFramebufferObject *EncodingTask::generateFramebufferObject(QQuickWindow *win)
{
    if (!m_fbo) {
        m_fbo.reset(new QOpenGLFramebufferObject(m_size, ApplicationContext::framebufferObjectFormat(win)));
        Q_ASSERT(m_fbo->isValid());
    }
    return m_fbo.data();
}

QString EncodingTask::generateFilename(const qreal &timeIndex)
{
    const QString &filename = QStringLiteral("%1-%2.%3")
            .arg(m_workerId)
            .arg(qRound64(timeIndex), 9, 10, QLatin1Char('0'))
            .arg(m_inputImageFormat);
    const QString &path = m_workerDirPath.absoluteFilePath(filename);
    return path;
}

void EncodingTask::stop()
{
    if (isRunning()) {
        m_process->kill();
        VPVL2_LOG(INFO, "Tried killing encode process " << m_process->pid());
        m_process->waitForFinished(5000);
        if (isRunning()) {
            m_process->terminate();
            VPVL2_LOG(INFO, "Tried terminating encode process " << m_process->pid());
            m_process->waitForFinished(5000);
        }
        if (isRunning()) {
            VPVL2_LOG(WARNING, "Cannot stop process: error=" << m_process->error());
        }
    }
}

void EncodingTask::release()
{
    QFile::remove(m_encoderFilePath);
    m_process.reset();
    m_workerDir.reset();
    m_fbo.reset();
    m_estimatedFrameCount = 0;
}

void EncodingTask::handleStarted()
{
    emit encodeDidBegin();
    VPVL2_VLOG(1, "Started encoding task");
}

void EncodingTask::handleReadyRead()
{
    static const QRegExp regexp("^frame\\s*=\\s*(\\d+)");
    const QByteArray &output = m_process->readAll();
    VPVL2_VLOG(2, output.constData());
    if (regexp.indexIn(output) >= 0) {
        quint64 proceeded = regexp.cap(1).toLongLong();
        emit encodeDidProceed(proceeded, m_estimatedFrameCount);
    }
}

void EncodingTask::handleStateChanged()
{
    QProcess::ProcessState state = m_process->state();
    if (m_lastState != state) {
        if (m_lastState == QProcess::Starting && state == QProcess::Running) {
            QFile::remove(m_encoderFilePath);
        }
        else if (m_lastState == QProcess::Running && state == QProcess::NotRunning) {
            QProcess::ExitStatus status = m_process->exitStatus();
            VPVL2_VLOG(1, "Finished encoding task: code=" << m_process->exitCode() << " status=" << status);
            QFile::remove(m_encoderFilePath);
            m_workerDir.reset();
            m_estimatedFrameCount = 0;
            emit encodeDidFinish(status == QProcess::NormalExit);
        }
    }
    m_lastState = state;
}

void EncodingTask::handleError(QProcess::ProcessError error)
{
    QFile::remove(m_encoderFilePath);
    VPVL2_LOG(ERROR, "Error happened at encoding: error=" << error << " message=" << m_process->errorString().toStdString());
    emit encodeDidFinish(false);
}

void EncodingTask::launch()
{
    stop();
    QStringList arguments;
    QScopedPointer<QTemporaryFile> file(new QTemporaryFile());
    file->open();
    m_encoderFilePath = file->fileName();
    file.reset();
    if (QFile::copy(":libav/avconv", m_encoderFilePath)) {
        qDebug() << QFile::permissions(m_encoderFilePath);
        QFile::setPermissions(m_encoderFilePath, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
        getArguments(arguments);
        m_process.reset(new QProcess(this));
        m_process->setArguments(arguments);
        m_process->setProgram(m_encoderFilePath);
        m_process->setProcessChannelMode(QProcess::MergedChannels);
        m_process->setWorkingDirectory(m_workerDir->path());
        /* disable color output from standard output */
        QStringList environments = m_process->environment();
        environments << "AV_LOG_FORCE_NOCOLOR" << "1";
        m_process->setEnvironment(environments);
        connect(m_process.data(), &QProcess::started, this, &EncodingTask::handleStarted);
        connect(m_process.data(), &QProcess::readyRead, this, &EncodingTask::handleReadyRead);
        connect(m_process.data(), &QProcess::stateChanged, this, &EncodingTask::handleStateChanged);
        connect(m_process.data(), SIGNAL(error(QProcess::ProcessError)), this, SLOT(handleError(QProcess::ProcessError)));
        m_process->start();
        VPVL2_VLOG(1, "executable=" << m_process->program().toStdString() << " arguments=" << arguments.join(" ").toStdString());
        VPVL2_VLOG(2, "Waiting for starting encoding task");
    }
    else {
        VPVL2_LOG(ERROR, "Cannot start encoder due to failed copying");
        emit encodeDidFinish(false);
    }
}

void EncodingTask::getArguments(QStringList &arguments)
{
#ifndef QT_NO_DEBUG
    arguments.append("-v");
    arguments.append("debug");
#endif
    arguments.append("-r");
    arguments.append(QStringLiteral("%1").arg(30));
    arguments.append("-s");
    arguments.append(QStringLiteral("%1x%2").arg(m_size.width()).arg(m_size.height()));
    arguments.append("-qscale");
    arguments.append("1");
    arguments.append("-vcodec");
    arguments.append(m_inputImageFormat);
    arguments.append("-metadata");
    arguments.append(QStringLiteral("title=\"%1\"").arg(m_title));
    arguments.append("-i");
    arguments.append(m_workerDirPath.absoluteFilePath(QStringLiteral("%1-%09d.%2").arg(m_workerId).arg(m_inputImageFormat)));
    arguments.append("-map");
    arguments.append("0");
    arguments.append("-c:v");
    arguments.append(m_outputFormat);
    arguments.append("-pix_fmt:v");
    arguments.append(m_pixelFormat);
    arguments.append("-y");
    arguments.append(m_outputPath);
}
