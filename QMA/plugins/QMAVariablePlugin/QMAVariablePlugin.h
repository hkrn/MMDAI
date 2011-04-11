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

#ifndef QMAVARIABLEPLUGIN_H
#define QMAVARIABLEPLUGIN_H

#include <QBasicTimer>
#include <QHash>
#include <QMap>
#include <QString>
#include <QTimerEvent>

#include "QMAPlugin.h"

class MMDAI::SceneController;

class QMAVariablePlugin : public QMAPlugin
{
    Q_OBJECT
    Q_INTERFACES(QMAPlugin);

public:
    static const char *kValueSet;
    static const char *kValueUnset;
    static const char *kValueEvaluate;
    static const char *kTimerStart;
    static const char *kTimerStop;

    static const char *kValueSetEvent;
    static const char *kValueUnsetEvent;
    static const char *kValueEvaluateEvent;
    static const char *kTimerStartEvent;
    static const char *kTimerStopEvent;

    QMAVariablePlugin();
    ~QMAVariablePlugin();

public slots:
    void initialize(MMDAI::SceneController *controller);
    void start();
    void stop();
    void receiveCommand(const QString &command, const QStringList &arguments);
    void receiveEvent(const QString &type, const QStringList &arguments);
    void update(const QRect &rect, const QPoint &pos, const double delta);
    void prerender();
    void postrender();

signals:
    void commandPost(const QString &command, const QStringList &arguments);
    void eventPost(const QString &type, const QStringList &arguments);

protected:
    void timerEvent(QTimerEvent *event);

private:
    void setValue(const QString &key, const QString &value, const QString &value2);
    void deleteValue(const QString &key);
    void evaluate(const QString &key, const QString &op, const QString &value);
    void startTimer0(const QString &key, const QString &value);
    void stopTimer0(const QString &key);

    QHash<QString, float> m_values;
    QMap<QString, QBasicTimer *> m_timers;
};

#endif // QMALOOKATPLUGIN_H
