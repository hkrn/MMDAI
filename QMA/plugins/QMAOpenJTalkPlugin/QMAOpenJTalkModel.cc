#include "QMAOpenJTalkModel.h"

#include <QByteArray>
#include <QBuffer>
#include <QFile>
#include <QStringList>
#include <QTextCodec>
#include <QTextStream>

#include <math.h>

#include "text2mecab.h"
#include "mecab2njd.h"
#include "njd2jpcommon.h"

#include "njd_set_pronunciation.h"
#include "njd_set_digit.h"
#include "njd_set_accent_phrase.h"
#include "njd_set_accent_type.h"
#include "njd_set_unvoiced_vowel.h"
#include "njd_set_long_vowel.h"

#define OPENJTALK_MAXBUFLEN     2048
#define OPENJTALK_MINLF0VAL     log(10.0)

#define OPENJTALK_GAMMA         0
#define OPENJTALK_LOGGAIN       false
#define OPENJTALK_SAMPLINGRATE  48000
#define OPENJTALK_FPERIOD       240
#define OPENJTALK_HALFTONE      0.0
#define OPENJTALK_ALPHA         0.55
#define OPENJTALK_VOLUME        1.0
#define OPENJTALK_AUDIOBUFFSIZE 4800

#define OPENJTALK_MAXFPERIOD  48000
#define OPENJTALK_MINFPERIOD  1
#define OPENJTALK_MAXHALFTONE 24.0
#define OPENJTALK_MINHALFTONE -24.0
#define OPENJTALK_MAXALPHA    1.0
#define OPENJTALK_MINALPHA    0.0
#define OPENJTALK_MAXVOLUME   10.0
#define OPENJTALK_MINVOLUME   0.0

QMAOpenJTalkModel::QMAOpenJTalkModel(QObject *parent) :
    QObject(parent),
    m_f0Shift(0.0)
{
  Mecab_initialize(&m_mecab);
  NJD_initialize(&m_njd);
  JPCommon_initialize(&m_jpcommon);
  HTS_Engine_initialize(&m_engine, 3);
  HTS_Engine_set_gamma(&m_engine, OPENJTALK_GAMMA);
  HTS_Engine_set_log_gain(&m_engine, OPENJTALK_LOGGAIN);
  HTS_Engine_set_sampling_rate(&m_engine, OPENJTALK_SAMPLINGRATE);
  HTS_Engine_set_fperiod(&m_engine, OPENJTALK_FPERIOD);
  HTS_Engine_set_alpha(&m_engine, OPENJTALK_ALPHA);
  HTS_Engine_set_volume(&m_engine, OPENJTALK_VOLUME);
  HTS_Engine_set_audio_buff_size(&m_engine, 0); /* disable direct audio output */
  m_f0Shift = OPENJTALK_HALFTONE;
}

QMAOpenJTalkModel::~QMAOpenJTalkModel()
{
  Mecab_clear(&m_mecab);
  NJD_clear(&m_njd);
  JPCommon_clear(&m_jpcommon);
  HTS_Engine_clear(&m_engine);
}

void QMAOpenJTalkModel::loadSetting(const QString &path, const QString &config)
{
  QFile file(config);
  if (file.open(QFile::ReadOnly)) {
    QTextStream stream(&file);
    int nmodels = stream.readLine().toInt();
    for (int i = 0; i < nmodels; i++) {
      QString model = stream.readLine();
      if (model.isEmpty())
        break;
      model = model.trimmed();
      if (model[0] == '#')
        continue;
      m_models.append(path + "/" + model);
    }
    int nstyles = stream.readLine().toInt();
    for (int i = 0; i < nstyles; i++) {
      QString style = stream.readLine();;
      if (style.isEmpty())
        break;
      style = style.trimmed();
      if (style[0] == '#')
        continue;
      QStringList weights = style.split(QRegExp("\\s"), QString::SkipEmptyParts);
      QString name = weights[0];
      weights.removeFirst();
      int nweights = weights.count();
      int expected = 3 * nmodels + 4;
      if (nweights != expected)
        break;
      m_styles[name] = i;
      for (int j = 0; j < nweights; j++) {
        m_weights.append(weights[j].toDouble());
      }
    }
    file.close();
  }
}

void QMAOpenJTalkModel::loadDictionary(const QString &mecab)
{
  int nmodels = m_models.count();
  if (nmodels == 0)
    return;

  char *dic = strdup(mecab.toUtf8().constData());
  Mecab_load(&m_mecab, dic);
  free(dic);

  char **fn_ws_mcp = static_cast<char **>(calloc(3, sizeof(char *)));
  char **fn_ws_lf0 = static_cast<char **>(calloc(3, sizeof(char *)));
  char **fn_ws_lpf = static_cast<char **>(calloc(1, sizeof(char *)));
  char **fn_ts_dur = static_cast<char **>(calloc(nmodels, sizeof(char *)));
  char **fn_ts_mcp = static_cast<char **>(calloc(nmodels, sizeof(char *)));
  char **fn_ts_lf0 = static_cast<char **>(calloc(nmodels, sizeof(char *)));
  char **fn_ts_lpf = static_cast<char **>(calloc(nmodels, sizeof(char *)));
  char **fn_ms_dur = static_cast<char **>(calloc(nmodels, sizeof(char *)));
  char **fn_ms_mcp = static_cast<char **>(calloc(nmodels, sizeof(char *)));
  char **fn_ms_lf0 = static_cast<char **>(calloc(nmodels, sizeof(char *)));
  char **fn_ms_lpf = static_cast<char **>(calloc(nmodels, sizeof(char *)));
  char **fn_ts_gvm = static_cast<char **>(calloc(nmodels, sizeof(char *)));
  char **fn_ts_gvl = static_cast<char **>(calloc(nmodels, sizeof(char *)));
  char **fn_ms_gvm = static_cast<char **>(calloc(nmodels, sizeof(char *)));
  char **fn_ms_gvl = static_cast<char **>(calloc(nmodels, sizeof(char *)));

  QString model = m_models[0];
  QString mgc1 = model + "/mgc.win1";
  fn_ws_mcp[0] = strdup(mgc1.toUtf8().constData());
  QString mgc2 = model + "/mgc.win2";
  fn_ws_mcp[1] = strdup(mgc2.toUtf8().constData());
  QString mgc3 = model + "/mgc.win3";
  fn_ws_mcp[2] = strdup(mgc3.toUtf8().constData());
  QString lf01 = model + "/lf0.win1";
  fn_ws_lf0[0] = strdup(lf01.toUtf8().constData());
  QString lf02 = model + "/lf0.win2";
  fn_ws_lf0[1] = strdup(lf02.toUtf8().constData());
  QString lf03 = model + "/lf0.win3";
  fn_ws_lf0[2] = strdup(lf03.toUtf8().constData());
  QString lpf = model + "/lpf.win1";
  fn_ws_lpf[0] = strdup(lpf.toUtf8().constData());
  QString gvSwitch = model + "/gv-switch.inf";
  char *fn_gv_switch = strdup(gvSwitch.toUtf8().constData());

  for (int i = 0; i < nmodels; i++) {
    model = m_models[i];
    QString treeDur = model + "/tree-dur.inf";
    fn_ts_dur[i] = strdup(treeDur.toUtf8().constData());
    QString treeMgc = model + "/tree-mgc.inf";
    fn_ts_mcp[i] = strdup(treeMgc.toUtf8().constData());
    QString treeLf0 = model + "/tree-lf0.inf";
    fn_ts_lf0[i] = strdup(treeLf0.toUtf8().constData());
    QString treeLpf = model + "/tree-lpf.inf";
    fn_ts_lpf[i] = strdup(treeLpf.toUtf8().constData());
    QString dur = model + "/dur.pdf";
    fn_ms_dur[i] = strdup(dur.toUtf8().constData());
    QString mgc = model + "/mgc.pdf";
    fn_ms_mcp[i] = strdup(mgc.toUtf8().constData());
    QString lf0 = model + "/lf0.pdf";
    fn_ms_lf0[i] = strdup(lf0.toUtf8().constData());
    QString lpf = model + "/lpf.pdf";
    fn_ms_lpf[i] = strdup(lpf.toUtf8().constData());
    QString treeGvMgc = model + "/tree-gv-mgc.inf";
    fn_ts_gvm[i] = strdup(treeGvMgc.toUtf8().constData());
    QString treeGvLf0 = model + "/tree-gv-lf0.inf";
    fn_ts_gvl[i] = strdup(treeGvLf0.toUtf8().constData());
    QString gvMgc = model + "/gv-mgc.pdf";
    fn_ms_gvm[i] = strdup(gvMgc.toUtf8().constData());
    QString gvLf0 = model + "/gv-lf0.pdf";
    fn_ms_gvl[i] = strdup(gvLf0.toUtf8().constData());
  }

  HTS_Engine_load_duration_from_fn(&m_engine, fn_ms_dur, fn_ts_dur, nmodels);
  HTS_Engine_load_parameter_from_fn(&m_engine, fn_ms_mcp, fn_ts_mcp, fn_ws_mcp, 0, /* FALSE */ 0, 3, nmodels);
  HTS_Engine_load_parameter_from_fn(&m_engine, fn_ms_lf0, fn_ts_lf0, fn_ws_lf0, 1, /* TRUE */  1, 3, nmodels);
  HTS_Engine_load_parameter_from_fn(&m_engine, fn_ms_lpf, fn_ts_lpf, fn_ws_lpf, 2, /* FALSE */ 0, 1, nmodels);
  HTS_Engine_load_gv_from_fn(&m_engine, fn_ms_gvm, fn_ts_gvm, 0, nmodels);
  HTS_Engine_load_gv_from_fn(&m_engine, fn_ms_gvl, fn_ts_gvl, 1, nmodels);
  HTS_Engine_load_gv_switch_from_fn(&m_engine, fn_gv_switch);

  for (int i = 0; i < 3; i++) {
    free(fn_ws_mcp[i]);
    free(fn_ws_lf0[i]);
  }
  for (int i = 0; i < 1; i++) {
    free(fn_ws_lpf[i]);
  }
  free(fn_ws_mcp);
  free(fn_ws_lf0);
  free(fn_ws_lpf);
  free(fn_gv_switch);
  for (int i = 0; i < nmodels; i++) {
    free(fn_ts_dur[i]);
    free(fn_ts_mcp[i]);
    free(fn_ts_lf0[i]);
    free(fn_ts_lpf[i]);
    free(fn_ms_dur[i]);
    free(fn_ms_mcp[i]);
    free(fn_ms_lf0[i]);
    free(fn_ms_lpf[i]);
    free(fn_ts_gvm[i]);
    free(fn_ts_gvl[i]);
    free(fn_ms_gvm[i]);
    free(fn_ms_gvl[i]);
  }
  free(fn_ts_dur);
  free(fn_ts_mcp);
  free(fn_ts_lf0);
  free(fn_ts_lpf);
  free(fn_ms_dur);
  free(fn_ms_mcp);
  free(fn_ms_lf0);
  free(fn_ms_lpf);
  free(fn_ts_gvm);
  free(fn_ts_gvl);
  free(fn_ms_gvm);
  free(fn_ms_gvl);
}

QByteArray QMAOpenJTalkModel::finalize()
{
  QByteArray ret;
  if (JPCommon_get_label_size(&m_jpcommon) > 2) {
    QBuffer buffer(&ret);
    buffer.open(QBuffer::WriteOnly);
    HTS_Engine_create_gstream(&m_engine);
    HTS_GStreamSet *gss = &m_engine.gss;
    HTS_Global *global = &m_engine.global;
    int nsamples = HTS_GStreamSet_get_total_nsample(gss);
    for (int i = 0; i < nsamples; i++) {
      short sample = HTS_GStreamSet_get_speech(gss, i);
      buffer.write(reinterpret_cast<const char *>(&sample), sizeof(sample));
    }
    m_duration = ((double) HTS_PStreamSet_get_total_frame(&m_engine.pss) * global->fperiod / global->sampling_rate) * 1000;
    HTS_Engine_refresh(&m_engine);
    buffer.close();
  }
  JPCommon_refresh(&m_jpcommon);
  NJD_refresh(&m_njd);
  Mecab_refresh(&m_mecab);
  return ret;
}

void QMAOpenJTalkModel::setText(const QString &text)
{
  HTS_Engine_set_stop_flag(&m_engine, false);

  char *buff = static_cast<char *>(calloc(2 * text.length() + 1, sizeof(char)));
  QTextCodec *codec= QTextCodec::codecForName("Shift-JIS");
  QByteArray bytes = codec->fromUnicode(text);
  char *str = strdup(bytes.constData());
  if (buff == NULL || str == NULL) {
    free(buff);
    free(str);
    return;
  }
  text2mecab(buff, str);
  Mecab_analysis(&m_mecab, buff);
  free(buff);
  free(str);

  mecab2njd(&m_njd, Mecab_get_feature(&m_mecab), Mecab_get_size(&m_mecab));
  njd_set_pronunciation(&m_njd);
  njd_set_digit(&m_njd);
  njd_set_accent_phrase(&m_njd);
  njd_set_accent_type(&m_njd);
  njd_set_unvoiced_vowel(&m_njd);
  njd_set_long_vowel(&m_njd);
  njd2jpcommon(&m_jpcommon, &m_njd);
  JPCommon_make_label(&m_jpcommon);

  if (JPCommon_get_label_size(&m_jpcommon) > 2) {
    char **labelFeature = NULL;
    int labelSize = 0;
    /* decision of state durations */
    labelFeature = JPCommon_get_label_feature(&m_jpcommon);
    labelSize = JPCommon_get_label_size(&m_jpcommon);
    HTS_Engine_load_label_from_string_list(&m_engine, &labelFeature[1], labelSize - 1); /* skip first silence */
    HTS_Engine_create_sstream(&m_engine);
    /* parameter generation */
    if (m_f0Shift != 0.0) {
      for (int i = 0; i < HTS_Engine_get_total_state(&m_engine); i++) {
        double f = HTS_Engine_get_state_mean(&m_engine, 1, i, 0);
        f += m_f0Shift * log(2.0) / 12;
        if (f < OPENJTALK_MINLF0VAL)
          f = OPENJTALK_MINLF0VAL;
        HTS_Engine_set_state_mean(&m_engine, 1, i, 0, f);
      }
    }
    HTS_Engine_create_pstream(&m_engine);
  }
}

const int QMAOpenJTalkModel::getDuration() const
{
  return m_duration;
}

const QString QMAOpenJTalkModel::getPhonemeSequence()
{
  QString ret;
  int size = JPCommon_get_label_size(&m_jpcommon);
  char **feature = JPCommon_get_label_feature(&m_jpcommon);
  int nstate = HTS_Engine_get_nstate(&m_engine);
  int fperiod = HTS_Engine_get_fperiod(&m_engine);
  int sampling_rate = HTS_Engine_get_sampling_rate(&m_engine);

  if (size <= 2)
    return ret;

  /* skip first and final silence */
  size -= 2;
  feature = &feature[1];

  for (int i = 0; i < size; i++) {
    if (i > 0)
      ret += ",";
    /* get phoneme from full-context label */
    char *start = strchr(feature[i], '-');
    char *end = strchr(feature[i], '+');
    if (start != NULL && end != NULL) {
      for (char *ch = start + 1; ch != end; ch++)
        ret += QChar(*ch);
    } else {
      ret += feature[i];
    }
    /* get ms */
    int j = 0, k = 0;
    for (; j < nstate; j++)
      k += (HTS_Engine_get_state_duration(&m_engine, i * nstate + j) * fperiod * 1000) / sampling_rate;
    ret += QString(",%1").arg(k);
  }

  return ret;
}

void QMAOpenJTalkModel::setStyle(const QString &style)
{
  if (!m_styles.contains(style))
    return;

  int styleIndex = m_styles[style];
  int nmodels = m_models.count();
  int index = styleIndex * (nmodels * 3 + 4);

  for (int i = 0; i < nmodels; i++)
    HTS_Engine_set_parameter_interpolation_weight(&m_engine, 0, i, m_weights[index + nmodels * 0 + i]);
  for (int i = 0; i < nmodels; i++)
    HTS_Engine_set_parameter_interpolation_weight(&m_engine, 1, i, m_weights[index + nmodels * 1 + i]);
  for (int i = 0; i < nmodels; i++)
    HTS_Engine_set_duration_interpolation_weight(&m_engine, i, m_weights[index + nmodels * 2 + i]);

  double value = m_weights[index + nmodels * 3 + 0];
  if (value == 0)
    value = 1;

  /* speed */
  double speed = OPENJTALK_FPERIOD / value;
  if(speed > OPENJTALK_MAXFPERIOD)
    HTS_Engine_set_fperiod(&m_engine, OPENJTALK_MAXFPERIOD);
  else if(speed < OPENJTALK_MINFPERIOD)
    HTS_Engine_set_fperiod(&m_engine, OPENJTALK_MINFPERIOD);
  else
    HTS_Engine_set_fperiod(&m_engine, (int) speed);

  /* pitch */
  double pitch = m_weights[index + nmodels * 3 + 1];
  if(pitch > OPENJTALK_MAXHALFTONE)
    m_f0Shift = OPENJTALK_MAXHALFTONE;
  else if(pitch < OPENJTALK_MINHALFTONE)
    m_f0Shift = OPENJTALK_MINHALFTONE;
  else
    m_f0Shift = pitch;

  /* alpha */
  double alpha = m_weights[index + nmodels * 3 + 2];
  if(alpha > OPENJTALK_MAXALPHA)
    HTS_Engine_set_alpha(&m_engine, OPENJTALK_MAXALPHA);
  else if(alpha < OPENJTALK_MINALPHA)
    HTS_Engine_set_alpha(&m_engine, OPENJTALK_MINALPHA);
  else
    HTS_Engine_set_alpha(&m_engine, alpha);

  /* volume */
  double volume = m_weights[index + nmodels * 3 + 3];
  if(volume > OPENJTALK_MAXVOLUME)
    HTS_Engine_set_volume(&m_engine, OPENJTALK_MAXVOLUME);
  else if(volume < OPENJTALK_MINVOLUME)
    HTS_Engine_set_volume(&m_engine, OPENJTALK_MINVOLUME);
  else
    HTS_Engine_set_volume(&m_engine, volume);
}
