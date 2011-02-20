#include "QMALogger.h"

#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QTextCodec>
#include <QTextStream>
#include <MMDME/Common.h>

namespace {
  QString g_text;
  QTextStream g_output(&g_text, QIODevice::WriteOnly);
}

static void LogHandler(const char *file, int line, enum MMDAILogLevel level, const char *format, va_list ap)
{
  switch (level) {
  case MMDAILogLevelDebug:
    g_output << "[DEBUG]";
    break;
  case MMDAILogLevelInfo:
    g_output << "[INFO]";
    break;
  case MMDAILogLevelWarning:
    g_output << "[WARNING]";
    break;
  case MMDAILogLevelError:
    g_output << "[ERROR]";
    break;
  }
  g_output << " " << QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
  QString name = QString(file).split("/").last();
  g_output << " " << name << ":" << line << " ";
  char buf[BUFSIZ];
  qvsnprintf(buf, sizeof(buf), format, ap);
  if (name == "PMDBone.cc"
      || name == "PMDFace.cc"
      || name == "PMDModel_parse.cc"
      || name == "PMDRigidBody.cc") {
    QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
    g_output << codec->toUnicode(QByteArray(buf));
  }
  else {
    g_output << buf;
  }
  g_output << "\n";
}

void QMALogger::initialize()
{
  static volatile bool initialized = false;
  if (!initialized) {
    MMDAILogSetHandler(LogHandler);
    initialized = true;
  }
}

const QString &QMALogger::getText()
{
  return g_text;
}
