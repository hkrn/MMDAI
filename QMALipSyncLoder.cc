#include "QMALipSyncLoder.h"

#include <QDir>
#include <QRegExp>
#include <QStringList>
#include <QTextCodec>
#include <QTextStream>

QMALipSyncLoder::QMALipSyncLoder(const char *filename)
  : m_expressionNames(0),
  m_phoneNames(0),
  m_interpolation(0),
  m_nexpressions(0),
  m_nphonemes(0)
{
  QString path = QFile::decodeName(filename).replace(QChar(0xa5), QChar('/')).replace(QRegExp(".pmd$"), ".lip");
  if (QDir::isAbsolutePath(path))
    m_file = new QFile(path);
  else
    m_file = new QFile("mmdai:" + path);
}

QMALipSyncLoder::~QMALipSyncLoder()
{
  int count = 0;
  if (m_expressionNames != NULL) {
    count = m_expressionNames->length();
    for (int i = 0; i < count; i++) {
      char *s = m_expressionNames->at(i);
      delete[] s;
    }
    delete m_expressionNames;
  }
  if (m_phoneNames != NULL) {
    count = m_phoneNames->length();
    for (int i = 0; i < count; i++) {
      char *s = m_phoneNames->at(i);
      delete[] s;
    }
    delete m_phoneNames;
  }
  delete m_interpolation;
  delete m_file;
}

bool QMALipSyncLoder::load()
{
  enum QMALipSyncLoaderState {
    GetNExpressions,
    GetExpressionNames,
    GetPhoneNames
  } state = GetNExpressions;
  bool ret = false;
  if (m_file->open(QFile::ReadOnly)) {
    QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
    QTextStream stream(m_file);
    QString line;
    stream.setCodec("Shift-JIS");
    int i = 0;
    do {
      line = stream.readLine().trimmed();
      if (!line.isEmpty() && line.at(0) != '#') {
        switch (state) {
        case GetNExpressions:
          m_nexpressions = line.toInt();
          m_expressionNames = new QList<char *>();
          state = GetExpressionNames;
          break;
        case GetExpressionNames:
          if (i < m_nexpressions) {
            QByteArray data = codec->fromUnicode(line);
            int len = data.length();
            char *s = new char[len];
            memcpy(s, data.constData(), len);
            m_expressionNames->append(s);
            i++;
          }
          else {
            m_nphonemes = line.toInt();
            m_phoneNames = new QList<char *>();
            m_interpolation = new QList<float>();
            state = GetPhoneNames;
            i = 0;
          }
          break;
        case GetPhoneNames:
          if (i < m_nphonemes) {
            QStringList a = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
            if (a.count() == 1 + m_nexpressions) {
              QByteArray data = a.at(0).toUtf8();
              int len = data.length();
              char *s = new char[len];
              memcpy(s, data.constData(), len);
              m_phoneNames->append(s);
              for (int j = 0; j < m_nexpressions; j++) {
                float f = a.at(j + 1).toFloat();
                m_interpolation->append(f);
              }
              i++;
            }
            if (i == m_nphonemes) {
              i = 0;
              ret = true;
            }
          }
          break;
        }
      }
    } while (!line.isNull());
  }
  m_file->close();
  return ret;
}

int QMALipSyncLoder::getNExpressions()
{
  return m_nexpressions;
}

const char *QMALipSyncLoder::getExpressionName(int i)
{
  if (i >= 0 && i < m_nexpressions) {
    return m_expressionNames->at(i);
  }
  return NULL;
}

int QMALipSyncLoder::getNPhonemes()
{
  return m_nphonemes;
}

const char *QMALipSyncLoder::getPhoneName(int i)
{
  if (i >= 0 && i < m_nphonemes) {
    return m_phoneNames->at(i);
  }
  return NULL;
}

float QMALipSyncLoder::getInterpolationWeight(int i, int j)
{
  int ne = m_nexpressions;
  int np = m_nphonemes;
  if (i >= 0 && i < np && j >= 0 && j < ne) {
    return m_interpolation->at((i * m_nexpressions) + j);
  }
  return 0.0;
}
