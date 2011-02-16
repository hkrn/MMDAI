#include "QMALogger.h"

#include <QtGui>
#include <MMDME/Common.h>

static void LogHandler(const char *file, int line, enum MMDAILogLevel level, const char *format, va_list ap)
{
  char buf[BUFSIZ];
  const char *ls;
  qvsnprintf(buf, sizeof(buf), format, ap);
  switch (level) {
  case MMDAILogLevelDebug:
    ls = "[DEBUG]";
    break;
  case MMDAILogLevelInfo:
    ls = "[INFO]";
    break;
  case MMDAILogLevelWarning:
    ls = "[WARNING]";
    break;
  case MMDAILogLevelError:
    ls = "[ERROR]";
    break;
  }
  QTextCodec *codec = QTextCodec::codecForName("UTF-8");
  qDebug().nospace() << ls << " " << file << ":" << line << " " << codec->toUnicode(buf, strlen(buf));
}

QMALogger::QMALogger()
{
}

QMALogger::~QMALogger()
{
}

void QMALogger::initialize()
{
  MMDAILogSetHandler(LogHandler);
}
