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

#include "ScenePlayer.h"

#include "SceneLoader.h"
#include "SceneWidget.h"
#include "PlaySettingDialog.h"

#include <vpvl2/vpvl2.h>
#include <vpvl2/extensions/AudioSource.h>
#include <QApplication>

namespace vpvm
{

using namespace vpvl2;

ScenePlayer::ScenePlayer(SceneWidget *sceneWidget, const PlaySettingDialog *dialog, QObject *parent)
    : QObject(parent),
      m_dialogRef(dialog),
      m_audioSource(new AudioSource()),
      m_sceneWidgetRef(sceneWidget),
      m_format(QApplication::tr("Playing scene frame %1 of %2...")),
      m_prevSceneFPS(0),
      m_lastCurrentTimeIndex(0),
      m_restoreState(false),
      m_cancelled(false)
{
}

ScenePlayer::~ScenePlayer()
{
}

void ScenePlayer::start()
{
    if (isActive()) {
        return;
    }
    float sceneFPS = m_dialogRef->sceneFPS();
    SceneLoader *loader = m_sceneWidgetRef->sceneLoaderRef();
    Scene *scene = loader->sceneRef();
    m_selectedModelRef = loader->selectedModelRef();
    m_prevSceneFPS = scene->preferredFPS();
    m_lastCurrentTimeIndex = m_sceneWidgetRef->currentTimeIndex();
    /* 再生用のタイマーからのみレンダリングを行わせるため、SceneWidget のタイマーを止めておく */
    m_sceneWidgetRef->stopAutomaticRendering();
    /* FPS を設定してから物理エンジンを有効にする(FPS設定を反映させるため) */
    m_sceneWidgetRef->setPreferredFPS(sceneFPS);
    loader->startPhysicsSimulation();
    /* 場面を開始位置にシーク */
    m_sceneWidgetRef->resetMotion();
    m_sceneWidgetRef->seekMotion(m_dialogRef->fromIndex(), true, true);
    /* ハンドルも情報パネルも消す */
    m_sceneWidgetRef->setHandlesVisible(false);
    m_sceneWidgetRef->setInfoPanelVisible(false);
    m_sceneWidgetRef->setBoneWireFramesVisible(m_dialogRef->isBoneWireframesVisible());
    if (!m_dialogRef->isModelSelected()) {
        m_sceneWidgetRef->revertSelectedModel();
    }
    /* 進捗ダイアログ作成 */
    emit playerDidPlay(tr("Playing scene"), true);
    /* 音声出力準備 */
    const QString &backgroundAudio = loader->backgroundAudio();
    if (!backgroundAudio.isEmpty()) {
        if (!m_audioSource->isLoaded() && !m_audioSource->load(backgroundAudio.toUtf8().constData())) {
            qWarning("Cannot load audio file %s: %s", qPrintable(backgroundAudio), m_audioSource->errorString());
        }
        else if (!m_audioSource->play()) {
            qWarning("Cannot play audio file %s: %s", qPrintable(backgroundAudio), m_audioSource->errorString());
        }
    }
    /* 再生用タイマー起動 */
    m_timeHolder.setUpdateInterval(btSelect(quint32(sceneFPS), sceneFPS / 1.0f, 60.0f));
    m_timeHolder.start();
    m_updateTimer.start(int(btSelect(quint32(sceneFPS), 1000.0f / sceneFPS, 0.0f)), this);
    emit renderFrameDidStart();
}

void ScenePlayer::stop()
{
    /* タイマーと音声出力オブジェクトの停止 */
    m_audioSource->stop();
    m_updateTimer.stop();
    emit playerDidStop();
    /* ハンドルと情報パネルを復帰させる */
    m_sceneWidgetRef->setHandlesVisible(true);
    m_sceneWidgetRef->setInfoPanelVisible(true);
    m_sceneWidgetRef->setBoneWireFramesVisible(true);
    m_sceneWidgetRef->setSelectedModel(m_selectedModelRef, SceneWidget::kSelect);
    /* 再生が終わったら物理を無効にする */
    m_sceneWidgetRef->sceneLoaderRef()->stopPhysicsSimulation();
    m_sceneWidgetRef->resetMotion();
    m_sceneWidgetRef->setPreferredFPS(m_prevSceneFPS);
    /* フレーム位置を再生前に戻す */
    m_sceneWidgetRef->seekMotion(m_lastCurrentTimeIndex, true, true);
    /* SceneWidget を常時レンダリング状態に戻しておく */
    m_sceneWidgetRef->startAutomaticRendering();
    m_counter.reset();
    m_cancelled = false;
    if (m_restoreState) {
        m_restoreState = false;
        emit renderFrameDidStopAndRestoreState();
    }
    else {
        emit renderFrameDidStop();
    }
}

bool ScenePlayer::isActive() const
{
    return m_updateTimer.isActive();
}

void ScenePlayer::timerEvent(QTimerEvent * /* event */)
{
    int64_t elapsed = 0;
    if (m_audioSource->isRunning()) {
        double offset, latency;
        m_audioSource->getOffsetLatency(offset, latency);
        elapsed = qRound64(offset + latency);
    }
    /*
     * 現在の経過時間位置を保存し、timeIndex/delta が返す値が呼び出し毎に変わらないようにする
     * また、現在の経過時間位置より音源の現在の再生位置が大きい場合は音源の位置を採用するように調整する
     */
    m_timeHolder.saveElapsed(elapsed);
    renderScene(m_timeHolder.timeIndex());
    emit renderFrameDidUpdate(m_timeHolder.delta());
}

void ScenePlayer::cancel()
{
    m_cancelled = true;
}

void ScenePlayer::renderScene(const IKeyframe::TimeIndex &timeIndex)
{
    Scene *scene = m_sceneWidgetRef->sceneLoaderRef()->sceneRef();
    bool isReached = scene->isReachedTo(m_dialogRef->toIndex());
    /* 再生完了かつループではない、またはユーザによってキャンセルされた場合再生用のタイマーイベントを終了する */
    if ((!m_dialogRef->isLoopEnabled() && isReached) || m_cancelled) {
        stop();
    }
    else {
        int fromIndex = m_dialogRef->fromIndex(), value = fromIndex;
        if (isReached) {
            /* ループする場合はモーションと物理演算をリセットしてから開始位置に移動する */
            SceneLoader *loader = m_sceneWidgetRef->sceneLoaderRef();
            loader->stopPhysicsSimulation();
            m_sceneWidgetRef->resetMotion();
            m_sceneWidgetRef->seekMotion(value, true, true);
            loader->startPhysicsSimulation();
            m_timeHolder.reset();
        }
        else {
            /* 開始位置に現在のフレーム値を加算してモーションをシークする */
            value += timeIndex;
            m_sceneWidgetRef->seekMotion(value, true, true);
        }
        /* FPS とウィンドウのタイトルと物理演算をシグナル経由で更新する */
        bool flushed;
        m_counter.update(m_timeHolder.elapsed(), flushed);
        int toIndex = m_dialogRef->toIndex() - fromIndex,
                currentFPS = qMin(m_counter.value(), int(m_dialogRef->sceneFPS()));
        emit playerDidUpdate(value - fromIndex, toIndex, m_format.arg(int(timeIndex)).arg(toIndex));
        emit playerDidUpdateTitle(tr("Current FPS: %1")
                                  .arg(currentFPS > 0 ? QVariant(currentFPS).toString() : "N/A"));
        /* 音源のバッファを更新する（OpenAL+ALURE の特性上定期的に呼び出さないといけない） */
        m_audioSource->update();
        if (m_dialogRef->isModelSelected()) {
            emit motionDidSeek(value);
        }
    }
}

} /* namespace vpvm */
