#ifndef QMAOPENJTALKMODEL_H
#define QMAOPENJTALKMODEL_H

#include <QObject>
#include <QIODevice>
#include <QList>
#include <QHash>
#include <QString>

#include <mecab.h>
#include <njd.h>
#include <jpcommon.h>
#include <HTS_engine.h>

class QMAOpenJTalkModel : public QObject
{
  Q_OBJECT
public:
  explicit QMAOpenJTalkModel(QObject *parent = 0);
  ~QMAOpenJTalkModel();

  void loadSetting(const QString &path, const QString &config);
  void loadDictionary(const QString &mecab);
  void setText(const QString &text);
  void setStyle(const QString &style);
  const int getDuration() const;
  const QString getPhonemeSequence();
  QByteArray finalize(bool withHeader);

  Mecab m_mecab;
  NJD m_njd;
  JPCommon m_jpcommon;
  HTS_Engine m_engine;
  QList<double> m_weights;
  QList<QString> m_models;
  QHash<QString, int> m_styles;
  int m_duration;
  double m_f0Shift;
};

#endif // QMAOPENJTALKMODEL_H
