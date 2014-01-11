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

#ifndef LOGGINGTHREAD_H_
#define LOGGINGTHREAD_H_

#include <QDebug>
#include <QDir>
#include <QMutex>
#include <QQueue>
#include <QRunnable>
#include <QWaitCondition>

class LoggingThread : public QRunnable {
public:
    static const QString &toString(QtMsgType type) {
        switch (type) {
        case QtDebugMsg: {
            static QString s("info");
            return s;
        }
        case QtWarningMsg: {
            static QString s("warning");
            return s;
        }
        case QtCriticalMsg: {
            static QString s("error");
            return s;
        }
        case QtFatalMsg: {
            static QString s("fatal");
            return s;
        }
        default: {
            static QString s("unknown");
            return s;
        }
        }
    }
    static void delegateMessage(QtMsgType type, const QMessageLogContext &context, const QString &message);

    LoggingThread()
        : m_active(true)
    {
        setAutoDelete(false);
    }
    ~LoggingThread() {
    }

    void post(const QString &message) {
        QMutexLocker locker(&m_mutex); Q_UNUSED(locker);
        m_queue.enqueue(message);
        m_cond.wakeAll();
    }
    void stop() {
        QMutexLocker locker(&m_mutex); Q_UNUSED(locker);
        m_active = false;
        m_cond.wakeAll();
    }
    void setDirectory(const QString &value) {
        m_directory = value;
    }

private:
    void run() {
        static const QString commitRevision = QString(vpvl2::libraryCommitRevisionString()).split(" ").at(1);
        QStringList stringList;
        stringList.reserve(4);
        qInstallMessageHandler(&LoggingThread::delegateMessage);
        qDebug("libvpvl2: version=%s commit=%s", vpvl2::libraryVersionString(), qPrintable(commitRevision));
        while (m_active) {
            QMutexLocker locker(&m_mutex); Q_UNUSED(locker);
            m_cond.wait(&m_mutex);
            const QDateTime &currentDateTime = QDateTime::currentDateTime();
            QFile f(m_directory.absoluteFilePath("%1.log").arg(currentDateTime.toString("yyyyMMdd")));
            f.open(QFile::Append);
            while (!m_queue.isEmpty()) {
                const QString &message = m_queue.dequeue();
                stringList.clear();
                stringList << currentDateTime.toString(Qt::ISODate);
                stringList << " ";
                stringList << message;
                stringList << "\n";
                f.write(stringList.join("").toUtf8());
#if !defined(QT_NO_DEBUG)
                fprintf(stderr, "%s", stringList.join("").toUtf8().constData());
#endif
            }
        }
        qInstallMessageHandler(0);
    }

    QMutex m_mutex;
    QWaitCondition m_cond;
    QQueue<QString> m_queue;
    QDir m_directory;
    volatile bool m_active;
} g_loggingThread;

void LoggingThread::delegateMessage(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    Q_UNUSED(type);
    Q_UNUSED(context);
#if !defined(QT_NO_DEBUG)
    /* supress warnings of Invalid font from Context2D */
    if (message.startsWith("Context2D:")) {
        return;
    }
#endif
    g_loggingThread.post(QString("[%1] %4 in %2 at line %3").arg(toString(type)).arg(context.function).arg(context.line).arg(message));
}

#endif
