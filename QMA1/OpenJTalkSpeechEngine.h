/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2012  hkrn                                    */
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

#ifndef OPENJTALKSPEECHENGINE_H
#define OPENJTALKSPEECHENGINE_H

#include <QtCore/QDir>
#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QTimer>

class OpenJTalkSpeechEngineInternal;

class OpenJTalkSpeechEngine : public QObject
{
    Q_OBJECT

public:
    static const QString kSynthStartCommand;
    static const QString kSynthStopCommand;
    static const QString kSynthStartEvent;
    static const QString kSynthStopEvent;
    static const QString kLipSyncStartCommand;
    static const QString kLipSyncStopCommand;

    explicit OpenJTalkSpeechEngine(QObject *parent = 0);
    ~OpenJTalkSpeechEngine();

    void load(const QDir &dir, const QString &baseName);
    void speech(const QString &name, const QString &style, const QString &text);

signals:
    void commandDidPost(const QString &type, const QList<QVariant> &arguments);
    void eventDidPost(const QString &type, const QList<QVariant> &arguments);

private:
    void run(const QString &name, const QString &style, const QString &text);

    QHash<QString, OpenJTalkSpeechEngineInternal *> m_models;
    QTimer m_timer;
    QString m_base;
    QString m_dir;
    QString m_config;

    Q_DISABLE_COPY(OpenJTalkSpeechEngine)
};

#endif // OPENJTALKSPEECHENGINE_H
