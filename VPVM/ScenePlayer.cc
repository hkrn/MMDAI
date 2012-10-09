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

ScenePlayer::ScenePlayer(SceneWidget *sceneWidget, PlaySettingDialog *dialog)
    : QObject(),
      m_sceneWidget(sceneWidget),
      m_dialog(dialog),
      m_progress(0),
      m_format(QApplication::tr("Playing scene frame %1 of %2...")),
      m_player(0),
      m_selected(0),
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
    m_progress = new QProgressDialog();
    m_progress->setWindowModality(Qt::ApplicationModal);
    m_player = new AudioPlayer();
}

ScenePlayer::~ScenePlayer()
{
    delete m_player;
    delete m_progress;
}

void ScenePlayer::start()
{
    if (isActive())
        return;
    int sceneFPS = m_dialog->sceneFPS();
    m_selected = m_sceneWidget->sceneLoader()->selectedModel();
    m_prevSceneFPS = m_sceneWidget->sceneLoader()->scene()->preferredFPS();
    m_prevFrameIndex = m_sceneWidget->currentTimeIndex();
    m_frameStep = 1.0 / (sceneFPS / Scene::defaultFPS());
    m_totalStep = 0;
    m_audioFrameIndex = 0;
    m_prevAudioFrameIndex = 0;
    m_sceneWidget->stop();
    /* 再生用のタイマーからのみレンダリングを行わせるため、SceneWidget のタイマーを止めておく */
    m_sceneWidget->stopAutomaticRendering();
    /* FPS を設定してから物理エンジンを有効にする(FPS設定を反映させるため) */
    m_sceneWidget->setPreferredFPS(sceneFPS);
    m_sceneWidget->sceneLoader()->startPhysicsSimulation();
    /* 場面を開始位置にシーク */
    m_sceneWidget->seekMotion(m_dialog->fromIndex(), true, true);
    /* ハンドルも情報パネルも消す */
    m_sceneWidget->setHandlesVisible(false);
    m_sceneWidget->setInfoPanelVisible(false);
    m_sceneWidget->setBoneWireFramesVisible(m_dialog->isBoneWireframesVisible());
    if (!m_dialog->isModelSelected())
        m_sceneWidget->setSelectedModel(0);
    /* 進捗ダイアログ作成 */
    m_progress->reset();
    m_progress->setCancelButtonText(tr("Cancel"));
    int maxRangeIndex = m_dialog->toIndex() - m_dialog->fromIndex();
    m_progress->setRange(0, maxRangeIndex);
    m_progress->setLabelText(m_format.arg(0).arg(maxRangeIndex));
    float renderTimerInterval = 1000.0 / sceneFPS;
    /* 音声出力準備 */
    m_player->setFilename(m_sceneWidget->sceneLoader()->backgroundAudio());
    if (m_player->initalize()) {
        connect(&m_renderTimer, SIGNAL(timeout()), SLOT(renderSceneFrameVariant()));
        connect(m_player, SIGNAL(audioDidDecodeComplete()), SLOT(stop()));
        connect(m_player, SIGNAL(positionDidAdvance(float)), SLOT(advanceAudioFrame(float)));
        m_player->start();
    }
    else {
        connect(&m_renderTimer, SIGNAL(timeout()), SLOT(renderSceneFrameFixed()));
    }
    /* 再生用タイマー起動 */
    m_renderTimer.start(renderTimerInterval);
    /* FPS 計測タイマー起動 */
    m_countForFPS = 0;
    m_elapsed.start();
    emit renderFrameDidStart();
}

void ScenePlayer::stop()
{
    /* 多重登録を防ぐためタイマーと音声出力オブジェクトのシグナルを解除しておく */
    disconnect(m_player, SIGNAL(audioDidDecodeComplete()), this, SLOT(stop()));
    disconnect(m_player, SIGNAL(positionDidAdvance(float)), this, SLOT(advanceAudioFrame(float)));
    disconnect(&m_renderTimer, SIGNAL(timeout()), this, SLOT(renderSceneFrameFixed()));
    disconnect(&m_renderTimer, SIGNAL(timeout()), this, SLOT(renderSceneFrameVariant()));
    /* タイマーと音声出力オブジェクトの停止 */
    m_player->stop();
    m_renderTimer.stop();
    m_progress->reset();
    /* ハンドルと情報パネルを復帰させる */
    m_sceneWidget->setHandlesVisible(true);
    m_sceneWidget->setInfoPanelVisible(true);
    m_sceneWidget->setBoneWireFramesVisible(true);
    m_sceneWidget->setSelectedModel(m_selected);
    /* 再生が終わったら物理を無効にする */
    m_sceneWidget->sceneLoader()->stopPhysicsSimulation();
    m_sceneWidget->resetMotion();
    m_sceneWidget->setPreferredFPS(m_prevSceneFPS);
    /* フレーム位置を再生前に戻す */
    m_sceneWidget->seekMotion(m_prevFrameIndex, true, true);
    /* SceneWidget を常時レンダリング状態に戻しておく */
    m_sceneWidget->startAutomaticRendering();
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
    float diff = m_audioFrameIndex - m_prevAudioFrameIndex;
    if (diff > 0) {
        renderSceneFrame0(diff);
        m_prevAudioFrameIndex = m_audioFrameIndex;
    }
}

void ScenePlayer::advanceAudioFrame(float step)
{
    if (step >= 0)
        m_audioFrameIndex += step * Scene::defaultFPS();
}

void ScenePlayer::renderSceneFrame0(float step)
{
    if (m_elapsed.elapsed() > 1000) {
        m_currentFPS = m_countForFPS;
        m_countForFPS = 0;
        m_elapsed.restart();
    }
    m_countForFPS++;
    Scene *scene = m_sceneWidget->sceneLoader()->scene();
    bool isReached = scene->isReachedTo(m_dialog->toIndex());
    /* 再生完了かつループではない、またはユーザによってキャンセルされた場合再生用のタイマーイベントを終了する */
    if ((!m_dialog->isLoopEnabled() && isReached) || m_progress->wasCanceled()) {
        stop();
    }
    else {
        int value;
        if (isReached) {
            /* ループする場合はモーションと物理演算をリセットしてから開始位置に移動する */
            SceneLoader *loader = m_sceneWidget->sceneLoader();
            value = m_dialog->fromIndex();
            loader->stopPhysicsSimulation();
            m_sceneWidget->resetMotion();
            loader->startPhysicsSimulation();
            m_sceneWidget->seekMotion(value, true, true);
            m_totalStep = 0.0f;
            value = 0;
        }
        else {
            value = int(m_totalStep);
            m_sceneWidget->advanceMotion(step);
            m_totalStep += step;
        }
        m_progress->setValue(value);
        m_progress->setLabelText(m_format.arg(value).arg(m_dialog->toIndex() - m_dialog->fromIndex()));
        if (m_currentFPS > 0)
            m_progress->setWindowTitle(tr("Current FPS: %1").arg(int(m_currentFPS)));
        else
            m_progress->setWindowTitle(tr("Current FPS: N/A"));
        if (m_dialog->isModelSelected())
            emit motionDidSeek(value);
    }
}

} /* namespace vpvm */
