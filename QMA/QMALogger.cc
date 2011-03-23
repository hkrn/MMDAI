/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn (libMMDAI)                         */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#include "QMALogger.h"

#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QTextCodec>
#include <QTextStream>
#include <MMDME/Common.h>

#ifndef NDEBUG
#include <QDebug>
#endif

namespace {
  QMALogger *g_instance = NULL;
}

static const QString LogGetLabel(const enum MMDAILogLevel level)
{
  switch (level) {
  case MMDAILogLevelDebug:
    return "[DEBUG]";
  case MMDAILogLevelInfo:
    return "[INFO]";
  case MMDAILogLevelWarning:
    return "[WARNING]";
  case MMDAILogLevelError:
    return "[ERROR]";
  default:
    return "[UNKNOWN]";
  }
}

static void LogHandlerSJIS(const char *file,
                           const int line,
                           const enum MMDAILogLevel level,
                           const char *format,
                           va_list ap)
{
  char buf[BUFSIZ];
  vsnprintf(buf, sizeof(buf), format, ap);
  QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
  QString message = codec->toUnicode(buf, strlen(buf));
  QString name = QString(file).split("/").last();
  QString text = QString("%1 %2 %3:%4 %5\n")
                 .arg(LogGetLabel(level))
                 .arg(QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss"))
                 .arg(name)
                 .arg(line)
                 .arg(message);
  QMALogger::getInstance()->sendLineWritten(text);
#ifndef NDEBUG
  qDebug() << text;
#endif
}

static void LogHandler(const char *file,
                       const int line,
                       const enum MMDAILogLevel level,
                       const char *format,
                       va_list ap)
{
  char buf[BUFSIZ];
  vsnprintf(buf, sizeof(buf), format, ap);
  QString message = QString(buf);
  QString name = QString(file).split("/").last();
  QString text = QString("%1 %2 %3:%4 %5\n")
                 .arg(LogGetLabel(level))
                 .arg(QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss"))
                 .arg(name)
                 .arg(line)
                 .arg(message);
  QMALogger::getInstance()->sendLineWritten(text);
#ifndef NDEBUG
  qDebug() << text;
#endif
}

QMALogger::QMALogger() : QObject()
{
}

QMALogger::~QMALogger()
{
}

void QMALogger::sendLineWritten(const QString &line)
{
  emit lineWritten(line);
}

void QMALogger::initialize()
{
  if (!g_instance) {
    MMDAILogSetHandler(LogHandler);
    MMDAILogSetHandlerSJIS(LogHandlerSJIS);
    g_instance = new QMALogger();
  }
}

QMALogger *QMALogger::getInstance()
{
  return g_instance;
}
