/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn                                    */
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

#ifndef QMAOPENJTALKMODEL_H
#define QMAOPENJTALKMODEL_H

#include <QtCore/QHash>
#include <QtCore/QIODevice>
#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QString>

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

    Q_DISABLE_COPY(QMAOpenJTalkModel)
};

#endif // QMAOPENJTALKMODEL_H
