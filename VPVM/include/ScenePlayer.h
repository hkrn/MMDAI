/**

 Copyright (c) 2010-2013  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef VPVM_SCENEPLAYER_H
#define VPVM_SCENEPLAYER_H

#include <vpvl2/Common.h>
#include <vpvl2/extensions/BaseTimeIndexHolder.h>
#include <vpvl2/extensions/FPSCounter.h>
#include <vpvl2/qt/RenderContext.h>

#include <QtCore/QElapsedTimer>
#include <QtCore/QTimer>
#include <QProgressDialog>

namespace vpvl2 {
namespace extensions {
class AudioSource;
}
class IModel;
}

namespace vpvm
{

using namespace vpvl2;
using namespace vpvl2::qt;
class AVFactory;
class SceneWidget;
class PlaySettingDialog;

class ScenePlayer : public QObject
{
    Q_OBJECT

public:
    ScenePlayer(SceneWidget *sceneWidget, const PlaySettingDialog *dialog, QObject *parent = 0);
    ~ScenePlayer();

    void start();
    bool isActive() const;

public slots:
    void stop();
    /* これを呼ぶと再生終了時 renderFrameDidStop ではなく renderFrameDidStopAndRestoreState が呼ばれる */
    void setRestoreState() { m_restoreState = true; }

protected:
    void timerEvent(QTimerEvent *event);

signals:
    void renderFrameDidStart();
    void renderFrameDidUpdate(const Scalar &delta);
    void renderFrameDidStop();
    void renderFrameDidStopAndRestoreState();
    /* motionDidSeek は int 型な点に注意 (他は float 型) */
    void motionDidSeek(int timeIndex);
    void playerDidPlay(const QString &title, bool cancellable);
    void playerDidUpdate(int value, int max, const QString &text);
    void playerDidUpdateTitle(const QString &title);
    void playerDidStop();

private slots:
    void cancel();

private:
    void renderScene(const IKeyframe::TimeIndex &timeIndex);
    class TimeIndexHolder : public BaseTimeIndexHolder {
    public:
        TimeIndexHolder()
            : BaseTimeIndexHolder()
        {
        }
        ~TimeIndexHolder() {
        }
    private:
        void timerStart() {
            m_timer.start();
        }
        void timerReset() {
            m_timer.restart();
        }
        int64_t timerElapsed() const {
            return m_timer.elapsed();
        }
        QElapsedTimer m_timer;
    };

    const PlaySettingDialog *m_dialogRef;
    QScopedPointer<AudioSource> m_audioSource;
    SceneWidget *m_sceneWidgetRef;
    IModelSharedPtr m_selectedModelRef;
    QBasicTimer m_updateTimer;
    QString m_format;
    QByteArray m_buffer;
    FPSCounter m_counter;
    TimeIndexHolder m_timeHolder;
    Scalar m_prevSceneFPS;
    qreal m_lastCurrentTimeIndex;
    bool m_restoreState;
    bool m_cancelled;
};

} /* namespace vpvm */

#endif // SCENEPLAYER_H
