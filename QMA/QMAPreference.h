/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn                                    */
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

#ifndef QMAPREFERENCE_H
#define QMAPREFERENCE_H

#include <MMDAI/IPreference.h>
#include <QFile>
#include <QSettings>
#include <QVariant>

class QMAPreference : public MMDAI::IPreference
{
public:
    QMAPreference(QSettings *settings);
    ~QMAPreference();

    void load(QFile &file);
    bool getBool(const MMDAI::PreferenceKeys key);
    int getInt(const MMDAI::PreferenceKeys key);
    float getFloat(const MMDAI::PreferenceKeys key);
    void  getFloat3(const MMDAI::PreferenceKeys key, float *values);
    void  getFloat4(const MMDAI::PreferenceKeys key, float *values);
    void  setBool(const MMDAI::PreferenceKeys key, bool value);
    void  setInt(const MMDAI::PreferenceKeys key, int value);
    void  setFloat(const MMDAI::PreferenceKeys key, float value);
    void  setFloat3(const MMDAI::PreferenceKeys key, float *values);
    void  setFloat4(const MMDAI::PreferenceKeys key, float *values);

    inline QSettings *getSettings() {
        return m_settings;
    }

private:
    void parse(const QString &key, const QString &value);
    bool validateBoolKey(const MMDAI::PreferenceKeys key);
    bool validateIntKey(const MMDAI::PreferenceKeys key);
    bool validateFloatKey(const MMDAI::PreferenceKeys key);
    bool validateFloat3Key(const MMDAI::PreferenceKeys key);
    bool validateFloat4Key(const MMDAI::PreferenceKeys key);
    QVariant getDefaultValue(const MMDAI::PreferenceKeys key);
    void round(const MMDAI::PreferenceKeys key, QVariant &value);

    QSettings *m_settings;
    QHash<MMDAI::PreferenceKeys, QVariant> m_values;
};

#endif // QMAPREFERENCE_H
