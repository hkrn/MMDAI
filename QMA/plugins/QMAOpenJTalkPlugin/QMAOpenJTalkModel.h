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
  static const float kMinLF0Val;
  static const float kHanfTone;
  static const float kMaxHanfTone;
  static const float kMinHanfTone;
  static const float kAlpha;
  static const float kMaxAlpha;
  static const float kMinAlpha;
  static const float kVolume;
  static const float kMaxVolume;
  static const float kMinVolume;
  static const bool kLogGain;
  static const int kGamma = 0;
  static const int kSamplingRate = 48000;
  static const int kFPeriod = 240;
  static const int kMaxFPeriod = 48000;
  static const int kMinFPeriod = 1;

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
