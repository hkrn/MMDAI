/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2010  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn (libMMDAI)                         */
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
/* - Neither the name of the MMDAgent project team nor the names of  */
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

/* headers */

#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "Open_JTalk.h"
#include "mecab.h"

#ifdef OPENJTALK_DEBUG
void dprint(char *str)
{
  FILE *fp = fopen("C:/cygwin/home/uratec/SRC/MMDAgentTask/MMDAgentExample/log.txt", "a");
  fprintf(fp, "[");
  fprintf(fp, str);
  fprintf(fp, "]/n");
  fclose(fp);
}
#endif

/* Open_JTalk::initialize: initialize system */
void Open_JTalk::initialize()
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
  HTS_Engine_set_audio_buff_size(&m_engine, OPENJTALK_AUDIOBUFFSIZE);

  m_f0Shift = OPENJTALK_HALFTONE;
  m_numModels = 0;
  m_loaded = false;
  m_styleWeights = NULL;
  m_numStyles = 0;
}

/* Open_JTalk::clear: free system */
void Open_JTalk::clear()
{
  Mecab_clear(&m_mecab);
  NJD_clear(&m_njd);
  JPCommon_clear(&m_jpcommon);
  HTS_Engine_clear(&m_engine);

  m_f0Shift = 0.0;
  m_numModels = 0;
  m_loaded = false;
  if (m_styleWeights != NULL)
    free(m_styleWeights);
  m_styleWeights = NULL;
  m_numStyles = 0;
}

/* Open_JTalk::Open_JTalk: constructor */
Open_JTalk::Open_JTalk()
{
  initialize();
}

/* Open_JTalk::~Open_JTalk: destructor */
Open_JTalk::~Open_JTalk()
{
  clear();
}

/* Open_JTalk::load: load dictionary and models */
bool Open_JTalk::load(const char *dicDir, char **modelDir, int numModels, double *styleWeights, int numStyles)
{
  int i, j;

  char buff[OPENJTALK_MAXBUFLEN];
  char *dn_mecab;
  char **fn_ws_mcp, **fn_ws_lf0, **fn_ws_lpf;
  char *fn_gv_switch;
  char **fn_ts_dur, **fn_ts_mcp, **fn_ts_lf0, **fn_ts_lpf;
  char **fn_ms_dur, **fn_ms_mcp, **fn_ms_lf0, **fn_ms_lpf;
  char **fn_ts_gvm, **fn_ts_gvl;
  char **fn_ms_gvm, **fn_ms_gvl;

  if (dicDir == NULL || modelDir == NULL || numModels <= 0 || styleWeights == NULL || numStyles <= 0)
    return false;
  m_numModels = numModels;
  m_numStyles = numStyles;

  /* set filenames */
  fn_ws_mcp = (char **) calloc(3, sizeof(char *));
  fn_ws_lf0 = (char **) calloc(3, sizeof(char *));
  fn_ws_lpf = (char **) calloc(1, sizeof(char *));
  fn_ts_dur = (char **) calloc(m_numModels, sizeof(char *));
  fn_ts_mcp = (char **) calloc(m_numModels, sizeof(char *));
  fn_ts_lf0 = (char **) calloc(m_numModels, sizeof(char *));
  fn_ts_lpf = (char **) calloc(m_numModels, sizeof(char *));
  fn_ms_dur = (char **) calloc(m_numModels, sizeof(char *));
  fn_ms_mcp = (char **) calloc(m_numModels, sizeof(char *));
  fn_ms_lf0 = (char **) calloc(m_numModels, sizeof(char *));
  fn_ms_lpf = (char **) calloc(m_numModels, sizeof(char *));
  fn_ts_gvm = (char **) calloc(m_numModels, sizeof(char *));
  fn_ts_gvl = (char **) calloc(m_numModels, sizeof(char *));
  fn_ms_gvm = (char **) calloc(m_numModels, sizeof(char *));
  fn_ms_gvl = (char **) calloc(m_numModels, sizeof(char *));

  dn_mecab = strdup(dicDir);
  sprintf(buff, "%s/%s", modelDir[0], "mgc.win1");
  fn_ws_mcp[0] = strdup(buff);
  sprintf(buff, "%s/%s", modelDir[0], "mgc.win2");
  fn_ws_mcp[1] = strdup(buff);
  sprintf(buff, "%s/%s", modelDir[0], "mgc.win3");
  fn_ws_mcp[2] = strdup(buff);
  sprintf(buff, "%s/%s", modelDir[0], "lf0.win1");
  fn_ws_lf0[0] = strdup(buff);
  sprintf(buff, "%s/%s", modelDir[0], "lf0.win2");
  fn_ws_lf0[1] = strdup(buff);
  sprintf(buff, "%s/%s", modelDir[0], "lf0.win3");
  fn_ws_lf0[2] = strdup(buff);
  sprintf(buff, "%s/%s", modelDir[0], "lpf.win1");
  fn_ws_lpf[0] = strdup(buff);
  sprintf(buff, "%s/%s", modelDir[0], "gv-switch.inf");
  fn_gv_switch = strdup(buff);
  for (i = 0; i < m_numModels; i++) {
    sprintf(buff, "%s/%s", modelDir[i], "tree-dur.inf");
    fn_ts_dur[i] = strdup(buff);
    sprintf(buff, "%s/%s", modelDir[i], "tree-mgc.inf");
    fn_ts_mcp[i] = strdup(buff);
    sprintf(buff, "%s/%s", modelDir[i], "tree-lf0.inf");
    fn_ts_lf0[i] = strdup(buff);
    sprintf(buff, "%s/%s", modelDir[i], "tree-lpf.inf");
    fn_ts_lpf[i] = strdup(buff);
    sprintf(buff, "%s/%s", modelDir[i], "dur.pdf");
    fn_ms_dur[i] = strdup(buff);
    sprintf(buff, "%s/%s", modelDir[i], "mgc.pdf");
    fn_ms_mcp[i] = strdup(buff);
    sprintf(buff, "%s/%s", modelDir[i], "lf0.pdf");
    fn_ms_lf0[i] = strdup(buff);
    sprintf(buff, "%s/%s", modelDir[i], "lpf.pdf");
    fn_ms_lpf[i] = strdup(buff);
    sprintf(buff, "%s/%s", modelDir[i], "tree-gv-mgc.inf");
    fn_ts_gvm[i] = strdup(buff);
    sprintf(buff, "%s/%s", modelDir[i], "tree-gv-lf0.inf");
    fn_ts_gvl[i] = strdup(buff);
    sprintf(buff, "%s/%s", modelDir[i], "gv-mgc.pdf");
    fn_ms_gvm[i] = strdup(buff);
    sprintf(buff, "%s/%s", modelDir[i], "gv-lf0.pdf");
    fn_ms_gvl[i] = strdup(buff);
  }

  /* load */
  Mecab_load(&m_mecab, dn_mecab);
  HTS_Engine_load_duration_from_fn(&m_engine, fn_ms_dur, fn_ts_dur, m_numModels);
  HTS_Engine_load_parameter_from_fn(&m_engine, fn_ms_mcp, fn_ts_mcp, fn_ws_mcp, 0, FALSE, 3, m_numModels);
  HTS_Engine_load_parameter_from_fn(&m_engine, fn_ms_lf0, fn_ts_lf0, fn_ws_lf0, 1, TRUE, 3, m_numModels);
  HTS_Engine_load_parameter_from_fn(&m_engine, fn_ms_lpf, fn_ts_lpf, fn_ws_lpf, 2, FALSE, 1, m_numModels);
  HTS_Engine_load_gv_from_fn(&m_engine, fn_ms_gvm, fn_ts_gvm, 0, m_numModels);
  HTS_Engine_load_gv_from_fn(&m_engine, fn_ms_gvl, fn_ts_gvl, 1, m_numModels);
  HTS_Engine_load_gv_switch_from_fn(&m_engine, fn_gv_switch);

  /* free filenames */
  free(dn_mecab);
  for (i = 0; i < 3; i++) {
    free(fn_ws_mcp[i]);
    free(fn_ws_lf0[i]);
  }
  for (i = 0; i < 1; i++) {
    free(fn_ws_lpf[i]);
  }
  free(fn_ws_mcp);
  free(fn_ws_lf0);
  free(fn_ws_lpf);
  free(fn_gv_switch);
  for (i = 0; i < m_numModels; i++) {
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

  /* set style interpolation weight */
  m_styleWeights = (double *) calloc(m_numStyles * (m_numModels * 3 + 4), sizeof(double));
  for (j = 0; j < m_numStyles * (m_numModels * 3 + 4); j++)
    m_styleWeights[j] = styleWeights[j];

  m_loaded = true;

  setStyle(0);
  return true;
}

/* Open_JTalk::prepare: text analysis, decision of state durations, and parameter generation */
void Open_JTalk::prepare(char *str)
{
  char *buff;
  char **label_feature = NULL;
  int label_size = 0;
  int i;
  double f;

  if (m_loaded == false)
    return;

  /* text analysis */
  HTS_Engine_set_stop_flag(&m_engine, false);
  buff = (char *) calloc(2 * strlen(str) + 1, sizeof(char));
  text2mecab(buff, str);
  Mecab_analysis(&m_mecab, buff);
  free(buff);
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
    /* decision of state durations */
    label_feature = JPCommon_get_label_feature(&m_jpcommon);
    label_size = JPCommon_get_label_size(&m_jpcommon);
    HTS_Engine_load_label_from_string_list(&m_engine, &label_feature[1], label_size - 1); /* skip first silence */
    HTS_Engine_create_sstream(&m_engine);
    /* parameter generation */
    if (m_f0Shift != 0.0) {
      for (i = 0; i < HTS_Engine_get_total_state(&m_engine); i++) {
        f = HTS_Engine_get_state_mean(&m_engine, 1, i, 0);
        f += m_f0Shift * log(2.0) / 12;
        if (f < OPENJTALK_MINLF0VAL)
          f = OPENJTALK_MINLF0VAL;
        HTS_Engine_set_state_mean(&m_engine, 1, i, 0, f);
      }
    }
    HTS_Engine_create_pstream(&m_engine);
  }
}

/* Open_JTalk::getPhonemeSequence: get phoneme sequence */
void Open_JTalk::getPhonemeSequence(char *str)
{
  int i, j, k;
  int size;
  char **feature;
  int nstate;
  int fperiod;
  int sampling_rate;
  char *ch, *start, *end;

  strcpy(str, "");

  if (m_loaded == false)
    return;

  size = JPCommon_get_label_size(&m_jpcommon);
  feature = JPCommon_get_label_feature(&m_jpcommon);
  nstate = HTS_Engine_get_nstate(&m_engine);
  fperiod = HTS_Engine_get_fperiod(&m_engine);
  sampling_rate = HTS_Engine_get_sampling_rate(&m_engine);

  if (size <= 2)
    return;

  /* skip first and final silence */
  size -= 2;
  feature = &feature[1];

  for (i = 0; i < size; i++) {
    if (i > 0)
      strcat(str, ",");
    /* get phoneme from full-context label */
    start = strstr(feature[i], "-");
    end = strstr(feature[i], "+");
    if (start != NULL && end != NULL) {
      for (ch = start + 1; ch != end; ch++)
        sprintf(str, "%s%c", str, *ch);
    } else {
      strcat(str, feature[i]);
    }
    /* get ms */
    for (j = 0, k = 0; j < nstate; j++)
      k += (HTS_Engine_get_state_duration(&m_engine, i * nstate + j) * fperiod * 1000) / sampling_rate;
    sprintf(str, "%s%d", str, k);
  }
}

/* Open_JTalk::synthesis: speech synthesis */
void Open_JTalk::synthesis()
{
  if (m_loaded == false)
    return;
  if (JPCommon_get_label_size(&m_jpcommon) > 2) {
    HTS_Engine_create_gstream(&m_engine);
    HTS_Engine_refresh(&m_engine);
  }
  JPCommon_refresh(&m_jpcommon);
  NJD_refresh(&m_njd);
  Mecab_refresh(&m_mecab);
}

/* Open_JTalk::stop: stop speech synthesis */
void Open_JTalk::stop()
{
  if (m_loaded == false)
    return;
  HTS_Engine_set_stop_flag(&m_engine, TRUE);
}

/* Open_JTalk::setStyle: set style interpolation weight */
bool Open_JTalk::setStyle(int val)
{
  int i, index;
  double f;

  if (m_loaded == false || m_styleWeights == NULL || m_numStyles <= 0 || val < 0 || val >= m_numStyles)
    return false;

  index = val * (m_numModels * 3 + 4);
  for (i = 0; i < m_numModels; i++)
    HTS_Engine_set_parameter_interpolation_weight(&m_engine, 0, i, m_styleWeights[index + m_numModels * 0 + i]);
  for (i = 0; i < m_numModels; i++)
    HTS_Engine_set_parameter_interpolation_weight(&m_engine, 1, i, m_styleWeights[index + m_numModels * 1 + i]);
  for (i = 0; i < m_numModels; i++)
    HTS_Engine_set_duration_interpolation_weight(&m_engine, i, m_styleWeights[index + m_numModels * 2 + i]);

  /* speed */
  f = OPENJTALK_FPERIOD / m_styleWeights[index + m_numModels * 3 + 0];
  if (f < 1.0) f = 1.0;
  if (f > 48000.0) f = 48000.0;
  HTS_Engine_set_fperiod(&m_engine, (int) f);

  /* pitch */
  f = m_styleWeights[index + m_numModels * 3 + 1];
  if (f < -24.0) f = -24.0;
  if (f > 24.0)  f = 24.0;
  m_f0Shift = f;

  /* alpha */
  f = m_styleWeights[index + m_numModels * 3 + 2];
  if (f < 0.0) f = 0.0;
  if (f > 1.0) f = 1.0;
  HTS_Engine_set_alpha(&m_engine, f);

  /* volume */
  f = m_styleWeights[index + m_numModels * 3 + 3];
  if (f < 0.0) f = 0.0;
  HTS_Engine_set_volume(&m_engine, f);

  return true;
}
