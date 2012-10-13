/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
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

#include "ScenePlayer.h"

#include "common/SceneLoader.h"
#include "common/SceneWidget.h"
#include "dialogs/PlaySettingDialog.h"
#include "video/AudioPlayer.h"

#include <vpvl2/vpvl2.h>

namespace vpvm
{

using namespace vpvl2;

ScenePlayer::ScenePlayer(SceneWidget *sceneWidget, const PlaySettingDialog *dialog, QObject *parent)
    : QObject(parent),
      m_dialogRef(dialog),
      m_player(new AudioPlayer()),
      m_progress(new QProgressDialog()),
      m_sceneWidgetRef(sceneWidget),
      m_selectedModelRef(0),
      m_format(QApplication::tr("Playing scene frame %1 of %2...")),
      m_currentFPS(0),
      m_prevSceneFPS(0),
      m_prevFrameIndex(0),
      m_frameStep(0),
      m_totalStep(0),
      m_audioFrameIndex(0),
      m_prevAudioFrameIndex(0),
      m_countForFPS(0),
      m_restoreState(false)
{
    m_renderTimer.setSingleShot(false);
    m_progress->setWindowModality(Qt::ApplicationModal);
}

ScenePlayer::~ScenePlayer()
{
}

void ScenePlayer::start()
{
    if (isActive())
        return;
    int sceneFPS = m_dialogRef->sceneFPS();
    m_selectedModelRef = m_sceneWidgetRef->sceneLoaderRef()->selectedModelRef();
    m_prevSceneFPS = m_sceneWidgetRef->sceneLoaderRef()->sceneRef()->preferredFPS();
    m_prevFrameIndex = m_sceneWidgetRef->currentTimeIndex();
    m_frameStep = 1.0 / (sceneFPS / Scene::defaultFPS());
    m_totalStep = 0;
    m_audioFrameIndex = 0;
    m_prevAudioFrameIndex = 0;
    m_sceneWidgetRef->stop();
    /* 再生用のタイマーからのみレンダリングを行わせるため、SceneWidget のタイマーを止めておく */
    m_sceneWidgetRef->stopAutomaticRendering();
    /* FPS を設定してから物理エンジンを有効にする(FPS設定を反映させるため) */
    m_sceneWidgetRef->setPreferredFPS(sceneFPS);
    m_sceneWidgetRef->sceneLoaderRef()->startPhysicsSimulation();
    /* 場面を開始位置にシーク */
    m_sceneWidgetRef->seekMotion(m_dialogRef->fromIndex(), true, true);
    /* ハンドルも情報パネルも消す */
    m_sceneWidgetRef->setHandlesVisible(false);
    m_sceneWidgetRef->setInfoPanelVisible(false);
    m_sceneWidgetRef->setBoneWireFramesVisible(m_dialogRef->isBoneWireframesVisible());
    if (!m_dialogRef->isModelSelected())
        m_sceneWidgetRef->setSelectedModel(0);
    /* 進捗ダイアログ作成 */
    m_progress->reset();
    m_progress->setCancelButtonText(tr("Cancel"));
    int maxRangeIndex = m_dialogRef->toIndex() - m_dialogRef->fromIndex();
    m_progress->setRange(0, maxRangeIndex);
    m_progress->setLabelText(m_format.arg(0).arg(maxRangeIndex));
    qreal renderTimerInterval = 1000.0 / sceneFPS;
    /* 音声出力準備 */
    const QString &backgroundAudio = m_sceneWidgetRef->sceneLoaderRef()->backgroundAudio();
    if (!backgroundAudio.isEmpty() && m_player->openOutputDevice()) {
        m_player->setFileName(backgroundAudio);
        connect(&m_renderTimer, SIGNAL(timeout()), SLOT(renderSceneFrameVariant()));
        connect(m_player.data(), SIGNAL(audioDidDecodeComplete()), SLOT(stop()));
        connect(m_player.data(), SIGNAL(positionDidAdvance(float)), SLOT(advanceAudioFrame(qreal)));
        m_player->startSession();
    }
    else {
        connect(&m_renderTimer, SIGNAL(timeout()), SLOT(renderSceneFrameFixed()));
    }
    /* 再生用タイマー起動 */
    m_renderTimer.start(qMax(int(renderTimerInterval) - 1, 1));
    /* FPS 計測タイマー起動 */
    m_countForFPS = 0;
    m_elapsed.start();
    emit renderFrameDidStart();
}

void ScenePlayer::stop()
{
    /* 多重登録を防ぐためタイマーと音声出力オブジェクトのシグナルを解除しておく */
    disconnect(m_player.data(), SIGNAL(audioDidDecodeComplete()), this, SLOT(stop()));
    disconnect(m_player.data(), SIGNAL(positionDidAdvance(float)), this, SLOT(advanceAudioFrame(qreal)));
    disconnect(&m_renderTimer, SIGNAL(timeout()), this, SLOT(renderSceneFrameFixed()));
    disconnect(&m_renderTimer, SIGNAL(timeout()), this, SLOT(renderSceneFrameVariant()));
    /* タイマーと音声出力オブジェクトの停止 */
    m_player->stopSession();
    m_renderTimer.stop();
    m_progress->reset();
    /* ハンドルと情報パネルを復帰させる */
    m_sceneWidgetRef->setHandlesVisible(true);
    m_sceneWidgetRef->setInfoPanelVisible(true);
    m_sceneWidgetRef->setBoneWireFramesVisible(true);
    m_sceneWidgetRef->setSelectedModel(m_selectedModelRef);
    /* 再生が終わったら物理を無効にする */
    m_sceneWidgetRef->sceneLoaderRef()->stopPhysicsSimulation();
    m_sceneWidgetRef->resetMotion();
    m_sceneWidgetRef->setPreferredFPS(m_prevSceneFPS);
    /* フレーム位置を再生前に戻す */
    m_sceneWidgetRef->seekMotion(m_prevFrameIndex, true, true);
    /* SceneWidget を常時レンダリング状態に戻しておく */
    m_sceneWidgetRef->startAutomaticRendering();
    m_totalStep = 0;
    m_audioFrameIndex = 0;
    m_prevAudioFrameIndex = 0;
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
    return m_renderTimer.isActive();
}

void ScenePlayer::renderSceneFrameFixed()
{
    /* start() 時に計算して固定値でモーションをすすめる */
    renderSceneFrame0(m_frameStep);
}

void ScenePlayer::renderSceneFrameVariant()
{
    /* advanceStep で増えた分を加算するため、値は可変 */
    qreal diff = m_audioFrameIndex - m_prevAudioFrameIndex;
    if (diff > 0) {
        renderSceneFrame0(diff);
        m_prevAudioFrameIndex = m_audioFrameIndex;
    }
}

void ScenePlayer::advanceAudioFrame(qreal step)
{
    if (step >= 0)
        m_audioFrameIndex += step * Scene::defaultFPS();
}

void ScenePlayer::renderSceneFrame0(qreal step)
{
    Scene *scene = m_sceneWidgetRef->sceneLoaderRef()->sceneRef();
    bool isReached = scene->isReachedTo(m_dialogRef->toIndex());
    updateCurrentFPS();
    /* 再生完了かつループではない、またはユーザによってキャンセルされた場合再生用のタイマーイベントを終了する */
    if ((!m_dialogRef->isLoopEnabled() && isReached) || m_progress->wasCanceled()) {
        stop();
    }
    else {
        int value;
        if (isReached) {
            /* ループする場合はモーションと物理演算をリセットしてから開始位置に移動する */
            SceneLoader *loader = m_sceneWidgetRef->sceneLoaderRef();
            value = m_dialogRef->fromIndex();
            loader->stopPhysicsSimulation();
            m_sceneWidgetRef->resetMotion();
            loader->startPhysicsSimulation();
            m_sceneWidgetRef->seekMotion(value, true, true);
            m_totalStep = 0;
            value = 0;
        }
        else {
            value = int(m_totalStep);
            m_sceneWidgetRef->advanceMotion(step);
            m_totalStep += step;
        }
        m_progress->setValue(value);
        m_progress->setLabelText(m_format.arg(value).arg(m_dialogRef->toIndex() - m_dialogRef->fromIndex()));
        if (m_currentFPS > 0) {
            m_progress->setWindowTitle(tr("Current FPS: %1").arg(int(m_currentFPS)));
        }
        else {
            m_progress->setWindowTitle(tr("Current FPS: N/A"));
        }
        if (m_dialogRef->isModelSelected())
            emit motionDidSeek(value);
    }
}

void ScenePlayer::updateCurrentFPS()
{
    if (m_elapsed.elapsed() > 1000) {
        m_currentFPS = m_countForFPS;
        m_countForFPS = 0;
        m_elapsed.restart();
    }
    m_countForFPS++;
}

} /* namespace vpvm */
