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

#ifndef SCRIPT_H
#define SCRIPT_H

#include "JuliusSpeechRecognitionEngine.h"
#include "LipSync.h"
#include "OpenJTalkSpeechEngine.h"

#include <QtCore/QDir>
#include <QtCore/QHash>
#include <QtCore/QLinkedList>
#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QQueue>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>
#include <QtCore/QTimer>

#include <LinearMath/btQuaternion.h>
#include <LinearMath/btVector3.h>

namespace vpvl
{
class PMDModel;
class VMDMotion;
}

struct ScriptArgument {
    QString type;
    QStringList arguments;
    ScriptArgument()
        : type(QString()), arguments(QStringList()) {
    }
    ScriptArgument(const ScriptArgument &value)
        : type(value.type), arguments(value.arguments) {
    }
    ScriptArgument(const QString &t, const QStringList &args)
        : type(t), arguments(args) {
    }
    void operator =(const ScriptArgument &value) {
        this->type = value.type;
        this->arguments = value.arguments;
    }
};

class ExtendedSceneWidget;
class QBasicTimer;

class Script : public QObject
{
    Q_OBJECT

public:
    typedef struct State State;
    typedef QList<QVariant> Arguments;
    struct MotionParameter {
        bool isLoopEnabled;
    };

    static const QString kEPS;
    static const QString kLipSyncName;

    explicit Script(ExtendedSceneWidget *parent);
    ~Script();

    bool load(QTextStream &stream);
    void loadGlobalLipSync(QTextStream &stream);
    bool loadScript(QTextStream &stream);
    void loadSpeechEngine(const QDir &dir, const QString &baseName);
    void loadSpeechRecognitionEngine(const QDir &dir, const QString &baseName);
    void start();
    void stop();

    void setDir(const QDir &value) { m_dir = value; }

public slots:
    void handleCommand(const QString &type, const QList<QVariant> &arguments);
    void handleEvent(const QString &type, const QList<QVariant> &arguments);

signals:
    void eventDidPost(const QString &type, const QList<QVariant> &arguments);
    void modelWillDelete(vpvl::PMDModel *model);

protected:
    void timerEvent(QTimerEvent *event);

private slots:
    void execute();
    void handleModelDelete(vpvl::PMDModel *model);
    void handleFinishedMotion(const QMultiMap<vpvl::PMDModel *, vpvl::VMDMotion *> &motions);

private:
    void addScriptArc(int from,
                      int to,
                      const ScriptArgument &input,
                      const ScriptArgument &output);
    const QString canonicalizePath(const QString &path);
    void executeEplisons();
    void handleCommand(const ScriptArgument &output);
    State *newScriptState(quint32 index);
    bool setTransition(const ScriptArgument &input, ScriptArgument &output);
    bool parseEnable(const QString &value, const QString &enable, const QString &disable, bool &output) const;
    bool parsePosition(const QString &value, btVector3 &v) const;
    bool parseColor(const QString &value, btVector4 &v) const;
    bool parseRotation(const QString &value, btQuaternion &v) const;

    ExtendedSceneWidget *m_parent;
    QLinkedList<State *> m_states;
    State *m_currentState;
    JuliusSpeechRecognitionEngine m_recog;
    OpenJTalkSpeechEngine m_speech;
    LipSync m_globalLipSync;
    QHash<QString, float> m_values;
    QMap<QString, vpvl::PMDModel *> m_models;
    QMap<QString, vpvl::VMDMotion *> m_motions;
    QMap<QString, QBasicTimer *> m_timers;
    QHash<vpvl::VMDMotion *, MotionParameter> m_motionParameters;
    QQueue<ScriptArgument> m_queue;
    QTimer m_timer;
    QDir m_dir;
    vpvl::PMDModel *m_stage;

    Q_DISABLE_COPY(Script)
};

#endif // SCRIPT_H
