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

#include "PlayerWidget.h"

#include "common/SceneLoader.h"
#include "common/SceneWidget.h"
#include "dialogs/PlaySettingDialog.h"
#include "video/AudioPlayer.h"

#include <vpvl/vpvl.h>

using namespace vpvl;

PlayerWidget::PlayerWidget(SceneWidget *sceneWidget, PlaySettingDialog *dialog)
    : QObject(),
      m_sceneWidget(sceneWidget),
      m_dialog(dialog),
      m_progress(0),
      m_format(QApplication::tr("Playing scene frame %1 of %2...")),
      m_player(0),
      m_selected(0),
      m_prevFrameIndex(0.0f),
      m_frameStep(0.0f),
      m_totalStep(0.0f),
      m_countForFPS(0),
      m_currentFPS(0),
      m_prevSceneFPS(0)
{
    connect(&m_renderTimer, SIGNAL(timeout()), SLOT(renderSceneFrame()));
    m_renderTimer.setSingleShot(false);
    m_progress = new QProgressDialog();
    m_progress->setWindowModality(Qt::ApplicationModal);
    m_player = new AudioPlayer();
    /* ※ renderSceneFrame(float) は絶対値によるシークではなく差分更新 */
}

PlayerWidget::~PlayerWidget()
{
    delete m_player;
    delete m_progress;
}

void PlayerWidget::start()
{
    if (isActive())
        return;
    int sceneFPS = m_dialog->sceneFPS();
    m_selected = m_sceneWidget->sceneLoader()->selectedModel();
    m_prevSceneFPS = m_sceneWidget->scene()->preferredFPS();
    m_prevFrameIndex = m_sceneWidget->currentFrameIndex();
    m_frameStep = 1.0f / (sceneFPS / static_cast<float>(Scene::kFPS));
    m_totalStep = 0.0f;
    m_sceneWidget->stop();
    /* 再生用のタイマーからのみレンダリングを行わせるため、SceneWidget のタイマーを止めておく */
    m_sceneWidget->stopAutomaticRendering();
    /* FPS を設定してから物理エンジンを有効にする(FPS設定を反映させるため) */
    m_sceneWidget->setPreferredFPS(sceneFPS);
    m_sceneWidget->startPhysicsSimulation();
    /* 場面を開始位置にシーク */
    m_sceneWidget->seekMotion(m_dialog->fromIndex());
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
    float renderTimerInterval = 1000.0f / sceneFPS;
    /* 音声出力準備 */
    m_player->setFilename(m_sceneWidget->sceneLoader()->backgroundAudio());
    if (m_player->initalize()) {
        /* 進捗ダイアログ暴走を防ぐため、シグナルは再生時に登録、停止時に解除しておく */
        connect(m_player, SIGNAL(audioDidDecodeComplete()), SLOT(stop()));
        connect(m_player, SIGNAL(positionDidAdvance(float)), SLOT(renderSceneFrame(float)));
        m_player->start();
    }
    else {
        /* 再生用タイマー起動 */
        m_renderTimer.start(renderTimerInterval);
    }
    /* FPS 計測タイマー起動 */
    m_countForFPS = 0;
    m_elapsed.start();
    emit renderFrameDidStart();
}

void PlayerWidget::stop()
{
    disconnect(m_player, SIGNAL(audioDidDecodeComplete()), this, SLOT(stop()));
    disconnect(m_player, SIGNAL(positionDidAdvance(float)), this, SLOT(renderSceneFrame(float)));
    m_player->stop();
    m_renderTimer.stop();
    m_progress->reset();
    /* ハンドルと情報パネルを復帰させる */
    m_sceneWidget->setHandlesVisible(true);
    m_sceneWidget->setInfoPanelVisible(true);
    m_sceneWidget->setSelectedModel(m_selected);
    /* 再生が終わったら物理を無効にする */
    m_sceneWidget->stopPhysicsSimulation();
    m_sceneWidget->resetMotion();
    m_sceneWidget->setPreferredFPS(m_prevSceneFPS);
    /* フレーム位置を再生前に戻す */
    m_sceneWidget->seekMotion(m_prevFrameIndex, true);
    /* SceneWidget を常時レンダリング状態に戻しておく */
    m_sceneWidget->startAutomaticRendering();
    m_totalStep = 0.0f;
    emit renderFrameDidStop();
}

bool PlayerWidget::isActive() const
{
    return m_renderTimer.isActive();
}

void PlayerWidget::renderSceneFrame()
{
    renderSceneFrame0(m_frameStep);
}

void PlayerWidget::renderSceneFrame(float step)
{
    renderSceneFrame0(step * vpvl::Scene::kFPS);
}

void PlayerWidget::renderSceneFrame0(float step)
{
    if (m_elapsed.elapsed() > 1000) {
        m_currentFPS = m_countForFPS;
        m_countForFPS = 0;
        m_elapsed.restart();
    }
    m_countForFPS++;
    Scene *scene = m_sceneWidget->mutableScene();
    bool isReached = scene->isMotionReachedTo(m_dialog->toIndex());
    /* 再生完了かつループではない、またはユーザによってキャンセルされた場合再生用のタイマーイベントを終了する */
    if ((!m_dialog->isLoopEnabled() && isReached) || m_progress->wasCanceled()) {
        stop();
    }
    else {
        int value;
        if (isReached) {
            /* ループする場合はモーションと物理演算をリセットしてから開始位置に移動する */
            value = m_dialog->fromIndex();
            m_sceneWidget->stopPhysicsSimulation();
            m_sceneWidget->resetMotion();
            m_sceneWidget->startPhysicsSimulation();
            m_sceneWidget->seekMotion(value, true);
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
            m_progress->setWindowTitle(tr("Current FPS: %1").arg(m_currentFPS));
        else
            m_progress->setWindowTitle(tr("Current FPS: N/A"));
        if (m_dialog->isModelSelected())
            emit motionDidSeek(value);
    }
}
