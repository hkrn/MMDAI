#ifndef QMALIPSYNCLODER_H
#define QMALIPSYNCLODER_H

#include <QFile>
#include <QList>
#include <QString>

#include "MMDAI/LipSyncLoader.h"

class QMALipSyncLoder : public LipSyncLoader
{
public:
  QMALipSyncLoder(const char *filename);
  ~QMALipSyncLoder();

  bool load();
  int getNExpressions();
  const char *getExpressionName(int i);
  int getNPhonemes();
  const char *getPhoneName(int i);
  float getInterpolationWeight(int i, int j);

private:
  QList<char *> *m_expressionNames;
  QList<char *> *m_phoneNames;
  QList<float> *m_interpolation;
  QFile *m_file;
  int m_nexpressions;
  int m_nphonemes;
};

#endif // QMALIPSYNCLODER_H
